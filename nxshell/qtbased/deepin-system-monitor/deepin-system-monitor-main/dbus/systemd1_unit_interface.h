// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEMD1_UNIT_INTERFACE_H
#define SYSTEMD1_UNIT_INTERFACE_H

#include "dbus_properties_interface.h"

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus>

#define SYSTEMD_UNIT_PATH "/org/freedesktop/systemd1/unit/"

/**
 * @brief Proxy class for interface org.freedesktop.systemd1.Unit
 */
class Systemd1UnitInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    /**
     * @brief staticInterfaceName Get static interface name
     * @return Static interface name
     */
    static inline const char *staticInterfaceName() { return "org.freedesktop.systemd1.Unit"; }

    /**
     * @brief normalizeUnitPath Normalize unit path, replace special character with escape format
     * @param id Unit name
     * @return Normalized unit's object path
     */
    static inline QDBusObjectPath normalizeUnitPath(const QString &id)
    {
        QString buf {};
        for (auto u : id) {
            if (u.isLetterOrNumber()) {
                buf.append(u);
            } else {
                // special character, needs encode
                buf.append("_").append(QString("%1").arg(uint(u.toLatin1()), 0, 16));
            }
        }
        return QDBusObjectPath {QString("%1%2").arg(SYSTEMD_UNIT_PATH).arg(buf)};
    }

public:
    /**
     * @brief Systemd1UnitInterface Constructor
     * @param service Service name
     * @param path Object path
     * @param connection DBus connection
     * @param parent Parent object
     */
    Systemd1UnitInterface(const QString &service,
                          const QString &path,
                          const QDBusConnection &connection,
                          QObject *parent = nullptr);
    ~Systemd1UnitInterface();

    /**
     * @brief getId Get Unit's name
     * @return Unit Id
     */
    QPair<ErrorContext, QString> getId() const;
    /**
     * @brief getLoadState Get unit's load state
     * @return Unit's load state
     */
    QPair<ErrorContext, QString> getLoadState() const;
    /**
     * @brief getActiveState Get unit's active state
     * @return Unit's active state
     */
    QPair<ErrorContext, QString> getActiveState() const;
    /**
     * @brief getSubState Get unit's sub state
     * @return Unit's sub state
     */
    QPair<ErrorContext, QString> getSubState() const;
    /**
     * @brief getDescription Get unit's description
     * @return Unit's description
     */
    QPair<ErrorContext, QString> getDescription() const;
    /**
     * @brief canReload Get unit's canReload property
     * @return Unit's canReload property
     */
    QPair<ErrorContext, bool> canReload() const;
    /**
     * @brief canStart Get unit's canStart property
     * @return Unit's canStart property
     */
    QPair<ErrorContext, bool> canStart() const;
    /**
     * @brief canStop Get unit's canStop property
     * @return Unit's canStop property
     */
    QPair<ErrorContext, bool> canStop() const;

Q_SIGNALS:  // SIGNALS

private:
    // dbus property interface instance
    DBusPropertiesInterface *m_DBusPropIface;
};

namespace org {
namespace freedesktop {
namespace systemd1 {
typedef ::Systemd1UnitInterface Unit;
}
}  // namespace freedesktop
}  // namespace org
#endif
