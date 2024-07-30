/*
    SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SEARCHPRESETMODEL_H
#define SEARCHPRESETMODEL_H

#include "enginebase.h"

#include <QAbstractListModel>

/**
 * @brief The SearchPresetModel class
 *
 * this class handles search presets.
 * @since 5.83
 */
class SearchPresetModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SearchPresetModel(KNSCore::EngineBase *parent);
    ~SearchPresetModel() override;

    enum Roles {
        DisplayNameRole = Qt::UserRole + 1,
        IconRole,
    };
    Q_ENUM(Roles)

    QHash<int, QByteArray> roleNames() const override;

    QVariant data(const QModelIndex &index, int role = DisplayNameRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE void loadSearch(const QModelIndex &index);

private:
    KNSCore::EngineBase *const m_engine;
};

#endif // SEARCHPRESETMODEL_H
