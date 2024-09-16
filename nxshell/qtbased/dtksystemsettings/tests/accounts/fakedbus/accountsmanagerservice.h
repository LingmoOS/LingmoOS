// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDBusObjectPath>
#include <QStringList>

class FakeAccountsManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.FakeAccounts")
    Q_CLASSINFO("D-Bus Introspection",
                ""
                "  <interface name=\"com.deepin.daemon.FakeAccounts\">\n"
                "    <method name=\"ListCachedUsers\">\n"
                "      <arg direction=\"out\" type=\"ao\" name=\"users\"/>\n"
                "    </method>\n"
                "    <method name=\"CreateUser\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"name\"/>\n"
                "      <arg direction=\"in\" type=\"s\" name=\"fullname\"/>\n"
                "      <arg direction=\"in\" type=\"i\" name=\"type\"/>\n"
                "      <arg direction=\"out\" type=\"o\" name=\"user\"/>\n"
                "    </method>\n"
                "    <method name=\"DeleteUser\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"name\"/>\n"
                "      <arg direction=\"in\" type=\"b\" name=\"rmfiles\"/>\n"
                "    </method>\n"
                "    <method name=\"FindUserById\">\n"
                "      <arg direction=\"in\" type=\"x\" name=\"id\"/>\n"
                "      <arg direction=\"out\" type=\"o\" name=\"user\"/>\n"
                "    </method>\n"
                "    <method name=\"FindUserByName\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"name\"/>\n"
                "      <arg direction=\"out\" type=\"o\" name=\"user\"/>\n"
                "    </method>\n"
                "    <method name=\"GetPresetGroups\">\n"
                "      <arg direction=\"in\" type=\"i\" name=\"type\"/>\n"
                "      <arg direction=\"out\" type=\"as\" name=\"groups\"/>\n"
                "    </method>\n"
                "    <method name=\"IsPasswordValid\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"password\"/>\n"
                "      <arg direction=\"out\" type=\"b\" name=\"valid\"/>\n"
                "      <arg direction=\"out\" type=\"s\" name=\"msg\"/>\n"
                "      <arg direction=\"out\" type=\"i\" name=\"code\"/>\n"
                "    </method>\n"
                "    <method name=\"IsUsernameValid\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"username\"/>\n"
                "      <arg direction=\"out\" type=\"b\" name=\"valid\"/>\n"
                "      <arg direction=\"out\" type=\"s\" name=\"msg\"/>\n"
                "      <arg direction=\"out\" type=\"i\" name=\"code\"/>\n"
                "    </method>\n"
                "  </interface>\n"
                "")

public:
    explicit FakeAccountsManagerService(QObject *parent = nullptr);
    virtual ~FakeAccountsManagerService();

    bool m_createUserTrigger{ false };
    bool m_deleteUserTrigger{ false };
    bool m_userListTrigger{ false };
    bool m_findUserByIdTrigger{ false };
    bool m_findUserByNameTrigger{ false };
    bool m_presetGroupsTrigger{ false };
    bool m_isPasswordValidTrigger{ false };
    bool m_isUsernameValidTrigger{ false };

public slots:

    Q_SCRIPTABLE QList<QDBusObjectPath> ListCachedUsers()
    {
        m_userListTrigger = true;
        return {};
    }

    Q_SCRIPTABLE QDBusObjectPath CreateUser(QString name, QString fullName, qint32 type)
    {
        Q_UNUSED(name)
        Q_UNUSED(fullName)
        Q_UNUSED(type)
        m_createUserTrigger = true;
        return QDBusObjectPath{ "/com/deepin/daemon/FakeAccounts/User1000" };
    }

    Q_SCRIPTABLE void DeleteUser(QString name, bool rmFiles)
    {
        Q_UNUSED(name)
        Q_UNUSED(rmFiles)
        m_deleteUserTrigger = true;
    }

    Q_SCRIPTABLE QDBusObjectPath FindUserById(qint64 uid)
    {
        Q_UNUSED(uid)
        m_findUserByIdTrigger = true;
        return QDBusObjectPath{ "/com/deepin/daemon/FakeAccounts/User1000" };
    }

    Q_SCRIPTABLE QDBusObjectPath FindUserByName(QString name)
    {
        Q_UNUSED(name)
        m_findUserByNameTrigger = true;
        return QDBusObjectPath{ "/com/deepin/daemon/FakeAccounts/User1000" };
    }

    Q_SCRIPTABLE QStringList GetPresetGroups(qint32 type)
    {
        Q_UNUSED(type)
        m_presetGroupsTrigger = true;
        return { "preset", "printer" };
    }

    Q_SCRIPTABLE bool IsPasswordValid(const QString &password, QString &msg, qint32 &code)
    {
        Q_UNUSED(password)
        Q_UNUSED(msg)
        Q_UNUSED(code)
        m_isPasswordValidTrigger = true;
        return true;
    }

    Q_SCRIPTABLE bool IsUsernameValid(const QString &username, QString &msg, qint32 &code)
    {
        Q_UNUSED(username)
        Q_UNUSED(msg)
        Q_UNUSED(code)
        m_isUsernameValidTrigger = true;
        return true;
    }

private:
    void registerService();
    void unregisterService();
};
