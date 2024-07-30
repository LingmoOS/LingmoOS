/*
SPDX-FileCopyrightText: 2019 Kevin Ottens <kevin.ottens@enioka.com>
SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kscreensaversettings.h"

#include <KActionCollection>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPackage/Package>
#include <KPackage/PackageLoader>

#include <QCollator>

QList<QKeySequence> KScreenSaverSettings::defaultShortcuts()
{
    return {Qt::META | Qt::Key_L, Qt::Key_ScreenSaver};
}

QString KScreenSaverSettings::defaultWallpaperPlugin()
{
    return QStringLiteral("org.kde.image");
}

class KScreenSaverSettingsStore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QKeySequence shortcut READ shortcut WRITE setShortcut)
public:
    KScreenSaverSettingsStore(KScreenSaverSettings *parent)
        : QObject(parent)
        , m_actionCollection(new KActionCollection(this, QStringLiteral("ksmserver")))
        , m_lockAction(nullptr)
    {
        m_actionCollection->setConfigGlobal(true);
        m_actionCollection->setComponentDisplayName(i18n("Session Management"));
        m_lockAction = m_actionCollection->addAction(QStringLiteral("Lock Session"));
        m_lockAction->setProperty("isConfigurationAction", true);
        m_lockAction->setText(i18n("Lock Session"));
        KGlobalAccel::self()->setShortcut(m_lockAction, parent->defaultShortcuts());
    }

    QKeySequence shortcut() const
    {
        const QList<QKeySequence> shortcuts = KGlobalAccel::self()->shortcut(m_lockAction);
        if (shortcuts.count() > 0) {
            return shortcuts.first();
        }
        return QKeySequence();
    }
    void setShortcut(const QKeySequence &sequence) const
    {
        auto shortcuts = KGlobalAccel::self()->shortcut(m_lockAction);
        if (shortcuts.isEmpty()) {
            shortcuts << QKeySequence();
        }
        shortcuts[0] = sequence;
        KGlobalAccel::self()->setShortcut(m_lockAction, shortcuts, KGlobalAccel::NoAutoloading);
    }

private:
    KActionCollection *m_actionCollection;
    QAction *m_lockAction;
};

KScreenSaverSettings &KScreenSaverSettings::getInstance()
{
    static KScreenSaverSettings instance;
    return instance;
}

KScreenSaverSettings::KScreenSaverSettings(QObject *parent)
    : KScreenSaverSettingsBase()
    , m_store(new KScreenSaverSettingsStore(this))
{
    setParent(parent);

    const auto wallpaperPackages = KPackage::PackageLoader::self()->listPackages(QStringLiteral("Plasma/Wallpaper"));
    for (auto &package : wallpaperPackages) {
        m_availableWallpaperPlugins.append({package.name(), package.pluginId()});
    }
    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(m_availableWallpaperPlugins.begin(), m_availableWallpaperPlugins.end(), [](const WallpaperInfo &w1, const WallpaperInfo &w2) {
        return w1.name < w2.name;
    });

    auto shortcutItem = new KPropertySkeletonItem(m_store, "shortcut", defaultShortcuts().first());
    addItem(shortcutItem, QStringLiteral("shortcut"));
    shortcutItem->setNotifyFunction([this] {
        Q_EMIT shortcutChanged();
    });
}

KScreenSaverSettings::~KScreenSaverSettings()
{
}

QList<WallpaperInfo> KScreenSaverSettings::availableWallpaperPlugins() const
{
    return m_availableWallpaperPlugins;
}

QKeySequence KScreenSaverSettings::shortcut() const
{
    return findItem(QStringLiteral("shortcut"))->property().value<QKeySequence>();
}

void KScreenSaverSettings::setShortcut(const QKeySequence &sequence)
{
    findItem(QStringLiteral("shortcut"))->setProperty(sequence);
}

#include "kscreensaversettings.moc"

#include "moc_kscreensaversettings.cpp"
