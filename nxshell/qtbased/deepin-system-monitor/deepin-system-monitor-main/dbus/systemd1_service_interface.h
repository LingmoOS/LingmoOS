// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEMD1_SERVICE_INTERFACE_H
#define SYSTEMD1_SERVICE_INTERFACE_H

#include "dbus_common.h"
#include "dbus_properties_interface.h"
#include "environment_file.h"

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus>

using namespace dbus::common;

class ErrorContext;

/**
 * @brief Proxy class for interface org.freedesktop.systemd1.Service
 */
class Systemd1ServiceInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:
    /**
     * @brief staticInterfaceName Get static interface name
     * @return Static interface name
     */
    static inline const char *staticInterfaceName() { return "org.freedesktop.systemd1.Service"; }

public:
    /**
     * @brief Systemd1ServiceInterface Constructor
     * @param service Service name
     * @param path Object path
     * @param connection DBus connection
     * @param parent Parent object
     */
    Systemd1ServiceInterface(const QString &service,
                             const QString &path,
                             const QDBusConnection &connection,
                             QObject *parent = nullptr);

    ~Systemd1ServiceInterface();

    /**
     * @brief getMainPID Get main pid of this service's process
     * @return Error context & pid pair
     */
    QPair<ErrorContext, quint32> getMainPID() const;
    /**
     * @brief getMemoryCurrent Get current memory usage of this service's process
     * @return Error context & memory usage pair
     */
    QPair<ErrorContext, quint64> getMemoryCurrent() const;
    /**
     * @brief getControlGroup Get control group of this service's process
     * @return Error context & control group pair
     */
    QPair<ErrorContext, QString> getControlGroup() const;
    /**
     * @brief getEnvironmentFiles Get environment files of this service
     * @return Error context & environment file list pair
     */
    QPair<ErrorContext, EnvironmentFileList> getEnvironmentFiles() const;

public Q_SLOTS:  // METHODS

Q_SIGNALS:  // SIGNALS

private:
    // dbus property interface instance
    DBusPropertiesInterface *m_DBusPropIface;
};

namespace org {
namespace freedesktop {
namespace systemd1 {
typedef ::Systemd1ServiceInterface Service;
}
}  // namespace freedesktop
}  // namespace org
#endif
