// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef XEVENTMONITOR_INTERFACE_H
#define XEVENTMONITOR_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

class ComDeepinApiXEventMonitorInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    typedef QList<QRect> AreaList;
    ComDeepinApiXEventMonitorInterface(const QString &service, const QString &path, const char *interface, QObject *parent = nullptr,
                                       const QDBusConnection &con = QDBusConnection::sessionBus());

    ~ComDeepinApiXEventMonitorInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<QString> RegisterArea(int in0, int in1, int in2, int in3, int in4)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1) << QVariant::fromValue(in2) << QVariant::fromValue(in3) << QVariant::fromValue(in4);
        return asyncCallWithArgumentList(QStringLiteral("RegisterArea"), argumentList);
    }

    inline QDBusPendingReply<QString> RegisterAreas(AreaList in0, int in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("RegisterAreas"), argumentList);
    }

    inline QDBusPendingReply<QString> RegisterFullScreen()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("RegisterFullScreen"), argumentList);
    }

    inline QDBusPendingReply<bool> UnregisterArea(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("UnregisterArea"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void ButtonPress(int in0, int in1, int in2, const QString &in3);
    void ButtonRelease(int in0, int in1, int in2, const QString &in3);
    void CancelAllArea();
    void CursorInto(int in0, int in1, const QString &in2);
    void CursorMove(int in0, int in1, const QString &in2);
    void CursorOut(int in0, int in1, const QString &in2);
    void KeyPress(const QString &in0, int in1, int in2, const QString &in3);
    void KeyRelease(const QString &in0, int in1, int in2, const QString &in3);
};

namespace com {
  namespace deepin {
    namespace api {
      typedef ::ComDeepinApiXEventMonitorInterface XEventMonitor;
    }
  }
}
#endif
