// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSECRETAGENTADAPTOR_H
#define DSECRETAGENTADAPTOR_H

#include "dsecretagent.h"
#include <QtDBus>
#include <QObject>
#include <QStringList>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DSecretAgentAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.NetworkManager.SecretAgent")
    Q_CLASSINFO("D-Bus Introspection",
                ""
                "  <interface name=\"org.freedesktop.NetworkManager.SecretAgent\">\n"
                "    <method name=\"GetSecrets\">\n"
                "      <annotation value=\"impl_secret_agent_get_secrets\" name=\"org.freedesktop.DBus.GLib.CSymbol\"/>\n"
                "      <annotation value=\"\" name=\"org.freedesktop.DBus.GLib.Async\"/>\n"
                "      <annotation value=\"SettingDesc\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
                "      <arg direction=\"in\" type=\"a{sa{sv}}\" name=\"connection\"/>\n"
                "      <arg direction=\"in\" type=\"o\" name=\"connection_path\"/>\n"
                "      <arg direction=\"in\" type=\"s\" name=\"setting_name\"/>\n"
                "      <arg direction=\"in\" type=\"as\" name=\"hints\"/>\n"
                "      <arg direction=\"in\" type=\"u\" name=\"flags\"/>\n"
                "      <annotation value=\"SettingDesc\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
                "      <arg direction=\"out\" type=\"a{sa{sv}}\" name=\"secrets\"/>\n"
                "    </method>\n"
                "    <method name=\"CancelGetSecrets\">\n"
                "      <annotation value=\"impl_secret_agent_cancel_get_secrets\" name=\"org.freedesktop.DBus.GLib.CSymbol\"/>\n"
                "      <annotation value=\"\" name=\"org.freedesktop.DBus.GLib.Async\"/>\n"
                "      <arg direction=\"in\" type=\"o\" name=\"connection_path\"/>\n"
                "      <arg direction=\"in\" type=\"s\" name=\"setting_name\"/>\n"
                "    </method>\n"
                "    <method name=\"SaveSecrets\">\n"
                "      <annotation value=\"impl_secret_agent_save_secrets\" name=\"org.freedesktop.DBus.GLib.CSymbol\"/>\n"
                "      <annotation value=\"\" name=\"org.freedesktop.DBus.GLib.Async\"/>\n"
                "      <annotation value=\"SettingDesc\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
                "      <arg direction=\"in\" type=\"a{sa{sv}}\" name=\"connection\"/>\n"
                "      <arg direction=\"in\" type=\"o\" name=\"connection_path\"/>\n"
                "    </method>\n"
                "    <method name=\"DeleteSecrets\">\n"
                "      <annotation value=\"impl_secret_agent_delete_secrets\" name=\"org.freedesktop.DBus.GLib.CSymbol\"/>\n"
                "      <annotation value=\"\" name=\"org.freedesktop.DBus.GLib.Async\"/>\n"
                "      <annotation value=\"SettingDesc\" name=\"org.qtproject.QtDBus.QtTypeName.In0\"/>\n"
                "      <arg direction=\"in\" type=\"a{sa{sv}}\" name=\"connection\"/>\n"
                "      <arg direction=\"in\" type=\"o\" name=\"connection_path\"/>\n"
                "    </method>\n"
                "  </interface>\n"
                "")
public:
    DSecretAgentAdaptor(DSecretAgent *parent);
    ~DSecretAgentAdaptor() override;

    DSecretAgent *parent() const { return static_cast<DSecretAgent *>(QObject::parent()); }

public Q_SLOTS:
    void CancelGetSecrets(const QDBusObjectPath &connection_path, const QString &setting_name);
    void DeleteSecrets(SettingDesc connection, const QDBusObjectPath &connection_path);
    SettingDesc GetSecrets(SettingDesc connection,
                           const QDBusObjectPath &connection_path,
                           const QString &setting_name,
                           const QStringList &hints,
                           uint flags);
    void SaveSecrets(SettingDesc connection, const QDBusObjectPath &connection_path);
};

DNETWORKMANAGER_END_NAMESPACE

#endif
