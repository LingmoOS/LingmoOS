// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "sessionservice.h"
#include "sessioncontainer.h"

using namespace network::sessionservice;

SessionService::SessionService(SessionContainer *container, QObject *parent)
    : QObject(parent)
    , m_container(container)
{
}
