// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <QGSettings>
#include <QDebug>

// system services
#define AT_SPI_SERVICE "at-spi-dbus-bus.service"
#define OBEX_SERVICE "obex.service"
#define PULSEAUDIO_SERVICE "pulseaudio.service"
#define PIPEWIRE_SOCKET "pipewire.socket"
#define PIPEWIRE_PULSE_SOCKET "pipewire-pulse.socket"
#define BAMFDAEMON_SERVICE "bamfdaemon.service"
#define REDSHIFT_SERVICE "redshift.service"
#define DDE_DOCK_SERVICE "dde-dock.service"
#define DDE_DESKTOP_SERVICE "dde-desktop.service"

// sound
#define SOUND_EFFECT_SCHEMA "com.deepin.dde.sound-effect"
#define SOUND_EFFECT_ENABLED_KEY "enabled"
#define SOUND_EFFECT_PLAYER_KEY "player"

// appearance
#define APPEARANCE_SCHEMA "com.deepin.dde.appearance"
#define APPEARANCE_SOUND_THEME_KEY "sound-theme"

// macro funtions
#define EXEC_COMMAND(cmd, args) \
    QProcess p;\
    p.start(cmd, args);\
    p.waitForFinished(-1);\
    if (p.exitCode() != 0) {\
    qWarning() << "failed to run: " << cmd << ", args:" << args;\
    }

namespace Utils {

const bool IS_WAYLAND_DISPLAY = qgetenv("XDG_SESSION_TYPE").startsWith("wayland");

/**
 * @brief SettingsPtr 根据给定信息返回一个QGSettings指针
 * @param schema_id The id of the schema
 * @param path If non-empty, specifies the path for a relocatable schema
 * @param parent 创建指针的付对象
 * @return
 */
inline QGSettings *SettingsPtr(const QString &schema_id, const QByteArray &path = QByteArray(), QObject *parent = nullptr)
{
    if (QGSettings::isSchemaInstalled(schema_id.toUtf8())) {
        QGSettings *settings = new QGSettings(schema_id.toUtf8(), path, parent);
        return settings;
    }
    qDebug() << "Cannot find gsettings, schema_id:" << schema_id;
    return nullptr;
}

/**
 * @brief SettingsPtr 根据给定信息返回一个QGSettings指针
 * @param module 传入QGSettings构造函数时，会添加"com.deepin.dde.dock.module."前缀
 * @param path If non-empty, specifies the path for a relocatable schema
 * @param parent 创建指针的付对象
 * @return
 */
inline const QGSettings *ModuleSettingsPtr(const QString &module, const QByteArray &path = QByteArray(), QObject *parent = nullptr)
{
    return SettingsPtr("com.deepin.dde.dock.module." + module, path, parent);
}

/* convert 'some-key' to 'someKey' or 'SomeKey'.
 * the second form is needed for appending to 'set' for 'setSomeKey'
 */
inline QString qtify_name(const char *name)
{
    bool next_cap = false;
    QString result;

    while (*name) {
        if (*name == '-') {
            next_cap = true;
        } else if (next_cap) {
            result.append(QChar(*name).toUpper().toLatin1());
            next_cap = false;
        } else {
            result.append(*name);
        }

        name++;
    }

    return result;
}

/**
 * @brief SettingValue 根据给定信息返回获取的值
 * @param schema_id The id of the schema
 * @param path If non-empty, specifies the path for a relocatable schema
 * @param key 对应信息的key值
 * @param fallback 如果找不到信息，返回此默认值
 * @example Utils::SettingValue("com.deepin.dde.startdde", QByteArray(), "quick-black-screen", false).toBool()
 */
inline const QVariant SettingValue(const QString &schema_id, const QByteArray &path = QByteArray(), const QString &key = QString(), const QVariant &fallback = QVariant())
{
    const QGSettings *settings = SettingsPtr(schema_id, path);

    if (settings && ((settings->keys().contains(key)) || settings->keys().contains(qtify_name(key.toUtf8().data())))) {
        QVariant v = settings->get(key);
        delete settings;
        return v;
    } else{
        qDebug() << "gsettings not exists, schema_id:" << schema_id
                 << " path:" << path << " key:" << key
                 << "Use fallback value:" << fallback;
        // 如果settings->keys()不包含key则会存在内存泄露，所以需要释放
        if (settings)
            delete settings;

        return fallback;
    }
}

inline bool SettingSaveValue(const QString &schema_id, const QByteArray &path, const QString &key, const QVariant &value)
{
    QGSettings *settings = SettingsPtr(schema_id, path);

    if (settings && ((settings->keys().contains(key)) || settings->keys().contains(qtify_name(key.toUtf8().data())))) {
        settings->set(key, value);
        delete settings;
        return true;
    } else{
        qDebug() << "gsettings not exists, schema_id:" << schema_id
                 << " path:" << path << " key:" << key;
        if (settings)
            delete settings;

        return false;
    }
}
}

#endif // UTILS_H
