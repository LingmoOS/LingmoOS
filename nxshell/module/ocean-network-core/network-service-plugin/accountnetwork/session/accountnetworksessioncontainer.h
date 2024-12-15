// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ACCOUNTNETWORKSESSIONCONTAINER_H
#define ACCOUNTNETWORKSESSIONCONTAINER_H

#include "sessioncontainer.h"

namespace accountnetwork {
namespace sessionservice {

class ActiveAccountNetwork;

class AccountNetworkSessionContainer : public network::sessionservice::SessionContainer
{
    Q_OBJECT

public:
    AccountNetworkSessionContainer(QObject *parent = Q_NULLPTR);
    ~AccountNetworkSessionContainer();

private:
    ActiveAccountNetwork *m_accountNetwork;
};

}
}

#endif // NETWORKSERVICE_H
