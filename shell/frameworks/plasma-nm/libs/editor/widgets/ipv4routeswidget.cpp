/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QNetworkAddressEntry>
#include <QStandardItem>
#include <QStandardItemModel>

#include <KAcceleratorManager>
#include <KLocalizedString>

#include "intdelegate.h"
#include "ipv4delegate.h"
#include "ipv4routeswidget.h"
#include "ui_ipv4routes.h"

class IpV4RoutesWidget::Private
{
public:
    Private()
        : model(0, 4)
    {
        auto headerItem = new QStandardItem(i18nc("Header text for IPv4 address", "Address"));
        model.setHorizontalHeaderItem(0, headerItem);
        headerItem = new QStandardItem(i18nc("Header text for IPv4 netmask", "Netmask"));
        model.setHorizontalHeaderItem(1, headerItem);
        headerItem = new QStandardItem(i18nc("Header text for IPv4 gateway", "Gateway"));
        model.setHorizontalHeaderItem(2, headerItem);
        headerItem = new QStandardItem(i18nc("Header text for IPv4 route metric", "Metric"));
        model.setHorizontalHeaderItem(3, headerItem);
    }
    Ui_RoutesIp4Config ui;
    QStandardItemModel model;
};

IpV4RoutesWidget::IpV4RoutesWidget(QWidget *parent)
    : QDialog(parent)
    , d(new IpV4RoutesWidget::Private())
{
    d->ui.setupUi(this);
    d->ui.tableViewAddresses->setModel(&d->model);
    d->ui.tableViewAddresses->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    auto ipDelegate = new IpV4Delegate(this);
    auto metricDelegate = new IntDelegate(this);
    d->ui.tableViewAddresses->setItemDelegateForColumn(0, ipDelegate);
    d->ui.tableViewAddresses->setItemDelegateForColumn(1, ipDelegate);
    d->ui.tableViewAddresses->setItemDelegateForColumn(2, ipDelegate);
    d->ui.tableViewAddresses->setItemDelegateForColumn(3, metricDelegate);

    connect(d->ui.pushButtonAdd, &QPushButton::clicked, this, &IpV4RoutesWidget::addRoute);
    connect(d->ui.pushButtonRemove, &QPushButton::clicked, this, &IpV4RoutesWidget::removeRoute);

    connect(d->ui.tableViewAddresses->selectionModel(), &QItemSelectionModel::selectionChanged, this, &IpV4RoutesWidget::selectionChanged);

    connect(&d->model, &QStandardItemModel::itemChanged, this, &IpV4RoutesWidget::tableViewItemChanged);

    connect(d->ui.buttonBox, &QDialogButtonBox::accepted, this, &IpV4RoutesWidget::accept);
    connect(d->ui.buttonBox, &QDialogButtonBox::rejected, this, &IpV4RoutesWidget::reject);

    KAcceleratorManager::manage(this);
}

IpV4RoutesWidget::~IpV4RoutesWidget()
{
    delete d;
}

void IpV4RoutesWidget::setNeverDefault(bool checked)
{
    d->ui.cbNeverDefault->setChecked(checked);
}

bool IpV4RoutesWidget::neverDefault() const
{
    return d->ui.cbNeverDefault->isChecked();
}

void IpV4RoutesWidget::setIgnoreAutoRoutes(bool checked)
{
    d->ui.cbIgnoreAutoRoutes->setChecked(checked);
}

void IpV4RoutesWidget::setIgnoreAutoRoutesCheckboxEnabled(bool enabled)
{
    d->ui.cbIgnoreAutoRoutes->setEnabled(enabled);
}

bool IpV4RoutesWidget::ignoreautoroutes() const
{
    return d->ui.cbIgnoreAutoRoutes->isChecked();
}

void IpV4RoutesWidget::setRoutes(const QList<NetworkManager::IpRoute> &list)
{
    d->model.removeRows(0, d->model.rowCount());
    for (const NetworkManager::IpRoute &route : list) {
        const QList<QStandardItem *> item{
            new QStandardItem(route.ip().toString()),
            new QStandardItem(route.netmask().toString()),
            new QStandardItem(route.nextHop().toString()),
            new QStandardItem(QString::number(route.metric(), 10)),
        };

        d->model.appendRow(item);
    }
}

QList<NetworkManager::IpRoute> IpV4RoutesWidget::routes()
{
    QList<NetworkManager::IpRoute> list;

    for (int i = 0, rowCount = d->model.rowCount(); i < rowCount; i++) {
        NetworkManager::IpRoute route;
        QStandardItem *item = d->model.item(i, 0);
        if (item) {
            route.setIp(QHostAddress(item->text()));
        }
        item = d->model.item(i, 1);
        if (item) {
            route.setNetmask(QHostAddress(item->text()));
        }
        item = d->model.item(i, 2);
        if (item) {
            route.setNextHop(QHostAddress(item->text()));
        }
        item = d->model.item(i, 3);
        if (item) {
            route.setMetric(item->text().toUInt());
        }

        list << route;
    }
    return list;
}

void IpV4RoutesWidget::addRoute()
{
    QList<QStandardItem *> item{new QStandardItem, new QStandardItem, new QStandardItem};
    d->model.appendRow(item);

    const int rowCount = d->model.rowCount();
    if (rowCount > 0) {
        d->ui.tableViewAddresses->selectRow(rowCount - 1);

        QItemSelectionModel *selectionModel = d->ui.tableViewAddresses->selectionModel();
        QModelIndexList list = selectionModel->selectedIndexes();
        if (list.size()) {
            // QTableView is configured to select only rows.
            // So, list[0] - IP address.
            d->ui.tableViewAddresses->edit(list[0]);
        }
    }
}

void IpV4RoutesWidget::removeRoute()
{
    QItemSelectionModel *selectionModel = d->ui.tableViewAddresses->selectionModel();
    if (selectionModel->hasSelection()) {
        QModelIndexList indexes = selectionModel->selectedIndexes();
        d->model.takeRow(indexes[0].row());
    }
    d->ui.pushButtonRemove->setEnabled(d->ui.tableViewAddresses->selectionModel()->hasSelection());
}

void IpV4RoutesWidget::selectionChanged(const QItemSelection &selected)
{
    // qCDebug(PLASMA_NM_EDITOR_LOG) << "selectionChanged";
    d->ui.pushButtonRemove->setEnabled(!selected.isEmpty());
}

extern quint32 suggestNetmask(quint32 ip);

void IpV4RoutesWidget::tableViewItemChanged(QStandardItem *item)
{
    if (item->text().isEmpty()) {
        return;
    }

    const int column = item->column();
    if (column == 0) { // ip
        int row = item->row();

        QStandardItem *netmaskItem = d->model.item(row, column + 1); // netmask
        if (netmaskItem && netmaskItem->text().isEmpty()) {
            QHostAddress addr(item->text());
            const quint32 netmask = suggestNetmask(addr.toIPv4Address());
            if (netmask) {
                QHostAddress v(netmask);
                netmaskItem->setText(v.toString());
            }
        }
    }
}

#include "moc_ipv4routeswidget.cpp"
