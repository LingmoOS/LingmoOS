/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PURPOSE_HELPER_H
#define PURPOSE_HELPER_H

#include <KPluginMetaData>
#include <QJsonObject>

namespace Purpose
{
QJsonObject readPluginType(const QString &pluginType);
KPluginMetaData createMetaData(const QString &file);

}

#endif
