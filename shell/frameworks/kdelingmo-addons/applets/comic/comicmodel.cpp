/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008-2010 Matthias Fuchs <mat69@gmx.net>
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "comicmodel.h"
#include "engine/comicprovider.h"

#include <QDebug>
#include <QIcon>

ComicModel::ComicModel(ComicEngine *engine, const QStringList &usedComics, QObject *parent)
    : QAbstractTableModel(parent)
    , mUsedComics(usedComics)
    , mEngine(engine)
{
    Q_ASSERT(engine);

    load();
}

QHash<int, QByteArray> ComicModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[Qt::DecorationRole] = "decoration";
    roles[Qt::UserRole] = "plugin";
    return roles;
}

int ComicModel::rowCount(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return mComics.count();
    }

    return 0;
}

int ComicModel::columnCount(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return 2;
}

QVariant ComicModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= mComics.count()) {
        return QVariant();
    }

    const ComicProviderInfo &info = mComics.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        return info.name;
    case Qt::DecorationRole:
        return QIcon::fromTheme(info.icon);
    case Qt::UserRole:
        return info.pluginId;
    }

    return QVariant();
}

Qt::ItemFlags ComicModel::flags(const QModelIndex &index) const
{
    if (index.isValid() && (index.column() == 0)) {
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void ComicModel::load()
{
    beginResetModel();
    mComics = mEngine->loadProviders();
    endResetModel();
}
