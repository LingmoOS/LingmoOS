// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include <QList>

namespace dde {
namespace network {

class NetworkDetails;
class NetworkDetailRealize;
class NetworkDeviceRealize;
class NetworkDeviceBase;

class ObjectManager
{
public:
    static ObjectManager *instance();

    NetworkDetails *createNetworkDetail(NetworkDetailRealize *realize);
    QList<NetworkDetails *> networkDetails() const;
    void cleanupNetworkDetails();

    NetworkDeviceRealize *deviceRealize(NetworkDeviceBase *device) const;

private:
    ObjectManager();
    ObjectManager(const ObjectManager &objectManager) = delete;
    ObjectManager operator=(const ObjectManager &objectManager) = delete;
    ~ObjectManager();

private:
    QList<NetworkDetails *> m_networkDetails;
};

}
}

#endif // OBJECTCREATOR_H
