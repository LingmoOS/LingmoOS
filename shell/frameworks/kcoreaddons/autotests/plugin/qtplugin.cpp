/*
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QObject>
#include <QtPlugin>

class MyQtPlugin : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "somepluginid" FILE "data/jsonplugin2.json")
};

#include "qtplugin.moc"
