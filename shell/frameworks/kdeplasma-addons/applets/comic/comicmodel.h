/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008-2010 Matthias Fuchs <mat69@gmx.net>
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef COMICMODEL_H
#define COMICMODEL_H

#include <QAbstractTableModel>

#include "engine/comic.h"

class ComicModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    ComicModel(ComicEngine *engine, const QStringList &usedComics, QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &index = QModelIndex()) const override;
    int columnCount(const QModelIndex &index = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::CheckStateRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void load();

private:
    QList<ComicProviderInfo> mComics;
    QStringList mUsedComics;
    ComicEngine *mEngine;
};

#endif
