/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef PIECHARTMATERIAL_H
#define PIECHARTMATERIAL_H

#include <QColor>
#include <QSGMaterial>
#include <QSGMaterialShader>

#include "SDFShader.h"

class PieChartMaterial : public QSGMaterial
{
public:
    PieChartMaterial();
    ~PieChartMaterial();

    QSGMaterialType *type() const override;
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode) const override;

    QVector2D aspectRatio() const;
    float innerRadius() const;
    float outerRadius() const;
    QColor backgroundColor() const;
    bool smoothEnds() const;
    float fromAngle() const;
    float toAngle() const;

    QList<QVector2D> segments() const;
    QList<QVector4D> colors() const;

    void setAspectRatio(const QVector2D &aspect);
    void setInnerRadius(float radius);
    void setOuterRadius(float radius);
    void setBackgroundColor(const QColor &color);
    void setSmoothEnds(bool smooth);
    void setFromAngle(float angle);
    void setToAngle(float angle);

    void setSegments(const QList<QVector2D> &triangles);
    void setColors(const QList<QVector4D> &colors);

private:
    QVector2D m_aspectRatio;
    float m_innerRadius = 0.0f;
    float m_outerRadius = 0.0f;
    QColor m_backgroundColor;
    bool m_smoothEnds = false;
    float m_fromAngle = 0.0;
    float m_toAngle = 6.28318; // 2 * pi

    QList<QVector2D> m_segments;
    QList<QVector4D> m_colors;
};

class PieChartShader : public SDFShader
{
public:
    PieChartShader();
    ~PieChartShader();

    bool updateUniformData(QSGMaterialShader::RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
};

#endif // PIECHARTMATERIAL_H
