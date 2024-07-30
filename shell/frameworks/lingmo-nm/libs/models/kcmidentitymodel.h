/*
    SPDX-FileCopyrightText: 2016-2018 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef LINGMO_NM_KCM_IDENTITY_MODEL_H
#define LINGMO_NM_KCM_IDENTITY_MODEL_H

#include "lingmonm_internal_export.h"

#include <QIdentityProxyModel>
#include <QModelIndex>

#include <qqmlregistration.h>

class LINGMONM_INTERNAL_EXPORT KcmIdentityModel : public QIdentityProxyModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    explicit KcmIdentityModel(QObject *parent = nullptr);
    ~KcmIdentityModel() override;

    enum KcmItemRole {
        KcmConnectionIconRole = Qt::UserRole + 100,
        KcmConnectionTypeRole,
        KcmVpnConnectionExportable,
    };

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
};

#endif // LINGMO_NM_KCM_IDENTITY_MODEL_H
