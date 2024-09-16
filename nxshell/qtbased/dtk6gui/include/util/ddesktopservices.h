// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <dtkgui_global.h>

#include <QMetaType>
#include <QUrl>

DGUI_BEGIN_NAMESPACE

class DDesktopServices
{
public:

    enum SystemSoundEffect {
        SSE_Notifications,
        SEE_Screenshot,
        SSE_EmptyTrash,
        SSE_SendFileComplete,
        SSE_BootUp,
        SSE_Shutdown,
        SSE_Logout,
        SSE_WakeUp,
        SSE_VolumeChange,
        SSE_LowBattery,
        SSE_PlugIn,
        SSE_PlugOut,
        SSE_DeviceAdded,
        SSE_DeviceRemoved,
        SSE_Error,
    };

    static bool showFolder(const QString &localFilePath, const QString &startupId = QString());
    static bool showFolders(const QList<QString> &localFilePaths, const QString &startupId = QString());
    static bool showFolder(const QUrl &url, const QString &startupId = QString());
    static bool showFolders(const QList<QUrl> &urls, const QString &startupId = QString());

    static bool showFileItemProperty(const QString &localFilePath, const QString &startupId = QString());
    static bool showFileItemProperties(const QList<QString> &localFilePaths, const QString &startupId = QString());
    static bool showFileItemProperty(const QUrl &url, const QString &startupId = QString());
    static bool showFileItemProperties(const QList<QUrl> &urls, const QString &startupId = QString());

    static bool showFileItem(const QString &localFilePath, const QString &startupId = QString());
    static bool showFileItems(const QList<QString> &localFilePaths, const QString &startupId = QString());
    static bool showFileItem(const QUrl &url, const QString &startupId = QString());
    static bool showFileItems(const QList<QUrl> &urls, const QString &startupId = QString());

    static bool trash(const QString &localFilePath);
    static bool trash(const QList<QString> &localFilePaths);
    static bool trash(const QUrl &url);
    static bool trash(const QList<QUrl> &urls);

    static bool playSystemSoundEffect(const SystemSoundEffect &effect);
    static bool playSystemSoundEffect(const QString &name);
    static bool previewSystemSoundEffect(const SystemSoundEffect &effect);
    static bool previewSystemSoundEffect(const QString &name);
    static QString getNameByEffectType(const SystemSoundEffect &effect);

    static QString errorMessage();
};

DGUI_END_NAMESPACE

#ifdef Q_OS_LINUX
Q_DECLARE_METATYPE(DTK_GUI_NAMESPACE::DDesktopServices::SystemSoundEffect)
#endif

