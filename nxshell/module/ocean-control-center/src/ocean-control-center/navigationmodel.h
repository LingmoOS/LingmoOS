// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef NAVIGATIONMODEL_H
#define NAVIGATIONMODEL_H

#include <QAbstractItemModel>

namespace oceanuiV25 {
class OceanUIObject;

class NavigationModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit NavigationModel(QObject *parent = nullptr);
    void setNavigationObject(const QVector<OceanUIObject *> &objs);

    enum OceanUIModelRole { NavTypeRole = Qt::UserRole + 300, NavUrlRole };

    QHash<int, QByteArray> roleNames() const override;
    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex &parentIndex = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    QVector<OceanUIObject *> m_data;
};
} // namespace oceanuiV25

#endif // NAVIGATIONMODEL_H