// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "objectmanager.h"
#include "networkdetails.h"
#include "networkdevicebase.h"

using namespace dde::network;

ObjectManager *ObjectManager::instance()
{
    static ObjectManager creatorInstance;
    return &creatorInstance;
}

NetworkDetails *ObjectManager::createNetworkDetail(NetworkDetailRealize *realize)
{
    NetworkDetails *detail = new NetworkDetails(realize);
    m_networkDetails << detail;
    return detail;
}

QList<NetworkDetails *> ObjectManager::networkDetails() const
{
    return m_networkDetails;
}

void ObjectManager::cleanupNetworkDetails()
{
    for (NetworkDetails *detail : m_networkDetails) {
        delete detail;
    }
    m_networkDetails.clear();
}

NetworkDeviceRealize *ObjectManager::deviceRealize(NetworkDeviceBase *device) const
{
    return device->deviceRealize();
}

ObjectManager::ObjectManager()
{
}

ObjectManager::~ObjectManager()
{
}
