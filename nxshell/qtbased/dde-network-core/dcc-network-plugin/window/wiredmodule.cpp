// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wiredmodule.h"
#include "controllitemsmodel.h"
#include "editpage/connectioneditpage.h"
#include "widgets/widgetmodule.h"
#include "widgets/switchwidget.h"
#include "widgets/settingsgroup.h"
#include "widgets/floatingbutton.h"

#include <DFloatingButton>
#include <DFontSizeManager>
#include <DListView>

#include <QApplication>
#include <QHBoxLayout>

#include <wireddevice.h>
#include <networkcontroller.h>

using namespace dde::network;
using namespace DCC_NAMESPACE;
DWIDGET_USE_NAMESPACE

WiredModule::WiredModule(WiredDevice *dev, QObject *parent)
    : PageModule("wired", dev->deviceName(), QString(), QIcon::fromTheme("dcc_ethernet"), parent)
    , m_device(dev)
{
    onNameChanged(m_device->deviceName());
    connect(m_device, &WiredDevice::nameChanged, this, &WiredModule::onNameChanged);
    appendChild(new WidgetModule<SwitchWidget>("wired_adapter", tr("Wired Network Adapter"), [this](SwitchWidget *devEnabled) {
        QLabel *lblTitle = new QLabel(tr("Wired Network Adapter")); // 无线网卡
        DFontSizeManager::instance()->bind(lblTitle, DFontSizeManager::T5, QFont::DemiBold);
        devEnabled->setLeftWidget(lblTitle);
        devEnabled->setChecked(m_device->isEnabled());
        connect(devEnabled, &SwitchWidget::checkedChanged, m_device, &WiredDevice::setEnabled);
        connect(m_device, &WiredDevice::enableChanged, devEnabled, [devEnabled](const bool enabled) {
            devEnabled->blockSignals(true);
            devEnabled->setChecked(enabled);
            devEnabled->blockSignals(false);
        });
    }));
    ModuleObject *nocableTips = new WidgetModule<SettingsGroup>("nocable_tips", tr("Plug in the network cable first"), [](SettingsGroup *tipsGroup) {
        QLabel *tips = new QLabel;
        tips->setAlignment(Qt::AlignCenter);
        tips->setWordWrap(true);
        tips->setFixedHeight(80);
        tips->setText(tr("Plug in the network cable first"));
        tipsGroup->setBackgroundStyle(SettingsGroup::GroupBackground);
        tipsGroup->insertWidget(tips);
    });
    nocableTips->setVisible(m_device->deviceStatus() <= DeviceStatus::Unavailable);
    connect(m_device, &WiredDevice::deviceStatusChanged, nocableTips, [this, nocableTips]() {
        nocableTips->setVisible(m_device->deviceStatus() <= DeviceStatus::Unavailable);
    });
    appendChild(nocableTips);
    appendChild(new WidgetModule<DListView>("wiredlist", QString(), this, &WiredModule::initWirelessList));

    ModuleObject *extra = new WidgetModule<FloatingButton>("addWired", tr("Add Network Connection"), [this](FloatingButton *createBtn) {
        createBtn->setIcon(DStyle::StandardPixmap::SP_IncreaseElement);

        createBtn->setToolTip(tr("Add Network Connection"));
        connect(createBtn, &FloatingButton::clicked, this, [this]() {
            editConnection(nullptr, qobject_cast<QWidget *>(sender()));
        });
    });
    extra->setExtra();
    appendChild(extra);
}

void WiredModule::initWirelessList(DListView *lvProfiles)
{
    lvProfiles->setAccessibleName("lvProfiles");
    ControllItemsModel *model = new ControllItemsModel(lvProfiles);
    auto updateItems = [model, this]() {
        const QList<WiredConnection *> conns = m_device->items();
        QList<ControllItems *> items;
        for (WiredConnection *it : conns) {
            items.append(it);
            if (!m_newConnectionPath.isEmpty() && it->connection()->path() == m_newConnectionPath) {
                m_device->connectNetwork(it);
                m_newConnectionPath.clear();
            }
        }
        model->updateDate(items);
    };
    updateItems();
    connect(m_device, &WiredDevice::connectionAdded, model, updateItems);
    connect(m_device, &WiredDevice::connectionRemoved, model, updateItems);
    connect(m_device, &WiredDevice::activeConnectionChanged, model, &ControllItemsModel::updateStatus);
    connect(m_device, &WiredDevice::enableChanged, model, &ControllItemsModel::updateStatus);
    connect(m_device, &WiredDevice::connectionChanged, model, &ControllItemsModel::updateStatus);
    connect(m_device, &WiredDevice::deviceStatusChanged, model, &ControllItemsModel::updateStatus);
    connect(m_device, &WiredDevice::activeConnectionChanged, model, &ControllItemsModel::updateStatus);
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
    connect(model, &ControllItemsModel::detailClick, this, &WiredModule::editConnection);

    // 点击有线连接按钮
    connect(lvProfiles, &DListView::clicked, this, [this](const QModelIndex &idx) {
        WiredConnection *connObj = static_cast<WiredConnection *>(idx.internalPointer());
        if (!connObj->connected())
            m_device->connectNetwork(connObj->connection()->path());
    });
}

void WiredModule::editConnection(dde::network::ControllItems *item, QWidget *parent)
{
    QString uuid = item ? item->connection()->uuid() : QString();
    ConnectionEditPage *editPage = new ConnectionEditPage(ConnectionEditPage::WiredConnection, m_device->path(), uuid, parent);
    editPage->initSettingsWidget();
    editPage->setAttribute(Qt::WA_DeleteOnClose);
    editPage->setButtonTupleEnable(!item);
    connect(editPage, &ConnectionEditPage::activateWiredConnection, this, [this](const QString &connectPath, const QString &uuid) {
        Q_UNUSED(uuid);
        if (!m_device->connectNetwork(connectPath))
            m_newConnectionPath = connectPath;
    });
    connect(editPage, &ConnectionEditPage::disconnect, m_device, &NetworkDeviceBase::disconnectNetwork);
    editPage->exec();
}

void WiredModule::onNameChanged(const QString &name)
{
    QString tmp;
    for (auto it = name.begin(); it != name.end(); ++it) {
        if ((*it) >= '0' && (*it) <= '9')
            tmp.append((*it));
    }
    setName("wired" + tmp);
    setDisplayName(name);
}
