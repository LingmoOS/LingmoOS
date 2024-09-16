// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wallpaper.h"
#include "personalization_manager_client.h"

#include <QFile>
#include <QLoggingCategory>
#include <QDBusMessage>
#include <qdbusconnection.h>
#include <qloggingcategory.h>
#include <QDBusPendingReply>

Q_LOGGING_CATEGORY(XdgDesktopDDEWallpaper, "xdg-dde-wallpaper")

WallPaperPortal::WallPaperPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
    , m_personalizationManager(new PersonalizationManager)
{
    qCDebug(XdgDesktopDDEWallpaper) << "WallPaper init";
}

WallPaperPortal::~WallPaperPortal()
{
    if (m_personalizationManager) {
        delete m_personalizationManager;
        m_personalizationManager = nullptr;
    }
}

uint WallPaperPortal::SetWallpaperURI(const QDBusObjectPath &handle,
                                      const QString &app_id,
                                      const QString &parent_window,
                                      const QString &uri,
                                      const QVariantMap &options)
{
    return set_Treeland_WallpaperURI(handle, app_id, parent_window, uri, options);
}

uint WallPaperPortal::set_V20_WallpaperURI(const QDBusObjectPath &handle,
                          const QString &app_id,
                          const QString &parent_window,
                          const QString &uri,
                          const QVariantMap &options)
{
    // TODO: 未处理options，仅实现设置主屏壁纸
    // options.value("show-preview").toBool(); // whether to show a preview of the picture. Note that the portal may decide to show a preview even if this option is not set
    // options.value("set-on").toString(); // where to set the wallpaper. Possible values are 'background', 'lockscreen' or 'both'

    qCDebug(XdgDesktopDDEWallpaper) << "Start set wallpaper";
    QDBusMessage primaryMsg = QDBusMessage::createMethodCall(QStringLiteral("org.deepin.dde.Display1"),
                                                             QStringLiteral("/org/deepin/dde/Display1"),
                                                             QStringLiteral("org.freedesktop.DBus.Properties"),
                                                             QStringLiteral("Get"));
    primaryMsg.setArguments({QVariant::fromValue(QStringLiteral("org.deepin.dde.Display1")), QVariant::fromValue(QStringLiteral("Primary"))});
    QDBusPendingReply<QDBusVariant> primaryReply = QDBusConnection::sessionBus().call(primaryMsg);
    if (primaryReply.isError()) {
        qCDebug(XdgDesktopDDEWallpaper) << "setting failed";
        return 1;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.deepin.dde.Appearance1"),
                                                      QStringLiteral("/org/deepin/dde/Appearance1"),
                                                      QStringLiteral("org.deepin.dde.Appearance1"),
                                                      QStringLiteral("SetMonitorBackground"));
    msg.setArguments({QVariant::fromValue(primaryReply.value().variant().toString()), QVariant::fromValue(uri)});
    QDBusPendingReply<> pcall = QDBusConnection::sessionBus().call(msg);
    if (pcall.isValid()) {
        qCDebug(XdgDesktopDDEWallpaper) << "setting succeed";
        return 0;
    }
    qCDebug(XdgDesktopDDEWallpaper) << "setting failed";

    return 1;
}

uint WallPaperPortal::set_Treeland_WallpaperURI(const QDBusObjectPath &handle,
                               const QString &app_id,
                               const QString &parent_window,
                               const QString &uri,
                               const QVariantMap &options)
{
    if (!m_personalizationManager)
        return 1;

    auto wallpaper = m_personalizationManager->wallpaper();
    if (!wallpaper)
        return 1;

    QFile file(uri);
    if (file.open(QIODevice::ReadOnly)) {
        wallpaper->set_on(setOn2Int(options));
        wallpaper->set_fd(file.handle(), "");
        wallpaper->commit();
    }

    return 1;
}

uint32_t WallPaperPortal::setOn2Int(const QVariantMap &options)
{
    QString set_on = options.value("set-on").toString();
    uint32_t op = 0;
    if (set_on == "background")
        op = personalization_wallpaper_context_v1_options::PERSONALIZATION_WALLPAPER_CONTEXT_V1_OPTIONS_BACKGROUND;
    else if (set_on == "lockscreen")
        op = personalization_wallpaper_context_v1_options::PERSONALIZATION_WALLPAPER_CONTEXT_V1_OPTIONS_LOCKSCREEN;
    else if (set_on == "both")
        op = personalization_wallpaper_context_v1_options::PERSONALIZATION_WALLPAPER_CONTEXT_V1_OPTIONS_LOCKSCREEN |
             personalization_wallpaper_context_v1_options::PERSONALIZATION_WALLPAPER_CONTEXT_V1_OPTIONS_BACKGROUND;

    if (options.value("show-preview").toBool())
        op = op | personalization_wallpaper_context_v1_options::PERSONALIZATION_WALLPAPER_CONTEXT_V1_OPTIONS_PREVIEW;

    return op;
}
