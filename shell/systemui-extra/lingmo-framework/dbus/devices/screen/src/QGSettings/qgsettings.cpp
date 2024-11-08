/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <glib.h>
#include <gio/gio.h>

#include <QDebug>
#include <QString>

#include "qgsettings.h"
#include "qconftype.h"

struct QGSettingsPrivate
{
    QByteArray          path;
    GSettingsSchema     *schema;
    QByteArray          schemaId;
    GSettings           *settings;
    gulong              signalHandlerId;

    static void settingChanged(GSettings *settings, const gchar *key, gpointer userData);
};

void QGSettingsPrivate::settingChanged(GSettings *, const gchar *key, gpointer userData)
{
    QGSettings *self = (QGSettings *)userData;

    /**
     * 这里不属于 QObject的子类，只能通过此方法强制调用 QObject 子类的方法或信号
     *
     * Qt::QueuedConnection         发送一个QEvent，并在应用程序进入主事件循环后立即调用该成员。
     * Qt::DirectConnection         立即调用
     * Qt::BlockingQueuedConnection 则将以与Qt::QueuedConnection相同的方式调用该方法，除了当前线程将阻塞直到事件被传递。使用此连接类型在同一线程中的对象之间进行通信将导致死锁
     * Qt::AutoConnection           则如果obj与调用者位于同一个线程中，则会同步调用该成员; 否则它将异步调用该成员
     *
     */
    QMetaObject::invokeMethod(self, "changed", Qt::AutoConnection, Q_ARG(QString, key));
}

static bool is_item_in_schema (const gchar* const* items, const QByteArray & item)
{
    while (*items) {
       if (g_strcmp0 (*items++, item) == 0) {
           return true;
       }
    }
    return false;
}

QGSettings::QGSettings(const QByteArray &schemaId, const QByteArray &path, QObject *parent) : QObject(parent)
{
    mPriv = new QGSettingsPrivate;
    mPriv->schemaId = schemaId;
    mPriv->path = path;

    if (false == is_item_in_schema (g_settings_list_schemas(), schemaId)){
        mPriv->settings = nullptr;
        return;
    }

    if (mPriv->path.isEmpty()) {
        mPriv->settings = g_settings_new(mPriv->schemaId.constData());
    } else {
        mPriv->settings = g_settings_new_with_path(mPriv->schemaId.constData(), mPriv->path.constData());
    }
    g_object_get(mPriv->settings, "settings-schema", &mPriv->schema, NULL);
    mPriv->signalHandlerId = g_signal_connect(mPriv->settings, "changed", G_CALLBACK(QGSettingsPrivate::settingChanged), this);
}

QGSettings::~QGSettings()
{
    if (mPriv->schema) {
        g_settings_sync ();
        g_signal_handler_disconnect(mPriv->settings, mPriv->signalHandlerId);
        g_object_unref (mPriv->settings);
        g_settings_schema_unref (mPriv->schema);
    }
    delete mPriv;
}

QVariant QGSettings::get(const QString &key) const
{
    gchar *gkey = unqtify_name(key);
    if (mPriv->settings == nullptr) {
        return -1;
    }

    if (!keys().contains(gkey)) {
//        USD_LOG(LOG_ERR, "can't find int key:%s in %s", gkey, mPriv->schemaId.data());
        return QVariant(0);
    }

    GVariant *value = g_settings_get_value(mPriv->settings, gkey);
    if( NULL == value) {
//        USD_LOG(LOG_DEBUG,"g_settings_get_value is faild");
        return QVariant(0);
    }
    QVariant qvalue = qconf_types_to_qvariant(value);
    g_variant_unref(value);
    g_free(gkey);

    return qvalue;
}

QString QGSettings::set(const QString &key, const QVariant &value)
{
    QString errorMsg = "";
    if (mPriv->settings == nullptr) {
        return "settings are null";
    }

    gchar *gkey = unqtify_name(key);
    if (keys().contains(QString(gkey))) {
        if (!trySet(key, value)) {
          errorMsg = QString("unable to set key '%1' to value '%2'\n").arg(key).arg(value.toString());
//          USD_LOG(LOG_ERR,"unable to set key '%s' to value '%s'", key.toUtf8().constData(), value.toString().toUtf8().constData());
        }
    } else {
//         USD_LOG(LOG_ERR, "can't find int key:%s in %s", gkey, mPriv->schemaId.data());
         errorMsg = QString("can't find int key:%1 in %2\n").arg(QString(gkey)).arg(QString(mPriv->schemaId.data()));
    }

    return errorMsg;
}

bool QGSettings::trySet(const QString &key, const QVariant &value)
{
    gchar *gkey = unqtify_name(key);
    bool success = false;


    /* fetch current value to find out the exact type */
    GVariant *cur = g_settings_get_value(mPriv->settings, gkey);
    GVariant *new_value = qconf_types_collect_from_variant(g_variant_get_type (cur), value);
    if (new_value)
        success = g_settings_set_value(mPriv->settings, gkey, new_value);

    g_free(gkey);
    g_variant_unref (cur);

    return success;
}

void QGSettings::setEnum(const QString& key,int value)
{
    if (mPriv->settings == nullptr) {
        return ;
    }
    g_settings_set_enum (mPriv->settings,key.toLatin1().data(),value);
}

int QGSettings::getEnum(const QString& key)
{
    int enumNum;
    if (mPriv->settings == nullptr) {
        return -1;
    }
    enumNum = g_settings_get_enum (mPriv->settings,key.toLatin1().data());
    return enumNum;
}

char **QGSettings::getStrv(const QString& key)
{
    return g_settings_get_strv(mPriv->settings,key.toLatin1().data());
}

void QGSettings::delay()
{
    g_settings_delay(mPriv->settings);
}

void QGSettings::apply()
{
    g_settings_apply(mPriv->settings);
}

QStringList QGSettings::keys() const
{
    QStringList list;
    //gchar **keys = g_settings_list_keys(mPriv->settings);
    gchar **keys = g_settings_schema_list_keys(mPriv->schema);
    for (int i = 0; keys[i]; i++)
        list.append(keys[i]);
//    list.append(qtify_name(keys[i]));

    g_strfreev(keys);

    return list;
}

QVariantList QGSettings::choices(const QString &qkey) const
{
    gchar *key = unqtify_name (qkey);
    GSettingsSchemaKey *schema_key = g_settings_schema_get_key (mPriv->schema, key);
    GVariant *range = g_settings_schema_key_get_range(schema_key);
    g_settings_schema_key_unref (schema_key);
    g_free(key);

    if (range == NULL)
        return QVariantList();

    const gchar *type;
    GVariant *value;
    g_variant_get(range, "(&sv)", &type, &value);

    QVariantList choices;
    if (g_str_equal(type, "enum")) {
        GVariantIter iter;
        GVariant *child;

        g_variant_iter_init (&iter, value);
        while ((child = g_variant_iter_next_value(&iter))) {
            choices.append(qconf_types_to_qvariant(child));
            g_variant_unref(child);
        }
    }

    g_variant_unref (value);
    g_variant_unref (range);

    return choices;
}

void QGSettings::reset(const QString &qkey)
{
    gchar *key = unqtify_name(qkey);
    g_settings_reset(mPriv->settings, key);
    g_free(key);
}

bool QGSettings::isSchemaInstalled(const QByteArray &schemaId)
{
    GSettingsSchemaSource *source = g_settings_schema_source_get_default ();
    GSettingsSchema *schema = g_settings_schema_source_lookup (source, schemaId.constData(), TRUE);
    if (schema) {
        g_settings_schema_unref (schema);
        return true;
    } else {
        return false;
    }
}

QString QGSettings::getSummary(const QString &key) const
{

    gchar* gKey = unqtify_name(key);
    if (!keys().contains(gKey)) {
//        USD_LOG(LOG_ERR, "can't find key:%s in %s", gKey, mPriv->schemaId.data());
        g_free(gKey);
        return QString();
    }

    GSettingsSchemaKey *schemaKey = nullptr;
    schemaKey = g_settings_schema_get_key(mPriv->schema, gKey);

    if (!schemaKey) {
//        USD_LOG(LOG_ERR, "can't get schema key:%s in %s", gKey, mPriv->schemaId.data());
        g_free(gKey);
        return QString();
    }
    g_free(gKey);

    const gchar* gSummary = g_settings_schema_key_get_summary(schemaKey);
    g_settings_schema_key_unref(schemaKey);

    return QString(gSummary);
}

QString QGSettings::getKeyType(const QString &key) const
{
    QString ret = "";
    gchar* gKey = unqtify_name(key);
    if (!keys().contains(gKey)) {
//        USD_LOG(LOG_ERR, "can't find key:%s in %s", gKey, mPriv->schemaId.data());
        g_free(gKey);
        return QString();
    }
    GVariant *cur = g_settings_get_value(mPriv->settings, gKey);
    ret = g_variant_get_type_string(cur);
    g_free(gKey);
    return ret;
}
