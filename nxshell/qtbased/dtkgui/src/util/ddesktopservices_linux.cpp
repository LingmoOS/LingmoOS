// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ddesktopservices.h"
#include <QtDBus/QtDBus>
#include <QDebug>
#include <QFile>

DGUI_BEGIN_NAMESPACE

#define EASY_CALL_DBUS(name)                                          \
  QDBusInterface *interface = fileManager1DBusInterface();            \
  return interface &&                                                 \
         interface->call(#name, urls2uris(urls), startupId).type() != \
             QDBusMessage::ErrorMessage;

static const QStringList SOUND_EFFECT_LIST {
    "message",
    "camera-shutter",
    "trash-empty",
    "x-deepin-app-sent-to-desktop",
    "desktop-login",
    "system-shutdown",
    "desktop-logout",
    "suspend-resume",
    "audio-volume-change",
    "power-unplug-battery-low",
    "power-plug",
    "power-unplug",
    "device-added",
    "device-removed",
    "dialog-error",
};

static QDBusInterface *fileManager1DBusInterface()
{
    static QDBusInterface interface(QStringLiteral("org.freedesktop.FileManager1"),
                                        QStringLiteral("/org/freedesktop/FileManager1"),
                                        QStringLiteral("org.freedesktop.FileManager1"));
    return &interface;
}

static QStringList urls2uris(const QList<QUrl> &urls)
{
    QStringList list;

    list.reserve(urls.size());

    for (const QUrl &url : urls) {
        list << url.toString();
    }

    return list;
}

static QList<QUrl> path2urls(const QList<QString> &paths)
{
    QList<QUrl> list;

    list.reserve(paths.size());

    for (const QString &path : paths) {
        list << QUrl::fromLocalFile(path);
    }

    return list;
}

static QDBusInterface soundEffectInterface()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    const auto& infc = QDBusConnection::sessionBus().interface();
    QStringList activatableServiceNames = infc->activatableServiceNames();
    bool isNewInterface = activatableServiceNames.contains(QLatin1String("org.deepin.dde.SoundEffect1"));
#else
    bool isNewInterface = false; // Qt 5.14 以下就直接用旧的接口
#endif
    const QLatin1String service(isNewInterface ? "org.deepin.dde.SoundEffect1" :"com.deepin.daemon.SoundEffect");
    const QLatin1String path(isNewInterface ? "/org/deepin/dde/SoundEffect1" : "/com/deepin/daemon/SoundEffect");
    const QLatin1String interface(isNewInterface ? "org.deepin.dde.SoundEffect1" :"com.deepin.daemon.SoundEffect");

    // 使用后端 dbus 接口播放系统音频，音频存放目录： /usr/share/sounds/deepin/stereo/
    return QDBusInterface(service, path, interface);
}

static bool systemSoundEffectEnabled(const QString &name)
{
    auto soundEffect = soundEffectInterface();
    if (!soundEffect.property("Enabled").toBool())
        return false;

    QDBusReply<bool> reply = soundEffect.call("IsSoundEnabled", name);

    return reply.isValid() && reply.value();
}

bool DDesktopServices::showFolder(const QString &localFilePath, const QString &startupId)
{
    return showFolder(QUrl::fromLocalFile(localFilePath), startupId);
}

bool DDesktopServices::showFolders(const QList<QString> &localFilePaths, const QString &startupId)
{
    return showFolders(path2urls(localFilePaths), startupId);
}

bool DDesktopServices::showFolder(const QUrl &url, const QString &startupId)
{
    return showFolders(QList<QUrl>() << url, startupId);
}

bool DDesktopServices::showFolders(const QList<QUrl> &urls, const QString &startupId)
{
    EASY_CALL_DBUS(ShowFolders)
}

bool DDesktopServices::showFileItemProperty(const QString &localFilePath, const QString &startupId)
{
    return showFileItemProperty(QUrl::fromLocalFile(localFilePath), startupId);
}

bool DDesktopServices::showFileItemProperties(const QList<QString> &localFilePaths, const QString &startupId)
{
    return showFileItemProperties(path2urls(localFilePaths), startupId);
}

bool DDesktopServices::showFileItemProperty(const QUrl &url, const QString &startupId)
{
    return showFileItemProperties(QList<QUrl>() << url, startupId);
}

bool DDesktopServices::showFileItemProperties(const QList<QUrl> &urls, const QString &startupId)
{
    EASY_CALL_DBUS(ShowItemProperties)
}

bool DDesktopServices::showFileItem(const QString &localFilePath, const QString &startupId)
{
    return showFileItem(QUrl::fromLocalFile(localFilePath), startupId);
}

bool DDesktopServices::showFileItems(const QList<QString> &localFilePaths, const QString &startupId)
{
    return showFileItems(path2urls(localFilePaths), startupId);
}

bool DDesktopServices::showFileItem(const QUrl &url, const QString &startupId)
{
    return showFileItems(QList<QUrl>() << url, startupId);
}

bool DDesktopServices::showFileItems(const QList<QUrl> &urls, const QString &startupId)
{
    EASY_CALL_DBUS(ShowItems)
}

bool DDesktopServices::trash(const QString &localFilePath)
{
    return trash(QUrl::fromLocalFile(localFilePath));
}

bool DDesktopServices::trash(const QList<QString> &localFilePaths)
{
    return trash(path2urls(localFilePaths));
}

bool DDesktopServices::trash(const QUrl &url)
{
    return trash(QList<QUrl>() << url);
}

bool DDesktopServices::trash(const QList<QUrl> &urls)
{
    QDBusInterface *interface = fileManager1DBusInterface();
    return interface && interface->call("Trash", urls2uris(urls)).type() != QDBusMessage::ErrorMessage;
}

bool DDesktopServices::playSystemSoundEffect(const DDesktopServices::SystemSoundEffect &effect)
{
    return playSystemSoundEffect(SOUND_EFFECT_LIST.at(static_cast<int>(effect)));
}

bool DDesktopServices::playSystemSoundEffect(const QString &name)
{
    if (!systemSoundEffectEnabled(name))
        return false;

    return previewSystemSoundEffect(name);
}

bool DDesktopServices::previewSystemSoundEffect(const DDesktopServices::SystemSoundEffect &effect)
{
    return previewSystemSoundEffect(SOUND_EFFECT_LIST.at(static_cast<int>(effect)));
}

bool DDesktopServices::previewSystemSoundEffect(const QString &name)
{
    if (name.isEmpty()) {
        return false;
    }

    // 使用后端 dbus 接口播放系统音频，音频存放目录： /usr/share/sounds/deepin/stereo/
    return soundEffectInterface().call("PlaySound", name).type() != QDBusMessage::ErrorMessage;
}

QString DDesktopServices::getNameByEffectType(const DDesktopServices::SystemSoundEffect &effect)
{
    return SOUND_EFFECT_LIST.at(static_cast<int>(effect));
}

QString DDesktopServices::errorMessage()
{
    return fileManager1DBusInterface()->lastError().message();
}

DGUI_END_NAMESPACE
