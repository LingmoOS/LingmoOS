/*
* Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
*
* Author: He MingYang Hao<hemingyang@uniontech.com>
*
* Maintainer: He MingYang <hemingyang@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef OCRINTERFACE_H
#define OCRINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include <QImage>
#include <QBuffer>
#include <QDebug>

class OcrInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    {
        return "com.lingmo.Ocr";
    }

public:
    /*
    * @param: serviceName QDBusConnection 注册的服务名字
    * @param: ObjectPath QDBusConnection 注册的对象路径
    */
    OcrInterface(const QString &serviceName, const QString &ObjectPath,
                 const QDBusConnection &connection, QObject *parent = nullptr);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    ~OcrInterface();

public Q_SLOTS: // METHODS
    /*
    * @bref:openFile 通过路径打开图片文件
    * @param: filePath 图片的路径
    * @return: QDBusPendingReply
    */
    inline QDBusPendingReply<> openFile(const QString &filePath)
    {
        return call(QStringLiteral("openFile"), filePath);
    }

    /*
    * @bref:openImages
    * @param: image 图片
    * @return: QDBusPendingReply
    * @note:
    */
    inline QDBusPendingReply<> openImage(const QImage &image)
    {
        qDebug() << __FUNCTION__;
        QByteArray data;
        QBuffer buf(&data);
        if (image.save(&buf, "PNG")) {
            data = qCompress(data, 9);
            data = data.toBase64();
        }
        return call(QStringLiteral("openImage"), QVariant::fromValue(data));
    }

    /*
    * @bref:openImages
    * @param: image 图片
    * @param: imageName 图片名称
    * @return: QDBusPendingReply
    * @note:
    */
    inline QDBusPendingReply<> openImageAndName(const QImage &image, const QString &imageName)
    {
        qDebug() << __FUNCTION__;
        QByteArray data;
        QBuffer buf(&data);
        if (image.save(&buf, "PNG")) {
            data = qCompress(data, 9);
            data = data.toBase64();
        }
        return call(QStringLiteral("openImageAndName"), QVariant::fromValue(data), imageName);
    }

Q_SIGNALS: // SIGNALS
};

namespace com {
namespace lingmo {
typedef ::OcrInterface Ocr;
}
}
#endif // DRAWINTERFACE_H
