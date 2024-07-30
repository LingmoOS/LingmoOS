/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "SDFShader.h"

#include <QOpenGLContext>
#include <QSurfaceFormat>

static const char shaderRoot[] = ":/qt/qml/org/kde/quickcharts/shaders/";

SDFShader::SDFShader()
{
}

SDFShader::~SDFShader()
{
}

void SDFShader::setShaders(const QString &vertex, const QString &fragment)
{
    setShaderFileName(QSGMaterialShader::VertexStage, QLatin1String(shaderRoot) + vertex + QLatin1String(".qsb"));
    setShaderFileName(QSGMaterialShader::FragmentStage, QLatin1String(shaderRoot) + fragment + QLatin1String(".qsb"));
}
