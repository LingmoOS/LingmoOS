// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "accountnetworksystemcontainer.h"
#include "ownernetcontroller.h"

using namespace accountnetwork::systemservice;

AccountNetworkSystemContainer::AccountNetworkSystemContainer(QObject *parent)
    : network::systemservice::SystemContainer(parent)
    , m_ownerNetController(new OwnerNetController(this))
{
}

AccountNetworkSystemContainer::~AccountNetworkSystemContainer()
{
}

OwnerNetController *AccountNetworkSystemContainer::authenInterface() const
{
    return m_ownerNetController;
}
