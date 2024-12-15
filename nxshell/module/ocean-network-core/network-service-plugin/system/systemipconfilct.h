// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYSTEMIPCONFILCT_H
#define SYSTEMIPCONFILCT_H

#include "ipconflicthandler.h"

#include <QObject>

namespace network {
namespace systemservice {

class SystemIPConflict : public IPConflictHandler
{
    Q_OBJECT

public:
    SystemIPConflict(QObject *parent);
    ~SystemIPConflict() override;
    bool ipConflicted(const QString &devicePath) const;

signals:
    void ipConflictChanged(const QString &devicePath, const QString &ip, bool conflicted);

protected:
    bool canReconnect(const QSharedPointer<NetworkManager::ActiveConnection> &activeConnection) override;
    void onIPConflicted(const QString &ip, const QString &localMac, const QString &remoteMac) override;
    void onIPConflictReslove(const QString &ip, const QString &localMac, const QString &remoteMac) override;
    bool ipConflicted(const QString &devicePath, const QString &ip) const override;
    void ipv4Changed(const NetworkManager::Device::Ptr &device) override;

private:
    IPConflictStore m_ipConfilctData;
};

}
}

#endif // IPCONFLICTHANDLER_H
