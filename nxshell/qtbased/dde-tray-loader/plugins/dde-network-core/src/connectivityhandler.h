// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONNECTIVITYHANDLER_H
#define CONNECTIVITYHANDLER_H

#include "networkconst.h"

#include <QObject>

class QTimer;

namespace dde {

namespace network {

class ConnectivityHandler : public QObject
{
    Q_OBJECT

public:
    explicit ConnectivityHandler(QObject *parent = nullptr);
    ~ConnectivityHandler();
    Connectivity connectivity() const;

signals:
    void connectivityChanged(const Connectivity &);

private:
    void initConnectivity();
    void initConnection();

private slots:
    void onConnectivityChanged(int connectivity);

private:
    Connectivity m_connectivity;
};

}
}

#endif // CONNECTIVITYPROCESSER_H
