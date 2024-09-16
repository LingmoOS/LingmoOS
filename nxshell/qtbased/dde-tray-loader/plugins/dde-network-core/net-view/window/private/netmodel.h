// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef NETMODEL_H
#define NETMODEL_H

#include <QAbstractItemModel>
#define CUSTOMROLE (Qt::UserRole + 100)

#define SORTROLE (CUSTOMROLE)
#define IDROLE (CUSTOMROLE + 1)
#define TYPEROLE (CUSTOMROLE + 2)

namespace dde {
namespace network {

class NetItem;

class NetModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit NetModel(QObject *parent = nullptr);
    ~NetModel() override;

    void setRoot(NetItem *object);
    NetItem *toObject(const QModelIndex &index);

    // inherited from QAbstractItemModel
    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parentIndex = QModelIndex()) const override;
    QModelIndex index(const NetItem *object);
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    void connectObject(const NetItem *obj);
    void disconnectObject(const NetItem *obj);

public Q_SLOTS:
    void updateObject();
    void AboutToAddObject(const NetItem *parent, int pos);
    void addObject(const NetItem *child);
    void AboutToRemoveObject(const NetItem *parent, int pos);
    void removeObject(const NetItem *child);

private:
    NetItem *m_treeRoot;
};

} // namespace network
} // namespace dde
#endif // NETMODEL_H
