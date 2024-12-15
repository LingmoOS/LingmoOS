// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACCOUNTNETWORKSYSTEMCONTAINER_H
#define ACCOUNTNETWORKSYSTEMCONTAINER_H

#include <QObject>

#include "systemcontainer.h"

namespace accountnetwork {
namespace systemservice {

class OwnerNetController;

class AccountNetworkSystemContainer : public network::systemservice::SystemContainer
{
    Q_OBJECT

public:
    AccountNetworkSystemContainer(QObject *parent = Q_NULLPTR);
    ~AccountNetworkSystemContainer();

    OwnerNetController *authenInterface() const;

private:
    OwnerNetController *m_ownerNetController;
};

}
}

#endif // NETWORKSERVICE_H
