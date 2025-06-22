/*
SPDX-FileCopyrightText: 2019 Kevin Ottens <kevin.ottens@enioka.com>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <QKeySequence>

#include "kscreensaversettingsbase.h"

class QAction;
class KActionCollection;

class KScreenSaverSettingsStore;

struct WallpaperInfo {
    Q_PROPERTY(QString name MEMBER name CONSTANT)
    Q_PROPERTY(QString id MEMBER id CONSTANT)
    QString name;
    QString id;
    Q_GADGET
};

class KScreenSaverSettings : public KScreenSaverSettingsBase
{
    Q_OBJECT
    Q_PROPERTY(QKeySequence shortcut READ shortcut WRITE setShortcut NOTIFY shortcutChanged)
public:
    static KScreenSaverSettings &getInstance();

    static QList<QKeySequence> defaultShortcuts();
    static QString defaultWallpaperPlugin();

    ~KScreenSaverSettings() override;

    QVector<WallpaperInfo> availableWallpaperPlugins() const;

    QKeySequence shortcut() const;
    void setShortcut(const QKeySequence &sequence);

    KScreenSaverSettings(KScreenSaverSettings const &) = delete;
    void operator=(KScreenSaverSettings const &) = delete;

Q_SIGNALS:
    void shortcutChanged();

protected:
    KScreenSaverSettings(QObject *parent = nullptr);

private:
    QVector<WallpaperInfo> m_availableWallpaperPlugins;
    KScreenSaverSettingsStore *m_store;
};
