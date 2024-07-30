// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef PASSWORDSORTPROXYMODEL_H_
#define PASSWORDSORTPROXYMODEL_H_

#include <QSortFilterProxyModel>

namespace PlasmaPass
{
class PasswordSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit PasswordSortProxyModel(QObject *parent = nullptr);

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};

}

#endif
