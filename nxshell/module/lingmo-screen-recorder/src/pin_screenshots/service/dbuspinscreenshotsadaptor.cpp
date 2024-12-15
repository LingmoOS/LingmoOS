// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbuspinscreenshotsadaptor.h"
#include <QtCore/QMetaObject>
#include <QImage>
DbusPinScreenShotsAdaptor::DbusPinScreenShotsAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

DbusPinScreenShotsAdaptor::~DbusPinScreenShotsAdaptor()
{
    // destructor
}

bool DbusPinScreenShotsAdaptor::openFile(QString filePath)
{
    qDebug() << "启动贴图" << __FUNCTION__ << __LINE__;
    QMetaObject::invokeMethod(parent(), "openFile", Q_ARG(QString, filePath));
    return true;
}


void DbusPinScreenShotsAdaptor::openImageAndName(QByteArray images, QString imageName, QPoint point)
{
    qDebug() << "启动贴图" <<  __FUNCTION__ << __LINE__;
    QByteArray data = images;
    QString tmp_data = QString::fromLatin1(data.data(), data.size());
    QByteArray srcData = QByteArray::fromBase64(tmp_data.toLatin1());
    data = qUncompress(srcData);
    QImage image;
    image.loadFromData(data);
    QMetaObject::invokeMethod(parent(), "openImageAndName", Q_ARG(QImage, image), Q_ARG(QString, imageName), Q_ARG(QPoint, point));
}

void DbusPinScreenShotsAdaptor::openImage(QByteArray images)
{
    qDebug() << "启动贴图" << __FUNCTION__ << __LINE__;
    QByteArray data = images;
    QString tmp_data = QString::fromLatin1(data.data(), data.size());
    QByteArray srcData = QByteArray::fromBase64(tmp_data.toLatin1());
    data = qUncompress(srcData);
    QImage image;
    image.loadFromData(data);
    QMetaObject::invokeMethod(parent(), "openImage", Q_ARG(QImage, image));
}
