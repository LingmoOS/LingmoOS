// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "servicefactory.h"
#include "systemcontainer.h"
#include "sessioncontainer.h"
#include "ipconflicthandler.h"
#include "systemservice.h"
#include "sessionservice.h"
#include "settingconfig.h"

#include "accountnetworksystemservice.h"
#include "accountnetworksystemcontainer.h"
#include "accountnetworksessioncontainer.h"

ServiceFactory::ServiceFactory(bool isSystem, QObject *parent)
    : QObject(parent)
    , m_serviceObject(createServiceObject(isSystem))
{
}

QObject *ServiceFactory::serviceObject() const
{
    return m_serviceObject;
}

QObject *ServiceFactory::createServiceObject(bool isSystem)
{
    if (SettingConfig::instance()->enableAccountNetwork()) {
        // 如果开启了用户私有账户网络（工银瑞信定制）
        if (isSystem)
            return new accountnetwork::systemservice::AccountNetworkSystemService(new accountnetwork::systemservice::AccountNetworkSystemContainer(this), this);

        return new network::sessionservice::SessionService(new accountnetwork::sessionservice::AccountNetworkSessionContainer(this), this);
    }

    if (isSystem)
        return new network::systemservice::SystemService(new network::systemservice::SystemContainer(this), this);

    return new network::sessionservice::SessionService(new network::sessionservice::SessionContainer(this), this);
}
