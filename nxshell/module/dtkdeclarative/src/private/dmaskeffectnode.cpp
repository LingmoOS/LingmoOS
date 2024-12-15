// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dmaskeffectnode_p.h"
#include "private/qsgdefaultimagenode_p.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>

DQUICK_BEGIN_NAMESPACE

inline static bool isPowerOfTwo(int x)
{
    // Assumption: x >= 1
    return x == (x & -x);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class OpaqueTextureMaterialShader : public QSGMaterialShader
{
public:
    OpaqueTextureMaterialShader();

    void updateState(const RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect) override;
    char const *const *attributeNames() const override;

protected:
    void initialize() override;

protected:
    int m_matrix_id;
};
#else
class OpaqueTextureMaterialShader : public QSGOpaqueTextureMaterialRhiShader
{
public:
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    OpaqueTextureMaterialShader(int viewCount);
#else
    OpaqueTextureMaterialShader();
#endif

    bool updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)  override;

    void updateSampledImage(RenderState &state, int binding, QSGTexture **texture, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)  override;
    bool updateGraphicsPipelineState(RenderState &state, GraphicsPipelineState *ps,
                                     QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
};
#endif

class TextureMaterialShader : public OpaqueTextureMaterialShader
{
public:
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    TextureMaterialShader(int viewCount);
#else
    TextureMaterialShader();
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void updateState(const RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect) override;
    void initialize() override;

protected:
    int m_opacity_id;
#else
    bool updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial);
#endif
};

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
OpaqueTextureMaterialShader::OpaqueTextureMaterialShader(int viewCount)
 : QSGOpaqueTextureMaterialRhiShader(viewCount)
#else
OpaqueTextureMaterialShader::OpaqueTextureMaterialShader()
#endif
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#if QT_CONFIG(opengl)
    setShaderSourceFile(QOpenGLShader::Vertex, QStringLiteral(":/dtk/declarative/shaders/quickitemviewport-opaque.vert"));
    setShaderSourceFile(QOpenGLShader::Fragment, QStringLiteral(":/dtk/declarative/shaders/quickitemviewport-opaque.frag"));
#endif
#else
    setShaderFileName(QSGMaterialShader::VertexStage, QStringLiteral(":/dtk/declarative/shaders_ng/quickitemviewport-opaque.vert.qsb"));
    setShaderFileName(QSGMaterialShader::FragmentStage, QStringLiteral(":/dtk/declarative/shaders_ng/quickitemviewport-opaque.frag.qsb"));
    setFlag(UpdatesGraphicsPipelineState);
#endif
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
char const *const *OpaqueTextureMaterialShader::attributeNames() const
{
    static char const *const attr[] = { "qt_VertexPosition", "qt_VertexTexCoord", nullptr };
    return attr;
}

void OpaqueTextureMaterialShader::initialize()
{
#if QT_CONFIG(opengl)
    m_matrix_id = program()->uniformLocation("qt_Matrix");
#endif
}

void OpaqueTextureMaterialShader::updateState(const RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect)
{
    Q_ASSERT(oldEffect == nullptr || newEffect->type() == oldEffect->type());
    const OpaqueTextureMaterial *newMaterial = static_cast<OpaqueTextureMaterial *>(newEffect);

    // TODO：一直刷新数据浪费性能，需要优化
    QSGTexture *t = newMaterial->texture();
    if (Q_UNLIKELY(!t))
        return;
    t->setFiltering(newMaterial->filtering());
    t->setHorizontalWrapMode(newMaterial->horizontalWrapMode());
    t->setVerticalWrapMode(newMaterial->verticalWrapMode());

#if QT_CONFIG(opengl)
    auto gl = const_cast<QOpenGLContext *>(state.context())->functions();
    bool npotSupported = gl->hasOpenGLFeature(QOpenGLFunctions::NPOTTextureRepeat);
    if (!npotSupported) {
        QSize size = t->textureSize();
        const bool isNpot = !isPowerOfTwo(size.width()) || !isPowerOfTwo(size.height());
        if (isNpot) {
            t->setHorizontalWrapMode(QSGTexture::ClampToEdge);
            t->setVerticalWrapMode(QSGTexture::ClampToEdge);
        }
    }
#else
    Q_UNUSED(state)
#endif

    t->setMipmapFiltering(newMaterial->mipmapFiltering());
    t->setAnisotropyLevel(newMaterial->anisotropyLevel());

    OpaqueTextureMaterial *oldTx = static_cast<OpaqueTextureMaterial *>(oldEffect);
    if (oldTx == nullptr || oldTx->texture()->textureId() != t->textureId())
        t->bind();
    else
        t->updateBindOptions();

#if QT_CONFIG(opengl)
    auto mask = newMaterial->maskTexture();
    gl->glActiveTexture(GL_TEXTURE1);
    if (oldTx == nullptr || oldTx->maskTexture()->textureId() != mask->textureId()) {
        mask->bind();
    } else {
        mask->updateBindOptions();
    }
    gl->glActiveTexture(GL_TEXTURE0);

    program()->setUniformValue("sourceScale", newMaterial->sourceScale());
    program()->setUniformValue("mask", 1);
    program()->setUniformValue("maskScale", newMaterial->maskScale());
    program()->setUniformValue("maskOffset", newMaterial->maskOffset());

    if (state.isMatrixDirty())
        program()->setUniformValue(m_matrix_id, state.combinedMatrix());
#else
    Q_UNUSED(state)
#endif
}
#else

// Mapping shader's ubuf of quickitemviewport-opaque
enum Ubuf {
    QtMatrixSize = 64,
    MaskScaleSize = 8,
    MaskOffsetSize = MaskScaleSize,
    SourceScaleSize = MaskScaleSize,

    QtMatrixOffset = 0,
    MaskScaleOffset = QtMatrixOffset + QtMatrixSize,
    MaskOffsetOffset = MaskScaleOffset + MaskScaleSize,
    SourceScaleOffset = MaskOffsetOffset + MaskOffsetSize,

    MaskBindingIndex = 2
};

bool OpaqueTextureMaterialShader::updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    Q_ASSERT(oldMaterial == nullptr || newMaterial->type() == oldMaterial->type());
    const OpaqueTextureMaterial *material = static_cast<OpaqueTextureMaterial *>(newMaterial);

    bool changed = QSGOpaqueTextureMaterialRhiShader::updateUniformData(state, newMaterial, oldMaterial);

    QByteArray *buf = state.uniformData();
    Q_ASSERT(buf->size() >= SourceScaleOffset + SourceScaleSize);
    if (state.isMatrixDirty()) {
        const QMatrix4x4 m = state.combinedMatrix();
        memcpy(buf->data() + QtMatrixOffset, m.constData(), QtMatrixSize);
        changed = true;
    }

    if (oldMaterial != newMaterial) {
        const QVector2D maskScale = material->maskScale();
        const QVector2D maskOffset = material->maskOffset();
        const QVector2D sourceScale = material->sourceScale();

        Q_ASSERT(sizeof(maskScale) == MaskScaleSize);

        memcpy(buf->data() + MaskScaleOffset, &maskScale, MaskScaleSize);
        memcpy(buf->data() + MaskOffsetSize, &maskOffset, MaskOffsetSize);
        memcpy(buf->data() + SourceScaleOffset, &sourceScale, SourceScaleSize);

        changed = true;
    }
    return changed;
}

void OpaqueTextureMaterialShader::updateSampledImage(RenderState &state, int binding, QSGTexture **texture, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    // update mask for `layout(binding = 2) uniform sampler2D mask;`
    if (binding != MaskBindingIndex)
        return QSGOpaqueTextureMaterialRhiShader::updateSampledImage(state, binding, texture, newMaterial, oldMaterial);

#ifdef QT_NO_DEBUG
    Q_UNUSED(oldMaterial);
#endif
    Q_ASSERT(oldMaterial == nullptr || newMaterial->type() == oldMaterial->type());
    OpaqueTextureMaterial *tx = static_cast<OpaqueTextureMaterial *>(newMaterial);
    QSGTexture *t = tx->maskTexture();
    if (!t) {
        *texture = nullptr;
        return;
    }

    t->commitTextureOperations(state.rhi(), state.resourceUpdateBatch());
    *texture = t;
}

bool OpaqueTextureMaterialShader::updateGraphicsPipelineState(RenderState &state, GraphicsPipelineState *ps,
                                                              QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    auto material = static_cast<OpaqueTextureMaterial*>(newMaterial);
    if (!material)
        return QSGOpaqueTextureMaterialRhiShader::updateGraphicsPipelineState(state, ps, newMaterial, oldMaterial);

    bool changed = false;

    if (material->blendSrcColor() != ps->srcColor || material->blendDstColor() != ps->dstColor) {
        ps->srcColor = material->blendSrcColor();
        ps->dstColor = material->blendDstColor();
        changed = true;
    }

    changed = QSGOpaqueTextureMaterialRhiShader::updateGraphicsPipelineState(state, ps, newMaterial, oldMaterial) || changed;
    return changed;
}
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
TextureMaterialShader::TextureMaterialShader(int viewCount)
    : OpaqueTextureMaterialShader(viewCount)
#else
TextureMaterialShader::TextureMaterialShader()
    : OpaqueTextureMaterialShader()
#endif
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) // TODO qt6
#if QT_CONFIG(opengl)
    setShaderSourceFile(QOpenGLShader::Fragment, ":/dtk/declarative/shaders/quickitemviewport.frag");
#endif
#else
    setShaderFileName(QSGMaterialShader::VertexStage, QStringLiteral(":/dtk/declarative/shaders_ng/quickitemviewport.vert.qsb"));
    setShaderFileName(QSGMaterialShader::FragmentStage, QStringLiteral(":/dtk/declarative/shaders_ng/quickitemviewport.frag.qsb"));
#endif
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void TextureMaterialShader::updateState(const QSGMaterialShader::RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect)
{
    Q_ASSERT(oldEffect == nullptr || newEffect->type() == oldEffect->type());
#if QT_CONFIG(opengl)
    if (state.isOpacityDirty())
        program()->setUniformValue(m_opacity_id, state.opacity());
#endif
    OpaqueTextureMaterialShader::updateState(state, newEffect, oldEffect);
}

void TextureMaterialShader::initialize()
{
    OpaqueTextureMaterialShader::initialize();
#if QT_CONFIG(opengl)
    m_opacity_id = program()->uniformLocation("opacity");
#endif
}
#else
enum Ubuf2 {
    opacitySize = 4,

    opacityOffset = Ubuf::SourceScaleOffset + Ubuf::SourceScaleSize,
};
bool TextureMaterialShader::updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    Q_ASSERT(oldMaterial == nullptr || newMaterial->type() == oldMaterial->type());

    bool changed = OpaqueTextureMaterialShader::updateUniformData(state, newMaterial, oldMaterial);
    QByteArray *buf = state.uniformData();
    Q_ASSERT(buf->size() >= opacityOffset + opacitySize);
    if (state.isOpacityDirty()) {
        const float opacity = state.opacity();
        memcpy(buf->data() + opacityOffset, &opacity, opacitySize);
        changed = true;
    }
    return changed;
}
#endif

MaskEffectNode::MaskEffectNode()
    : m_geometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4)
    , m_texCoordMode(QSGDefaultImageNode::NoTransform)
    , m_isAtlasTexture(false)
    , m_ownsTexture(false)
{
    setGeometry(&m_geometry);
    setMaterial(&m_material);
    setOpaqueMaterial(&m_opaque_material);
    m_material.setMipmapFiltering(QSGTexture::None);
    m_opaque_material.setMipmapFiltering(QSGTexture::None);
}

MaskEffectNode::~MaskEffectNode()
{
    if (m_ownsTexture)
        delete m_material.texture();
}

void MaskEffectNode::setFiltering(QSGTexture::Filtering filtering)
{
    if (m_material.filtering() == filtering)
        return;

    m_material.setFiltering(filtering);
    m_opaque_material.setFiltering(filtering);
    markDirty(DirtyMaterial);
}

QSGTexture::Filtering MaskEffectNode::filtering() const
{
    return m_material.filtering();
}

void MaskEffectNode::setMipmapFiltering(QSGTexture::Filtering filtering)
{
    if (m_material.mipmapFiltering() == filtering)
        return;

    m_material.setMipmapFiltering(filtering);
    m_opaque_material.setMipmapFiltering(filtering);
    markDirty(DirtyMaterial);
}

QSGTexture::Filtering MaskEffectNode::mipmapFiltering() const
{
    return m_material.mipmapFiltering();
}

void MaskEffectNode::setAnisotropyLevel(QSGTexture::AnisotropyLevel level)
{
    if (m_material.anisotropyLevel() == level)
        return;

    m_material.setAnisotropyLevel(level);
    m_opaque_material.setAnisotropyLevel(level);
    markDirty(DirtyMaterial);
}

void MaskEffectNode::setMaskTexture(QSGTexture *texture)
{
    if (texture == m_material.maskTexture())
        return;

    m_material.setMaskTexture(texture);
    m_opaque_material.setMaskTexture(texture);
    markDirty(DirtyMaterial);
}

void MaskEffectNode::setMaskScale(QVector2D maskScale)
{
    if (maskScale == m_material.maskScale())
        return;

    m_material.setMaskScale(maskScale);
    m_opaque_material.setMaskScale(maskScale);
    markDirty(DirtyMaterial);
}

void MaskEffectNode::setMaskOffset(QVector2D maskOffset)
{
    if (m_material.maskOffset() == maskOffset)
        return;

    m_material.setMaskOffset(maskOffset);
    m_opaque_material.setMaskOffset(maskOffset);
    markDirty(DirtyMaterial);
}

void MaskEffectNode::setSourceScale(QVector2D sourceScale)
{
    if (sourceScale == m_material.sourceScale())
        return;

    m_material.setSourceScale(sourceScale);
    m_opaque_material.setSourceScale(sourceScale);
    markDirty(DirtyMaterial);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void MaskEffectNode::setCompositionMode(QPainter::CompositionMode mode)
{
    if (m_compositionMode == mode)
        return;
    m_compositionMode = mode;

    QSGMaterialShader::GraphicsPipelineState::BlendFactor srcColor
        = QSGMaterialShader::GraphicsPipelineState::BlendFactor::One;
    QSGMaterialShader::GraphicsPipelineState::BlendFactor dstColor
        = QSGMaterialShader::GraphicsPipelineState::BlendFactor::OneMinusSrcAlpha;

    switch (mode) {
    case QPainter::CompositionMode_Source:
        dstColor = QSGMaterialShader::GraphicsPipelineState::BlendFactor::Zero;
        break;
    case QPainter::CompositionMode_Destination:
        srcColor = QSGMaterialShader::GraphicsPipelineState::BlendFactor::Zero;
        dstColor = QSGMaterialShader::GraphicsPipelineState::BlendFactor::One;
        break;
    case QPainter::CompositionMode_DestinationOver:
        srcColor = QSGMaterialShader::GraphicsPipelineState::BlendFactor::OneMinusDstAlpha;
        dstColor = QSGMaterialShader::GraphicsPipelineState::BlendFactor::One;
        break;
    case QPainter::CompositionMode_Clear:
        srcColor = QSGMaterialShader::GraphicsPipelineState::BlendFactor::Zero;
        dstColor = QSGMaterialShader::GraphicsPipelineState::BlendFactor::Zero;
        break;
    default: break;
    }

    m_material.setBlendSrcColor(srcColor);
    m_material.setBlendDstColor(dstColor);
    m_opaque_material.setBlendSrcColor(srcColor);
    m_opaque_material.setBlendDstColor(dstColor);

    markDirty(DirtyForceUpdate);
}
#endif

QSGTexture::AnisotropyLevel MaskEffectNode::anisotropyLevel() const
{
    return m_material.anisotropyLevel();
}

void MaskEffectNode::setRect(const QRectF &r)
{
    if (m_rect == r)
        return;

    m_rect = r;
    rebuildGeometry(&m_geometry, texture(), m_rect, m_sourceRect, m_texCoordMode);
    markDirty(DirtyGeometry);
}

QRectF MaskEffectNode::rect() const
{
    return m_rect;
}

void MaskEffectNode::setSourceRect(const QRectF &r)
{
    if (m_sourceRect == r)
        return;

    m_sourceRect = r;
    rebuildGeometry(&m_geometry, texture(), m_rect, m_sourceRect, m_texCoordMode);
    markDirty(DirtyGeometry);
}

QRectF MaskEffectNode::sourceRect() const
{
    return m_sourceRect;
}

void MaskEffectNode::setTexture(QSGTexture *texture)
{
    Q_ASSERT(texture);
    DirtyState dirty = DirtyMaterial;
    if (m_material.texture() == texture) {
        markDirty(dirty);
        return;
    }

    if (m_ownsTexture)
        delete m_material.texture();
    m_material.setTexture(texture);
    m_opaque_material.setTexture(texture);
    rebuildGeometry(&m_geometry, texture, m_rect, m_sourceRect, m_texCoordMode);

    bool wasAtlas = m_isAtlasTexture;
    m_isAtlasTexture = texture->isAtlasTexture();
    if (wasAtlas || m_isAtlasTexture)
        dirty |= DirtyGeometry;
    markDirty(dirty);
}

QSGTexture *MaskEffectNode::texture() const
{
    return m_material.texture();
}

void MaskEffectNode::setTextureCoordinatesTransform(TextureCoordinatesTransformMode mode)
{
    if (m_texCoordMode == mode)
        return;
    m_texCoordMode = mode;
    rebuildGeometry(&m_geometry, texture(), m_rect, m_sourceRect, m_texCoordMode);
    markDirty(DirtyMaterial);
}

QSGDefaultImageNode::TextureCoordinatesTransformMode MaskEffectNode::textureCoordinatesTransform() const
{
    return m_texCoordMode;
}

void MaskEffectNode::setOwnsTexture(bool owns)
{
    m_ownsTexture = owns;
}

bool MaskEffectNode::ownsTexture() const
{
    return m_ownsTexture;
}

QSGMaterialType *TextureMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QSGMaterialShader *TextureMaterial::createShader() const
{
    return new TextureMaterialShader;
}
#else
QSGMaterialShader *TextureMaterial::createShader(QSGRendererInterface::RenderMode renderMode) const
{
    Q_UNUSED(renderMode)
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    return new TextureMaterialShader(viewCount());
#else
    return new TextureMaterialShader;
#endif
}
#endif

int TextureMaterial::compare(const QSGMaterial *o) const
{
    return OpaqueTextureMaterial::compare(o);
}

QSGMaterialType *OpaqueTextureMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QSGMaterialShader *OpaqueTextureMaterial::createShader() const
{
    return new OpaqueTextureMaterialShader;
}
#else
QSGMaterialShader *OpaqueTextureMaterial::createShader(QSGRendererInterface::RenderMode renderMode) const
{
    Q_UNUSED(renderMode)
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    return new OpaqueTextureMaterialShader(viewCount());
#else
    return new OpaqueTextureMaterialShader;
#endif
}
#endif

int OpaqueTextureMaterial::compare(const QSGMaterial *o) const
{
    Q_ASSERT(o && type() == o->type());
    return Q_UNLIKELY(o == this) ? 0 : 1;
}

void OpaqueTextureMaterial::setMaskTexture(QSGTexture *texture)
{
    Q_ASSERT(texture);
    if (!m_maskTexture) {
        m_maskTexture = texture;
        return;
    }
    if (!texture || !m_maskTexture)
        return;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (texture->textureId() == m_maskTexture->textureId())
        return;
#else
    // TODO support vulkan
    auto sourceTextureId = (m_maskTexture->nativeInterface<QNativeInterface::QSGOpenGLTexture>())->nativeTexture();
    auto targetTextureId = (texture->nativeInterface<QNativeInterface::QSGOpenGLTexture>())->nativeTexture();
    if (sourceTextureId == targetTextureId)
        return;
#endif

    m_maskTexture = texture;
}

void OpaqueTextureMaterial::setMaskScale(QVector2D maskScale)
{
    if (maskScale == m_maskScale)
        return;

    m_maskScale = maskScale;
}

void OpaqueTextureMaterial::setMaskOffset(QVector2D maskOffset)
{
    if (maskOffset == m_maskOffset)
        return;

    m_maskOffset = maskOffset;
}

void OpaqueTextureMaterial::setSourceScale(QVector2D sourceScale)
{
    if (sourceScale == m_sourceScale)
        return;

    m_sourceScale = sourceScale;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void OpaqueTextureMaterial::setBlendSrcColor(QSGMaterialShader::GraphicsPipelineState::BlendFactor factor)
{
    if (m_blendSrcColor == factor)
        return;

    m_blendSrcColor = factor;
}

void OpaqueTextureMaterial::setBlendDstColor(QSGMaterialShader::GraphicsPipelineState::BlendFactor factor)
{
    if (m_blendDstColor == factor)
        return;

    m_blendDstColor = factor;
}
#endif

DQUICK_END_NAMESPACE
