// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUPSCONNECTIONFACTORY_H
#define CUPSCONNECTIONFACTORY_H

#include "cupsconnection.h"

#include <QString>

#include <memory>
using namespace std;
class CupsConnectionFactory
{
public:

    static std::unique_ptr<Connection> createConnection(QString strHost, int port, int encryption = 0);
    static std::unique_ptr<Connection> createConnectionBySettings();
private:
    CupsConnectionFactory() {}
    Q_DISABLE_COPY(CupsConnectionFactory)
};


#define g_cupsConnection CupsConnectionFactory::createConnectionBySettings()
#endif // CUPSCONNECTIONFACTORY_H
