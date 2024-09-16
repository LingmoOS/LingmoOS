// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WirelessDeviceModel_H
#define WirelessDeviceModel_H

#include <QAbstractItemModel>

#include <DStyledItemDelegate>

class QModelIndex;
struct ItemAction;

namespace dde {
namespace network {
class AccessPoints;
class WirelessDevice;
}
}
DWIDGET_BEGIN_NAMESPACE
class DSpinner;
DWIDGET_END_NAMESPACE

class WirelessDeviceModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit WirelessDeviceModel(const dde::network::WirelessDevice *dev, QWidget *parent = nullptr);
    virtual ~WirelessDeviceModel() override;

    QModelIndex index(const dde::network::AccessPoints *ap);
    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

Q_SIGNALS:
    void detailClick(dde::network::AccessPoints *ap, QWidget *parent);

public Q_SLOTS:
    void updateApStatus();
    void sortAPList();

private Q_SLOTS:
    void addAccessPoints(QList<dde::network::AccessPoints *> aps);
    void removeAccessPoints(QList<dde::network::AccessPoints *> aps);

    void onDetailTriggered();

private:
    QList<ItemAction *> m_data;
    const dde::network::WirelessDevice *m_device;
    QWidget *m_parent;
    ItemAction *m_hiddenItem;
};

#endif // WirelessDeviceModel_H
