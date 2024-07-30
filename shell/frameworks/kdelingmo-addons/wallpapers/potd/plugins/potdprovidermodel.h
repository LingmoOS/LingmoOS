/*
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>

#include <KPluginMetaData>

/**
 * This class provides the list of PoTD providers.
 */
class PotdProviderModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        Id = Qt::UserRole + 1,
        NotSafeForWork, /**< Whether the provider may contain NSFW images **/
    };

    explicit PotdProviderModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE int indexOf(const QString &identifier);
    Q_INVOKABLE bool isNSFW(int row);

    void loadPluginMetaData();

private:
    std::vector<KPluginMetaData> m_providers;
};
