/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PURPOSE_PROCESSJOB_H
#define PURPOSE_PROCESSJOB_H

#include "job.h"
#include <QJsonObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QPointer>
#include <QProcess>

namespace Purpose
{
/**
 * @internal
 *
 * Purpose jobs can optionally run on an external process. This class interfaces
 * with the external process.
 */
class ProcessJob : public Job
{
    Q_OBJECT
public:
    ProcessJob(const QString &pluginPath, const QString &pluginType, const QJsonObject &data, QObject *parent);
    ~ProcessJob() override;

    void start() override;

private:
    void writeSocket();
    void readSocket();
    void processStateChanged(QProcess::ProcessState state);

    QPointer<QProcess> m_process;

    QString m_pluginPath;
    QString m_pluginType;
    QJsonObject m_data;
    QLocalServer m_socket;
    QPointer<QLocalSocket> m_localSocket;
};

}

#endif
