// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

#include "netstathelper.h"

#include <KLocalizedString>

#include <QDebug>
#include <QStandardPaths>
#include <QStringList>
#include <QTimer>

Q_LOGGING_CATEGORY(NetstatHelperDebug, "netstat.helper")

void NetstatHelper::query()
{
    if (m_executableProcess) {
        // If we get queried again before the process finished, we'll forcefully terminate the process and start from scratch
        // without host name resolution to hopefully speed things up.
        stopProcess();
    }

    m_executableProcess = new QProcess(this);

    /* parameters passed to ss
     *  -r, --resolve       resolve host names
     *  -a, --all           display all sockets
     *  -p, --processes     show process using socket
     *  -u, --udp           display only UDP sockets
     *  -t, --tcp           display only TCP sockets
     */

    const QStringList netstatArgs(m_hasTimeoutError ? QStringList({"-tuap"}) : QStringList({"-tuapr"}));
    const QString executable = QStringLiteral("ss");

    connect(m_executableProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &NetstatHelper::stepExecuteFinished);
    m_executableProcess->start(executable, netstatArgs, QIODevice::ReadOnly);

    qDebug() << "Running process";
}

void NetstatHelper::stopProcess()
{
    qDebug() << "Timing out!";
    m_hasTimeoutError = true;

    m_executableProcess->disconnect();
    m_executableProcess->kill();
    m_executableProcess->deleteLater();
    m_executableProcess = nullptr;
}

void NetstatHelper::stepExecuteFinished(int exitCode)
{
    m_hasError = false;

    if (0 != exitCode) {
        m_hasError = true;
        m_errorString = m_executableProcess->readAllStandardError();
    } else {
        QList<QStringList> result = parseSSOutput(m_executableProcess->readAllStandardOutput());
        Q_EMIT queryFinished(result);
    }

    m_executableProcess->deleteLater();
    m_executableProcess = nullptr;
}

bool NetstatHelper::hasError() const
{
    return m_hasError;
}

QString NetstatHelper::errorString() const
{
    return m_errorString;
}

QList<QStringList> NetstatHelper::parseSSOutput(const QByteArray &netstatOutput)
{
    QString rawOutput = netstatOutput;
    QStringList outputLines = rawOutput.split(QStringLiteral("\n"));

    QList<QStringList> connections;

    // discard lines.
    while (outputLines.size()) {
        if (outputLines.first().indexOf(QLatin1String("Recv-Q"))) {
            outputLines.removeFirst();
            break;
        }
        outputLines.removeFirst();
    }

    // can't easily parse because of the spaces in Local and Peer AddressPort.
    QStringList headerLines = {
        QStringLiteral("Netid"),
        QStringLiteral("State"),
        QStringLiteral("Recv-Q"),
        QStringLiteral("Send-Q"),
        QStringLiteral("Local Address:Port"),
        QStringLiteral("Peer Address:Port"),
        QStringLiteral("Process"),
    };

    // Extract Information
    for (const auto &line : std::as_const(outputLines)) {
        QStringList values = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (values.isEmpty()) {
            continue;
        }

        // Some lines lack one or two values.
        while (values.size() < headerLines.size()) {
            values.append(QString());
        }

        QString appName;
        QString pid;
        if (values[6].size()) {
            values[6].remove(0, QStringLiteral("users:((").size());
            values[6].chop(QStringLiteral("))").size());

            QStringList substrings = values[6].split(',');
            appName = substrings[0].remove(QStringLiteral("\""));
            pid = substrings[1].split('=')[1];
        }

        /* Insertion order needs to match the Model Columns:
            ProtocolRole = Qt::UserRole + 1,
            LocalAddressRole,
            ForeignAddressRole,
            StatusRole,
            PidRole,
            ProgramRole
        */
        QStringList connection{
            values[0], // NetId
            values[4], // Local Address
            values[5], // Peer Address,
            values[1], // State
            pid,
            appName,
        };

        connections.append(connection);
    }

    return connections;
}

QString NetstatHelper::extractAndStrip(const QString &src, int index, int size)
{
    QString str = src.mid(index, size);
    str.remove(QLatin1String(" "));
    return str;
}
