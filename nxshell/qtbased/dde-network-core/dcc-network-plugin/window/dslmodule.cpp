// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dslmodule.h"
#include "wireddevice.h"
#include "controllitemsmodel.h"
#include <editpage/connectioneditpage.h>

#include <DFloatingButton>
#include <DListView>

#include <QHBoxLayout>
#include <QApplication>

#include <widgets/widgetmodule.h>
#include "widgets/floatingbutton.h"

#include <networkcontroller.h>
#include <dslcontroller.h>

using namespace dde::network;
using namespace DCC_NAMESPACE;
DWIDGET_USE_NAMESPACE

DSLModule::DSLModule(QObject *parent)
    : PageModule("networkDsl", tr("DSL"), tr("DSL"), QIcon::fromTheme("dcc_dsl"), parent)
{
    appendChild(new WidgetModule<DListView>("pppoelist", QString(), this, &DSLModule::initDSLList));
    ModuleObject *extra = new WidgetModule<FloatingButton>("createDSL", tr("Create PPPoE Connection"), [this](FloatingButton *createBtn) {
        createBtn->setIcon(DStyle::StandardPixmap::SP_IncreaseElement);

        createBtn->setToolTip(tr("Create PPPoE Connection"));
        createBtn->setAccessibleName(tr("Create PPPoE Connection"));
        connect(createBtn, &DFloatingButton::clicked, this, [this]() {
            editConnection(nullptr, qobject_cast<QWidget *>(sender()));
        });
    });
    extra->setExtra();
    appendChild(extra);
}

void DSLModule::initDSLList(DListView *lvsettings)
{
    lvsettings->setAccessibleName("List_pppoelist");
    ControllItemsModel *model = new ControllItemsModel(lvsettings);
    auto updateItems = [model]() {
        const QList<DSLItem *> conns = NetworkController::instance()->dslController()->items();
        QList<ControllItems *> items;
        for (DSLItem *it : conns) {
            items.append(it);
        }
        model->updateDate(items);
    };
    DSLController *dslController = NetworkController::instance()->dslController();
    updateItems();
    connect(dslController, &DSLController::itemAdded, model, updateItems);
    connect(dslController, &DSLController::itemRemoved, model, updateItems);
    connect(dslController, &DSLController::itemChanged, model, &ControllItemsModel::updateStatus);
    connect(dslController, &DSLController::activeConnectionChanged, model, &ControllItemsModel::updateStatus);

    lvsettings->setModel(model);
    lvsettings->setEditTriggers(QAbstractItemView::NoEditTriggers);
    lvsettings->setBackgroundType(DStyledItemDelegate::BackgroundType::ClipCornerBackground);
    lvsettings->setSelectionMode(QAbstractItemView::NoSelection);
    lvsettings->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto adjustHeight = [lvsettings]() {
        lvsettings->setMinimumHeight(lvsettings->model()->rowCount() * 41);
    };
    adjustHeight();
    connect(model, &ControllItemsModel::modelReset, lvsettings, adjustHeight);
    connect(model, &ControllItemsModel::detailClick, this, &DSLModule::editConnection);
    connect(lvsettings, &DListView::clicked, this, [](const QModelIndex &idx) {
        DSLItem *item = static_cast<DSLItem *>(idx.internalPointer());
        if (!item->connected())
            NetworkController::instance()->dslController()->connectItem(item);
    });
}

void DSLModule::editConnection(dde::network::ControllItems *item, QWidget *parent)
{
    QString devPath = "/";
    QString connUuid;
    if (item) {
        QList<NetworkDeviceBase *> devices = NetworkController::instance()->devices();
        const QString macAddress = item->connection()->hwAddress();
        connUuid = item->connection()->uuid();
        for (NetworkDeviceBase *device : devices) {
            if (device->realHwAdr() == macAddress) {
                devPath = device->path();
                break;
            }
        }
    }
    ConnectionEditPage *editPage = new ConnectionEditPage(ConnectionEditPage::ConnectionType::PppoeConnection, devPath, connUuid, parent);
    editPage->initSettingsWidget();
    editPage->setAttribute(Qt::WA_DeleteOnClose);
    connect(editPage, &ConnectionEditPage::disconnect, editPage, [] {
        NetworkController::instance()->dslController()->disconnectItem();
    });

    if (item) {
        editPage->setLeftButtonEnable(true);
    } else {
        editPage->setButtonTupleEnable(true);
    }
    editPage->exec();
}
