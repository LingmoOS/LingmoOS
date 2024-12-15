// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SESSIONCONTAINER_H
#define SESSIONCONTAINER_H

#include <QObject>

namespace network {
namespace sessionservice {

class SessionIPConflict;

class SessionContainer : public QObject
{
    Q_OBJECT

public:
    SessionContainer(QObject *parent = Q_NULLPTR);
    ~SessionContainer();
    SessionIPConflict *ipConfilctedChecker() const;

private:
    void initConnection();
    void initMember();

private slots:
    void onIPConflictChanged(const QString &devicePath, const QString &ip, bool conflicted);
    void onPortalDetected(const QString &url);
    void onProxyMethodChanged(const QString &method);

private:
    SessionIPConflict *m_ipConflictHandler;
};

}
}

#endif // NETWORKSERVICE_H
