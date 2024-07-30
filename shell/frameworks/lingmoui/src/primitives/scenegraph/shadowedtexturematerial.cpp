/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "shadowedtexturematerial.h"

#include <QOpenGLContext>

QSGMaterialType ShadowedTextureMaterial::staticType;

ShadowedTextureMaterial::ShadowedTextureMaterial()
    : ShadowedRectangleMaterial()
{
    setFlag(QSGMaterial::Blending, true);
}

QSGMaterialShader *ShadowedTextureMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new ShadowedTextureShader{shaderType};
}

QSGMaterialType *ShadowedTextureMaterial::type() const
{
    return &staticType;
}

int ShadowedTextureMaterial::compare(const QSGMaterial *other) const
{
    auto material = static_cast<const ShadowedTextureMaterial *>(other);

    auto result = ShadowedRectangleMaterial::compare(other);
    if (result == 0) {
        if (material->textureSource == textureSource) {
            return 0;
        } else {
            return (material->textureSource < textureSource) ? 1 : -1;
        }
    }

    return QSGMaterial::compare(other);
}

ShadowedTextureShader::ShadowedTextureShader(ShadowedRectangleMaterial::ShaderType shaderType)
    : ShadowedRectangleShader(shaderType)
{
    setShader(shaderType, QStringLiteral("shadowedtexture"));
}

void ShadowedTextureShader::updateSampledImage(QSGMaterialShader::RenderState &state,
                                               int binding,
                                               QSGTexture **texture,
                                               QSGMaterial *newMaterial,
                                               QSGMaterial *oldMaterial)
{
    Q_UNUSED(state);
    Q_UNUSED(oldMaterial);
    if (binding == 1) {
        *texture = static_cast<ShadowedTextureMaterial *>(newMaterial)->textureSource;
    }
}
