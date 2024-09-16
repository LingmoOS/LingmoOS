// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "drectanglenode_p.h"

#include <QSGRendererInterface>
#include <QPainter>
#include <QPainterPath>
#include <QQuickWindow>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/private/qshader_p.h>
#endif

DQUICK_USE_NAMESPACE
DQUICK_BEGIN_NAMESPACE

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
const char *CornerColorShader::vertexShader() const
{
    return "uniform highp mat4 qt_Matrix;                      \n"
           "attribute highp vec4 qt_VertexPosition;            \n"
           "attribute highp vec2 qt_VertexTexCoord;            \n"
           "attribute highp vec4 vertexColor;                  \n"
           "varying highp vec2 qt_TexCoord;                    \n"
           "varying lowp vec4 color;                           \n"
           "void main() {                                      \n"
           "    qt_TexCoord = qt_VertexTexCoord;               \n"
           "    color = vertexColor;                           \n"
           "    gl_Position = qt_Matrix * qt_VertexPosition;   \n"
           "}";
}

const char *CornerColorShader::fragmentShader() const
{
    return "varying highp vec2 qt_TexCoord;                                               \n"
           "varying lowp vec4 color;                                                      \n"
           "uniform lowp float qt_Opacity;                                                \n"
           "uniform sampler2D qt_Texture;                                                 \n"
           "void main() {                                                                 \n"
           "    gl_FragColor = color * texture2D(qt_Texture, qt_TexCoord).a * qt_Opacity; \n"
           "}";
}

const char * const *CornerColorShader::attributeNames() const
{
    static const char *attributes[] = {
        "qt_VertexPosition",
        "qt_VertexTexCoord",
        "vertexColor",
        nullptr
    };
    return attributes;
}

void CornerColorShader::updateState(const QSGMaterialShader::RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    QOpenGLShaderProgram * const program = QSGMaterialShader::program();
    CornerColorMaterial *newSurface = static_cast<CornerColorMaterial *>(newMaterial);

    QSGOpaqueTextureMaterialShader::updateState(state, newSurface, oldMaterial);

    if (state.isOpacityDirty()) {
        program->setUniformValue(m_idQtOpacity, state.opacity());
    }
}

void CornerColorShader::initialize()
{
    QSGOpaqueTextureMaterialShader::initialize();
    QOpenGLShaderProgram * const program = QSGMaterialShader::program();
    m_idQtOpacity = program->uniformLocation("qt_Opacity");
}
#else
CornerColorShader::CornerColorShader()
{
    setShaderFileName(QSGMaterialShader::VertexStage, QStringLiteral(":/dtk/declarative/shaders_ng/cornerscolorshader.vert.qsb"));
    setShaderFileName(QSGMaterialShader::FragmentStage, QStringLiteral(":/dtk/declarative/shaders_ng/cornerscolorshader.frag.qsb"));
}

// Mapping shader's ubuf of cornerscolorshader
enum Ubuf {
    QtMatrixSize = 64,
    QtOpacitySize = 4,

    QtMatrixOffset = 0,
    QtOpacityOffset = QtMatrixOffset + QtMatrixSize
};

bool CornerColorShader::updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    CornerColorMaterial *newSurface = static_cast<CornerColorMaterial *>(newMaterial);

    bool changed = QSGOpaqueTextureMaterialRhiShader::updateUniformData(state, newSurface, oldMaterial);
    QByteArray *buf = state.uniformData();
    Q_ASSERT(buf->size() >= QtOpacityOffset + QtOpacitySize);
    if (state.isOpacityDirty()) {
        const float opacity = state.opacity();
        memcpy(buf->data() + QtOpacityOffset, &opacity, QtOpacitySize);
        changed = true;
    }

    return changed;
}
#endif

CornerColorMaterial::CornerColorMaterial()
    : QSGOpaqueTextureMaterial()
    , m_radius(0.0)
{
    // blending must be turned on to handle cornor mask
    setFlag(Blending, true);
}

QSGMaterialType *CornerColorMaterial::type() const
{
    static QSGMaterialType type;

    return &type;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QSGMaterialShader *CornerColorMaterial::createShader() const
{
    return new CornerColorShader;
}
#else
QSGMaterialShader *CornerColorMaterial::createShader(QSGRendererInterface::RenderMode renderMode) const
{
    Q_UNUSED(renderMode)
    return new CornerColorShader;
}
#endif

int CornerColorMaterial::compare(const QSGMaterial *other) const
{
    return QSGOpaqueTextureMaterial::compare(other);
}

DRectangleNode::DRectangleNode()
    : m_geometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 8)
    , m_maskTexture(nullptr)
{
    setGeometry(&m_geometry);
    setMaterial(&m_material);

    m_cornerNode.setGeometry(&m_cornerGeometry);
    m_cornerNode.setMaterial(&m_cornerMaterial);
    m_cornerGeometry.setDrawingMode(GL_TRIANGLES);
}

void DRectangleNode::setRect(const QRectF &r)
{
    if (m_rect == r)
        return;

    m_geometryChanged = true;
    m_rect = r;
}

void DRectangleNode::setRadius(qreal radius)
{
    if (qFuzzyCompare(m_radius, radius))
        return;

    m_radius = std::min({rect().width() / 2.0, rect().height() / 2.0, radius});
    m_cornerMaterial.setRadius(m_radius);
    m_cornerMaterial.setColor(color());
    m_geometryChanged = true;

    const bool needCornerNode = m_radius > 0;
    const bool existsCornerNode = childCount() > 0;
    if (needCornerNode == existsCornerNode)
        return;

    if (!existsCornerNode) {
        appendChildNode(&m_cornerNode);
    } else {
        removeChildNode(&m_cornerNode);
    }
    m_cornerNode.markDirty(DirtyMaterial);
}

void DRectangleNode::setColor(const QColor &color)
{
    if (color == m_color)
        return;

    m_color = color;
    m_cornerMaterial.setColor(color);
    m_geometryChanged = true;
}

void DRectangleNode::setMakTexture(QSGTexture *texture)
{
    if (texture == m_maskTexture)
        return;

    m_maskTexture = texture;
    m_maskTexture->setFiltering(QSGTexture::Linear);
    m_maskTexture->setVerticalWrapMode(QSGTexture::ClampToEdge);
    m_maskTexture->setHorizontalWrapMode(QSGTexture::ClampToEdge);
    if (!m_maskTexture->hasAlphaChannel())
        qWarning() << "The mask material does not contain an alpha channel";

    m_cornerMaterial.setTexture(m_maskTexture);
    m_geometryChanged = true;
}

void DRectangleNode::setCorners(DQuickRectangle::Corners corners)
{
    if (m_coners == corners)
        return;

    m_geometryChanged = true;
    m_coners = corners;
}

void DRectangleNode::update()
{
    if (m_geometryChanged) {
        updateGeometry();
        m_geometryChanged = false;
    }
}

void DRectangleNode::updateGeometry()
{
    qreal cornorRadius = m_radius;
    uchar r = uchar(qRound(color().redF() * color().alphaF() * 255));
    uchar g = uchar(qRound(color().greenF() * color().alphaF() * 255));
    uchar b = uchar(qRound(color().blueF() * color().alphaF() * 255));
    uchar a = uchar(qRound(color().alphaF() * 255));
    if (cornorRadius > 0)
    {
        qreal radius = std::min({ m_rect.width() / 2, m_rect.height() / 2, cornorRadius });

        m_geometry.allocate(8);
        const float outerL = m_rect.left();
        const float innerL = m_rect.left() + radius;
        const float innerR = m_rect.right() - radius;
        const float outerR = m_rect.right();

        const float outerT = m_rect.top();
        const float innerT = m_rect.top() + radius;
        const float innerB = m_rect.bottom() - radius;
        const float outerB = m_rect.bottom();

        // inner polygon
        QSGGeometry::ColoredPoint2D *vertices = m_geometry.vertexDataAsColoredPoint2D();

        bool bottomLeft = m_coners.testFlag(DQuickRectangle::BottomLeftCorner);
        bool topLeft = m_coners.testFlag(DQuickRectangle::TopLeftCorner);
        bool bottomRight = m_coners.testFlag(DQuickRectangle::BottomRightCorner);
        bool topRight = m_coners.testFlag(DQuickRectangle::TopRightCorner);
        //          (3)*********(5)
        //            *         *
        //           *           *
        //          *             *
        //         *(1)            *(7)
        //         *               *
        //         *(0)            *
        //          *              *(6)
        //           *            *
        //            *          *
        //          (2)**********(4)
        vertices[0].set(outerL, innerB, r, g, b, a); // Outer left, inner bottom
        vertices[1].set(outerL, innerT, r, g, b, a); // Outer left, inner top
        vertices[2].set(bottomLeft ? innerL : m_rect.left(), outerB, r, g, b, a); // Inner left, outer bottom
        vertices[3].set(topLeft ? innerL : m_rect.left(), outerT, r, g, b, a); // Inner left, outer top
        vertices[4].set(bottomRight ? innerR : m_rect.right(), outerB, r, g, b, a); // Inner right, outer botton
        vertices[5].set(topRight ? innerR : m_rect.right(), outerT, r, g, b, a); // Inner right, outer top
        vertices[6].set(outerR, innerB, r, g, b, a); // Outer right, inner bottom
        vertices[7].set(outerR, innerT, r, g, b, a); // Outer right, inner top

        // four corner triangle
        int num = 0;
        if (bottomLeft)
            num += 3;
        if (topLeft)
            num += 3;
        if (bottomRight)
            num += 3;
        if (topRight)
            num += 3;

        m_cornerGeometry.allocate(num);
        ColoredCornerPoint2D *corners = static_cast<ColoredCornerPoint2D *>(m_cornerGeometry.vertexData());
        int vertexNum = 0;
        // Bottom left
        //      (1)*
        //         **
        //         * *
        //         *   *
        //      (0)******(2)
        if (bottomLeft) {
            corners[0].set(outerL, outerB, 0, 0, r, g, b, a);
            corners[1].set(outerL, innerB, 0, 1, r, g, b, a);
            corners[2].set(innerL, outerB, 1, 0, r, g, b, a);
            vertexNum += 3;
        }

        // Top left
        //      (3)*****(5)
        //         *  *
        //         * *
        //         **
        //         *(4)
        if (topLeft) {
            corners[vertexNum + 0].set(outerL, outerT, 0, 0, r, g, b, a);
            corners[vertexNum + 1].set(outerL, innerT, 0, 1, r, g, b, a);
            corners[vertexNum + 2].set(innerL, outerT, 1, 0, r, g, b, a);
            vertexNum += 3;
        }

        // Bottom right
        //               *(7)
        //              **
        //             * *
        //            *  *
        //        (8)*****(6)
        if (bottomRight) {
            corners[vertexNum + 0].set(outerR, outerB, 0, 0, r, g, b, a);
            corners[vertexNum + 1].set(outerR, innerB, 0, 1, r, g, b, a);
            corners[vertexNum + 2].set(innerR, outerB, 1, 0, r, g, b, a);
            vertexNum += 3;
        }

        // Top right
        //     (11)*****(9)
        //          *  *
        //           * *
        //            **
        //             *(10)
        if (topRight) {
            corners[vertexNum + 0].set(outerR, outerT, 0, 0, r, g, b, a);
            corners[vertexNum + 1].set(outerR, innerT, 0, 1, r, g, b, a);
            corners[vertexNum + 2].set(innerR, outerT, 1, 0, r, g, b, a);
        }

        m_cornerNode.markDirty(DirtyGeometry);
        markDirty(QSGNode::DirtyGeometry);
    } else {
        m_geometry.allocate(4);
        m_geometry.setDrawingMode(QSGGeometry::DrawTriangleStrip);
        QSGGeometry::ColoredPoint2D *vertices = m_geometry.vertexDataAsColoredPoint2D();

        vertices[0].set(m_rect.left(), m_rect.top(), r, g, b, a);
        vertices[1].set(m_rect.left(), m_rect.bottom(), r, g, b, a);
        vertices[2].set(m_rect.right(), m_rect.top(), r, g, b, a);
        vertices[3].set(m_rect.right(), m_rect.bottom(), r, g, b, a);
        markDirty(QSGNode::DirtyGeometry);
    }
}

DSoftRectangleNode::DSoftRectangleNode(QQuickItem *owner)
    : m_item(owner)
{
    Q_ASSERT(owner);
    m_window = m_item->window();
}

QSGRenderNode::StateFlags DSoftRectangleNode::changedStates() const
{
    return {};
}

void DSoftRectangleNode::render(const RenderState *state)
{
    Q_ASSERT(m_window);

    QSGRendererInterface *rif = m_window->rendererInterface();
    QPainter *p = static_cast<QPainter *>(rif->getResource(m_window, QSGRendererInterface::PainterResource));
    Q_ASSERT(p);

    const QRegion *clipRegion = state->clipRegion();
    if (clipRegion && !clipRegion->isEmpty())
        p->setClipRegion(*clipRegion, Qt::ReplaceClip);

    p->setTransform(matrix()->toTransform());
    p->setOpacity(inheritedOpacity());

    QPainterPath rectPath;
    p->setPen(Qt::NoPen);
    rectPath.addRect(0, 0, m_item->width() - 1, m_item->height() - 1);

    bool bottomLeft = m_coners.testFlag(DQuickRectangle::BottomLeftCorner);
    bool topLeft = m_coners.testFlag(DQuickRectangle::TopLeftCorner);
    bool bottomRight = m_coners.testFlag(DQuickRectangle::BottomRightCorner);
    bool topRight = m_coners.testFlag(DQuickRectangle::TopRightCorner);

    p->setRenderHint(QPainter::Antialiasing, m_item->antialiasing());
    QPainterPath path;
    qreal radius = std::min({m_item->width() / 2.0, m_item->height() / 2.0, m_radius});

    if (topLeft) {
        path.moveTo(0, 0);
        path.arcTo(0, 0,  radius * 2,  radius * 2, 90, 90);
        path.lineTo(0, 0);
    }

    if (topRight) {
        path.moveTo(m_item->width(), 0);
        QRectF rect1(m_item->width() -  radius * 2, 0,
                     radius * 2,  radius * 2);
        path.arcTo(rect1, 0, 90);
        path.lineTo(m_item->width(), 0);
    }

    if (bottomLeft) {
        path.moveTo(0, m_item->height());
        QRectF rect2(0, m_item->height() -  radius * 2,
                     radius * 2, radius * 2);
        path.arcTo(rect2, 180, 90);
        path.lineTo(0, m_item->height());
    }

    if (bottomRight) {
        path.moveTo(m_item->width(), m_item->height());
        QRectF f2(m_item->width() -  radius * 2,
                  m_item->height() -  radius * 2,
                  radius * 2,  radius * 2);
        path.arcTo(f2, 270, 90);
        path.lineTo(m_item->width(),  m_item->height());
    }

    rectPath = rectPath - path;
    p->fillPath(rectPath, m_color);
}

QSGRenderNode::RenderingFlags DSoftRectangleNode::flags() const
{
    return BoundedRectRendering;
}

QRectF DSoftRectangleNode::rect() const
{
    return QRectF(0, 0, m_item->width(), m_item->height());
}

void DSoftRectangleNode::setRadius(qreal radius)
{
    if (qFuzzyCompare(radius, m_radius))
        return;

    m_radius = radius;
    markDirty(DirtyMaterial);
}

void DSoftRectangleNode::setColor(const QColor &color)
{
    if (color == m_color)
        return;

    m_color = color;
    markDirty(DirtyMaterial);
}

void DSoftRectangleNode::setCorners(DQuickRectangle::Corners corners)
{
    if (corners == m_coners)
        return;

    m_coners = corners;
    markDirty(DirtyMaterial);
}

DQUICK_END_NAMESPACE
