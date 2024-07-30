/*
 * SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef PIECHARTMATERIAL_H
#define PIECHARTMATERIAL_H

#include <QColor>
#include <QSGMaterial>
#include <QSGMaterialShader>

#include "SDFShader.h"

class BarChartMaterial : public QSGMaterial
{
public:
    BarChartMaterial();
    ~BarChartMaterial();

    QSGMaterialType *type() const override;
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode) const override;
    int compare(const QSGMaterial *other) const override;

    QVector2D aspect = QVector2D{1.0, 1.0};
    float radius = 0.0;
    QColor backgroundColor = Qt::transparent;
};

class BarChartShader : public SDFShader
{
public:
    BarChartShader();
    ~BarChartShader();

    bool updateUniformData(QSGMaterialShader::RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
};

#endif // PIECHARTMATERIAL_H
