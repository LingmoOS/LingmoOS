/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "activecomicmodel.h"

ActiveComicModel::ActiveComicModel(QObject *parent)
    : QStandardItemModel(0, 1, parent)
{
    connect(this, &ActiveComicModel::modelReset, this, &ActiveComicModel::countChanged);
    connect(this, &ActiveComicModel::rowsInserted, this, &ActiveComicModel::countChanged);
    connect(this, &ActiveComicModel::rowsRemoved, this, &ActiveComicModel::countChanged);
}

QHash<int, QByteArray> ActiveComicModel::roleNames() const
{
    auto roleNames = QStandardItemModel::roleNames();
    roleNames.insert(ComicKeyRole, "key");
    roleNames.insert(ComicTitleRole, "title");
    roleNames.insert(ComicIconRole, "icon");
    roleNames.insert(ComicHighlightRole, "highlight");

    return roleNames;
}

void ActiveComicModel::addComic(const QString &key, const QString &title, const QIcon &icon, bool highlight)
{
    QList<QStandardItem *> newRow;
    QStandardItem *item = new QStandardItem(title);

    item->setData(key, ComicKeyRole);
    item->setData(title, ComicTitleRole);
    item->setData(icon, ComicIconRole);
    item->setData(highlight, ComicHighlightRole);

    newRow << item;
    appendRow(newRow);
}

QVariantHash ActiveComicModel::get(int row) const
{
    QModelIndex idx = index(row, 0);
    QVariantHash hash;

    const auto roleNames = this->roleNames();
    hash.reserve(roleNames.size());
    for (auto end = roleNames.constEnd(), it = roleNames.constBegin(); it != end; ++it) {
        hash.insert(QString::fromUtf8(it.value()), data(idx, it.key()));
    }

    return hash;
}
