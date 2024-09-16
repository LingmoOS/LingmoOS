// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRAWINTERFACE_H
#define DRAWINTERFACE_H

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

/*
* @bref: 此类用于其它程序调用画板打开文件
* @require: QT += dbus
* @example:

    (1)
    DrawInterface *m_draw = new DrawInterface("com.deepin.Draw",
                                              "/com/deepin/Draw", QDBusConnection::sessionBus(), &a);
    QList<QString> list;
    list.append("/home/kingderzhang/Desktop/1.ddf");
    list.append("/home/kingderzhang/Desktop/2.ddf");
    list.append("/home/kingderzhang/Desktop/3.ddf");
    list.append("/home/kingderzhang/Desktop/2.png");
    m_draw->openFiles(list);

    (2)
    DrawInterface *m_draw = new DrawInterface("com.deepin.Draw",
                                              "/com/deepin/Draw", QDBusConnection::sessionBus(), &a);
    QList<QImage> list;
    list.append(QImage("/home/kingderzhang/Desktop/1.jpg"));
    list.append(QImage("/home/kingderzhang/Desktop/2.png"));
    list.append(QImage("/home/kingderzhang/Desktop/3.jpg"));
    list.append(QImage("/home/kingderzhang/Desktop/4.jpg"));
    m_draw->openImages(list);
*/
class DrawInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    {
        return "com.deepin.Draw";
    }

public:
    /*
    * @param: serviceName QDBusConnection 注册的服务名字
    * @param: ObjectPath QDBusConnection 注册的对象路径
    */
    DrawInterface(const QString &serviceName, const QString &ObjectPath,
                  const QDBusConnection &connection, QObject *parent = nullptr);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    ~DrawInterface();

public Q_SLOTS: // METHODS
    /*
    * @bref:openFiles 通过路径打开图片文件
    * @param: filePaths 图片的路径
    * @return: QDBusPendingReply
    */
    inline QDBusPendingReply<> openFiles(const QList<QString> &filePaths)
    {
        QList<QVariant> argumentList;
        for (QString path : filePaths) {
            argumentList << QVariant::fromValue(path.toLocal8Bit());
        }
        return call(QStringLiteral("openFiles"), argumentList);
    }

    /*
    * @bref:openImages 通过QImage打开图片
    * @param: filePaths 图片
    * @return: QDBusPendingReply
    * @note: 建议不要一次打开多个图片大文件，会比较卡
    */
    inline QDBusPendingReply<> openImages(const QList<QImage> &images)
    {
        QList<QVariant> argumentList;
        for (QImage img : images) {
            QByteArray data;
            QBuffer buf(&data);
            if (img.save(&buf, "PNG")) {
                data = qCompress(data, 9);
                data = data.toBase64();
                argumentList << QVariant::fromValue(data);
            }
        }

        return call(QStringLiteral("openImages"), argumentList);
    }

Q_SIGNALS: // SIGNALS
};

namespace com {
namespace deepin {
typedef ::DrawInterface Draw;
}
}
#endif // DRAWINTERFACE_H
