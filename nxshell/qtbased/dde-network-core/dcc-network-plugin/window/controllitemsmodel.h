// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONTROLLITEMSMODEL_H
#define CONTROLLITEMSMODEL_H

#include <QAbstractItemModel>

#include <DStyledItemDelegate>

class QModelIndex;
struct ControllItemsAction;

namespace dde {
namespace network {
class ControllItems;
class WiredDevice;
}
}
DWIDGET_BEGIN_NAMESPACE
class DSpinner;
DWIDGET_END_NAMESPACE

class ControllItemsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum DisplayRole{
        id,
        ssid,
    };
    explicit ControllItemsModel(QWidget *parent = nullptr);
    virtual ~ControllItemsModel() override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setParentWidget(QWidget *parentWidget);
    void setDisplayRole(DisplayRole role);

Q_SIGNALS:
    void detailClick(dde::network::ControllItems *conn, QWidget *parent);

public Q_SLOTS:
    void updateDate(QList<dde::network::ControllItems *> conns);
    void addConnection(QList<dde::network::ControllItems *> conns);
    void removeConnection(QList<dde::network::ControllItems *> conns);
    void updateStatus();
    void sortList();

private Q_SLOTS:

    void onDetailTriggered();

private:
    QList<ControllItemsAction *> m_data;
    QWidget *m_parentWidget;
    DisplayRole m_displayRole;
};

#endif // CONTROLLITEMSMODEL_H
