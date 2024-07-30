/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "discoverwidget.h"
#include "bluedevil_sendfile.h"

#include <QAction>
#include <QIcon>
#include <QLabel>
#include <QListView>
#include <QListWidgetItem>
#include <QSortFilterProxyModel>
#include <QTimer>

#include <KMessageWidget>

#include <BluezQt/Adapter>
#include <BluezQt/Device>
#include <BluezQt/DevicesModel>
#include <BluezQt/Manager>
#include <BluezQt/Services>

class DevicesProxyModel : public QSortFilterProxyModel
{
public:
    explicit DevicesProxyModel(QObject *parent);

    void setDevicesModel(BluezQt::DevicesModel *model);

    QVariant data(const QModelIndex &index, int role) const override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    BluezQt::DevicePtr device(const QModelIndex &index) const;

private:
    BluezQt::DevicesModel *m_devicesModel = nullptr;
};

DevicesProxyModel::DevicesProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::DescendingOrder);
}

void DevicesProxyModel::setDevicesModel(BluezQt::DevicesModel *model)
{
    m_devicesModel = model;
    setSourceModel(model);
}

QVariant DevicesProxyModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DecorationRole:
        return QIcon::fromTheme(index.data(BluezQt::DevicesModel::IconRole).toString(), QIcon::fromTheme(QStringLiteral("preferences-system-bluetooth")));

    default:
        return QSortFilterProxyModel::data(index, role);
    }
}

bool DevicesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    const QStringList &uuids = index.data(BluezQt::DevicesModel::UuidsRole).toStringList();
    bool adapterPowered = index.data(BluezQt::DevicesModel::AdapterPoweredRole).toBool();

    return adapterPowered && uuids.contains(BluezQt::Services::ObexObjectPush);
}

BluezQt::DevicePtr DevicesProxyModel::device(const QModelIndex &index) const
{
    Q_ASSERT(m_devicesModel);
    return m_devicesModel->device(mapToSource(index));
}

DiscoverWidget::DiscoverWidget(BluezQt::Manager *manager, QWidget *parent)
    : QWidget(parent)
    , m_manager(manager)
    , m_warningWidget(nullptr)
{
    setupUi(this);

    m_model = new DevicesProxyModel(this);
    m_model->setDevicesModel(new BluezQt::DevicesModel(m_manager, this));
    devices->setModel(m_model);

    checkAdapters();
    connect(m_manager, &BluezQt::Manager::adapterAdded, this, &DiscoverWidget::checkAdapters);
    connect(m_manager, &BluezQt::Manager::adapterChanged, this, &DiscoverWidget::checkAdapters);
    connect(m_manager, &BluezQt::Manager::usableAdapterChanged, this, &DiscoverWidget::checkAdapters);
    connect(m_manager, &BluezQt::Manager::bluetoothBlockedChanged, this, &DiscoverWidget::checkAdapters);

    connect(devices->selectionModel(), &QItemSelectionModel::currentChanged, this, &DiscoverWidget::indexSelected);
}

void DiscoverWidget::indexSelected(const QModelIndex index)
{
    Q_EMIT deviceSelected(m_model->device(index));
}

void DiscoverWidget::checkAdapters()
{
    bool error = false;

    Q_FOREACH (BluezQt::AdapterPtr adapter, m_manager->adapters()) {
        if (!adapter->isPowered()) {
            error = true;
            break;
        }
    }

    delete m_warningWidget;
    m_warningWidget = nullptr;

    if (!error && !m_manager->isBluetoothBlocked()) {
        return;
    }

    m_warningWidget = new KMessageWidget(this);
    m_warningWidget->setMessageType(KMessageWidget::Warning);
    m_warningWidget->setCloseButtonVisible(false);
    QString fixBluetoothButtonString;

    if (m_manager->isBluetoothBlocked()) {
        m_warningWidget->setText(i18n("Bluetooth is disabled."));
        fixBluetoothButtonString = i18nc("Action to enable Bluetooth adapter", "Enable");
    } else {
        m_warningWidget->setText(i18n("Your Bluetooth adapter is powered off."));
        fixBluetoothButtonString = i18nc("Action to turn on Bluetooth adapter", "Turn On");
    }

    QAction *fixAdapters = new QAction(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")), fixBluetoothButtonString, m_warningWidget);
    connect(fixAdapters, &QAction::triggered, this, &DiscoverWidget::fixAdaptersError);
    m_warningWidget->addAction(fixAdapters);
    verticalLayout->insertWidget(0, m_warningWidget);
}

void DiscoverWidget::fixAdaptersError()
{
    m_manager->setBluetoothBlocked(false);

    Q_FOREACH (BluezQt::AdapterPtr adapter, m_manager->adapters()) {
        adapter->setPowered(true);
    }
}

#include "moc_discoverwidget.cpp"
