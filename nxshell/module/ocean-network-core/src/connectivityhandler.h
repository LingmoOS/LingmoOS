// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONNECTIVITYHANDLER_H
#define CONNECTIVITYHANDLER_H

#include "networkconst.h"

#include <QObject>

class QTimer;

namespace ocean {

namespace network {

class ConnectivityHandler : public QObject
{
    Q_OBJECT

public:
    explicit ConnectivityHandler(QObject *parent = nullptr);
    ~ConnectivityHandler();
    void init();
    Connectivity connectivity() const;

signals:
    void connectivityChanged(const Connectivity &);

private:
    void initConnection();
    int getConnectivity();

private slots:
    void onConnectivityChanged(int connectivity);

private:
    Connectivity m_connectivity;
};

}
}

#endif // CONNECTIVITYPROCESSER_H
