// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cupsconnectionfactory.h"
#include "zsettings.h"

#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(FACTORY, "org.deepin.dde-printer.factory")

std::unique_ptr<Connection> CupsConnectionFactory::createConnection(QString strHost, int port, int encryption)

{
    std::unique_ptr<Connection> connectionPtr = std::unique_ptr<Connection>(new Connection());
    try {
        if (0 != connectionPtr->init(strHost.toUtf8().data(), port, encryption)) {
            qCWarning(FACTORY) << "Unable to connect cups server"  ;
            connectionPtr.reset();
        }
    } catch (const std::exception &ex) {
        qCWarning(FACTORY) << "Got execpt: " << QString::fromUtf8(ex.what());
        connectionPtr.reset();

    }
    return connectionPtr;
}

std::unique_ptr<Connection> CupsConnectionFactory::createConnectionBySettings()
{
    std::unique_ptr<Connection> connectionPtr = std::unique_ptr<Connection>(new Connection());
    try {
        if (0 != connectionPtr->init(g_Settings->getCupsServerHost().toLocal8Bit(), g_Settings->getCupsServerPort(), g_Settings->getCupsServerEncryption())) {
            qCWarning(FACTORY) << "Unable to connect deafult cups server";
            connectionPtr.reset();
        }
    } catch (const std::exception &ex) {
        qCWarning(FACTORY) << "Got execpt: " << QString::fromUtf8(ex.what());
        connectionPtr.reset();
    }
    return connectionPtr;
}
