/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef LINECHARTMATERIAL_H
#define LINECHARTMATERIAL_H

#include <QColor>
#include <QSGMaterial>
#include <QSGMaterialShader>

#include "SDFShader.h"

class LineChartMaterial : public QSGMaterial
{
public:
    LineChartMaterial();
    ~LineChartMaterial();

    QSGMaterialType *type() const override;
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode) const override;
    int compare(const QSGMaterial *other) const override;

    float aspect = 1.0;
    float lineWidth = 0.0;
    float smoothing = 0.1;
};

class LineChartShader : public SDFShader
{
public:
    LineChartShader();
    ~LineChartShader();

    bool updateUniformData(QSGMaterialShader::RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
};

#endif // LINECHARTMATERIAL_H
