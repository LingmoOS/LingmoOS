/*
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef CATEGORIESMODEL_H
#define CATEGORIESMODEL_H

#include <QAbstractListModel>

#include "enginebase.h"

/**
 * @short A model which shows the categories found in an Engine
 * @since 5.63
 */
class CategoriesModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit CategoriesModel(KNSCore::EngineBase *parent);
    ~CategoriesModel() override;

    enum Roles {
        NameRole = Qt::UserRole + 1,
        IdRole,
        DisplayNameRole,
    };
    Q_ENUM(Roles)

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Get the display name for the category with the id passed to the function
     *
     * @param id The ID of the category you want to get the display name for
     * @return The display name (or the translated string "Unknown Category" for the requested category
     */
    Q_INVOKABLE QString idToDisplayName(const QString &id) const;

private:
    KNSCore::EngineBase *const m_engine;
};

#endif // CATEGORIESMODEL_H
