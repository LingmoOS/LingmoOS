// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONNECTIVITYPROCESSER_H
#define CONNECTIVITYPROCESSER_H

#include "constants.h"

#include <QObject>

class QTimer;

namespace network {
namespace systemservice {

class SystemIPConflict;
class ConnectivityChecker;

class ConnectivityProcesser : public QObject
{
    Q_OBJECT

public:
    explicit ConnectivityProcesser(SystemIPConflict *ipConflict, QObject *parent = nullptr);
    ~ConnectivityProcesser();
    void checkConnectivity();
    network::service::Connectivity connectivity() const;
    QString portalUrl() const;

signals:
    void connectivityChanged(const network::service::Connectivity &);
    void portalDetected(const QString &);

private:
    ConnectivityChecker *createConnectivityChecker(bool enableConnectivity);

private slots:
    void onEnableConnectivityChanged(bool enableConnectivity);

private:
    QScopedPointer<ConnectivityChecker> m_checker;
    SystemIPConflict *m_ipConflictHandler;
};

}
}

#endif // CONNECTIVITYPROCESSER_H
