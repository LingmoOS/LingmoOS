// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SESSIONIPCONFILCT_H
#define SESSIONIPCONFILCT_H

#include "ipconflicthandler.h"

#include <QObject>

namespace network {
namespace sessionservice {

class SessionIPConflict : public IPConflictHandler
{
    Q_OBJECT

public:
    SessionIPConflict(QObject *parent);
    ~SessionIPConflict() override;

protected:
    bool canReconnect(const QSharedPointer<NetworkManager::ActiveConnection> &activeConnection) override;
    void onIPConflicted(const QString &ip, const QString &localMac, const QString &remoteMac) override;
    void onIPConflictReslove(const QString &ip, const QString &localMac, const QString &remoteMac) override;

private:
    IPConflictStore m_ipStore;
};

}
}

#endif // IPCONFLICTHANDLER_H
