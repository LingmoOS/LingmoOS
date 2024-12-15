// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WirelessDeviceModel_H
#define WirelessDeviceModel_H

#include <QAbstractItemModel>

#include <DStyledItemDelegate>

class QModelIndex;
struct ItemAction;

namespace ocean {
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
    explicit WirelessDeviceModel(const ocean::network::WirelessDevice *dev, QWidget *parent = nullptr);
    virtual ~WirelessDeviceModel() override;

    QModelIndex index(const ocean::network::AccessPoints *ap);
    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

Q_SIGNALS:
    void detailClick(ocean::network::AccessPoints *ap, QWidget *parent);

public Q_SLOTS:
    void updateApStatus();
    void sortAPList();

private Q_SLOTS:
    void addAccessPoints(QList<ocean::network::AccessPoints *> aps);
    void removeAccessPoints(QList<ocean::network::AccessPoints *> aps);

    void onDetailTriggered();

private:
    QList<ItemAction *> m_data;
    const ocean::network::WirelessDevice *m_device;
    QWidget *m_parent;
    ItemAction *m_hioceannItem;
};

#endif // WirelessDeviceModel_H
