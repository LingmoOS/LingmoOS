// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUS_PROPERTIES_INTERFACE_H
#define DBUS_PROPERTIES_INTERFACE_H

#include "common/error_context.h"

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QtDBus>

/**
 * @brief Proxy class for interface org.freedesktop.DBus.Properties
 */
class DBusPropertiesInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    /**
     * @brief staticInterfaceName Get interface name
     * @return Interface name
     */
    static inline const char *staticInterfaceName() { return "org.freedesktop.DBus.Properties"; }

public:
    /**
     * @brief DBusPropertiesInterface Overloaded constructor
     * @param service Service name
     * @param path Object path
     * @param connection DBus connection
     * @param parent Parent object
     */
    DBusPropertiesInterface(const QString &service, const QString &path,
                            const QDBusConnection &connection, QObject *parent = nullptr);
    ~DBusPropertiesInterface();

public Q_SLOTS:  // METHODS
    /**
     * @brief Get Get service's property
     * @param in0 Service name
     * @param in1 Property name
     * @return Error context and DBus Message resutl pair
     */
    inline QPair<ErrorContext, QDBusMessage> Get(const QString &in0, const QString &in1)
    {
        QList<QVariant> args;
        ErrorContext ec;

        args << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        // make dbus call
        QDBusMessage msg = callWithArgumentList(QDBus::Block, QStringLiteral("Get"), args);
        if (lastError().isValid()) {
            ec.setCode(ErrorContext::kErrorTypeDBus);
            ec.setSubCode(lastError().type());
            ec.setErrorName(lastError().name());
            ec.setErrorMessage(lastError().message());
        }

        return {ec, msg};
    }

Q_SIGNALS:  // SIGNALS
    /**
     * @brief PropertiesChanged Service properties changed signal
     * @param ifname Interface name
     * @param changedProperties Changed properties map
     * @param invalidatedProperties Invalidated properties list
     */
    void PropertiesChanged(const QString &ifname, const QVariantMap &changedProperties,
                           const QStringList &invalidatedProperties);
};

namespace org {
namespace freedesktop {
namespace DBus {
typedef ::DBusPropertiesInterface Properties;
}
}  // namespace freedesktop
}  // namespace org
#endif
