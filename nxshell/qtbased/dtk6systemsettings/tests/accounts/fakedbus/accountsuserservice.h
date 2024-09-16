// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "daccountstypes_p.h"

#include <qdbusmetatype.h>
#include <QStringList>
#include <QDBusArgument>

using questionsType = QMap<qint32, QByteArray>;
using anwserType = QMap<qint32, QString>;
using anwserIndex = QList<qint32>;


class FakeAccountsUserService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.FakeAccounts.User")
    Q_CLASSINFO("D-Bus Introspection",
                ""
                "  <interface name=\"com.deepin.daemon.FakeAccounts.User\">\n"
                "    <method name=\"AddGroup\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"group\"/>\n"
                "    </method>\n"
                "    <method name=\"DeleteGroup\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"group\"/>\n"
                "    </method>\n"
                "    <method name=\"DeleteIconFile\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"icon\"/>\n"
                "    </method>\n"
                "    <method name=\"EnableNoPasswdLogin\">\n"
                "      <arg direction=\"in\" type=\"b\" name=\"enabled\"/>\n"
                "    </method>\n"
                "    <method name=\"GetReminderInfo\">\n"
                "      <arg direction=\"out\" type=\"(s(iiiiii)(sssss)(sssss)i)\" name=\"info\"/>\n"
                "      <annotation value=\"Dtk::Accounts::ReminderInfo_p\" "
                "name=\"org.qtproject.QtDBus.QtTypeName.info\"/>\n"
                "    </method>\n"
                "    <method name=\"SetAutomaticLogin\">\n"
                "      <arg direction=\"in\" type=\"b\" name=\"enabled\"/>\n"
                "    </method>\n"
                "    <method name=\"SetRealName\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"name\"/>\n"
                "    </method>\n"
                "    <method name=\"SetGroups\">\n"
                "      <arg direction=\"in\" type=\"as\" name=\"groups\"/>\n"
                "    </method>\n"
                "    <method name=\"SetHistoryLayout\">\n"
                "      <arg direction=\"in\" type=\"as\" name=\"list\"/>\n"
                "    </method>\n"
                "    <method name=\"SetHomeDirectory\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"home\"/>\n"
                "    </method>\n"
                "    <method name=\"SetIconFile\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"iconURI\"/>\n"
                "    </method>\n"
                "    <method name=\"SetLayout\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"layout\"/>\n"
                "    </method>\n"
                "    <method name=\"SetLocale\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"locale\"/>\n"
                "    </method>\n"
                "    <method name=\"SetLocked\">\n"
                "      <arg direction=\"in\" type=\"b\" name=\"locked\"/>\n"
                "    </method>\n"
                "    <method name=\"SetMaxPasswordAge\">\n"
                "      <arg direction=\"in\" type=\"i\" name=\"nDays\"/>\n"
                "    </method>\n"
                "    <method name=\"SetPassword\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"password\"/>\n"
                "    </method>\n"
                "    <method name=\"SetPasswordHint\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"hint\"/>\n"
                "    </method>\n"
                "    <method name=\"SetShell\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"shell\"/>\n"
                "    </method>\n"
                "    <property access=\"read\" type=\"s\" name=\"Locale\"/>\n"
                "    <property access=\"read\" type=\"b\" name=\"Locked\"/>\n"
                "    <property access=\"read\" type=\"t\" name=\"LoginTime\"/>\n"
                "    <property access=\"read\" type=\"s\" name=\"UserName\"/>\n"
                "    <property access=\"read\" type=\"s\" name=\"RealName\"/>\n"
                "    <property access=\"read\" type=\"s\" name=\"Shell\"/>\n"
                "    <property access=\"read\" type=\"s\" name=\"PasswordHint\"/>\n"
                "    <property access=\"read\" type=\"b\" name=\"AutomaticLogin\"/>\n"
                "    <property access=\"read\" type=\"as\" name=\"HistoryLayout\"/>\n"
                "    <property access=\"read\" type=\"s\" name=\"Layout\"/>\n"
                "    <property access=\"read\" type=\"b\" name=\"NoPasswdLogin\"/>\n"
                "    <property access=\"read\" type=\"i\" name=\"AccountType\"/>\n"
                "    <property access=\"read\" type=\"s\" name=\"UUID\"/>\n"
                "    <property access=\"read\" type=\"s\" name=\"HomeDirectory\"/>\n"
                "    <property access=\"read\" type=\"s\" name=\"IconFile\"/>\n"
                "    <property access=\"read\" type=\"i\" name=\"PasswordMode\"/>\n"
                "    <property access=\"read\" type=\"i\" name=\"PasswordLastChange\"/>\n"
                "    <property access=\"read\" type=\"i\" name=\"MaxPasswordAge\"/>\n"
                "    <property access=\"read\" type=\"t\" name=\"CreatedTime\"/>\n"
                "  </interface>\n"
                "")
public:
    explicit FakeAccountsUserService(QObject *parent = nullptr);
    virtual ~FakeAccountsUserService();

    Q_PROPERTY(qint32 AccountType READ AccountType)
    Q_PROPERTY(bool AutomaticLogin READ AutomaticLogin WRITE SetAutomaticLogin)
    Q_PROPERTY(QString RealName READ RealName WRITE SetRealName)
    Q_PROPERTY(QStringList HistoryLayout READ HistoryLayout WRITE SetHistoryLayout)
    Q_PROPERTY(QString HomeDirectory READ HomeDirectory WRITE SetHomeDirectory)
    Q_PROPERTY(QString IconFile READ IconFile WRITE SetIconFile)
    Q_PROPERTY(QString Layout READ Layout WRITE SetLayout)
    Q_PROPERTY(QString Locale READ Locale WRITE SetLocale)
    Q_PROPERTY(bool Locked READ Locked WRITE SetLocked)
    Q_PROPERTY(QString PasswordHint READ PasswordHint WRITE SetPasswordHint)
    Q_PROPERTY(QString Shell READ Shell WRITE SetShell)
    Q_PROPERTY(QString UUID READ UUID)
    Q_PROPERTY(QString UserName READ UserName)
    Q_PROPERTY(bool NoPasswdLogin READ NoPasswdLogin WRITE EnableNoPasswdLogin)
    Q_PROPERTY(qint64 LoginTime READ LoginTime)
    Q_PROPERTY(qint32 PasswordMode READ PasswordMode)
    Q_PROPERTY(qint32 MaxPasswordAge READ MaxPasswordAge)
    Q_PROPERTY(quint64 CreatedTime READ CreatedTime)
    Q_PROPERTY(qint32 PasswordLastChange READ PasswordLastChange)

    qint32 m_accountType{ 0 };
    bool m_automaticLogin{ false };
    QString m_realName{ "test1" };
    QStringList m_historylayout{ "cn;" };
    QString m_homedirectory{ "/home/test" };
    QString m_iconFile{ "file:///var/lib/AccountsService/icons/11.png" };
    QString m_layout{ "cn;" };
    QString m_locale{ "zh_CN.UTF-8" };
    bool m_locked{ false };
    QString m_passwordhint{ "hint" };
    QString m_shell{ "/bin/bash" };
    QString m_UUID{ "3778df97-91e8-46b2-a136-bfdcdede27fe" };
    QString m_username{ "test" };
    bool m_noPasswdLogin{ false };
    qint64 m_loginTime{ 1234567 };
    qint32 m_passwordMode{ 0 };
    qint32 m_maxpasswordage{ 99999 };
    quint64 m_createdtime{ 1656048743 };
    qint32 m_passwordlastchange{ 19167 };

    inline qint32 MaxPasswordAge() const { return m_maxpasswordage; }

    inline quint64 CreatedTime() const { return m_createdtime; }

    inline qint32 PasswordLastChange() const { return m_passwordlastchange; }

    inline qint32 AccountType() const { return m_accountType; }

    inline bool AutomaticLogin() const { return m_automaticLogin; }

    Q_SCRIPTABLE inline void SetAutomaticLogin(bool enabled) { m_automaticLogin = enabled; }

    inline QString RealName() const { return m_realName; }

    Q_SCRIPTABLE inline void SetRealName(QString name) { m_realName = name; }

    inline QStringList HistoryLayout() const { return m_historylayout; }

    Q_SCRIPTABLE inline void SetHistoryLayout(QStringList list) { m_historylayout = list; }

    inline QString HomeDirectory() const { return m_homedirectory; }

    Q_SCRIPTABLE inline void SetHomeDirectory(QString dir) { m_homedirectory = dir; }

    inline QString IconFile() const { return m_iconFile; }

    Q_SCRIPTABLE inline void SetIconFile(QString file) { m_iconFile = file; }

    inline QString Layout() const { return m_layout; }

    Q_SCRIPTABLE inline void SetLayout(QString layout) { m_layout = layout; }

    inline QString Locale() const { return m_locale; }

    Q_SCRIPTABLE inline void SetLocale(QString locale) { m_locale = locale; }

    inline bool Locked() const { return m_locked; }

    Q_SCRIPTABLE inline void SetLocked(bool locked) { m_locked = locked; }

    inline QString PasswordHint() const { return m_passwordhint; }

    Q_SCRIPTABLE inline void SetPasswordHint(QString hint) { m_passwordhint = hint; }

    inline QString Shell() const { return m_shell; }

    Q_SCRIPTABLE inline void SetShell(QString shell) { m_shell = shell; }

    inline QString UUID() const { return m_UUID; }

    inline QString UserName() const { return m_username; }

    inline bool NoPasswdLogin() const { return m_noPasswdLogin; }

    Q_SCRIPTABLE inline void EnableNoPasswdLogin(bool enabled) { m_noPasswdLogin = enabled; }

    inline qint64 LoginTime() const { return m_loginTime; }

    inline qint32 PasswordMode() const { return m_passwordMode; }

    bool m_addGroupTrigger{ false };
    bool m_deleteGroupTrigger{ false };
    bool m_deleteIconFileTrigger{ false };
    bool m_getReminderInfoTrigger{ false };
    bool m_setGroupsTrigger{ false };
    bool m_setMaxPasswordAgeTrigger{ false };
    bool m_setPasswordTrigger{ false };
    bool m_passwordExpiredInfoTirgger{ false };

public slots:

    Q_SCRIPTABLE void AddGroup(QString group)
    {
        Q_UNUSED(group)
        m_addGroupTrigger = true;
    }

    Q_SCRIPTABLE void DeleteGroup(QString group)
    {
        Q_UNUSED(group)
        m_deleteGroupTrigger = true;
    }

    Q_SCRIPTABLE void DeleteIconFile(QString icon)
    {
        Q_UNUSED(icon)
        m_deleteIconFileTrigger = true;
    }

    Q_SCRIPTABLE DTK_ACCOUNTS_NAMESPACE::ReminderInfo_p GetReminderInfo()
    {
        m_getReminderInfoTrigger = true;
        return {};
    }

    Q_SCRIPTABLE void SetGroups(QStringList newgroups)
    {
        Q_UNUSED(newgroups)
        m_setGroupsTrigger = true;
    }

    Q_SCRIPTABLE void SetMaxPasswordAge(qint32 nDays)
    {
        Q_UNUSED(nDays)
        m_setMaxPasswordAgeTrigger = true;
    }

    Q_SCRIPTABLE void SetPassword(QString password)
    {
        Q_UNUSED(password)
        m_setPasswordTrigger = true;
    }

    Q_SCRIPTABLE qint32 PasswordExpiredInfo(qint64 &dayLeft)
    {
        Q_UNUSED(dayLeft)
        m_passwordExpiredInfoTirgger = true;
        return 0;
    }

private:
    void registerType()
    {
        qRegisterMetaType<questionsType>("questionsType");
        qDBusRegisterMetaType<questionsType>();

        qRegisterMetaType<anwserType>("anwserType");
        qDBusRegisterMetaType<anwserType>();

        qRegisterMetaType<anwserIndex>("anwserIndex");
        qDBusRegisterMetaType<anwserIndex>();
    }

    void registerService();
    void unregisterService();
};
