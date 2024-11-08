/*
    SPDX-FileCopyrightText: 2022 Jiří Wolker <woljiri@gmail.com>
    SPDX-FileCopyrightText: 2022 Eugene Popov <popov895@ukr.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>
#include <QIcon>
#include <QUrl>

class RecentItemsModel : public QAbstractListModel
{
public:
    explicit RecentItemsModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void refresh(const QList<QUrl> &urls);
    QUrl url(const QModelIndex &index) const;

private:
    struct RecentItemInfo {
        QIcon icon;
        QString name;
        QUrl url;
    };

    std::vector<RecentItemInfo> m_recentItems;
};
