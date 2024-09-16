// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDBusMessage>

#include "global.h"

int main(int argc, char *argv[])
{
    QCoreApplication app{argc, argv};
    QCoreApplication::setApplicationName("app-identifier");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Identify in what capacity the process is running.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("PIDs", "PIDs to identify.", "[pid1,pid2,pid3 ...]");

    parser.process(app);
    auto PIDs = parser.positionalArguments();
    if (PIDs.isEmpty()) {
        return 0;
    }

    auto slice = PIDs.first().split(',', Qt::SkipEmptyParts);
    QList<pid_t> PIDList{};
    std::transform(slice.cbegin(), slice.cend(), std::back_inserter(PIDList), [](const QString &pid) {
        bool ok{true};
        auto result = pid.toInt(&ok);
        if (!ok) {
            qCritical() << "syntax error:" << pid;
            std::terminate();
        }

        return static_cast<pid_t>(result);
    });

    auto con = QDBusConnection::sessionBus();
    std::for_each(PIDList.cbegin(), PIDList.cend(), [&con](pid_t pid) {
        auto pidfd = pidfd_open(pid, 0);
        if (pidfd == -1) {
            qCritical() << "failed to open pidfd:" << std::strerror(errno) << "skip.";
            return;
        }

        auto msg = QDBusMessage::createMethodCall(
            DDEApplicationManager1ServiceName, DDEApplicationManager1ObjectPath, ApplicationManager1Interface, "Identify");
        msg.setArguments({QVariant::fromValue(QDBusUnixFileDescriptor{pidfd})});

        auto reply = con.call(msg);
        if (reply.type() != QDBusMessage::ReplyMessage) {
            qWarning() << "failed to Identify process" << pid << reply.errorMessage();
            return;
        }

        auto appID = reply.arguments().first().toString();
        if (!appID.isEmpty()) {
            qInfo() << "The capacity of process" << pid << "is:" << appID;
            return;
        }

        qWarning() << "failed to get appID of process" << pid;
    });

    return 0;
}
