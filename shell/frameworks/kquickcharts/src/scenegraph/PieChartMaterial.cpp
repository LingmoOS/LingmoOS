/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "PieChartMaterial.h"

constexpr int MaximumSegmentCount = 100;

PieChartMaterial::PieChartMaterial()
{
    setFlag(QSGMaterial::Blending);
}

PieChartMaterial::~PieChartMaterial()
{
}

QSGMaterialType *PieChartMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

QSGMaterialShader *PieChartMaterial::createShader(QSGRendererInterface::RenderMode) const
{
    return new PieChartShader();
}

QVector2D PieChartMaterial::aspectRatio() const
{
    return m_aspectRatio;
}

float PieChartMaterial::innerRadius() const
{
    return m_innerRadius;
}

float PieChartMaterial::outerRadius() const
{
    return m_outerRadius;
}

QColor PieChartMaterial::backgroundColor() const
{
    return m_backgroundColor;
}

QList<QVector2D> PieChartMaterial::segments() const
{
    return m_segments;
}

QList<QVector4D> PieChartMaterial::colors() const
{
    return m_colors;
}

bool PieChartMaterial::smoothEnds() const
{
    return m_smoothEnds;
}

float PieChartMaterial::fromAngle() const
{
    return m_fromAngle;
}

float PieChartMaterial::toAngle() const
{
    return m_toAngle;
}

void PieChartMaterial::setAspectRatio(const QVector2D &aspect)
{
    m_aspectRatio = aspect;
}

void PieChartMaterial::setInnerRadius(float radius)
{
    m_innerRadius = radius;
}

void PieChartMaterial::setOuterRadius(float radius)
{
    m_outerRadius = radius;
}

void PieChartMaterial::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
}

void PieChartMaterial::setSegments(const QList<QVector2D> &segments)
{
    m_segments = segments;
}

void PieChartMaterial::setColors(const QList<QVector4D> &colors)
{
    m_colors = colors;
}

void PieChartMaterial::setSmoothEnds(bool smooth)
{
    m_smoothEnds = smooth;
}

void PieChartMaterial::setFromAngle(float angle)
{
    m_fromAngle = angle;
}

void PieChartMaterial::setToAngle(float angle)
{
    m_toAngle = angle;
}

PieChartShader::PieChartShader()
{
    setShaders(QStringLiteral("piechart.vert"), QStringLiteral("piechart.frag"));
}

PieChartShader::~PieChartShader()
{
}

bool PieChartShader::updateUniformData(QSGMaterialShader::RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
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
        const auto material = static_cast<PieChartMaterial *>(newMaterial);

        uniformData << material->aspectRatio() << material->innerRadius() << material->outerRadius() << material->backgroundColor() //
                    << int(material->smoothEnds()) << material->fromAngle() << material->toAngle();

        const auto segmentCount = uint(material->segments().size());
        uniformData << segmentCount;

        uniformData << material->segments();
        uniformData.skipComponents((MaximumSegmentCount - segmentCount) * 4);

        uniformData << material->colors();
        uniformData.skipComponents((MaximumSegmentCount - segmentCount) * 4);

        changed = true;
    }

    return changed;
}
