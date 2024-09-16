@page dtkmount dtkmount
@brief dtk 挂载业务模块

# 项目介绍

DtkMount是对deepin/UOS系统的块设备业务接口的封装，为应用层开发者提供了对单个块设备的管理和操作功能，例如监听、挂载、卸载等，以及对系统中的设备统一管理的功能，例如获取设备列表、创建块设备对象等。 其目的在于帮助开发者便捷的调用接口进行开发。

@ref group_dtkmount "接口文档"


## 项目结构

对外暴露出`dblockdevice.h`、 `dblockdevicemonitor.h`、`dblockpartition.h`、`ddevicemanager.h`、`ddiskdrive.h`、`ddiskjob.h`、`dmounttypes.h`、`dprotocoldevice.h`、`dprotocoldevicemonitor.h`和`dtkmount_global.h`这10个头文件

# 如何使用项目

如果要使用此项目，可以阅读相关帮助文档再参考以下实例进行调用

## 使用示例

`devicedef.h`

```cpp
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DEVICEDEF_H
#define DEVICEDEF_H

#include <QMetaType>

struct DeviceData
{
    bool isMount;
    quint64 size;
    quint64 usedSize;
    QString deviceName;
    QString devicePath;
    QString mountPoint;
};

Q_DECLARE_METATYPE(DeviceData)
#endif  // DEVICEDEF_H
```

`itemdelegate.h`

```cpp
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ITEMDELEGATE_H
#define ITEMDELEGATE_H

#include <QModelIndex>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

class ItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

Q_SIGNALS:

public:
    explicit ItemDelegate(QObject *parent = nullptr);
    ~ItemDelegate();

    // 重写绘画函数
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    static QStringList m_unit;
};

#endif   // ITEMDELEGATE_H

```

`itemdelegate.cpp`

```cpp
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "itemdelegate.h"
#include "devicedef.h"

#include <DApplication>
#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyle>
#include <QStyleOptionProgressBar>
#include <QStyledItemDelegate>

QStringList ItemDelegate::m_unit{" B", " KB", " MB", " GB", " TB", " PB", tr("Invalid Size")};

ItemDelegate::ItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

ItemDelegate::~ItemDelegate() {}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();

        QVariant variant = index.data(Qt::UserRole);
        DeviceData data = variant.value<DeviceData>();

        QStyleOptionViewItem viewOption(option);

        const QRectF &rect(option.rect);

        const QRect &iconRect = QRect(rect.left() + 10, rect.top() + 10, 80, 80);
        const QRect &nameRect = QRect(rect.left() + 60, rect.top() + 15, rect.width() - 40, 20);
        const QRect &sizeTextRect = QRect(rect.left() + 60, rect.top() + 40, rect.width() - 40, 20);
        const QRect &unmountTextRect = QRect(rect.left() + 60, rect.bottom() - 40, rect.width() - 40, 20);
        const QRect &sizeBarRect = QRect(rect.left() + 100, rect.bottom() - 35, rect.width() - 110, 10);

        painter->drawPixmap(iconRect, QIcon::fromTheme(QString::fromUtf8("drive-removable-media")).pixmap(iconRect.size()));

        painter->setPen(QPen(Qt::black));
        qDebug() << data.mountPoint;
        if (data.mountPoint.size() == 1 && data.mountPoint.at(0) == '/') {
            data.deviceName = tr("System Device");
        }

        int sizeUnit = 0;                                    // 总容量单位
        quint64 usedSize = data.usedSize, size = data.size;  // 用于临时容量换算

        while ((size >>= 10) >= 1024)
            ++sizeUnit;
        sizeUnit > 0 ? ++sizeUnit : sizeUnit;  // 计算单位

        if (sizeUnit > 5) {
            qDebug() << "Invalid Size";
            sizeUnit = 6;
        }
        if (data.deviceName.isEmpty()) {
            data.deviceName = QString::number(size) + m_unit[sizeUnit] + tr(" Volume");
        }
        painter->drawText(nameRect, Qt::AlignHCenter, data.deviceName);

        if (data.isMount) {
            int usedSizeUnit = 0;  // 已使用容量单位

            while ((usedSize >>= 10) >= 1024)
                ++usedSizeUnit;
            usedSizeUnit > 0 ? ++usedSizeUnit : usedSizeUnit;

            if (usedSizeUnit > 5) {
                qDebug() << "Invalid Size";
                usedSizeUnit = 6;
            }
            painter->drawText(sizeTextRect,
                              Qt::AlignHCenter,
                              QString::number(usedSize) + m_unit[usedSizeUnit] + " / " + QString::number(size) +
                                  m_unit[sizeUnit]);

            QStyleOptionProgressBar bar;

            bar.rect = sizeBarRect;
            bar.progress = data.usedSize * 100.0 / data.size;
            bar.minimum = 0;
            bar.maximum = 100;

            QApplication::style()->drawControl(QStyle::CE_ProgressBar, &bar, painter);
        } else {
            painter->drawText(unmountTextRect, Qt::AlignHCenter, tr("Unmounted"));
        }

        painter->restore();
    }
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(250, 100);
}

```

`sortproxy.h`

```cpp
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SORTPROXY_H
#define SORTPROXY_H

#include <QSortFilterProxyModel>

class SortProxy : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    SortProxy(QObject *parent = nullptr);
    ~SortProxy() override;

protected:
    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;
};
#endif   // SORTPROXY_H

```

`sortproxy.cpp`

```cpp
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sortproxy.h"
#include "devicedef.h"
#include <QDebug>

SortProxy::SortProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

SortProxy::~SortProxy() {}

bool SortProxy::lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const
{
    QVariant variant = sourceLeft.data(Qt::UserRole);
    DeviceData dataLeft = variant.value<DeviceData>();
    variant = sourceRight.data(Qt::UserRole);
    DeviceData dataRight = variant.value<DeviceData>();

    if (dataLeft.isMount == dataRight.isMount) {
        return dataLeft.deviceName.toLower() <= dataRight.deviceName.toLower();
    } else if (dataLeft.isMount == true && dataRight.isMount == false) {
        return true;
    }
    return false;
}
```

`window.h`

```cpp
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WINDOW_H
#define WINDOW_H

#include "itemdelegate.h"
#include "sortproxy.h"
#include <DMenu>
#include <DMenuBar>
#include <DScrollArea>
#include <DWidget>
#include <QHBoxLayout>
#include <QListView>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class Window : public DWidget
{
    Q_OBJECT
public:
    explicit Window(DWidget *parent = nullptr);
    ~Window();

private:
    SortProxy *m_sortProxy = nullptr;
    QStandardItemModel *m_model = nullptr;

    void initItems();

private Q_SLOTS:
    void deviceMountAdd(const QString &devPath, const QString &mountPoint);
    void deviceMountRemove(const QString &devPath);
    void deviceAdd(const QString &devPath);
    void deviceRemove(const QString &devPath);
};

#endif   // WINDOW_H

```

`window.cpp`

```cpp
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

```

`main.cpp`

```cpp
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "window.h"
#include <DApplication>
#include <QTranslator>
#include <memory>

int main(int argc, char *argv[])
{
    std::unique_ptr<DApplication> a(DApplication::globalApplication(argc, argv));
    DApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    a->loadTranslator();

    Window w;
    w.show();
    return a->exec();
}

```

@defgroup dtkmount
@brief 挂载业务模块
@details 示例文档:
@subpage dtkmount

@anchor group_dtkmount