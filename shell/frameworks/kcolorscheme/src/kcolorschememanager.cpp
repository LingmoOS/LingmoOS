/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcolorschememanager.h"
#include "kcolorschememanager_p.h"

#include "kcolorschememodel.h"

#include <KConfigGroup>
#include <KConfigGui>
#include <KLocalizedString>
#include <KSharedConfig>
#include <kcolorscheme.h>

#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QPainter>
#include <QStandardPaths>

// ensure we are linking KConfigGui, so QColor I/O from KConfig works
KCONFIGGUI_EXPORT int initKConfigGroupGui();
static int s_init = initKConfigGroupGui();

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
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS) || defined(Q_OS_ANDROID)
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
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS) || defined(Q_OS_ANDROID)
    connect(&d->m_colorSchemeWatcher, &KColorSchemeWatcher::systemPreferenceChanged, this, [this]() {
        if (!d->m_activatedScheme.isEmpty()) {
            // Don't override what has been manually set
            return;
        }

        d->activateSchemeInternal(d->automaticColorSchemePath());
    });
#endif

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, QStringLiteral("UiSettings"));
    const QString scheme = cg.readEntry("ColorScheme", QString());

    QString schemePath;

    if (scheme.isEmpty() || scheme == QLatin1String("Default")) {
        // Color scheme might be already set from a platform theme
        // This is used for example by QGnomePlatform that can set color scheme
        // matching GNOME settings. This avoids issues where QGnomePlatform sets
        // QPalette for dark theme, but end up mixing it also with Breeze light
        // that is going to be used as a fallback for apps using KColorScheme.
        // BUG: 447029
        schemePath = qApp->property("KDE_COLOR_SCHEME_PATH").toString();
        if (schemePath.isEmpty()) {
            schemePath = d->automaticColorSchemePath();
        }
    } else {
        const auto index = indexForScheme(scheme);
        schemePath = index.data(KColorSchemeModel::PathRole).toString();
        d->m_activatedScheme = index.data(KColorSchemeModel::IdRole).toString();
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

void KColorSchemeManager::activateScheme(const QModelIndex &index)
{
    const bool isDefaultEntry = index.data(KColorSchemeModel::PathRole).toString().isEmpty();

    if (index.isValid() && index.model() == d->model.get() && !isDefaultEntry) {
        d->activateSchemeInternal(index.data(KColorSchemeModel::PathRole).toString());
        d->m_activatedScheme = index.data(KColorSchemeModel::IdRole).toString();
        if (d->m_autosaveChanges) {
            saveSchemeToConfigFile(index.data(KColorSchemeModel::NameRole).toString());
        }
    } else {
        d->activateSchemeInternal(d->automaticColorSchemePath());
        d->m_activatedScheme = QString();
        if (d->m_autosaveChanges) {
            saveSchemeToConfigFile(QString());
        }
    }
}

void KColorSchemeManager::saveSchemeToConfigFile(const QString &schemeName) const
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, QStringLiteral("UiSettings"));
    cg.writeEntry("ColorScheme", KLocalizedString::removeAcceleratorMarker(schemeName));
    cg.sync();
}

QString KColorSchemeManager::activeSchemeId() const
{
    return d->m_activatedScheme;
}

#include "moc_kcolorschememanager.cpp"
