// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "systemcontainer.h"
#include "systemipconfilct.h"
#include "connectivityprocesser.h"
#include "networkinitialization.h"

using namespace network::systemservice;

SystemContainer::SystemContainer(QObject *parent)
    : QObject (parent)
    , m_ipConflictHandler(new SystemIPConflict(this))
    , m_connectivityHelper(new ConnectivityProcesser(m_ipConflictHandler, this))
{
    NetworkInitialization::doInit();
}

SystemContainer::~SystemContainer()
{
}

SystemIPConflict *SystemContainer::ipConfilctedChecker() const
{
    return m_ipConflictHandler;
}

ConnectivityProcesser *SystemContainer::connectivityProcesser() const
{
    return m_connectivityHelper;
}
