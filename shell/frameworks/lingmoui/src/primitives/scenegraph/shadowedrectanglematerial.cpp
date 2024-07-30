/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "shadowedrectanglematerial.h"

#include <QOpenGLContext>

QSGMaterialType ShadowedRectangleMaterial::staticType;

ShadowedRectangleMaterial::ShadowedRectangleMaterial()
{
    setFlag(QSGMaterial::Blending, true);
}

QSGMaterialShader *ShadowedRectangleMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new ShadowedRectangleShader{shaderType};
}

QSGMaterialType *ShadowedRectangleMaterial::type() const
{
    return &staticType;
}

int ShadowedRectangleMaterial::compare(const QSGMaterial *other) const
{
    auto material = static_cast<const ShadowedRectangleMaterial *>(other);
    /* clang-format off */
    if (material->color == color
        && material->shadowColor == shadowColor
        && material->offset == offset
        && material->aspect == aspect
        && qFuzzyCompare(material->size, size)
        && qFuzzyCompare(material->radius, radius)) { /* clang-format on */
        return 0;
    }

    return QSGMaterial::compare(other);
}

ShadowedRectangleShader::ShadowedRectangleShader(ShadowedRectangleMaterial::ShaderType shaderType)
{
    setShader(shaderType, QStringLiteral("shadowedrectangle"));
}

bool ShadowedRectangleShader::updateUniformData(RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    bool changed = false;
    QByteArray *buf = state.uniformData();
    Q_ASSERT(buf->size() >= 160);

    if (state.isMatrixDirty()) {
        const QMatrix4x4 m = state.combinedMatrix();
        memcpy(buf->data(), m.constData(), 64);
        changed = true;
    }

    if (state.isOpacityDirty()) {
        const float opacity = state.opacity();
        memcpy(buf->data() + 72, &opacity, 4);
        changed = true;
    }

    if (!oldMaterial || newMaterial->compare(oldMaterial) != 0) {
        const auto material = static_cast<ShadowedRectangleMaterial *>(newMaterial);
        memcpy(buf->data() + 64, &material->aspect, 8);
        memcpy(buf->data() + 76, &material->size, 4);
        memcpy(buf->data() + 80, &material->radius, 16);
        float c[4];
        material->color.getRgbF(&c[0], &c[1], &c[2], &c[3]);
        memcpy(buf->data() + 96, c, 16);
        material->shadowColor.getRgbF(&c[0], &c[1], &c[2], &c[3]);
        memcpy(buf->data() + 112, c, 16);
        memcpy(buf->data() + 128, &material->offset, 8);
        changed = true;
    }

    return changed;
}

void ShadowedRectangleShader::setShader(ShadowedRectangleMaterial::ShaderType shaderType, const QString &shader)
{
    const auto shaderRoot = QStringLiteral(":/qt/qml/org/kde/lingmoui/primitives/shaders/");

    setShaderFileName(QSGMaterialShader::VertexStage, shaderRoot + QStringLiteral("shadowedrectangle.vert.qsb"));

    auto shaderFile = shader;
    if (shaderType == ShadowedRectangleMaterial::ShaderType::LowPower) {
        shaderFile += QStringLiteral("_lowpower");
    }
    setShaderFileName(QSGMaterialShader::FragmentStage, shaderRoot + shaderFile + QStringLiteral(".frag.qsb"));
}
