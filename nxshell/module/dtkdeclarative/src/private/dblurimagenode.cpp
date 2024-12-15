// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dblurimagenode_p.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <private/qsgtexture_p.h>
#else
#include <private/qsgplaintexture_p.h>
#endif
#include <private/qquickitem_p.h>
#include <private/qmemrotate_p.h>
#include <QPainter>
#ifndef QT_NO_OPENGL
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#endif

DQUICK_BEGIN_NAMESPACE

// Copy from qpixmapfilter.cpp begin
template <int shift>
inline int qt_static_shift(int value)
{
    if (shift == 0)
        return value;
    else if (shift > 0)
        return value << (uint(shift) & 0x1f);
    else
        return value >> (uint(-shift) & 0x1f);
}

template<int aprec, int zprec>
inline void qt_blurinner(uchar *bptr, int &zR, int &zG, int &zB, int &zA, int alpha)
{
    QRgb *pixel = (QRgb *)bptr;

#define Z_MASK (0xff << zprec)
    const int A_zprec = qt_static_shift<zprec - 24>(*pixel) & Z_MASK;
    const int R_zprec = qt_static_shift<zprec - 16>(*pixel) & Z_MASK;
    const int G_zprec = qt_static_shift<zprec - 8>(*pixel)  & Z_MASK;
    const int B_zprec = qt_static_shift<zprec>(*pixel)      & Z_MASK;
#undef Z_MASK

    const int zR_zprec = zR >> aprec;
    const int zG_zprec = zG >> aprec;
    const int zB_zprec = zB >> aprec;
    const int zA_zprec = zA >> aprec;

    zR += alpha * (R_zprec - zR_zprec);
    zG += alpha * (G_zprec - zG_zprec);
    zB += alpha * (B_zprec - zB_zprec);
    zA += alpha * (A_zprec - zA_zprec);

#define ZA_MASK (0xff << (zprec + aprec))
    *pixel =
        qt_static_shift<24 - zprec - aprec>(zA & ZA_MASK)
        | qt_static_shift<16 - zprec - aprec>(zR & ZA_MASK)
        | qt_static_shift<8 - zprec - aprec>(zG & ZA_MASK)
        | qt_static_shift<-zprec - aprec>(zB & ZA_MASK);
#undef ZA_MASK
}

const static int alphaIndex = (QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3);

template<int aprec, int zprec>
inline void qt_blurinner_alphaOnly(uchar *bptr, int &z, int alpha)
{
    const int A_zprec = int(*(bptr)) << zprec;
    const int z_zprec = z >> aprec;
    z += alpha * (A_zprec - z_zprec);
    *(bptr) = z >> (zprec + aprec);
}

template<int aprec, int zprec, bool alphaOnly>
inline void qt_blurrow(QImage & im, int line, int alpha)
{
    uchar *bptr = im.scanLine(line);

    int zR = 0, zG = 0, zB = 0, zA = 0;

    if (alphaOnly && im.format() != QImage::Format_Indexed8)
        bptr += alphaIndex;

    const int stride = im.depth() >> 3;
    const int im_width = im.width();
    for (int index = 0; index < im_width; ++index) {
        if (alphaOnly)
            qt_blurinner_alphaOnly<aprec, zprec>(bptr, zA, alpha);
        else
            qt_blurinner<aprec, zprec>(bptr, zR, zG, zB, zA, alpha);
        bptr += stride;
    }

    bptr -= stride;

    for (int index = im_width - 2; index >= 0; --index) {
        bptr -= stride;
        if (alphaOnly)
            qt_blurinner_alphaOnly<aprec, zprec>(bptr, zA, alpha);
        else
            qt_blurinner<aprec, zprec>(bptr, zR, zG, zB, zA, alpha);
    }
}

/*
*  expblur(QImage &img, int radius)
*
*  Based on exponential blur algorithm by Jani Huhtanen
*
*  In-place blur of image 'img' with kernel
*  of approximate radius 'radius'.
*
*  Blurs with two sided exponential impulse
*  response.
*
*  aprec = precision of alpha parameter
*  in fixed-point format 0.aprec
*
*  zprec = precision of state parameters
*  zR,zG,zB and zA in fp format 8.zprec
*/
template <int aprec, int zprec, bool alphaOnly>
static void expblur(QImage &img, qreal radius, bool improvedQuality = false, int transposed = 0)
{
    // halve the radius if we're using two passes
    if (improvedQuality)
        radius *= qreal(0.5);

    Q_ASSERT(img.format() == QImage::Format_ARGB32_Premultiplied
             || img.format() == QImage::Format_RGB32
             || img.format() == QImage::Format_Indexed8
             || img.format() == QImage::Format_Grayscale8);

    // choose the alpha such that pixels at radius distance from a fully
    // saturated pixel will have an alpha component of no greater than
    // the cutOffIntensity
    const qreal cutOffIntensity = 2;
    int alpha = radius <= qreal(1e-5)
        ? ((1 << aprec)-1)
        : qRound((1<<aprec)*(1 - qPow(cutOffIntensity * (1 / qreal(255)), 1 / radius)));

    int img_height = img.height();
    for (int row = 0; row < img_height; ++row) {
        for (int i = 0; i <= int(improvedQuality); ++i)
            qt_blurrow<aprec, zprec, alphaOnly>(img, row, alpha);
    }

    QImage temp(img.height(), img.width(), img.format());
    temp.setDevicePixelRatio(img.devicePixelRatioF());
    if (transposed >= 0) {
        if (img.depth() == 8) {
            qt_memrotate270(reinterpret_cast<const quint8*>(img.bits()),
                            img.width(), img.height(), img.bytesPerLine(),
                            reinterpret_cast<quint8*>(temp.bits()),
                            temp.bytesPerLine());
        } else {
            qt_memrotate270(reinterpret_cast<const quint32*>(img.bits()),
                            img.width(), img.height(), img.bytesPerLine(),
                            reinterpret_cast<quint32*>(temp.bits()),
                            temp.bytesPerLine());
        }
    } else {
        if (img.depth() == 8) {
            qt_memrotate90(reinterpret_cast<const quint8*>(img.bits()),
                           img.width(), img.height(), img.bytesPerLine(),
                           reinterpret_cast<quint8*>(temp.bits()),
                           temp.bytesPerLine());
        } else {
            qt_memrotate90(reinterpret_cast<const quint32*>(img.bits()),
                           img.width(), img.height(), img.bytesPerLine(),
                           reinterpret_cast<quint32*>(temp.bits()),
                           temp.bytesPerLine());
        }
    }

    img_height = temp.height();
    for (int row = 0; row < img_height; ++row) {
        for (int i = 0; i <= int(improvedQuality); ++i)
            qt_blurrow<aprec, zprec, alphaOnly>(temp, row, alpha);
    }

    if (transposed == 0) {
        if (img.depth() == 8) {
            qt_memrotate90(reinterpret_cast<const quint8*>(temp.bits()),
                           temp.width(), temp.height(), temp.bytesPerLine(),
                           reinterpret_cast<quint8*>(img.bits()),
                           img.bytesPerLine());
        } else {
            qt_memrotate90(reinterpret_cast<const quint32*>(temp.bits()),
                           temp.width(), temp.height(), temp.bytesPerLine(),
                           reinterpret_cast<quint32*>(img.bits()),
                           img.bytesPerLine());
        }
    } else {
        img = temp;
    }
}

static inline void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0)
{
    if (blurImage.format() == QImage::Format_Indexed8 || blurImage.format() == QImage::Format_Grayscale8)
        expblur<12, 10, true>(blurImage, radius, quality, transposed);
    else
        expblur<12, 10, false>(blurImage, radius, quality, transposed);
}
// Copy from qpixmapfilter.cpp end

DSGBlurNode::DSGBlurNode(QQuickItem *owner)
    : m_item(owner)
{

}

void DSGBlurNode::setRenderCallback(RenderCallback callback, void *data)
{
    m_renderCallback = callback;
    m_callbackData = data;
}

void DSGBlurNode::setTexture(QSGTexture *texture)
{
    if (m_texture == texture)
        return;
    m_texture = texture;
    releaseResources();
    markDirty(DirtyMaterial);
}

bool DSGBlurNode::writeToTexture(QSGPlainTexture *) const
{
    return false;
}

void DSGBlurNode::setRadius(qreal radius)
{
    if (qFuzzyCompare(radius, this->m_radius))
        return;
    this->m_radius = radius;
    markDirty(DirtyMaterial);
}

void DSGBlurNode::setSourceRect(const QRectF &source)
{
    if (this->m_sourceRect == source)
        return;
    this->m_sourceRect = source;
    markDirty(DirtyMaterial);
}

void DSGBlurNode::setRect(const QRectF &target)
{
    if (this->m_targetRect == target)
        return;
    this->m_targetRect = target;
    markDirty(DirtyMaterial);
}

void DSGBlurNode::setDisabledOpaqueRendering(bool disabled)
{
    if (this->m_disabledOpaqueRendering == disabled)
        return;
    this->m_disabledOpaqueRendering = disabled;
    markDirty(DirtyForceUpdate);
}

void DSGBlurNode::setBlendColor(const QColor &color)
{
    if (this->m_blendColor == color)
        return;
    this->m_blendColor = color;
    markDirty(DirtyMaterial);
}

void DSGBlurNode::setFollowMatrixForSource(bool on)
{
    if (m_followMatrixForSource == on)
        return;
    m_followMatrixForSource = on;
    markDirty(DirtyMaterial);
}

QSGRenderNode::RenderingFlags DSGBlurNode::flags() const
{
    RenderingFlags rf = BoundedRectRendering | DepthAwareRendering;
    if (!m_disabledOpaqueRendering &&( !m_texture || !m_texture->hasAlphaChannel()))
        rf |= OpaqueRendering;
    return rf;
}

QRectF DSGBlurNode::rect() const
{
    return m_targetRect;
}

void DSGBlurNode::setWindow(QQuickWindow *window)
{
    m_window = window;
}

DSoftwareBlurImageNode::DSoftwareBlurImageNode(QQuickItem *owner)
    : DSGBlurNode(owner)
{

}

static inline QImage refQImage(QImage &source, const QRectF &rect) {
    uchar *offset = source.bits() + qRound(rect.x() + rect.y() * source.bytesPerLine());
    QImage image(offset, rect.width(), rect.height(), source.bytesPerLine(), source.format());
    image.setDevicePixelRatio(source.devicePixelRatio());
    return image;
}

void DSoftwareBlurImageNode::render(const RenderState *state)
{
    if (!m_sourceRect.isValid() || !m_texture)
        return;

    if (!m_item->window() || !m_window)
        return;

    updateCachedImage();

    if (cachedSource.isNull())
        return;

    QSGRendererInterface *rif = m_window->rendererInterface();
    QPainter *p = static_cast<QPainter *>(rif->getResource(m_window,
                                                           QSGRendererInterface::PainterResource));
    Q_ASSERT(p);

    const QRegion *clipRegion = state->clipRegion();
    if (clipRegion && !clipRegion->isEmpty())
        p->setClipRegion(*clipRegion, Qt::ReplaceClip); // must be done before setTransform

    p->setOpacity(inheritedOpacity());

    const qreal dpr = cachedSource.devicePixelRatio();
    QPointF offset = QPointF(m_sourceRect.x() * dpr, m_sourceRect.y() * dpr);
    if (m_texture->isAtlasTexture()) {
        const QRectF subRect = m_texture->normalizedTextureSubRect();
        offset.rx() += subRect.x() * cachedSource.width();
        offset.ry() += subRect.y() * cachedSource.height();
    }
    QRectF actualSourceRect(offset.x(), offset.y(),
                            m_sourceRect.width() * dpr,
                            m_sourceRect.height() * dpr);

    const QTransform t = matrix()->toTransform();
    QRectF actualTargetRect = m_targetRect;
    if (m_followMatrixForSource) {
        // map the targetRect to the actualSourceRect
        QPainterPath path;
        path.addRect(m_targetRect);
        path = t.map(path);
        p->setClipPath(path, Qt::IntersectClip);

        actualSourceRect.setSize(t.mapRect(actualSourceRect).size());
        actualTargetRect = t.mapRect(m_targetRect);
    } else {
        p->setTransform(t);
    }

    // do blur
    QImage sourceRef = refQImage(cachedSource, actualSourceRect);
    // TODO: Don't transparent the borders
    qt_blurImage(sourceRef, m_radius, false);

    if (!m_offscreen) {
        p->drawImage(actualTargetRect, cachedSource, actualSourceRect);
        if (m_blendColor.isValid()) {
            p->fillRect(actualTargetRect, m_blendColor);
        }
    }

    doRenderCallback();
}

void DSoftwareBlurImageNode::releaseResources()
{
    static QImage globalNullImage;
    cachedSource = globalNullImage;
}

void DSoftwareBlurImageNode::updateCachedImage()
{
    Q_ASSERT(m_texture);
    if (auto image = qobject_cast<QSGPlainTexture*>(m_texture)) {
        cachedSource = image->image();
    } else if (QSGLayer *pt = qobject_cast<QSGLayer *>(m_texture)) {
        cachedSource = pt->toImage();
    } else {
        DSoftwareBlurImageNode::releaseResources();
    }
}

bool DSoftwareBlurImageNode::writeToTexture(QSGPlainTexture *targetTexture) const
{
    targetTexture->setImage(cachedSource);
    return true;
}

#ifndef QT_NO_OPENGL

DOpenGLBlurEffectNode::DOpenGLBlurEffectNode(QQuickItem *owner)
    : DSGBlurNode(owner)
{
}

DOpenGLBlurEffectNode::~DOpenGLBlurEffectNode()
{
    delete m_programKawaseUp;
    m_programKawaseUp = nullptr;
    delete m_programKawaseDown;
    m_programKawaseDown = nullptr;
    delete m_program;
    m_program = nullptr;
    delete m_programNoise;
    m_programNoise = nullptr;

    qDeleteAll(m_fboVector);
    m_fboVector.clear();

    delete m_vbo;
    m_vbo = nullptr;

    delete  m_sampleVbo;
    m_sampleVbo = nullptr;

    delete m_noiseVbo;
    m_noiseVbo = nullptr;
}

void DOpenGLBlurEffectNode::setTexture(QSGTexture *texture)
{
    if (m_texture == texture)
        return;

    m_texture = texture;
    m_needUpdateFBO = true;
    markDirty(DirtyMaterial);
}

void DOpenGLBlurEffectNode::setRadius(qreal radius)
{
    // TODO(xiaoyaobing): I don't want to do this, but the radius of software rendering and
    //                    hardware rendering can't be unified
    if (qRound(radius / 10) == m_radius)
        return;

    this->m_radius = qRound(radius / 10);
    m_needUpdateFBO = true;
    markDirty(DirtyMaterial);
}

void DOpenGLBlurEffectNode::render(const QSGRenderNode::RenderState *state)
{
    if (Q_UNLIKELY(!m_sourceRect.isValid() || !m_texture))
        return;

    if (Q_UNLIKELY(!m_item || !m_item->window() || !m_window))
        return;

#if(QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    const bool needsWrap = QSGRendererInterface::isApiRhiBased(m_window->rendererInterface()->graphicsApi());
    if (Q_LIKELY(needsWrap)) {
        m_window->beginExternalCommands();
    }
#endif

    if (Q_UNLIKELY(!m_programKawaseUp))
        initialize();

    if (Q_UNLIKELY(m_needUpdateFBO)) {
        initFBOTextures();
        m_needUpdateFBO = false;
    }

    if (Q_UNLIKELY(m_fboVector.isEmpty()))
        return;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto textureId = m_texture->textureId();
#else
    //TODO support vulkan
    auto textureId = (m_texture->nativeInterface<QNativeInterface::QSGOpenGLTexture>())->nativeTexture();
#endif
    applyDaulBlur(m_fboVector[1], textureId, m_programKawaseDown, state,
                  m_matrixKawaseDownUniform, 2);

    for (int i = 1; i < m_radius; i++) {
        applyDaulBlur(m_fboVector[i + 1], m_fboVector[i]->texture(), m_programKawaseDown, state,
                m_matrixKawaseDownUniform, qPow(2, i + 1));
    }

    for (int i = m_radius; i > 0; i--) {
        applyDaulBlur(m_fboVector[i - 1], m_fboVector[i]->texture(), m_programKawaseUp, state,
                m_matrixKawaseUpUniform, qPow(2, i - 1));
    }

    if (!m_offscreen)
        renderToScreen(m_fboVector[0]->texture(), state);

#if(QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    if (Q_LIKELY(needsWrap))
        m_window->endExternalCommands();
#endif

    doRenderCallback();
}

QSGRenderNode::StateFlags DOpenGLBlurEffectNode::changedStates() const
{
    if (m_offscreen)
        return BlendState;
    return BlendState | StencilState | ScissorState;
}

bool DOpenGLBlurEffectNode::writeToTexture(QSGPlainTexture *targetTexture) const
{
    if (Q_UNLIKELY(m_fboVector.isEmpty()))
        return false;

    const auto fbo = m_fboVector.first();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    targetTexture->setTextureId(fbo->texture());
#elif QT_VERSION <= QT_VERSION_CHECK(6, 2, 4)
    auto wp = QQuickWindowPrivate::get(m_window);
    targetTexture->setTextureFromNativeTexture(wp->rhi, static_cast<quint64>(fbo->texture()),
                                           0, fbo->size(), {}, {});
#else
    auto wp = QQuickWindowPrivate::get(m_window);
    targetTexture->setTextureFromNativeTexture(wp->rhi, static_cast<quint64>(fbo->texture()),
                                           0, 0, fbo->size(), {}, {});
#endif
    targetTexture->setHasAlphaChannel(m_texture->hasAlphaChannel());
    targetTexture->setTextureSize(fbo->size());
    return true;
}

void DOpenGLBlurEffectNode::initialize()
{
    initDispalyShader();
    initBlurSahder();
}

void DOpenGLBlurEffectNode::initBlurSahder()
{
    m_programKawaseUp = new QOpenGLShaderProgram;
    m_programKawaseDown = new QOpenGLShaderProgram;

    m_programKawaseUp->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex,
                                                        ":/dtk/declarative/shaders/dualkawaseup.vert");
    m_programKawaseUp->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment,
                                                        ":/dtk/declarative/shaders/dualkawaseup.frag");
    m_programKawaseUp->bindAttributeLocation("posAttr", 0);
    m_programKawaseUp->bindAttributeLocation("qt_VertexTexCoord", 1);
    m_programKawaseUp->link();

    m_matrixKawaseUpUniform = m_programKawaseUp->uniformLocation("matrix");

    m_programKawaseDown->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex,
                                                          ":/dtk/declarative/shaders/dualkawasedown.vert");
    m_programKawaseDown->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment,
                                                          ":/dtk/declarative/shaders/dualkawasedown.frag");
    m_programKawaseDown->bindAttributeLocation("posAttr", 0);
    m_programKawaseDown->bindAttributeLocation("qt_VertexTexCoord", 1);
    m_programKawaseDown->link();

    m_matrixKawaseDownUniform = m_programKawaseDown->uniformLocation("matrix");

    const int VERTEX_SIZE = 8 * sizeof(GLfloat);

    static GLfloat texCoord[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };

    m_sampleVbo = new QOpenGLBuffer;
    m_sampleVbo->create();
    m_sampleVbo->bind();
    m_sampleVbo->allocate(VERTEX_SIZE + sizeof(texCoord));
    m_sampleVbo->write(VERTEX_SIZE, texCoord, sizeof(texCoord));
}

void DOpenGLBlurEffectNode::applyDaulBlur(QOpenGLFramebufferObject *targetFBO, GLuint sourceTexture, QOpenGLShaderProgram *shader
                                  , const QSGRenderNode::RenderState *state, int matrixUniform, int scale)
{
    if (Q_UNLIKELY(!m_item || !m_window))
        return;

    auto context = QOpenGLContext::currentContext();
    Q_ASSERT(context);
    GLuint prevFbo = 0;
    context->functions()->glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *)&prevFbo);

    targetFBO->bind();
    QOpenGLFunctions *f = context->functions();
    shader->bind();

    glEnable(GL_BLEND);
    f->glBlendFunc(GL_ONE, GL_ZERO);

    // TODO(xiaoyaobing): Shader pixel offset value during hardware rendering, because software rendering
    //                    does not, so write a fixed value here
    shader->setUniformValue("offset", QVector2D(8, 8));
    shader->setUniformValue("iResolution", QVector2D(targetFBO->size().width(), targetFBO->size().height()));
    shader->setUniformValue("halfpixel", QVector2D(0.5 / targetFBO->size().width(), 0.5 / targetFBO->size().height()));
    float yOffset = m_window->height() - qRound(m_targetRect.height() / scale);
    shader->setUniformValue(matrixUniform, *state->projectionMatrix() * QMatrix4x4(1, 0, 0, 0,
                                                                                   0, 1, 0, yOffset,
                                                                                   0, 0, 1, 0,
                                                                                   0, 0, 0, 1));
    m_sampleVbo->bind();
    QPointF p0(0, 0);
    QPointF p1(0, qRound(m_targetRect.height() / scale));
    QPointF p2(qRound( m_targetRect.width() / scale), 0);
    QPointF p3(qRound(m_targetRect.width() / scale), qRound(m_targetRect.height() / scale));

    GLfloat vertices[8] = { GLfloat(p0.x()), GLfloat(p0.y()),
                            GLfloat(p1.x()), GLfloat(p1.y()),
                            GLfloat(p2.x()), GLfloat(p2.y()),
                            GLfloat(p3.x()), GLfloat(p3.y()) };

    m_sampleVbo->write(0, vertices, sizeof(vertices));

    shader->setAttributeBuffer(0, GL_FLOAT, 0, 2);
    shader->setAttributeBuffer(1, GL_FLOAT, sizeof(vertices), 2);
    shader->enableAttributeArray(0);
    shader->enableAttributeArray(1);
    m_sampleVbo->release();

    glEnable(GL_TEXTURE_2D);
    f->glBindTexture(GL_TEXTURE_2D, sourceTexture);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisable(GL_TEXTURE_2D);
    shader->release();
    targetFBO->release();

    if (prevFbo != targetFBO->handle())
        f->glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
}

void DOpenGLBlurEffectNode::applyNoise(GLuint sourceTexture, const QSGRenderNode::RenderState *state)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glBindTexture(GL_TEXTURE_2D, sourceTexture);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    QOpenGLFramebufferObject::bindDefault();
    m_programNoise->bind();
    m_programNoise->setUniformValue("matrix", *state->projectionMatrix() * *matrix());
    m_programNoise->setUniformValue("opacity", float(inheritedOpacity()));
    m_programNoise->setUniformValue("blendColor", m_blendColor);

    m_noiseVbo->bind();

    QPointF p0(0, 0);
    QPointF p1(0, m_targetRect.height());
    QPointF p2(m_targetRect.width(), 0);
    QPointF p3(m_targetRect.width(), m_targetRect.height());

    GLfloat vertices[8] = { GLfloat(p0.x()), GLfloat(p0.y()),
                            GLfloat(p1.x()), GLfloat(p1.y()),
                            GLfloat(p2.x()), GLfloat(p2.y()),
                            GLfloat(p3.x()), GLfloat(p3.y()) };

    m_noiseVbo->write(0, vertices, sizeof(vertices));

    m_programNoise->setAttributeBuffer(0, GL_FLOAT, 0, 2);
    m_programNoise->setAttributeBuffer(1, GL_FLOAT, sizeof(vertices), 2);
    m_programNoise->enableAttributeArray(0);
    m_programNoise->enableAttributeArray(1);
    m_noiseVbo->release();

    f->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if (state->scissorEnabled()) {
        f->glEnable(GL_SCISSOR_TEST);
        const QRect r = state->scissorRect();
        f->glScissor(r.x(), r.y(), r.width(), r.height());
    }
    if (state->stencilEnabled()) {
        f->glEnable(GL_STENCIL_TEST);
        f->glStencilFunc(GL_EQUAL, state->stencilValue(), 0xFF);
        f->glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    }
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    markDirty(QSGNode::DirtyGeometry);
    m_programNoise->release();
}

void DOpenGLBlurEffectNode::renderToScreen(GLuint sourceTexture, const QSGRenderNode::RenderState *state)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glBindTexture(GL_TEXTURE_2D, sourceTexture);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    QOpenGLFramebufferObject::bindDefault();
    m_program->bind();
    m_program->setUniformValue(m_matrixUniform, *state->projectionMatrix() * *matrix());
    m_program->setUniformValue(m_opacityUniform, float(inheritedOpacity()));
    m_program->setUniformValue("blendColor", m_blendColor);

    m_vbo->bind();

    QPointF p0(0, 0);
    QPointF p1(0, m_targetRect.height());
    QPointF p2(m_targetRect.width(), 0);
    QPointF p3(m_targetRect.width(), m_targetRect.height());

    GLfloat vertices[8] = { GLfloat(p0.x()), GLfloat(p0.y()),
                            GLfloat(p1.x()), GLfloat(p1.y()),
                            GLfloat(p2.x()), GLfloat(p2.y()),
                            GLfloat(p3.x()), GLfloat(p3.y()) };

    m_vbo->write(0, vertices, sizeof(vertices));

    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 2);
    m_program->setAttributeBuffer(1, GL_FLOAT, sizeof(vertices), 2);
    m_program->enableAttributeArray(0);
    m_program->enableAttributeArray(1);
    m_vbo->release();

    f->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if (state->scissorEnabled()) {
        f->glEnable(GL_SCISSOR_TEST);
        const QRect r = state->scissorRect();
        f->glScissor(r.x(), r.y(), r.width(), r.height());
    }
    if (state->stencilEnabled()) {
        f->glEnable(GL_STENCIL_TEST);
        f->glStencilFunc(GL_EQUAL, state->stencilValue(), 0xFF);
        f->glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    }
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    markDirty(QSGNode::DirtyGeometry);
    m_program->release();
}

void DOpenGLBlurEffectNode::initFBOTextures()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    for (int i = 0; i < m_fboVector.size(); i++) {
        delete m_fboVector[i];
    }

    m_fboVector.clear();

    if (m_radius <= 0)
        return;

    qreal scale = m_window->effectiveDevicePixelRatio();
    QSize size;
    /* when opengl rendering, the projectionmatrix matrix has high accuracy,
       which will lead to deviation from the FBO size. When i reduce the FBO size by 1,
       at most one boundary pixel will be lost, which will not affect the blurred imaging
       boundary. */
    size.setWidth(m_targetRect.width() * scale - 1);
    size.setHeight(m_targetRect.height() * scale - 1);
    m_fboVector.append(new QOpenGLFramebufferObject(size,
                                                    QOpenGLFramebufferObject::CombinedDepthStencil, GL_TEXTURE_2D));
    for (int i = 1; i <= m_radius; i++) {
        size.setWidth(m_targetRect.width() / qPow(2, i) * scale - 1);
        size.setHeight(m_targetRect.height() / qPow(2, i) * scale - 1);
        m_fboVector.append(new QOpenGLFramebufferObject(size,
                                                        QOpenGLFramebufferObject::CombinedDepthStencil, GL_TEXTURE_2D));

        f->glBindTexture(GL_TEXTURE_2D, m_fboVector.last()->texture());
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

void DOpenGLBlurEffectNode::initDispalyShader()
{
    m_program = new QOpenGLShaderProgram;

    static const char *vertexShaderSource =
            "attribute highp vec4 posAttr;              \n"
            "attribute highp vec2 qt_VertexTexCoord;    \n"
            "varying highp vec2 qt_TexCoord;            \n"
            "uniform highp mat4 matrix;                 \n"
            "void main() {                              \n"
            "   qt_TexCoord = qt_VertexTexCoord;        \n"
            "   gl_Position = matrix * posAttr;         \n"
            "}\n";

    static const char *fragmentShaderSource =
            "varying highp vec2 qt_TexCoord;                                                           \n"
            "uniform lowp float opacity;                                                               \n"
            "uniform sampler2D qt_Texture;                                                             \n"
            "uniform highp vec4 blendColor;                                                            \n"
            "void main() {                                                                             \n"
            "   highp vec4 color = texture2D(qt_Texture, qt_TexCoord) * opacity;                       \n"
            "   lowp vec3 rgb = blendColor.rgb;                                                        \n"
            "   gl_FragColor = color * (1.0 - blendColor.a) + vec4(rgb * blendColor.a, blendColor.a);  \n"
            "}\n";

    m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->bindAttributeLocation("posAttr", 0);
    m_program->bindAttributeLocation("qt_VertexTexCoord", 1);
    m_program->link();

    m_matrixUniform = m_program->uniformLocation("matrix");
    m_opacityUniform = m_program->uniformLocation("opacity");

    const int VERTEX_SIZE = 8 * sizeof(GLfloat);

    static GLfloat texCoord[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };

    m_vbo = new QOpenGLBuffer;
    m_vbo->create();
    m_vbo->bind();
    m_vbo->allocate(VERTEX_SIZE + sizeof(texCoord));
    m_vbo->write(VERTEX_SIZE, texCoord, sizeof(texCoord));
}

void DOpenGLBlurEffectNode::initNoiseShader()
{
    m_programNoise = new QOpenGLShaderProgram;
    m_programNoise->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/dtk/declarative/shaders/noise.vert");
    m_programNoise->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/dtk/declarative/shaders/noise.frag");
    m_programNoise->bindAttributeLocation("posAttr", 0);
    m_programNoise->bindAttributeLocation("qt_VertexTexCoord", 1);
    m_programNoise->link();

    const int VERTEX_SIZE = 8 * sizeof(GLfloat);

    static GLfloat texCoord[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
    };

    m_noiseVbo = new QOpenGLBuffer;
    m_noiseVbo->create();
    m_noiseVbo->bind();
    m_noiseVbo->allocate(VERTEX_SIZE + sizeof(texCoord));
    m_noiseVbo->write(VERTEX_SIZE, texCoord, sizeof(texCoord));
}

#endif

DQUICK_END_NAMESPACE
