// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "window.h"
#include "devicedef.h"
#include "itemdelegate.h"
#include <DBlockDevice>
#include <DBlockDeviceMonitor>
#include <DDeviceManager>
#include <QDebug>
#include <QIcon>
#include <QStorageInfo>

DWIDGET_USE_NAMESPACE

Window::Window(DWidget *parent)
    : DWidget(parent)
{
    this->resize(800, 500);
    this->setMinimumSize(QSize(350, 200));
    this->setMaximumSize(QSize(1350, 720));
    this->setWindowTitle("Dtkmount");

    QAction *ShowMounted = new QAction(this);
    QAction *ShowUnmounted = new QAction(this);
    QAction *ShowAll = new QAction(this);
    QAction *Exit = new QAction(this);
    DMenuBar *Menubar = new QMenuBar(this);
    DMenu *Menu = new QMenu(Menubar);

    ShowMounted->setText(tr("ShowMounted"));
    ShowUnmounted->setText(tr("ShowUnmounted"));
    ShowAll->setText(tr("ShowAll"));
    Exit->setText(tr("Exit"));
    Menu->setTitle(tr("Menu"));

    Menubar->addAction(Menu->menuAction());
    Menu->addAction(ShowAll);
    Menu->addAction(ShowMounted);
    Menu->addAction(ShowUnmounted);
    Menu->addAction(Exit);

    QVBoxLayout *WindowVerticalLayout = new QVBoxLayout(this);
    WindowVerticalLayout->addWidget(Menubar);

    QListView *itemDisplay = new QListView(this);

    WindowVerticalLayout->addWidget(itemDisplay);

    Dtk::Mount::DBlockDeviceMonitor *monitor = Dtk::Mount::DDeviceManager::globalBlockDeviceMonitor();
    monitor->setWatchChanges(true);

    connect(monitor, &Dtk::Mount::DBlockDeviceMonitor::mountAdded, this, &Window::deviceMountAdd, Qt::QueuedConnection);
    connect(monitor, &Dtk::Mount::DBlockDeviceMonitor::mountRemoved, this, &Window::deviceMountRemove, Qt::QueuedConnection);
    connect(monitor, &Dtk::Mount::DBlockDeviceMonitor::deviceAdded, this, &Window::deviceAdd, Qt::QueuedConnection);
    connect(monitor, &Dtk::Mount::DBlockDeviceMonitor::deviceRemoved, this, &Window::deviceRemove, Qt::QueuedConnection);

    connect(ShowAll, &QAction::triggered, this, [this]() {
        if (m_sortProxy) {
            m_sortProxy->setFilterFixedString(QString());
        }
    });
    connect(ShowMounted, &QAction::triggered, this, [this]() {
        if (m_sortProxy) {
            m_sortProxy->setFilterFixedString(QString("true"));
        }
    });
    connect(ShowUnmounted, &QAction::triggered, this, [this]() {
        if (m_sortProxy) {
            m_sortProxy->setFilterFixedString(QString("false"));
        }
    });
    connect(Exit, &QAction::triggered, this, [this]() { this->close(); });

    m_model = new QStandardItemModel();

    ItemDelegate *delegate = new ItemDelegate(this);

    m_sortProxy = new SortProxy(itemDisplay);
    m_sortProxy->setSourceModel(m_model);
    m_sortProxy->setSortRole(Qt::UserRole);
    m_sortProxy->setFilterRole(Qt::UserRole + 1);
    m_sortProxy->setDynamicSortFilter(true);

    itemDisplay->setItemDelegate(delegate);
    itemDisplay->setSpacing(10);
    itemDisplay->setGridSize(QSize(250, 100));

    itemDisplay->setModel(m_sortProxy);
    itemDisplay->setViewMode(QListView::IconMode);
    itemDisplay->setFlow(QListView::LeftToRight);
    itemDisplay->setWrapping(true);
    itemDisplay->setResizeMode(QListView::Adjust);
    itemDisplay->setDragEnabled(false);
    itemDisplay->setEditTriggers(QAbstractItemView::NoEditTriggers);
    itemDisplay->setSelectionMode(QListView::NoSelection);

    initItems();
}

Window::~Window() {}

void Window::initItems()
{
    DExpected<QStringList> deviceList = Dtk::Mount::DDeviceManager::blockDevices();

    if (!deviceList.hasValue()) {
        return;
    }

    for (QString devPath : deviceList.value()) {
        DExpected<Dtk::Mount::DBlockDevice *> device(Dtk::Mount::DDeviceManager::createBlockDevice(devPath, this));
        if (!device.hasValue() || device.value()->isLoopDevice() || device.value()->size() == 0 || !device.value()->hasFileSystem())
            continue;

        QStandardItem *Item = new QStandardItem;
        DeviceData deviceData;

        deviceData.deviceName = device.value()->idLabel();
        deviceData.size = device.value()->size();
        deviceData.devicePath = devPath;

        QByteArrayList mountPoints(device.value()->mountPoints());
        if (QString firstMountPoint; mountPoints.size() > 0) {
            firstMountPoint = mountPoints.at(0);
            deviceData.mountPoint = firstMountPoint;
            deviceData.isMount = true;
            deviceData.usedSize = deviceData.size - (new QStorageInfo(firstMountPoint))->bytesAvailable();
        } else {
            deviceData.isMount = false;
        }

        Item->setData(QVariant::fromValue(deviceData), Qt::UserRole);
        Item->setData(deviceData.isMount, Qt::UserRole + 1);
        m_model->appendRow(Item);
    }
    m_sortProxy->sort(0);
}

void Window::deviceMountAdd(const QString &devPath, const QString &mountPoint)
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i, 0);
        QVariant variant = index.data(Qt::UserRole);
        DeviceData data = variant.value<DeviceData>();
        if (data.devicePath == devPath) {
            data.mountPoint = mountPoint;
            data.isMount = true;
            data.usedSize = data.size - (new QStorageInfo(mountPoint))->bytesAvailable();
            variant.setValue<DeviceData>(data);
            m_model->setData(index, variant, Qt::UserRole);
            m_model->setData(index, QVariant(data.isMount), Qt::UserRole + 1);
            return;
        }
    }
}

void Window::deviceMountRemove(const QString &devPath)
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i, 0);
        QVariant variant = index.data(Qt::UserRole);
        DeviceData data = variant.value<DeviceData>();
        if (data.devicePath == devPath) {
            data.mountPoint = "";
            data.isMount = false;
            data.usedSize = 0;
            variant.setValue<DeviceData>(data);
            m_model->setData(index, variant, Qt::UserRole);
            m_model->setData(index, QVariant(data.isMount), Qt::UserRole + 1);
            return;
        }
    }
}

void Window::deviceAdd(const QString &devPath)
{
    DExpected<Dtk::Mount::DBlockDevice *> device(Dtk::Mount::DDeviceManager::createBlockDevice(devPath, this));
    if (!device.hasValue() || device.value()->isLoopDevice() || device.value()->size() == 0)
        return;

    QStandardItem *Item = new QStandardItem;
    DeviceData deviceData;

    deviceData.deviceName = device.value()->idLabel();
    deviceData.size = device.value()->size();
    deviceData.devicePath = devPath;
    deviceData.isMount = false;
    deviceData.mountPoint = "";

    Item->setData(QVariant::fromValue(deviceData), Qt::UserRole);
    m_model->appendRow(Item);
}

void Window::deviceRemove(const QString &devPath)
{
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex index = m_model->index(i, 0);
        QVariant variant = index.data(Qt::UserRole);
        DeviceData data = variant.value<DeviceData>();
        if (data.devicePath == devPath) {
            m_model->removeRow(i);
            return;
        }
    }
}
