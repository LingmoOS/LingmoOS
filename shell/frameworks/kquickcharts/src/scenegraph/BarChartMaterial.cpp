/*
 * SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "BarChartMaterial.h"

BarChartMaterial::BarChartMaterial()
{
    setFlag(QSGMaterial::Blending);
}

BarChartMaterial::~BarChartMaterial()
{
}

QSGMaterialType *BarChartMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *BarChartMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new BarChartShader();
}

int BarChartMaterial::compare(const QSGMaterial *other) const
{
    auto material = static_cast<const BarChartMaterial *>(other);

    /* clang-format off */
    if (material->aspect == aspect
        && qFuzzyCompare(material->radius, radius)
        && material->backgroundColor == backgroundColor) { /* clang-format on */
        return 0;
    }

    return QSGMaterial::compare(other);
}

BarChartShader::BarChartShader()
{
    setShaders(QStringLiteral("barchart.vert"), QStringLiteral("barchart.frag"));
}

BarChartShader::~BarChartShader()
{
}

bool BarChartShader::updateUniformData(QSGMaterialShader::RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    bool changed = false;

    UniformDataStream uniformData(state);

    if (state.isMatrixDirty()) {
        uniformData << state.combinedMatrix();
        changed = true;
    } else {
        uniformData.skip<QMatrix4x4>();
    }

    if (state.isOpacityDirty()) {
        uniformData << state.opacity();
        changed = true;
    } else {
        uniformData.skip<float>();
    }

    if (!oldMaterial || newMaterial->compare(oldMaterial) != 0) {
        const auto material = static_cast<BarChartMaterial *>(newMaterial);
        uniformData << material->aspect;
        uniformData << material->radius;
        uniformData << material->backgroundColor;
        changed = true;
    }

    return changed;
}
