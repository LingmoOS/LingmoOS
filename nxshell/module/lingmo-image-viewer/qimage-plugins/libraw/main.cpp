// Copyright (C) 2012 Alberto Mardegan <info@mardy.it>
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QByteArray>
#include <QImageIOHandler>
#include <QIODevice>
#include <QStringList>

#include "rawiohandler.h"

class FreeimageQt5Plugin: public QImageIOPlugin
{
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "raw.json")
#endif

public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device,
                              const QByteArray &format) const Q_DECL_OVERRIDE;
    QImageIOHandler *create(QIODevice *device,
                            const QByteArray &format = QByteArray()) const Q_DECL_OVERRIDE;
};

QStringList FreeimageQt5Plugin::keys() const
{

    const QStringList raws = QStringList()
                             << QLatin1String("CR2") << QLatin1String("CRW")   // Canon cameras
                             << QLatin1String("DCR") << QLatin1String("KDC")   // Kodak cameras
                             << QLatin1String("MRW")            // Minolta cameras
                             << QLatin1String("NEF")            // Nikon cameras
                             << QLatin1String("ORF")            // Olympus cameras
                             << QLatin1String("PEF")            // Pentax cameras
                             << QLatin1String("RAF")            // Fuji cameras
                             << QLatin1String("SRF")            // Sony cameras
                             << QLatin1String("DNG")            //
                             << QLatin1String("RAW");           // RAW formats.
    return raws;
}

QImageIOPlugin::Capabilities
FreeimageQt5Plugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    Capabilities cap;
    if (keys().contains(format.toUpper()) ||
            format == "tif" ||
            format == "tiff") {
        if (device->isReadable() && RawIOHandler::canRead(device))
            cap |= CanRead;
    }

    return cap;
}

QImageIOHandler *FreeimageQt5Plugin::create(QIODevice *device,
                                            const QByteArray &format) const
{
    RawIOHandler *handler = new RawIOHandler();
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(qtraw, RawPlugin)
#endif

#include "main.moc"
