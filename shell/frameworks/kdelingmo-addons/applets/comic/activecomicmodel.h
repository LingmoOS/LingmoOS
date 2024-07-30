/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef ACTIVE_COMIC_MODEL_H
#define ACTIVE_COMIC_MODEL_H

#include <QStandardItemModel>

class ActiveComicModel : public QStandardItemModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        ComicKeyRole = Qt::UserRole + 1,
        ComicTitleRole = Qt::UserRole + 2,
        ComicIconRole = Qt::UserRole + 3,
        ComicHighlightRole = Qt::UserRole + 4,
    };

    explicit ActiveComicModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;

    void addComic(const QString &key, const QString &title, const QIcon &icon, bool highlight = true);

    int count()
    {
        return rowCount(QModelIndex());
    }

    Q_INVOKABLE QVariantHash get(int i) const;

Q_SIGNALS:
    void countChanged();
};

#endif
