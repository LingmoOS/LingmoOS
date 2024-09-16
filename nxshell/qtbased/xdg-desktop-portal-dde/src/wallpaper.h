// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <qobjectdefs.h>

class PersonalizationManager;

class WallPaperPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Wallpaper")

public:
    explicit WallPaperPortal(QObject *parent);
    ~WallPaperPortal();

public slots:
    uint SetWallpaperURI(const QDBusObjectPath &handle,
                         const QString &app_id,
                         const QString &parent_window,
                         const QString &uri,
                         const QVariantMap &options);

private:
    uint set_V20_WallpaperURI(const QDBusObjectPath &handle,
                              const QString &app_id,
                              const QString &parent_window,
                              const QString &uri,
                              const QVariantMap &options);

    uint set_Treeland_WallpaperURI(const QDBusObjectPath &handle,
                                   const QString &app_id,
                                   const QString &parent_window,
                                   const QString &uri,
                                   const QVariantMap &options);

    uint32_t setOn2Int(const QVariantMap &options);

private:
    PersonalizationManager *m_personalizationManager = nullptr;
};
