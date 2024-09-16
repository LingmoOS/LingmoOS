// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "networkinfomodule.h"

#include <networkcontroller.h>
#include <networkdetails.h>

#include <widgets/settingshead.h>
#include <widgets/widgetmodule.h>
#include <widgets/titlevalueitem.h>
#include <widgets/settingsgroupmodule.h>

using namespace dde::network;
using namespace DCC_NAMESPACE;

NetworkInfoModule::NetworkInfoModule(QObject *parent)
    : PageModule("networkDetails", tr("Network Details"), tr("Network Details"), QIcon::fromTheme("dcc_network"), parent)
{
    connect(NetworkController::instance(), &NetworkController::activeConnectionChange, this, &NetworkInfoModule::onUpdateNetworkInfo);

    onUpdateNetworkInfo();
}

void NetworkInfoModule::onUpdateNetworkInfo()
{
    while (getChildrenSize() > 0) {
        removeChild(0);
    }
    QList<NetworkDetails *> netDetails = NetworkController::instance()->networkDetails();
    int size = netDetails.size();

    for (int i = 0; i < size; i++) {
        NetworkDetails *detail = netDetails[i];
        SettingsGroupModule *networkModuleGroup = new SettingsGroupModule("", tr(""));
        appendChild(new WidgetModule<SettingsHead>("", tr(""), [detail](SettingsHead *head) {
            head->setEditEnable(false);
            head->setContentsMargins(10, 0, 0, 0);
            head->setTitle(detail->name());
        }));
        QList<QPair<QString, QString>> items = detail->items();
        for (const QPair<QString, QString> &item : items)
            networkModuleGroup->appendChild(new WidgetModule<TitleValueItem>("", tr(""), [item](TitleValueItem *valueItem) {
                valueItem->setTitle(item.first);
                valueItem->setValue(item.second);
                valueItem->addBackground();
                if (item.first == "IPv6")
                    valueItem->setWordWrap(false);
            }));
        if (i < size - 1)
            networkModuleGroup->appendChild(new WidgetModule<QWidget>());

        appendChild(networkModuleGroup);
    }
}
