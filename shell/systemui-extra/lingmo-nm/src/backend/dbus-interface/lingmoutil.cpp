/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "lingmoutil.h"
#include <QTextCodec>

#include <dbus/dbus.h>
#include <glib-2.0/glib.h>
#include <dbus/dbus-glib.h>
#include <gio/gio.h>

#define  LOG_FLAG  "[lingmo-util]"

QString getConnectTypeByDbus(QString &connectPath)
{
    QString connectType = "";

    if (connectPath.isEmpty()) {
        qWarning()<< LOG_FLAG << "connect path is empty, so can not get connect type";
        return connectType;
    }

    QDBusInterface dbusInterface("org.freedesktop.NetworkManager",
                              connectPath,
                              "org.freedesktop.NetworkManager.Settings.Connection",
                              QDBusConnection::systemBus());

    QDBusMessage result = dbusInterface.call("GetSettings");
    const QDBusArgument &dbusArg1st = result.arguments().at( 0 ).value<QDBusArgument>();
    QMap<QString, QMap<QString, QVariant>> map;
    dbusArg1st >> map;

    if (map.isEmpty()) {
        qWarning() << LOG_FLAG <<"get connection settings failed.";
        return connectType;
    }

    QMap<QString,QVariant> connectMap = map.value(KEY_CONNECTION);
    if (connectMap.isEmpty()) {
        qWarning() << LOG_FLAG <<"threre is not connection settings";
        return connectType;
    }

    connectType = connectMap.value(KEY_CONNECT_TYPE).toString();

    return connectType;
}

QString getSsidFromByteArray(QByteArray &rawSsid)
{
    QString wifiSsid = "";

    if (rawSsid.isEmpty()) {
        qWarning() << LOG_FLAG << "wifi raw ssid is empty";
        return wifiSsid;
    }

    /*
    * 由于区分GB2312和UTF-8的方法比较困难，加之会存在中英文混合的情况，所以暂时
    * 不区分，统一经过gb2312转换，经过测试没有影响。
    */
//    QTextCodec *p_textGBK = QTextCodec::codecForName("GB2312");
//    wifiSsid = p_textGBK->toUnicode(rawSsid);

    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    codec->toUnicode( rawSsid.constData(), rawSsid.size(), &state);
    if (state.invalidChars > 0)
    {
        wifiSsid = QTextCodec::codecForName("GBK")->toUnicode(rawSsid);
    }
    else
    {
        wifiSsid = rawSsid;
    }

    return wifiSsid;
}

void setWiredEnabledByGDbus(bool enabled)
{
    GDBusProxy *props_proxy;
    GVariant *ret = NULL;
    GError *error = NULL;

    /* Create a D-Bus object proxy for the active connection object's properties */
    props_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                 G_DBUS_PROXY_FLAGS_NONE,
                                                 NULL,
                                                 "org.freedesktop.NetworkManager",
                                                 "/org/freedesktop/NetworkManager",
                                                 "org.freedesktop.DBus.Properties",
                                                 NULL, NULL);
    g_assert (props_proxy);

    /* Get the object path of the Connection details */
    ret = g_dbus_proxy_call_sync (props_proxy,
                                  "Set",
                                  g_variant_new ("(ssv)",
                                                 "org.freedesktop.NetworkManager",
                                                 "WiredEnabled",
                                                 g_variant_new_boolean(enabled)),
                                  G_DBUS_CALL_FLAGS_NONE, -1,
                                  NULL, &error);
    if (!ret) {
        g_dbus_error_strip_remote_error (error);
        qDebug() << "failed to setWiredEnabledByGDbus";
        g_error_free (error);
    }

out:
    if (ret)
        g_variant_unref (ret);
    g_object_unref (props_proxy);
}

bool getWiredEnabledByGDbus()
{
    GDBusProxy *props_proxy;
    GVariant *ret = NULL, *path_value = NULL;
    GError *error = NULL;
    gboolean bRet = false;

    /* Create a D-Bus object proxy for the active connection object's properties */
    props_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                 G_DBUS_PROXY_FLAGS_NONE,
                                                 NULL,
                                                 "org.freedesktop.NetworkManager",
                                                 "/org/freedesktop/NetworkManager",
                                                 "org.freedesktop.DBus.Properties",
                                                 NULL, NULL);
    g_assert (props_proxy);

    /* Get the object path of the Connection details */
    ret = g_dbus_proxy_call_sync (props_proxy,
                                  "Get",
                                  g_variant_new ("(ss)",
                                                 "org.freedesktop.NetworkManager",
                                                 "WiredEnabled"),
                                  G_DBUS_CALL_FLAGS_NONE, -1,
                                  NULL, &error);
    if (!ret) {
        g_dbus_error_strip_remote_error (error);
        qDebug() << "failed to setWiredEnabledByGDbus";
        g_error_free (error);
    }

    g_variant_get (ret, "(v)", &path_value);
    if (!g_variant_is_of_type (path_value, G_VARIANT_TYPE_BOOLEAN)) {
        g_warning ("Unexpected type returned getting Connection property: %s",
                   g_variant_get_type_string (path_value));
        goto out;
    }

    bRet = g_variant_get_boolean (path_value);

out:
    if (path_value)
        g_variant_unref (path_value);
    if (ret)
        g_variant_unref (ret);
    g_object_unref (props_proxy);

    return bRet;
}


void setDeviceManagedByGDbus(QString dbusPath, bool managed)
{
    GDBusProxy *props_proxy;
    GVariant *ret = NULL;
    GError *error = NULL;

    /* Create a D-Bus object proxy for the active connection object's properties */
    props_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                 G_DBUS_PROXY_FLAGS_NONE,
                                                 NULL,
                                                 "org.freedesktop.NetworkManager",
                                                 dbusPath.toStdString().c_str(),
                                                 "org.freedesktop.DBus.Properties",
                                                 NULL, NULL);
    g_assert (props_proxy);

    /* Get the object path of the Connection details */
    ret = g_dbus_proxy_call_sync (props_proxy,
                                  "Set",
                                  g_variant_new ("(ssv)",
                                                 "org.freedesktop.NetworkManager.Device",
                                                 "Managed",
                                                 g_variant_new_boolean(managed)),
                                  G_DBUS_CALL_FLAGS_NONE, -1,
                                  NULL, &error);
    if (!ret) {
        g_dbus_error_strip_remote_error (error);
        qDebug() << "failed to setWiredEnabledByGDbus";
        g_error_free (error);
    }

out:
    if (ret)
        g_variant_unref (ret);
    g_object_unref (props_proxy);
}

QString getConnectivityCheckSpareUriByGDbus()
{
    GDBusProxy *props_proxy;
    GVariant *ret = NULL, *path_value = NULL;
    GError *error = NULL;
    QString str;

    /* Create a D-Bus object proxy for the active connection object's properties */
    props_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                 G_DBUS_PROXY_FLAGS_NONE,
                                                 NULL,
                                                 "org.freedesktop.NetworkManager",
                                                 "/org/freedesktop/NetworkManager",
                                                 "org.freedesktop.DBus.Properties",
                                                 NULL, NULL);
    g_assert (props_proxy);

    /* Get the object path of the Connection details */
    ret = g_dbus_proxy_call_sync (props_proxy,
                                  "Get",
                                  g_variant_new ("(ss)",
                                                 "org.freedesktop.NetworkManager",
                                                 "ConnectivityCheckSpareUri"),
                                  G_DBUS_CALL_FLAGS_NONE, -1,
                                  NULL, &error);
    if (!ret) {
        g_dbus_error_strip_remote_error (error);
        qDebug() << "failed to getConnectivityCheckSpareUri";
        g_error_free (error);
    }

    g_variant_get (ret, "(v)", &path_value);
//    if (!g_variant_is_of_type (path_value, G_VARIANT_TYPE_VARIANT)) {
//        g_warning ("Unexpected type returned getting Connection property: %s",
//                   g_variant_get_type_string (path_value));
//        goto out;
//    }

    str = QString(g_variant_get_string(path_value, NULL));

out:
    if (path_value)
        g_variant_unref (path_value);
    if (ret)
        g_variant_unref (ret);
    g_object_unref (props_proxy);

    return str;
}

void setConnectivityCheckSpareUriByGDbus(QString uri)
{
    GDBusProxy *props_proxy;
    GVariant *ret = NULL;
    GError *error = NULL;

    /* Create a D-Bus object proxy for the active connection object's properties */
    props_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
                                                 G_DBUS_PROXY_FLAGS_NONE,
                                                 NULL,
                                                 "org.freedesktop.NetworkManager",
                                                 "/org/freedesktop/NetworkManager",
                                                 "org.freedesktop.DBus.Properties",
                                                 NULL, NULL);
    g_assert (props_proxy);

    /* Get the object path of the Connection details */
    ret = g_dbus_proxy_call_sync (props_proxy,
                                  "Set",
                                  g_variant_new ("(ssv)",
                                                 "org.freedesktop.NetworkManager",
                                                 "ConnectivityCheckSpareUri",
                                                 g_variant_new_string(uri.toStdString().c_str())),
                                  G_DBUS_CALL_FLAGS_NONE, -1,
                                  NULL, &error);
    if (!ret) {
        g_dbus_error_strip_remote_error (error);
        qDebug() << "failed to setConnectivityCheckSpareUri";
        g_error_free (error);
    }

out:
    if (ret)
        g_variant_unref (ret);
    g_object_unref (props_proxy);
}
