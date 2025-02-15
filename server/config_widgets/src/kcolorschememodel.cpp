/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcolorschememodel.h"

#include "kcolorschememanager_p.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <kcolorscheme.h>

#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QPainter>
#include <QStandardPaths>

#include <map>

struct KColorSchemeModelData {
    QString id; // e.g. BreezeDark
    QString name; // e.g. "Breeze Dark" or "Breeze-Dunkel"
    QString path;
    QIcon preview;
};

struct KColorSchemeModelPrivate {
    mutable QVector<KColorSchemeModelData> m_data;
};

KColorSchemeModel::KColorSchemeModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new KColorSchemeModelPrivate)
{
    beginResetModel();
    d->m_data.clear();

#ifndef Q_OS_ANDROID
    // Fill the model with all *.colors files from the XDG_DATA_DIRS, sorted by "Name".
    // If two color schemes, in user's $HOME and e.g. /usr, respectively, have the same
    // name, the one under $HOME overrides the other one
    const QStringList dirPaths =
        QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes"), QStandardPaths::LocateDirectory);
#else
    const QStringList dirPaths{QStringLiteral("assets:/share/color-schemes")};
#endif

    std::map<QString, QString> map;
    for (const QString &dirPath : dirPaths) {
        const QDir dir(dirPath);
        const QStringList fileNames = dir.entryList({QStringLiteral("*.colors")});
        for (const auto &file : fileNames) {
            map.insert({file, dir.filePath(file)});
        }
    }

    for (const auto &[key, schemeFilePath] : map) {
        KSharedConfigPtr config = KSharedConfig::openConfig(schemeFilePath, KConfig::SimpleConfig);
        KConfigGroup group(config, QStringLiteral("General"));
        const QString name = group.readEntry("Name", QFileInfo(schemeFilePath).baseName());
        const QString id = key.chopped(QLatin1String(".colors").size()); // Remove .colors ending
        const KColorSchemeModelData data = {id, name, schemeFilePath, QIcon()};
        d->m_data.append(data);
    }

    d->m_data.insert(0, {QStringLiteral("Default"), i18n("Default"), QString(), QIcon::fromTheme(QStringLiteral("edit-undo"))});
    endResetModel();
}

KColorSchemeModel::~KColorSchemeModel() = default;

int KColorSchemeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return d->m_data.count();
}

QVariant KColorSchemeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (index.row() >= d->m_data.count())) {
        return QVariant();
    }

    switch (role) {
    case NameRole:
        return d->m_data.at(index.row()).name;
    case IconRole: {
        auto &item = d->m_data[index.row()];
        if (item.preview.isNull()) {
            item.preview = KColorSchemeManagerPrivate::createPreview(item.path);
        }
        return item.preview;
    }
    case PathRole:
        return d->m_data.at(index.row()).path;
    case IdRole:
        return d->m_data.at(index.row()).id;
    default:
        return QVariant();
    }
}
