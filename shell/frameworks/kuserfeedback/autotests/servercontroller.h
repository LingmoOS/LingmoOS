/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef SERVERCONTROLLER_H
#define SERVERCONTROLLER_H

#include <QProcess>

/** Controls a local PHP test server for automatic tests. */
class ServerController
{
public:
    ServerController();
    ~ServerController();

    bool start();
    void stop();

    QUrl url() const;

private:
    QProcess m_process;
};

#endif // SERVERCONTROLLER_H
