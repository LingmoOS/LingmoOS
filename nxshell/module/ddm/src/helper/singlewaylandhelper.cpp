// Copyright (C) 2023 Dingyuan Zhang <lxz@mkacg.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "singlewaylandhelper.h"

#include <QProcess>
#include <QDebug>
#include <QCoreApplication>
#include <qprocess.h>

SingleWaylandHelper::SingleWaylandHelper(QObject *parent)
    : QObject(parent)
{}

bool SingleWaylandHelper::start(const QString &compositor, const QString &cmd)
{
    auto args = QProcess::splitCommand(cmd);

    m_process = new QProcess(this);
    m_process->setProgram(compositor);
    m_process->setArguments(args);
    m_process->setStandardOutputFile(QProcess::nullDevice());
    m_process->setStandardErrorFile(QProcess::nullDevice());
    m_process->setStandardInputFile(QProcess::nullDevice());
    m_process->setProcessEnvironment([]{
        auto env = QProcessEnvironment::systemEnvironment();
        env.insert("LIBSEAT_BACKEND", "seatd");
        env.insert("DSG_APP_ID", "org.lingmo.ocean.treeland");
#if DEBUG
        env.insert("WAYLAND_DEBUG", "1");
#endif
        return env;
    }());

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            qApp, &QCoreApplication::quit);

    m_process->start();
    if (!m_process->waitForStarted(10000)) {
        qWarning("Failed to start \"%s\": %s",
                 qPrintable(cmd),
                 qPrintable(m_process->errorString()));
        return false;
    }

    return true;
}
