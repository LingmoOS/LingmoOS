// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLOR_PICKER_INTERFACE_H
#define COLOR_PICKER_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.deepin.Picker
 */
class ColorPickerInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    {
        return "com.deepin.Picker";
    }

public:
    ColorPickerInterface(const QString &service, const QString &path,
                         const QDBusConnection &connection, QObject *parent = nullptr);

    ~ColorPickerInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> StartPick(const QString &appid)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(appid);
        return asyncCallWithArgumentList(QStringLiteral("StartPick"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void colorPicked(const QString &appid, const QString &color);
};

namespace com {
namespace deepin {
typedef ::ColorPickerInterface Picker;
}
}
#endif
