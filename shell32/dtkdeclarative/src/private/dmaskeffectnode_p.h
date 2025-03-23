// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMASKEFFECTNODE_P_H
#define DMASKEFFECTNODE_P_H

#include "private/qsgtexturematerial_p.h"

#include <dtkdeclarative_global.h>

#include <QSGImageNode>
#include <QPointer>
#include <QPainter>

DQUICK_BEGIN_NAMESPACE

class OpaqueTextureMaterial : public QSGOpaqueTextureMaterial
{
public:
    QSGMaterialType *type() const override;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QSGMaterialShader *createShader() const override;
#else
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode renderMode) const override;
#endif
    int compare(const QSGMaterial *other) const override;

    void setMaskTexture(QSGTexture *texture);
    QSGTexture *maskTexture() const { return m_maskTexture.data(); }

    void setMaskScale(QVector2D maskScale);
    QVector2D maskScale() const { return m_maskScale; }

    void setMaskOffset(QVector2D maskOffset);
    QVector2D maskOffset() const { return m_maskOffset; }

    void setSourceScale(QVector2D sourceScale);
    QVector2D sourceScale() const { return m_sourceScale; }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QSGMaterialShader::GraphicsPipelineState::BlendFactor blendSrcColor() const
    { return m_blendSrcColor; }

    QSGMaterialShader::GraphicsPipelineState::BlendFactor blendDstColor() const
    { return m_blendDstColor; }

    void setBlendSrcColor(QSGMaterialShader::GraphicsPipelineState::BlendFactor factor);
    void setBlendDstColor(QSGMaterialShader::GraphicsPipelineState::BlendFactor factor);
#endif

private:
    QPointer<QSGTexture> m_maskTexture = nullptr;
    QVector2D m_maskScale;
    QVector2D m_maskOffset;
    QVector2D m_sourceScale;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QSGMaterialShader::GraphicsPipelineState::BlendFactor m_blendSrcColor = QSGMaterialShader::GraphicsPipelineState::One;
    QSGMaterialShader::GraphicsPipelineState::BlendFactor m_blendDstColor = QSGMaterialShader::GraphicsPipelineState::OneMinusSrcAlpha;
#endif
};

class TextureMaterial : public OpaqueTextureMaterial
{
public:
    QSGMaterialType *type() const override;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QSGMaterialShader *createShader() const override;
#else
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode renderMode) const override;
#endif
    int compare(const QSGMaterial *other) const override;
};

class MaskEffectNode : public QSGImageNode
{
public:
    MaskEffectNode();
    ~MaskEffectNode() override;

    void setRect(const QRectF &rect) override;
    QRectF rect() const override;

    void setSourceRect(const QRectF &r) override;
    QRectF sourceRect() const override;

    void setTexture(QSGTexture *texture) override;
    QSGTexture *texture() const override;

    void setFiltering(QSGTexture::Filtering filtering) override;
    QSGTexture::Filtering filtering() const override;

    void setMipmapFiltering(QSGTexture::Filtering filtering) override;
    QSGTexture::Filtering mipmapFiltering() const override;

    void setTextureCoordinatesTransform(TextureCoordinatesTransformMode mode) override;
    TextureCoordinatesTransformMode textureCoordinatesTransform() const override;

    void setOwnsTexture(bool owns) override;
    bool ownsTexture() const override;

    void setAnisotropyLevel(QSGTexture::AnisotropyLevel level);

    void setMaskTexture(QSGTexture *texture);
    void setMaskScale(QVector2D maskScale);
    void setMaskOffset(QVector2D maskOffset);
    void setSourceScale(QVector2D sourceScale);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void setCompositionMode(QPainter::CompositionMode mode);
#endif

    QSGTexture::AnisotropyLevel anisotropyLevel() const;
private:
    QSGGeometry m_geometry;
    OpaqueTextureMaterial m_opaque_material;
    TextureMaterial m_material;
    QRectF m_rect;
    QRectF m_sourceRect;
    TextureCoordinatesTransformMode m_texCoordMode;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPainter::CompositionMode m_compositionMode = QPainter::CompositionMode_SourceOver;
#endif

    uint m_isAtlasTexture : 1;
    uint m_ownsTexture : 1;
};

DQUICK_END_NAMESPACE

#endif // DMASKEFFECTNODE_P_H
