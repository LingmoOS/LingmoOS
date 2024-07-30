/*
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2019-2022 Harald Sitter <sitter@kde.org>
*/

#include <thread>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QLocalSocket>
#include <QScopeGuard>

#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <coredump.h>
#include <coredumpwatcher.h>
#include <socket.h>

using namespace std::chrono_literals;
using namespace Qt::StringLiterals;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("drkonqi-coredump-helper"));
    app.setOrganizationDomain(QStringLiteral("kde.org"));

    // This binary is for internal use and intentionally has no i18n!
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption settleOption("settle-first"_L1, "Wait for system to settle before starting to process."_L1);
    parser.addOption(settleOption);
    QCommandLineOption pickupOption("pickup"_L1, "Picking up old crashes, don't handle them if you can't tell if they were handled."_L1);
    parser.addOption(pickupOption);
    QCommandLineOption uidOption("uid"_L1, "UID to filter"_L1, "uid"_L1);
    parser.addOption(uidOption);
    QCommandLineOption bootIdOption("boot-id"_L1, "systemd boot id to filter"_L1, "bootId"_L1);
    parser.addOption(bootIdOption);
    QCommandLineOption instanceOption("instance"_L1, "systemd-coredump@.service instance to filter"_L1, "instance"_L1);
    parser.addOption(instanceOption);
    parser.process(app);
    const bool settle = parser.isSet(settleOption);
    const bool pickup = parser.isSet(pickupOption);
    const QString uid = parser.value(uidOption);
    const QString bootId = parser.value(bootIdOption);
    const QString instance = parser.value(instanceOption);

    if (settle) {
        std::this_thread::sleep_for(1min);
    }

    auto expectedJournal = owning_ptr_call<sd_journal>(sd_journal_open, SD_JOURNAL_LOCAL_ONLY);
    Q_ASSERT(expectedJournal.ret == 0);
    Q_ASSERT(expectedJournal.value);

    CoredumpWatcher watcher(std::move(expectedJournal.value), bootId, instance, nullptr);
    if (!uid.isEmpty()) {
        watcher.addMatch(u"COREDUMP_UID=%1"_s.arg(uid));
    }
    QObject::connect(&watcher, &CoredumpWatcher::newDump, &app, [&watcher, pickup](const Coredump &dump) {
        if (pickup && !QFile::exists(dump.filename)) {
            // We only ignore missing cores when picking up old crashes. When dealing with new ones we may still wish
            // to notify that something has crashed, even when we can't debug it.
            return;
        }

        // We only try to find the socket file at this point in time because we need to know the UID and on older systemd's we'll not
        // be able to figure this out from just the instance information.
        // When systemd 245 (Ubuntu 20.04) no longer is out in the wild we can move this into the main and get the
        // uid from the instance.
        // TODO: move this to main scope as per the comment above
        const QByteArray socketPath = QByteArrayLiteral("/run/user/") + QByteArray::number(dump.uid) + QByteArrayLiteral("/drkonqi-coredump-launcher");
        if (!QFile::exists(QString::fromUtf8(socketPath))) {
            // This is intentionally not an error or fatal, not all users necessarily have
            // a socket or drkonqi!
            qWarning() << "The socket path doesn't exist @" << socketPath;
            Q_EMIT watcher.finished();
            return;
        }

        sockaddr_un sa{};
        sa.sun_family = AF_UNIX;
        // size_t is signed, ensure path is too
        Q_ASSERT(socketPath.size() >= 0);
        const std::make_unsigned<decltype(socketPath.size())>::type pathSize = socketPath.size();
        if (pathSize > sizeof(sa.sun_path) /* '>' because we need an extra byte for null */) {
            Q_EMIT watcher.error(QStringLiteral("The socket path has too many characters:") + QString::fromLatin1(socketPath));
            return;
        }
        strncpy(static_cast<char *>(sa.sun_path), socketPath.constData(), sizeof(sa.sun_path));

        const int fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
        if (fd < 0) {
            Q_EMIT watcher.error(QString::fromLocal8Bit(strerror(errno)));
            Q_UNREACHABLE();
        }
        QScopeGuard closeFD([fd] {
            close(fd);
        });

        if (::connect(fd, (sockaddr *)&sa, sizeof(sa)) < 0) { // NOLINT
            Q_EMIT watcher.error(QString::fromLocal8Bit(strerror(errno)));
            return;
        }

        // Convert the raw data to JSON and send that over the socket. This means
        // the client side doesn't need to talk to journald again. A tad more efficient,
        // and it makes nary a difference in code.
        QVariantMap variantMap;
        if (pickup) { // forward this into the launcher so it can choose to not have dump trucks handle dumps without metadata
            variantMap.insert(QString::fromUtf8(Coredump::keyPickup()), "TRUE"_ba);
        }
        for (auto it = dump.m_rawData.cbegin(); it != dump.m_rawData.cend(); ++it) {
            variantMap.insert(QString::fromUtf8(it.key()), it.value());
        }

        QLocalSocket s;
        s.setSocketDescriptor(fd, QLocalSocket::ConnectedState, QLocalSocket::WriteOnly);
        QByteArray data = QJsonDocument::fromVariant(variantMap).toJson();
        while (!data.isEmpty()) {
            // NB: we need to constrain the segment size to not run into QLocalSocket::DatagramTooLargeError
            const qint64 written = s.write(data.constData(), std::min<int>(data.size(), Socket::DatagramSize));
            if (written > 0) {
                data = data.mid(written);
                s.waitForBytesWritten();
            } else if (s.state() != QLocalSocket::ConnectedState) {
                qWarning() << "socket state unexpectedly" << s.state() << "aborting crash processing";
                qApp->quit();
                return;
            }
        }
        s.flush();
        s.waitForBytesWritten();

        Q_EMIT watcher.finished();
        return;
    });

    QObject::connect(&watcher, &CoredumpWatcher::finished, &app, &QCoreApplication::quit, Qt::QueuedConnection);
    QObject::connect(
        &watcher,
        &CoredumpWatcher::error,
        &app,
        [](const QString &msg) {
            qWarning() << msg;
            qApp->exit(1);
        },
        Qt::QueuedConnection);
    watcher.start();

    return app.exec();
}
