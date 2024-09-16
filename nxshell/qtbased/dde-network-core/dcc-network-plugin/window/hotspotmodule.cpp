// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "controllitemsmodel.h"
#include "hotspotmodule.h"
#include "editpage/connectionhotspoteditpage.h"
#include "widgets/floatingbutton.h"

#include <DFloatingButton>
#include <DFontSizeManager>
#include <DListView>

#include <QHBoxLayout>
#include <QApplication>
#include <QPushButton>

#include <widgets/switchwidget.h>
#include <widgets/widgetmodule.h>

#include <wirelessdevice.h>
#include <networkcontroller.h>
#include <hotspotcontroller.h>

using namespace dde::network;
using namespace DCC_NAMESPACE;
DWIDGET_USE_NAMESPACE

HotspotDeviceItem::HotspotDeviceItem(WirelessDevice *dev, QObject *parent)
    : QObject(parent)
    , m_device(dev)
{
    m_modules.append(new WidgetModule<SwitchWidget>("hotspotSwitch", tr("Hotspot"), [this](SwitchWidget *hotspotSwitch) {
        QLabel *lblTitle = new QLabel(tr("Hotspot")); // 个人热点
        DFontSizeManager::instance()->bind(lblTitle, DFontSizeManager::T5, QFont::DemiBold);
        hotspotSwitch->setLeftWidget(lblTitle);
        hotspotSwitch->setChecked(NetworkController::instance()->hotspotController()->enabled(m_device));

        connect(hotspotSwitch, &SwitchWidget::checkedChanged, this, &HotspotDeviceItem::onSwitchToggled);

        connect(m_device, &WirelessDevice::hotspotEnableChanged, hotspotSwitch, [hotspotSwitch, this](const bool &enable) {
            hotspotSwitch->blockSignals(true);
            hotspotSwitch->setChecked(NetworkController::instance()->hotspotController()->enabled(m_device));
            hotspotSwitch->blockSignals(false);
        });
        connect(NetworkController::instance()->hotspotController(), &HotspotController::enableHotspotSwitch, hotspotSwitch, &SwitchWidget::setEnabled);
    }));
    m_modules.append(new WidgetModule<DListView>("list_hotspot", QString(), this, &HotspotDeviceItem::initHotspotList));
    m_modules.append(new WidgetModule<QPushButton>("hotspot_createBtn", tr("Add Settings"), [this](QPushButton *createBtn) {
        createBtn->setText(tr("Add Settings"));
        connect(createBtn, &QPushButton::clicked, this, [this]() {
            openEditPage(nullptr, qobject_cast<QWidget *>(sender()));
        });
    }));
}

void HotspotDeviceItem::initHotspotList(DListView *lvProfiles)
{
    ControllItemsModel *model = new ControllItemsModel(lvProfiles);
    model->setDisplayRole(ControllItemsModel::ssid);
    auto updateItems = [model, this]() {
        HotspotController *hotspotController = NetworkController::instance()->hotspotController();
        QList<HotspotItem *> conns = hotspotController->items(m_device);
        QList<ControllItems *> items;
        for (HotspotItem *it : conns) {
            items.append(it);
        }
        model->updateDate(items);
    };
    updateItems();
    HotspotController *hotspotController = NetworkController::instance()->hotspotController();
    connect(hotspotController, &HotspotController::itemAdded, model, updateItems);
    connect(hotspotController, &HotspotController::itemRemoved, model, updateItems);
    connect(hotspotController, &HotspotController::itemChanged, model, &ControllItemsModel::updateStatus);
    connect(hotspotController, &HotspotController::activeConnectionChanged, model, &ControllItemsModel::updateStatus);

    lvProfiles->setModel(model);
    lvProfiles->setEditTriggers(QAbstractItemView::NoEditTriggers);
    lvProfiles->setBackgroundType(DStyledItemDelegate::BackgroundType::ClipCornerBackground);
    lvProfiles->setSelectionMode(QAbstractItemView::NoSelection);
    lvProfiles->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto adjustHeight = [lvProfiles]() {
        lvProfiles->setMinimumHeight(lvProfiles->model()->rowCount() * 41);
    };
    adjustHeight();
    connect(model, &ControllItemsModel::modelReset, lvProfiles, adjustHeight);
    connect(model, &ControllItemsModel::detailClick, this, &HotspotDeviceItem::openEditPage);

    connect(lvProfiles, &QListView::clicked, this, [this](const QModelIndex &index) {
        if (NetworkController::instance()->hotspotController()->enabled(m_device)) {
            HotspotItem *item = static_cast<HotspotItem *>(index.internalPointer());
            if (item->connected())
                return;

            NetworkController::instance()->hotspotController()->connectItem(item);
        } else if (qEnvironmentVariableIsSet("DCC_NET_AC_AUTO_ON")) {
            // 有这个环境变量时可以自动打开热点（无线已连接的情况下）
            NetworkController::instance()->hotspotController()->setEnabled(m_device, true);
        }
    });
}

void HotspotDeviceItem::onSwitchToggled(const bool checked)
{
    SwitchWidget *switchWidget = qobject_cast<SwitchWidget *>(sender());
    if (!switchWidget)
        return;

    switchWidget->setEnabled(false);
    if (checked) {
        openHotspot(switchWidget);
    } else {
        closeHotspot();
    }
}

void HotspotDeviceItem::closeHotspot()
{
    HotspotController *hotspotController = NetworkController::instance()->hotspotController();
    // 关闭热点
    hotspotController->setEnabled(m_device, false);
}

void HotspotDeviceItem::openHotspot(SwitchWidget *switchWidget)
{
    HotspotController *hotspotController = NetworkController::instance()->hotspotController();
    QList<HotspotItem *> items = hotspotController->items(m_device);
    if (items.isEmpty()) {
        switchWidget->setChecked(false);
        switchWidget->setEnabled(true);
        openEditPage(nullptr, switchWidget);
    } else {
        // 开启热点
        hotspotController->setEnabled(m_device, true);
    }
}

void HotspotDeviceItem::openEditPage(ControllItems *item, QWidget *parent)
{
    QString uuid;
    if (item)
        uuid = item->connection()->uuid();
    ConnectionHotspotEditPage *editPage = new ConnectionHotspotEditPage(m_device->path(), uuid, parent);
    editPage->initSettingsWidget();
    editPage->setAttribute(Qt::WA_DeleteOnClose);
    editPage->setButtonTupleEnable(true);
    editPage->exec();
}
///////////////////////////////////////////////////////////////////////////
HotspotModule::HotspotModule(QObject *parent)
    : PageModule("personalHotspot", tr("Personal Hotspot"), tr("Personal Hotspot"), QIcon::fromTheme("dcc_hotspot"), parent)
{
    HotspotController *hotspotController = NetworkController::instance()->hotspotController();
    connect(hotspotController, &HotspotController::deviceAdded, this, &HotspotModule::onDeviceAdded);
    connect(hotspotController, &HotspotController::deviceRemove, this, &HotspotModule::onDeviceRemove);
    ModuleObject *extra = new WidgetModule<FloatingButton>("createHotspot", tr("Create Hotspot"), [this](FloatingButton *newprofile) {
        newprofile->setIcon(DStyle::StandardPixmap::SP_IncreaseElement);

        newprofile->setToolTip(tr("Create Hotspot"));
        connect(newprofile, &QAbstractButton::clicked, this, [this] {
            if (m_items.empty())
                return;

            m_items.front()->openEditPage(nullptr, qobject_cast<QWidget *>(sender()));
        });
    });
    extra->setExtra();
    connect(this, &HotspotModule::updateItemOnlyOne, extra, [extra](bool visiable) {
        extra->setHidden(!visiable);
    });
    appendChild(extra);
    onDeviceAdded(hotspotController->devices());
}

void HotspotModule::onDeviceAdded(const QList<WirelessDevice *> &devices)
{
    QSet<WirelessDevice *> currentDevices;
    for (HotspotDeviceItem *item : m_items)
        currentDevices << item->device();
    for (WirelessDevice *device : devices) {
        if (currentDevices.contains(device))
            continue;
        m_items.append(new HotspotDeviceItem(device, this));
    }
    HotspotController *hotspotController = NetworkController::instance()->hotspotController();
    QList<WirelessDevice *> hotspotDevices = hotspotController->devices();
    std::sort(m_items.begin(), m_items.end(), [=](HotspotDeviceItem *item1, HotspotDeviceItem *item2) {
        return hotspotDevices.indexOf(item1->device()) < hotspotDevices.indexOf(item2->device());
    });
    updateVisiable();
}

void HotspotModule::onDeviceRemove(const QList<WirelessDevice *> &rmDevices)
{
    for (auto it = m_items.begin(); it != m_items.end();) {
        if (rmDevices.contains((*it)->device())) {
            QList<ModuleObject *> &modules = (*it)->modules();
            for (auto *&module : modules) {
                removeChild(module);
            }
            delete (*it);
            it = m_items.erase(it);
        } else {
            ++it;
        }
    }
    updateVisiable();
}

void HotspotModule::updateVisiable()
{
    int row = 0;
    bool onlyOne = m_items.size() == 1;
    for (HotspotDeviceItem *&item : m_items) {
        QList<ModuleObject *> &modules = item->modules();
        insertChild(row++, modules.at(0));
        insertChild(row++, modules.at(1));
        if (onlyOne) {
            removeChild(modules.at(2));
        } else {
            insertChild(row++, modules.at(2));
        }
    }
    emit updateItemOnlyOne(onlyOne);
}
