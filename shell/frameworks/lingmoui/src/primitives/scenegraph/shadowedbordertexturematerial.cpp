/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "shadowedbordertexturematerial.h"

#include <QOpenGLContext>

QSGMaterialType ShadowedBorderTextureMaterial::staticType;

ShadowedBorderTextureMaterial::ShadowedBorderTextureMaterial()
    : ShadowedBorderRectangleMaterial()
{
    setFlag(QSGMaterial::Blending, true);
}

QSGMaterialShader *ShadowedBorderTextureMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new ShadowedBorderTextureShader{shaderType};
}

QSGMaterialType *ShadowedBorderTextureMaterial::type() const
{
    return &staticType;
}

int ShadowedBorderTextureMaterial::compare(const QSGMaterial *other) const
{
    auto material = static_cast<const ShadowedBorderTextureMaterial *>(other);

    auto result = ShadowedBorderRectangleMaterial::compare(other);
    if (result == 0) {
        if (material->textureSource == textureSource) {
            return 0;
        } else {
            return (material->textureSource < textureSource) ? 1 : -1;
        }
    }

    return QSGMaterial::compare(other);
}

ShadowedBorderTextureShader::ShadowedBorderTextureShader(ShadowedRectangleMaterial::ShaderType shaderType)
    : ShadowedBorderRectangleShader(shaderType)
{
    setShader(shaderType, QStringLiteral("shadowedbordertexture"));
}

void ShadowedBorderTextureShader::updateSampledImage(QSGMaterialShader::RenderState &state,
                                                     int binding,
                                                     QSGTexture **texture,
                                                     QSGMaterial *newMaterial,
                                                     QSGMaterial *oldMaterial)
{
    Q_UNUSED(state);
    Q_UNUSED(oldMaterial);
    if (binding == 1) {
        *texture = static_cast<ShadowedBorderTextureMaterial *>(newMaterial)->textureSource;
    }
}
