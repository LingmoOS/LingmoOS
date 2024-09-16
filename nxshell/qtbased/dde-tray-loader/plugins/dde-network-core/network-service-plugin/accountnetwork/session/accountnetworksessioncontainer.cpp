// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "accountnetworksessioncontainer.h"
#include "activeaccoutnetwork.h"

using namespace accountnetwork::sessionservice;

AccountNetworkSessionContainer::AccountNetworkSessionContainer(QObject *parent)
    : network::sessionservice::SessionContainer (parent)
    , m_accountNetwork(new ActiveAccountNetwork(this))
{
}

AccountNetworkSessionContainer::~AccountNetworkSessionContainer()
{
}
