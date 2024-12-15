// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSTEMD1_MANAGER_INTERFACE_H
#define SYSTEMD1_MANAGER_INTERFACE_H

#include "common/error_context.h"
#include "dbus_common.h"
#include "unit_file_info.h"
#include "unit_info.h"

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus>

using namespace dbus::common;

/**
 * @brief Proxy class for interface org.freedesktop.systemd1.Manager
 */
class Systemd1ManagerInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    /**
     * @brief staticInterfaceName Get interface static name
     * @return Interface static name
     */
    static inline const char *staticInterfaceName() { return "org.freedesktop.systemd1.Manager"; }

public:
    Systemd1ManagerInterface(const QString &service, const QString &path,
                             const QDBusConnection &connection, QObject *parent = nullptr);

    ~Systemd1ManagerInterface();

public Q_SLOTS:  // METHODS
    /**
     * @brief ListUnitFiles List installed unit files
     * @return Error context & installed unit files list pair
     */
    inline QPair<ErrorContext, UnitFileInfoList> ListUnitFiles()
    {
        // installed unit files list
        UnitFileInfoList list;
        // argument list
        QList<QVariant> args;
        // error context
        ErrorContext ec {};

        // call dbus interface method: ListUnitFiles
        QDBusMessage reply = callWithArgumentList(QDBus::BlockWithGui, "ListUnitFiles", args);

        // check dbus reply
        if (reply.type() == QDBusMessage::ErrorMessage) {
            ec.setCode(ErrorContext::kErrorTypeDBus);
            ec.setSubCode(lastError().type());
            ec.setErrorName(reply.errorName());
            ec.setErrorMessage(reply.errorMessage());
        } else {
            Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
            // check signature
            if (reply.signature() == "a(ss)") {
                qvariant_cast<QDBusArgument>(reply.arguments()[0]) >> list;
            }
        }

        return {ec, list};
    }

    /**
     * @brief ListUnits List units
     * @return Error context & units list pair
     */
    inline QPair<ErrorContext, UnitInfoList> ListUnits()
    {
        // units list
        UnitInfoList list;
        // argument list
        QList<QVariant> args;
        // error context
        ErrorContext ec;

        // call dbus interface method: ListUnits
        QDBusMessage reply = callWithArgumentList(QDBus::BlockWithGui, "ListUnits", args);
        // check dbus reply
        if (reply.type() == QDBusMessage::ErrorMessage) {
            ec.setCode(ErrorContext::kErrorTypeDBus);
            ec.setSubCode(lastError().type());
            ec.setErrorName(reply.errorName());
            ec.setErrorMessage(reply.errorMessage());
        } else {
            Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
            // check signature
            if (reply.signature() == "a(ssssssouso)") {
                qvariant_cast<QDBusArgument>(reply.arguments()[0]) >> list;
            }
        }

        return {ec, list};
    }

    /**
     * @brief GetUnit Get unit object path by path
     * @param path Object path
     * @return Error context & unit object path pair
     */
    inline QPair<ErrorContext, QDBusObjectPath> GetUnit(const QString &path)
    {
        // error context
        ErrorContext ec {};
        // object path
        QDBusObjectPath o;
        // argument list
        QList<QVariant> args;
        args << path;

        // dbus interface method call: GetUnit
        QDBusMessage reply = callWithArgumentList(QDBus::Block, "GetUnit", args);
        // check dbus reply
        if (reply.type() == QDBusMessage::ErrorMessage) {
            ec.setCode(ErrorContext::kErrorTypeDBus);
            ec.setSubCode(lastError().type());
            ec.setErrorName(reply.errorName());
            ec.setErrorMessage(reply.errorMessage());
        } else {
            Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
            // check signature
            if (reply.signature() == "o") {
                o = qvariant_cast<QDBusObjectPath>(reply.arguments()[0]);
            }
        }

        return {ec, o};
    }

    /**
     * @brief StartUnit Start unit with specified mode
     * @param name Unit name
     * @param mode Unit start mode
     * @return Error context & started unit object path pair
     */
    inline QPair<ErrorContext, QDBusObjectPath> StartUnit(const QString &name, const QString &mode)
    {
        // error context
        ErrorContext ec;
        // object path
        QDBusObjectPath o;
        // argument list
        QList<QVariant> args;
        args << name << mode;

        // dbus interface method call: StartUnit
        QDBusMessage reply = callWithArgumentList(QDBus::Block, "StartUnit", args);
        // check dbus reply
        if (reply.type() == QDBusMessage::ErrorMessage) {
            ec.setCode(ErrorContext::kErrorTypeDBus);
            ec.setSubCode(lastError().type());
            ec.setErrorName(reply.errorName());
            ec.setErrorMessage(reply.errorMessage());
        } else {
            Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
            // check signature
            if (reply.signature() == "o") {
                o = qvariant_cast<QDBusObjectPath>(reply.arguments()[0]);
            }
        }

        return {ec, o};
    }

    /**
     * @brief StopUnit Stop unit with specified mode
     * @param name Unit name
     * @param mode Unit stop mode
     * @return Error context & stopped unit object path pair
     */
    inline QPair<ErrorContext, QDBusObjectPath> StopUnit(const QString &name, const QString &mode)
    {
        // error context
        ErrorContext ec;
        // object path
        QDBusObjectPath o;
        // argument list
        QList<QVariant> args;
        args << name << mode;

        // dbus interface method call: StopUnit
        QDBusMessage reply = callWithArgumentList(QDBus::Block, "StopUnit", args);
        // check dbus reply
        if (reply.type() == QDBusMessage::ErrorMessage) {
            ec.setCode(ErrorContext::kErrorTypeDBus);
            ec.setSubCode(lastError().type());
            ec.setErrorName(reply.errorName());
            ec.setErrorMessage(reply.errorMessage());
        } else {
            Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
            // check signature
            if (reply.signature() == "o") {
                o = qvariant_cast<QDBusObjectPath>(reply.arguments()[0]);
            }
        }

        return {ec, o};
    }

    /**
     * @brief RestartUnit Restart unit with specified mode
     * @param name Unit name
     * @param mode Unit restart mode
     * @return Error context & restarted unit object path pair
     */
    inline QPair<ErrorContext, QDBusObjectPath> RestartUnit(const QString &name,
                                                            const QString &mode)
    {
        // error context
        ErrorContext ec;
        // object path
        QDBusObjectPath o;
        // argument list
        QList<QVariant> args;
        args << name << mode;

        // dbus interface method call: RestartUnit
        QDBusMessage reply = callWithArgumentList(QDBus::Block, "RestartUnit", args);
        // check dbus reply
        if (reply.type() == QDBusMessage::ErrorMessage) {
            ec.setCode(ErrorContext::kErrorTypeDBus);
            ec.setSubCode(lastError().type());
            ec.setErrorName(reply.errorName());
            ec.setErrorMessage(reply.errorMessage());
        } else {
            Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
            // check signature
            if (reply.signature() == "o") {
                o = qvariant_cast<QDBusObjectPath>(reply.arguments()[0]);
            }
        }

        return {ec, o};
    }

    /**
     * @brief GetUnitFileState Get unit file's state
     * @param unit Unit name
     * @return Error context & unit file state pair
     */
    inline QPair<ErrorContext, QString> GetUnitFileState(const QString &unit)
    {
        // error context
        ErrorContext ec;
        // unit file state
        QString state;
        // argument list
        QList<QVariant> args;
        args << unit;

        // dbus interface method call: GetUnitFileState
        QDBusMessage reply = callWithArgumentList(QDBus::Block, "GetUnitFileState", args);
        // check dbus reply
        if (reply.type() == QDBusMessage::ErrorMessage) {
            ec.setCode(ErrorContext::kErrorTypeDBus);
            ec.setSubCode(lastError().type());
            ec.setErrorName(reply.errorName());
            ec.setErrorMessage(reply.errorMessage());
        } else {
            Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
            // check signature
            if (reply.signature() == "s") {
                state = qvariant_cast<QString>(reply.arguments()[0]);
            }
        }

        return {ec, state};
    }

    // =========================================================================
    // TODO: half baked!!
    // call callWithArgumentList directly without calling CheckAuthorization
    // (from org.freedesktop.PolicyKit1.Authority Interface) to auth the user
    // will always end up failing with an "interactive authentication needed" error.
    // considering sysv type services need a different way to enable/disable
    // (e.g. run /lib/systemd/systemd-sysv-install script as root).
    // for simplicity, we use pkexec+systemctl to do the job as now.
    // =========================================================================
    /**
     * @brief EnableUnitFiles Set unit file startup mode as auto
     * @param nlist Unit file list
     * @param runtime Runtime only flag
     * @param force Force restart?
     * @return Error context & unit file enabled result
     */
    inline QPair<ErrorContext, EnableUnitFilesResult> EnableUnitFiles(const QStringList &nlist,
                                                                      bool runtime = false, bool force = true)
    {
        // error context
        ErrorContext ec {};
        // unit file enabled result
        EnableUnitFilesResult result {};
        // argument list
        QList<QVariant> args;
        args << nlist << runtime << force;

        // dbus interface method call: EnableUnitFiles
        QDBusMessage reply = callWithArgumentList(QDBus::Block, "EnableUnitFiles", args);
        // check dbus reply
        if (reply.type() == QDBusMessage::ErrorMessage) {
            ec.setCode(ErrorContext::kErrorTypeDBus);
            ec.setSubCode(lastError().type());
            ec.setErrorName(reply.errorName());
            ec.setErrorMessage(reply.errorMessage());
        } else {
            Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
            // check signature
            if (reply.signature() == "ba(sss)") {
                // unfinished...
            }
        }

        return {ec, result};
    }

    // =========================================================================
    // TODO: half baked!!
    // call callWithArgumentList directly without calling CheckAuthorization
    // (from org.freedesktop.PolicyKit1.Authority Interface) to auth the user
    // will always end up failing with an "interactive authentication needed" error.
    // considering sysv type services need a different way to enable/disable
    // (e.g. run /lib/systemd/systemd-sysv-install script as root).
    // for simplicity, we use pkexec+systemctl to do the job as now.
    // =========================================================================
    /**
     * @brief DisableUnitFiles Set unit file startup mode as manual
     * @param nlist Unit file list
     * @param runtime Runtime only flag
     * @return Error context & unit file disabled result
     */
    inline QPair<ErrorContext, DisableUnitFilesResult> DisableUnitFiles(const QStringList &nlist,
                                                                        bool runtime = false)
    {
        // error context
        ErrorContext ec;
        // unit file disabled result
        DisableUnitFilesResult result {};
        // argument list
        QList<QVariant> args;
        args << nlist << runtime;

        // dbus interface method call: DisableUnitFiles
        QDBusMessage reply = callWithArgumentList(QDBus::Block, "DisableUnitFiles", args);
        // check dbus reply
        if (reply.type() == QDBusMessage::ErrorMessage) {
            ec.setCode(ErrorContext::kErrorTypeDBus);
            ec.setSubCode(lastError().type());
            ec.setErrorName(reply.errorName());
            ec.setErrorMessage(reply.errorMessage());
        } else {
            Q_ASSERT(reply.type() == QDBusMessage::ReplyMessage);
            // check signature
            if (reply.signature() == "a(sss)") {
                // unfinished...
            }
        }

        return {ec, result};
    }

Q_SIGNALS:  // SIGNALS
};

namespace org {
namespace freedesktop {
namespace systemd1 {
typedef ::Systemd1ManagerInterface Manager;
}
}  // namespace freedesktop
}  // namespace org
#endif
