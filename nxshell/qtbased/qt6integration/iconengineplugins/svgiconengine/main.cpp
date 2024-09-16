// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LGPL-3.0-or-later OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <qiconengineplugin.h>
#include <qstringlist.h>

#include "qsvgiconengine.h"

#include <qiodevice.h>
#include <qbytearray.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

class QSvgIconPlugin : public QIconEnginePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QIconEngineFactoryInterface" FILE "qsvgiconengine.json")

public:
    QStringList keys() const;
    QIconEngine *create(const QString &filename = QString());
};

QStringList QSvgIconPlugin::keys() const
{
    QStringList keys(QLatin1String("svg"));
#ifndef QT_NO_COMPRESS
    if (Q_LIKELY(!qEnvironmentVariableIsSet("QT_NO_COMPRESS"))) {
        keys << QLatin1String("svgz") << QLatin1String("svg.gz");
    }
#endif
    return keys;
}

QIconEngine *QSvgIconPlugin::create(const QString &file)
{
    QSvgIconEngine *engine = new QSvgIconEngine;
    if (!file.isNull())
        engine->addFile(file, QSize(), QIcon::Normal, QIcon::Off);
    return engine;
}

QT_END_NAMESPACE

#include "main.moc"
