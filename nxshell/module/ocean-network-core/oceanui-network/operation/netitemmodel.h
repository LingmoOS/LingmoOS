// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef NETITEMMODEL_H
#define NETITEMMODEL_H

#include <QSortFilterProxyModel>

namespace ocean {
namespace network {
class NetItem;

class NetItemModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(NetItem * root READ root WRITE setRoot NOTIFY rootChanged)
public:
    explicit NetItemModel(QObject *parent = nullptr);
    ~NetItemModel() override;

    NetItem *root() const;
    void setRoot(NetItem *root);

    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void rootChanged(NetItem *root);

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};
} // namespace network
} // namespace ocean

#endif // NETITEMMODEL_H
