/*
    SPDX-FileCopyrightText: 2013 Reza Fatahilah Shah <rshah0385@kireihana.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "themesmodel.h"
#include "config.h"
#include "thememetadata.h"

#include <QDir>
#include <QStandardPaths>
#include <QString>
#include <QUrl>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <QDebug>

ThemesModel::ThemesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    populate();
    m_customInstalledThemes = KSharedConfig::openConfig(QStringLiteral("sddmthemeinstallerrc"))->group(QStringLiteral("DownloadedThemes")).entryMap().values();
}

ThemesModel::~ThemesModel()
{
}

int ThemesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return mThemeList.size();
}

QHash<int, QByteArray> ThemesModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {IdRole, QByteArrayLiteral("id")},
        {AuthorRole, QByteArrayLiteral("author")},
        {DescriptionRole, QByteArrayLiteral("description")},
        {LicenseRole, QByteArrayLiteral("license")},
        {EmailRole, QByteArrayLiteral("email")},
        {WebsiteRole, QByteArrayLiteral("website")},
        {CopyrightRole, QByteArrayLiteral("copyright")},
        {VersionRole, QByteArrayLiteral("version")},
        {ThemeApiRole, QByteArrayLiteral("themeApi")},
        {PreviewRole, QByteArrayLiteral("preview")},
        {PathRole, QByteArrayLiteral("path")},
        {ConfigFileRole, QByteArrayLiteral("configFile")},
        {CurrentBackgroundRole, QByteArrayLiteral("currentBackground")},
        {DeletableRole, QByteArrayLiteral("deletable")},
    };
}

QVariant ThemesModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return QVariant();
    }

    const ThemeMetadata metadata = mThemeList[index.row()];

    switch (role) {
    case Qt::DisplayRole:
        return metadata.name();
    case ThemesModel::IdRole:
        return metadata.themeid();
    case ThemesModel::AuthorRole:
        return metadata.author();
    case ThemesModel::DescriptionRole:
        return metadata.description();
    case ThemesModel::LicenseRole:
        return metadata.license();
    case ThemesModel::EmailRole:
        return metadata.email();
    case ThemesModel::WebsiteRole:
        return metadata.website();
    case ThemesModel::CopyrightRole:
        return metadata.copyright();
    case ThemesModel::VersionRole:
        return metadata.version();
    case ThemesModel::ThemeApiRole:
        return metadata.themeapi();
    case ThemesModel::PreviewRole:
        return QUrl::fromLocalFile(metadata.path() + metadata.screenshot());
    case ThemesModel::PathRole:
        return metadata.path();
    case ThemesModel::ConfigFileRole:
        return metadata.configfile();
    case ThemesModel::CurrentBackgroundRole:
        if (metadata.supportsBackground()) {
            return m_currentWallpapers[metadata.themeid()];
        }
        break;
    case DeletableRole:
        return m_customInstalledThemes.contains(metadata.path().chopped(1)); // Chop the trailing /
    }

    return QVariant();
}

bool ThemesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        return false;
    }
    if (role != ThemesModel::CurrentBackgroundRole) {
        return false;
    }
    m_currentWallpapers[mThemeList[index.row()].themeid()] = value.toString();
    Q_EMIT dataChanged(index, index, {ThemesModel::CurrentBackgroundRole});
    return true;
}

void ThemesModel::populate()
{
    if (!mThemeList.isEmpty()) {
        beginResetModel();
        mThemeList.clear();
        endResetModel();
    }

    const QString themesBaseDir =
        KSharedConfig::openConfig(QStringLiteral(SDDM_CONFIG_FILE), KConfig::SimpleConfig)->group(QStringLiteral("Theme")).readEntry("ThemeDir");
    QStringList themesBaseDirs;
    if (!themesBaseDir.isEmpty()) {
        themesBaseDirs.append(themesBaseDir);
    } else {
        themesBaseDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("sddm/themes"), QStandardPaths::LocateDirectory);
    }

    auto alreadyHave = [this](const QString &theme) {
        return std::any_of(mThemeList.cbegin(), mThemeList.cend(), [&theme](const ThemeMetadata &data) {
            return data.themeid() == theme;
        });
    };
    for (const auto &folder : themesBaseDirs) {
        QDir dir(folder);
        if (!dir.exists()) {
            return;
        }
        for (const QString &theme : dir.entryList(QDir::AllDirs | QDir::Readable | QDir::NoDotAndDotDot)) {
            QString path = folder + QLatin1Char('/') + theme;
            if (!alreadyHave(theme) && QFile::exists(path + QStringLiteral("/metadata.desktop"))) {
                add(theme, path);
            }
        }
    }
}

void ThemesModel::add(const QString &id, const QString &path)
{
    beginInsertRows(QModelIndex(), mThemeList.count(), mThemeList.count());

    const auto data = ThemeMetadata(id, path);
    mThemeList.append(data);
    if (data.supportsBackground()) {
        const QString themeConfigPath = data.path() + data.configfile();
        auto themeConfig = KSharedConfig::openConfig(themeConfigPath + QStringLiteral(".user"), KConfig::CascadeConfig);
        themeConfig->addConfigSources({themeConfigPath});
        const QString backgroundPath = themeConfig->group(QStringLiteral("General")).readEntry("background");
        if (backgroundPath.startsWith(QStringLiteral("/"))) {
            m_currentWallpapers.insert(data.themeid(), backgroundPath);
        } else {
            m_currentWallpapers.insert(data.themeid(), data.path() + backgroundPath);
        }
    }
    endInsertRows();
}

void ThemesModel::dump(const QString &id, const QString &path)
{
    Q_UNUSED(id)

    ThemeMetadata metadata(path);

    qDebug() << "Theme Path:" << metadata.path();
    qDebug() << "Name: " << metadata.name();
    qDebug() << "Version: " << metadata.version();
    qDebug() << "Author: " << metadata.author();
    qDebug() << "Description: " << metadata.description();
    qDebug() << "Email: " << metadata.email();
    qDebug() << "License: " << metadata.license();
    qDebug() << "Copyright: " << metadata.copyright();
    qDebug() << "Screenshot: " << metadata.screenshot();
}

int ThemesModel::currentIndex() const
{
    return m_currentIndex;
}

QString ThemesModel::currentTheme() const
{
    return mThemeList[m_currentIndex].themeid();
}

void ThemesModel::setCurrentTheme(const QString &theme)
{
    auto it = std::find_if(mThemeList.cbegin(), mThemeList.cend(), [&theme](const ThemeMetadata &themeData) {
        return themeData.themeid() == theme;
    });
    const int index = it - mThemeList.cbegin();
    if (it == mThemeList.cend() || index == m_currentIndex) {
        return;
    }
    m_currentIndex = index;
    Q_EMIT currentIndexChanged();
}
