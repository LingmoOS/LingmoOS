/*
    SPDX-FileCopyrightText: 2013-2018 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef LINGMO_NM_EDITOR_PROXY_MODEL_H
#define LINGMO_NM_EDITOR_PROXY_MODEL_H

#include "lingmonm_internal_export.h"

#include <QSortFilterProxyModel>

#include <qqmlregistration.h>

class LINGMONM_INTERNAL_EXPORT EditorProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QAbstractItemModel *sourceModel READ sourceModel WRITE setSourceModel)
public:
    explicit EditorProxyModel(QObject *parent = nullptr);
    ~EditorProxyModel() override;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

#endif // LINGMO_NM_EDITOR_PROXY_MODEL_H
