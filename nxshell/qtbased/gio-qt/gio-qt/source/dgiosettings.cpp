// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dgiosettings.h"
#include "private/dgiohelper.h"

#include <QVariant>
#include <QDebug>

#include <glib.h>
#include <gio/gio.h>

static QVariant qconf_types_to_qvariant(GVariant* value)
{
    switch (g_variant_classify(value)) {
    case G_VARIANT_CLASS_BOOLEAN:
        return QVariant(static_cast<bool>(g_variant_get_boolean(value)));

    case G_VARIANT_CLASS_BYTE:
        return QVariant(static_cast<char>(g_variant_get_byte(value)));

    case G_VARIANT_CLASS_INT16:
        return QVariant(static_cast<int>(g_variant_get_int16(value)));

    case G_VARIANT_CLASS_UINT16:
        return QVariant(static_cast<unsigned int>(g_variant_get_uint16(value)));

    case G_VARIANT_CLASS_INT32:
        return QVariant(static_cast<int>(g_variant_get_int32(value)));

    case G_VARIANT_CLASS_UINT32:
        return QVariant(static_cast<unsigned int>(g_variant_get_uint32(value)));

    case G_VARIANT_CLASS_INT64:
        return QVariant(static_cast<long long>(g_variant_get_int64(value)));

    case G_VARIANT_CLASS_UINT64:
        return QVariant(static_cast<unsigned long long>(g_variant_get_uint64(value)));

    case G_VARIANT_CLASS_DOUBLE:
        return QVariant(g_variant_get_double(value));

    case G_VARIANT_CLASS_STRING:
        return QVariant(g_variant_get_string(value, nullptr));

    case G_VARIANT_CLASS_ARRAY:
        if (g_variant_is_of_type(value, G_VARIANT_TYPE_STRING_ARRAY)) {
            GVariantIter iter;
            QStringList list;
            const gchar *str;

            g_variant_iter_init (&iter, value);
            while (g_variant_iter_next(&iter, "&s", &str)) {
                list.append (str);
            }

            return QVariant(list);
        }
        else if (g_variant_is_of_type(value, G_VARIANT_TYPE_BYTESTRING)) {
            return QVariant(QByteArray(g_variant_get_bytestring(value)));
        }
        else if (g_variant_is_of_type(value, G_VARIANT_TYPE("a{ss}"))) {
            GVariantIter iter;
            QMap<QString, QVariant> map;
            const gchar* key;
            const gchar* val;

            g_variant_iter_init (&iter, value);
            while (g_variant_iter_next(&iter, "{&s&s}", &key, &val)) {
                map.insert(key, QVariant(val));
            }

            return map;
        }
    }

    qWarning() << "No matching type! " << g_variant_classify(value);
    return QVariant();
}

static GVariant *qconf_types_collect_from_variant(const GVariantType* gtype, const QVariant& v)
{
    switch (g_variant_type_peek_string(gtype)[0]) {
    case G_VARIANT_CLASS_BOOLEAN:
        return g_variant_new_boolean(v.toBool());

    case G_VARIANT_CLASS_BYTE:
        return g_variant_new_byte(v.toChar().cell());

    case G_VARIANT_CLASS_INT16:
        return g_variant_new_int16(v.toInt());

    case G_VARIANT_CLASS_UINT16:
        return g_variant_new_uint16(v.toUInt());

    case G_VARIANT_CLASS_INT32:
        return g_variant_new_int32(v.toInt());

    case G_VARIANT_CLASS_UINT32:
        return g_variant_new_uint32(v.toUInt());

    case G_VARIANT_CLASS_INT64:
        return g_variant_new_int64(v.toLongLong());

    case G_VARIANT_CLASS_UINT64:
        return g_variant_new_int64(v.toULongLong());

    case G_VARIANT_CLASS_DOUBLE:
        return g_variant_new_double(v.toDouble());

    case G_VARIANT_CLASS_STRING:
        return g_variant_new_string(v.toString().toUtf8());

    case G_VARIANT_CLASS_ARRAY:
        if (g_variant_type_equal(gtype, G_VARIANT_TYPE_STRING_ARRAY)) {
            const QStringList list = v.toStringList();
            GVariantBuilder builder;

            g_variant_builder_init(&builder, G_VARIANT_TYPE_STRING_ARRAY);

            for (const QString& string : list) {
                g_variant_builder_add(&builder, "s", string.toUtf8().constData());
            }

            return g_variant_builder_end(&builder);
        }
        else if (g_variant_type_equal(gtype, G_VARIANT_TYPE_BYTESTRING)) {
            const QByteArray& array = v.toByteArray();
            gsize size = static_cast<gsize>(array.size());
            gpointer data;

            data = g_memdup(array.data(), static_cast<guint>(size));

            return g_variant_new_from_data(G_VARIANT_TYPE_BYTESTRING,
                                           data, size, TRUE, g_free, data);
        }
        else if (g_variant_type_equal(gtype, G_VARIANT_TYPE("a{ss}"))) {
            GVariantBuilder builder;
            g_variant_builder_init(&builder, G_VARIANT_TYPE ("a{ss}"));
            QMapIterator<QString, QVariant> it(v.toMap());
            while (it.hasNext()) {
                it.next();
                const QByteArray& key = it.key().toUtf8();
                const QByteArray& val = it.value().toByteArray();
                g_variant_builder_add (&builder, "{ss}", key.constData(), val.constData());
            }
            return g_variant_builder_end (&builder);
        }
    }

    qWarning() << "No matching type! " << g_variant_type_peek_string(gtype)[0];
    return nullptr;
}

class DGioSettingsPrivate
{
public:
    DGioSettingsPrivate(DGioSettings* qq)
        : ptr(qq)
    {
    }

    QString          schemaId;
    QString          path;
    DGioSettings*    ptr;
    GSettings*       settings;
    GSettingsSchema* schema;
    gulong           signalHandlerId;

    bool strvHasString (gchar **haystack, const gchar *needle) const {
        if (needle == nullptr) return false;

        for (guint n = 0; haystack != NULL && haystack[n] != NULL; n++) {
            if (g_strcmp0 (haystack[n], needle) == 0)
                return true;
        }

        return false;
    }

    bool includeKey(const gchar* gkey) const {
        gchar **allKeys = g_settings_list_keys(settings);
        bool ret = strvHasString (allKeys, gkey);
        g_strfreev (allKeys);

        return ret;
    }

    QVariant value(GSettings* gsettings, const QString& key) const {
        gchar* gkey = DGioPrivate::converToGChar(key.toUtf8());

        if(!includeKey(gkey)) {
            g_free(gkey);
            return QVariant();
        }

        GVariant* variant = g_settings_get_value(gsettings, gkey);
        QVariant qvalue = qconf_types_to_qvariant(variant);
        g_variant_unref(variant);
        g_free(gkey);

        return qvalue;
    }

    bool trySet(const QString& key, const QVariant& value)
    {
        gchar* gkey = DGioPrivate::converToGChar(key.toUtf8());

        if(!includeKey(gkey)) {
            g_free(gkey);
            return false;
        }

        bool success = false;

        /* fetch current value to find out the exact type */
        GVariant* cur = g_settings_get_value(settings, gkey);

        GVariant* new_value = qconf_types_collect_from_variant(g_variant_get_type(cur), value);

        if (new_value) {
            success = g_settings_set_value(settings, gkey, new_value);
        }

        g_variant_unref(cur);
        g_free(gkey);

        return success;
    }

    void sync() {
        g_settings_sync();
    }

    static void onSettingChanged(GSettings* settings, const gchar* key, gpointer pointer) {
        DGioSettingsPrivate* self = static_cast<DGioSettingsPrivate*>(pointer);
        Q_EMIT self->ptr->valueChanged(key, self->value(settings, key));
    }
};

/*!
 * \class DGioSettings
 * \brief Access GSettings in a more Qt way.
 *
 * This is not a directly wrapper of Gio::Settings class, so interfaces are not matched to the
 * giomm ome.
 */

/*!
 * \brief Create a DGioSettings object for a given \a schemaId.
 *
 * In case you would like to skip path but provide a \a parent.
 *
 * Warning: not existed schemaId will cause the program crashed.
 */
DGioSettings::DGioSettings(const QString& schemaId, QObject* parent)
    : DGioSettings(schemaId, QString(), parent)
{
}

/*!
 * \brief Create a DGioSettings object for a given \a schemaId and \a path.
 *
 * Warning: not existed schemaId will cause the program crashed.
 */
DGioSettings::DGioSettings(const QString& schemaId, const QString& path, QObject* parent)
    : QObject(parent)
    , d_private(new DGioSettingsPrivate(this))
{
    d_private->schemaId = schemaId;
    d_private->path     = path;

    d_private->settings = path.isEmpty()
                          ? g_settings_new(d_private->schemaId.toUtf8().constData())
                          : g_settings_new_with_path(d_private->schemaId.toUtf8().constData(),
                                                     path.toUtf8().constData());

    g_object_get(d_private->settings, "settings-schema", &d_private->schema, nullptr);
    d_private->signalHandlerId = g_signal_connect(d_private->settings, "changed", G_CALLBACK(DGioSettingsPrivate::onSettingChanged), d_private.data());
}

DGioSettings::~DGioSettings()
{
    Q_D(DGioSettings);

    if (d->schema) {
        g_settings_sync ();
        g_signal_handler_disconnect(d->settings, d->signalHandlerId);
        g_object_unref (d->settings);
        g_settings_schema_unref (d->schema);
    }
}

/*!
 * \brief Sets the value at \a key to \a value
 *
 * Not all values that a QVariant can hold can be serialized into a setting. Basic types are supported. the
 * provided variant value will be converted according to the original GSetting value type.
 *
 * List of supported types:
 *
 * - boolean
 * - byte/char
 * - int, uint, long long
 * - double
 * - string
 * - string list.
 *
 * \param sync call sync() after value applied if success.
 * \return true if success, false if failed.
 */
bool DGioSettings::setValue(const QString& key, const QVariant& value, bool sync)
{
    Q_D(DGioSettings);

    if (!d->trySet(key, value)) {
        qWarning() << QString("unable to set key %1 to value %2").arg(key).arg(value.toString());
        return false;
    }

    if (sync) {
        d->sync();
    }

    return true;
}

QVariant DGioSettings::value(const QString& key) const
{
    Q_D(const DGioSettings);

    return d->value(d->settings, key);
}

/*!
 * \brief Retrieves the list of avaliable keys
 *
 * According to g_settings_list_keys() document: You should probably not be calling this function
 * from "normal" code (since you should already know what keys are in your schema). This function
 * is intended for introspection reasons.
 *
 * Thus we also mark this function as DEPRECATED just like what glib does.
 */
QStringList DGioSettings::keys() const
{
    Q_D(const DGioSettings);

    QStringList list;
    gchar** keys = g_settings_list_keys(d->settings);

    for (int i = 0; keys[i]; i++) {
        list.append(keys[i]);
    }

    g_strfreev(keys);

    return list;
}

void DGioSettings::reset(const QString& key)
{
    Q_D(DGioSettings);

    g_settings_reset(d->settings, key.toUtf8().constData());
}

void DGioSettings::sync()
{
    Q_D(DGioSettings);

    d->sync();
}

bool DGioSettings::isSchemaInstalled(const QString& schemaId)
{
    GSettingsSchemaSource* source = g_settings_schema_source_get_default();

    if (GSettingsSchema* schema = g_settings_schema_source_lookup(source, schemaId.toUtf8().constData(), true)) {
        g_settings_schema_unref (schema);
        return true;
    }
    else {
        return false;
    }
}
