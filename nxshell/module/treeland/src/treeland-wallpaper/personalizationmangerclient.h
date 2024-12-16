// Copyright (C) 2023 Dingyuan Zhang <lxz@mkacg.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "qwayland-treeland-personalization-manager-v1.h"
#include "wallpapercardmodel.h"

#include <QGuiApplication>
#include <QQmlEngine>
#include <QtWaylandClient/QWaylandClientExtension>

class PersonalizationWindow;
class PersonalizationWallpaper;

class PersonalizationV1 : public QWaylandClientExtensionTemplate<PersonalizationV1>,
                               public QtWayland::treeland_personalization_manager_v1
{
    Q_OBJECT
    Q_PROPERTY(QString output READ output WRITE setOutput NOTIFY outputChanged FINAL)
    Q_PROPERTY(QString currentGroup READ currentGroup WRITE setCurrentGroup NOTIFY currentGroupChanged FINAL)
    Q_PROPERTY(QString cacheDirectory READ cacheDirectory CONSTANT)
    QML_ELEMENT

public:
    struct WallpaperMetaData
    {
        QString group;
        QString imagePath; // wallpaper path
        bool isdark;
        QString output; // Output which wallpaper belongs
        quint32 options;
        QScreen *screen;
        int currentIndex;
    };

    explicit PersonalizationV1();
    ~PersonalizationV1();

    void onActiveChanged();
    QString cacheDirectory();

    QString currentGroup();
    void setCurrentGroup(const QString &group);

    QString output();
    void setOutput(const QString &name);

signals:
    void wallpaperChanged(const QString &path);
    void currentGroupChanged(const QString &path);
    void outputChanged(const QString &name);

public slots:
    void addWallpaper(const QString &path);
    void setBackground(const QString &path, const QString &group, int index, bool isdark);
    void setLockscreen(const QString &path, const QString &group, int index, bool isdark);
    void setBoth(const QString &path, const QString &group, int index);
    void removeWallpaper(const QString &path, const QString &group, int index);
    QStringList outputModel();

    WallpaperCardModel *wallpaperModel(const QString &group, const QString &dir);

private:
    QString converToJson(QMap<QString, WallpaperMetaData *> screens);
    void onMetadataChanged(const QString &meta);
    void changeWallpaper(const QString &path,
                         const QString &output,
                         const QString &group,
                         int index,
                         quint32 op,
                         bool isdark = true);

private:
    PersonalizationWallpaper *m_wallpaperContext = nullptr;
    QMap<QString, WallpaperCardModel *> m_modes;
    QMap<QString, WallpaperMetaData *> m_screens;
    QString m_cacheDirectory;
    QString m_currentOutput;
};

class PersonalizationWindow : public QWaylandClientExtensionTemplate<PersonalizationWindow>,
                              public QtWayland::personalization_window_context_v1
{
    Q_OBJECT
public:
    explicit PersonalizationWindow(struct ::personalization_window_context_v1 *object);
};

class PersonalizationWallpaper : public QWaylandClientExtensionTemplate<PersonalizationWallpaper>,
                                 public QtWayland::personalization_wallpaper_context_v1
{
    Q_OBJECT
public:
    explicit PersonalizationWallpaper(struct ::personalization_wallpaper_context_v1 *object);

signals:
    void metadataChanged(const QString &meta);

protected:
    void personalization_wallpaper_context_v1_metadata(const QString &metadata) override;
};
