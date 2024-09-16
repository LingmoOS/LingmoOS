// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "daemonlibrary.h"

#include <QLibrary>
#include <QDebug>

using namespace GrandSearch;

DaemonLibrary::DaemonLibrary(const QString &fileName, QObject *parent)
    : QObject(parent)
    , m_fileName(fileName)
{

}

bool DaemonLibrary::load()
{
    if (m_lib)
        return true;
    m_lib = new QLibrary(m_fileName, this);
    if (!m_lib->load()) {
        qCritical() << "can not load" << m_fileName << m_lib->errorString();
        delete m_lib;
        m_lib = nullptr;
        return false;
    }

    verFunc = (VerDaemon)m_lib->resolve("grandSearchDaemonAppVersion");
    if (!verFunc) {
        qCritical() << "no such api grandSearchDaemonAppVersion in" << m_fileName;
        delete m_lib;
        m_lib = nullptr;
        return false;
    }

    startFunc = (StartDaemon)m_lib->resolve("startGrandSearchDaemon");
    if (!startFunc) {
        qCritical() << "no such api startGrandSearchDaemon in" << m_fileName;
        delete m_lib;
        m_lib = nullptr;
        return false;
    }

    stopFunc = (StopDaemon)m_lib->resolve("stopGrandSearchDaemon");
    if (!stopFunc) {
        qCritical() << "no such api stopGrandSearchDaemon in" << m_fileName;
        delete m_lib;
        m_lib = nullptr;
        return false;
    }

    return true;
}

void DaemonLibrary::unload()
{
    if (m_lib) {
        m_lib->unload();
        delete m_lib;
        m_lib = nullptr;
        startFunc = nullptr;
        stopFunc = nullptr;
    }
}

int DaemonLibrary::start(int argc, char *argv[])
{
    if (!startFunc) {
        qCritical() << "no such api startGrandSearchDaemon";
        return -1;
    }
    return startFunc(argc, argv);
}

int DaemonLibrary::stop()
{
    if (!stopFunc) {
        qCritical() << "no such api stopGrandSearchDaemon";
        return -1;
    }
    return stopFunc();
}

QString DaemonLibrary::version()
{
    if (!verFunc) {
        qCritical() << "no such api grandSearchDaemonAppVersion";
        return "";
    }
    return QString(verFunc());
}
