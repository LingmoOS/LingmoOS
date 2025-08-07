/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcolorschememanager.h"
#include "kcolorschememanager_p.h"

#include <KActionMenu>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <kcolorscheme.h>

#include <QActionGroup>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QPainter>
#include <QStandardPaths>
#include <QStyle>

constexpr int defaultSchemeRow = 0;

void KColorSchemeManagerPrivate::activateSchemeInternal(const QString &colorSchemePath)
{
    // hint for plasma-integration to synchronize the color scheme with the window manager/compositor
    // The property needs to be set before the palette change because is is checked upon the
    // ApplicationPaletteChange event.
    qApp->setProperty("KDE_COLOR_SCHEME_PATH", colorSchemePath);
    if (colorSchemePath.isEmpty()) {
        qApp->setPalette(KColorScheme::createApplicationPalette(KSharedConfig::Ptr(nullptr)));
    } else {
        qApp->setPalette(KColorScheme::createApplicationPalette(KSharedConfig::openConfig(colorSchemePath)));
    }
}

// The meaning of the Default entry depends on the platform
// On Windows and macOS we automatically apply Breeze/Breeze Dark depending on the system preference
// On other platforms we apply a default KColorScheme
QString KColorSchemeManagerPrivate::automaticColorSchemePath() const
{
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    const QString colorSchemeId = m_colorSchemeWatcher.systemPreference() == KColorSchemeWatcher::PreferDark ? getDarkColorScheme() : getLightColorScheme();
    return indexForSchemeId(colorSchemeId).data(KColorSchemeModel::PathRole).toString();
#else
    return QString();
#endif
}

QIcon KColorSchemeManagerPrivate::createPreview(const QString &path)
{
    KSharedConfigPtr schemeConfig = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    QIcon result;

    KColorScheme activeWindow(QPalette::Active, KColorScheme::Window, schemeConfig);
    KColorScheme activeButton(QPalette::Active, KColorScheme::Button, schemeConfig);
    KColorScheme activeView(QPalette::Active, KColorScheme::View, schemeConfig);
    KColorScheme activeSelection(QPalette::Active, KColorScheme::Selection, schemeConfig);

    auto pixmap = [&](int size) {
        QPixmap pix(size, size);
        pix.fill(Qt::black);
        QPainter p;
        p.begin(&pix);
        const int itemSize = size / 2 - 1;
        p.fillRect(1, 1, itemSize, itemSize, activeWindow.background());
        p.fillRect(1 + itemSize, 1, itemSize, itemSize, activeButton.background());
        p.fillRect(1, 1 + itemSize, itemSize, itemSize, activeView.background());
        p.fillRect(1 + itemSize, 1 + itemSize, itemSize, itemSize, activeSelection.background());
        p.end();
        result.addPixmap(pix);
    };
    // 16x16
    pixmap(16);
    // 24x24
    pixmap(24);

    return result;
}

KColorSchemeManagerPrivate::KColorSchemeManagerPrivate()
    : model(new KColorSchemeModel())
{
}

KColorSchemeManager::KColorSchemeManager(QObject *parent)
    : QObject(parent)
    , d(new KColorSchemeManagerPrivate)
{
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    connect(&d->m_colorSchemeWatcher, &KColorSchemeWatcher::systemPreferenceChanged, this, [this]() {
        if (!d->m_defaultSchemeSelected) {
            // Don't override what has been manually set
            return;
        }

        d->activateSchemeInternal(d->automaticColorSchemePath());
    });
#endif

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, "UiSettings");
    const QString scheme = cg.readEntry("ColorScheme", QString());

    QString schemePath;

    if (scheme.isEmpty() || scheme == QLatin1String("Default")) {
        schemePath = d->automaticColorSchemePath();
        d->m_defaultSchemeSelected = true;
    } else {
        schemePath = indexForScheme(scheme).data(KColorSchemeModel::PathRole).toString();
        d->m_defaultSchemeSelected = false;
    }
    d->activateSchemeInternal(schemePath);
}

KColorSchemeManager::~KColorSchemeManager()
{
}

QAbstractItemModel *KColorSchemeManager::model() const
{
    return d->model.get();
}

QModelIndex KColorSchemeManagerPrivate::indexForSchemeId(const QString &id) const
{
    for (int i = 1; i < model->rowCount(); ++i) {
        QModelIndex index = model->index(i);
        if (index.data(KColorSchemeModel::IdRole).toString() == id) {
            return index;
        }
    }
    return QModelIndex();
}

void KColorSchemeManager::setAutosaveChanges(bool autosaveChanges)
{
    d->m_autosaveChanges = autosaveChanges;
}

QModelIndex KColorSchemeManager::indexForScheme(const QString &name) const
{
    // Empty string is mapped to "reset to the system scheme"
    if (name.isEmpty()) {
        return d->model->index(defaultSchemeRow);
    }
    for (int i = 1; i < d->model->rowCount(); ++i) {
        QModelIndex index = d->model->index(i);
        if (index.data(KColorSchemeModel::NameRole).toString() == name) {
            return index;
        }
    }
    return QModelIndex();
}

KActionMenu *KColorSchemeManager::createSchemeSelectionMenu(const QIcon &icon, const QString &name, const QString &selectedSchemeName, QObject *parent)
{
    // Be careful here when connecting to signals. The menu can outlive the manager
    KActionMenu *menu = new KActionMenu(icon, name, parent);
    QActionGroup *group = new QActionGroup(menu);
    connect(group, &QActionGroup::triggered, qApp, [this](QAction *action) {
        const QString schemePath = action->data().toString();

        if (schemePath.isEmpty()) {
            // Reset to default
            d->activateSchemeInternal(d->automaticColorSchemePath());
            if (d->m_autosaveChanges) {
                saveSchemeToConfigFile(QString());
            }
            d->m_defaultSchemeSelected = true;
        } else {
            d->activateSchemeInternal(schemePath);
            if (d->m_autosaveChanges) {
                saveSchemeToConfigFile(action->text());
            }
            d->m_defaultSchemeSelected = false;
        }
    });
    for (int i = 0; i < d->model->rowCount(); ++i) {
        QModelIndex index = d->model->index(i);
        QAction *action = new QAction(index.data(KColorSchemeModel::NameRole).toString(), menu);
        action->setData(index.data(KColorSchemeModel::PathRole));
        action->setActionGroup(group);
        action->setCheckable(true);
        if (index.data(KColorSchemeModel::NameRole).toString() == selectedSchemeName) {
            action->setChecked(true);
        }
        menu->addAction(action);
    }
    const auto groupActions = group->actions();
    if (!group->checkedAction()) {
        // If no (valid) color scheme has been selected we select the default one
        groupActions[defaultSchemeRow]->setChecked(true);
    }
    groupActions[defaultSchemeRow]->setIcon(QIcon::fromTheme(QStringLiteral("edit-undo")));
    connect(menu->menu(), &QMenu::aboutToShow, group, [group] {
        const auto actions = group->actions();
        for (QAction *action : actions) {
            if (action->icon().isNull()) {
                action->setIcon(KColorSchemeManagerPrivate::createPreview(action->data().toString()));
            }
        }
    });

    return menu;
}

KActionMenu *KColorSchemeManager::createSchemeSelectionMenu(const QString &text, const QString &selectedSchemeName, QObject *parent)
{
    return createSchemeSelectionMenu(QIcon::fromTheme(QStringLiteral("preferences-desktop-color")), text, selectedSchemeName, parent);
}

KActionMenu *KColorSchemeManager::createSchemeSelectionMenu(const QString &selectedSchemeName, QObject *parent)
{
    return createSchemeSelectionMenu(QIcon::fromTheme(QStringLiteral("preferences-desktop-color")), i18n("Color Scheme"), selectedSchemeName, parent);
}

KActionMenu *KColorSchemeManager::createSchemeSelectionMenu(QObject *parent)
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, "UiSettings");
    auto scheme = cg.readEntry("ColorScheme", QString());

    return createSchemeSelectionMenu(QIcon::fromTheme(QStringLiteral("preferences-desktop-color")), i18n("Color Scheme"), scheme, parent);
}

void KColorSchemeManager::activateScheme(const QModelIndex &index)
{
    const bool isDefaultEntry = index.data(KColorSchemeModel::PathRole).toString().isEmpty();

    if (index.isValid() && index.model() == d->model.get() && !isDefaultEntry) {
        d->activateSchemeInternal(index.data(KColorSchemeModel::PathRole).toString());
        if (d->m_autosaveChanges) {
            saveSchemeToConfigFile(index.data(KColorSchemeModel::NameRole).toString());
        }
    } else {
        d->activateSchemeInternal(d->automaticColorSchemePath());
        if (d->m_autosaveChanges) {
            saveSchemeToConfigFile(QString());
        }
    }
}

void KColorSchemeManager::saveSchemeToConfigFile(const QString &schemeName) const
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, "UiSettings");
    cg.writeEntry("ColorScheme", KLocalizedString::removeAcceleratorMarker(schemeName));
    cg.sync();
}
