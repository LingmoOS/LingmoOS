// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYSTEMCONTAINER_H
#define SYSTEMCONTAINER_H

#include <QObject>


namespace network {
namespace systemservice {

class SystemIPConflict;
class ConnectivityProcesser;

class SystemContainer : public QObject
{
    Q_OBJECT

public:
    SystemContainer(QObject *parent = Q_NULLPTR);
    ~SystemContainer();
    SystemIPConflict *ipConfilctedChecker() const;
    ConnectivityProcesser *connectivityProcesser() const;

private:
    SystemIPConflict *m_ipConflictHandler;
    ConnectivityProcesser *m_connectivityHelper;
};

}
}

#endif // NETWORKSERVICE_H
