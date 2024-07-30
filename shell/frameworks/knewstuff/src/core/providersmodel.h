/*
    SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KNSCORE_PROVIDERSMODELL_H
#define KNSCORE_PROVIDERSMODELL_H

#include <QAbstractListModel>

#include "enginebase.h"

#include "knewstuffcore_export.h"

#include <memory>

namespace KNSCore
{
class ProvidersModelPrivate;
/**
 * @brief A model which holds information on all known Providers for a specific Engine
 *
 * @since 5.85
 */
class KNEWSTUFFCORE_EXPORT ProvidersModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The Engine for which this model displays Providers
     */
    Q_PRIVATE_PROPERTY(d, EngineBase *engine READ getEngine WRITE setEngine NOTIFY engineChanged)
public:
    explicit ProvidersModel(QObject *parent = nullptr);
    ~ProvidersModel() override;

    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        VersionRole,
        WebsiteRole,
        HostRole,
        ContactEmailRole,
        SupportsSslRole,
        IconRole,
        ObjectRole, ///< The actual Provider object. Do not hold this locally and expect it to disappear at a moment's notice
    };
    Q_ENUM(Roles)

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // TODO KF7: Port property to back to public getter/setter, narrow types to EngineBase
    QObject *engine() const;
    void setEngine(QObject *engine);
    Q_SIGNAL void engineChanged();

private:
    std::unique_ptr<ProvidersModelPrivate> d;
};
}

#endif // KNSCORE_PROVIDERSMODELL_H
