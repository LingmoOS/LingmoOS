// Copyright (C) 2022 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "qdciiohandler.h"

#include <QImageIOHandler>
#include <QStringList>
#include <QIODevice>
#include <QByteArray>

QT_BEGIN_NAMESPACE

class QDciPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "dci.json")

public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

QStringList QDciPlugin::keys() const
{
    return QStringList() << QLatin1String("dci");
}

QImageIOPlugin::Capabilities QDciPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == QByteArrayLiteral("dci") || format.startsWith(QByteArrayLiteral("dci-")))
        return Capabilities(CanRead);
    if (!format.isEmpty())
        return {};

    Capabilities cap;
    if (device->isReadable() && QDciIOHandler::canRead(device))
        cap |= CanRead;
    return cap;
}

QImageIOHandler *QDciPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QDciIOHandler *hand = new QDciIOHandler();
    hand->setDevice(device);
    hand->setFormat(format);
    return hand;
}

QT_END_NAMESPACE

#include "main.moc"
