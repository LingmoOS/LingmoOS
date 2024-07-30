/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "ksignalhandler.h"
#include "kcoreaddons_debug.h"
#include <QSocketNotifier>
#include <QTimer>

#ifndef Q_OS_WIN
#include <cerrno>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

class KSignalHandlerPrivate : public QObject
{
public:
    static void signalHandler(int signal);
    void handleSignal();

    QSet<int> m_signalsRegistered;
    static int signalFd[2];
    QSocketNotifier *m_handler = nullptr;

    KSignalHandler *q;
};
int KSignalHandlerPrivate::signalFd[2];

KSignalHandler::KSignalHandler()
    : d(new KSignalHandlerPrivate)
{
    d->q = this;
#ifndef Q_OS_WIN
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, KSignalHandlerPrivate::signalFd)) {
        qCWarning(KCOREADDONS_DEBUG) << "Couldn't create a socketpair";
        return;
    }

    // ensure the sockets are not leaked to child processes, SOCK_CLOEXEC not supported on macOS
    fcntl(KSignalHandlerPrivate::signalFd[0], F_SETFD, FD_CLOEXEC);
    fcntl(KSignalHandlerPrivate::signalFd[1], F_SETFD, FD_CLOEXEC);

    QTimer::singleShot(0, [this] {
        d->m_handler = new QSocketNotifier(KSignalHandlerPrivate::signalFd[1], QSocketNotifier::Read, this);
        connect(d->m_handler, &QSocketNotifier::activated, d.get(), &KSignalHandlerPrivate::handleSignal);
    });
#endif
}

KSignalHandler::~KSignalHandler()
{
#ifndef Q_OS_WIN
    for (int sig : std::as_const(d->m_signalsRegistered)) {
        signal(sig, nullptr);
    }
    close(KSignalHandlerPrivate::signalFd[0]);
    close(KSignalHandlerPrivate::signalFd[1]);
#endif
}

void KSignalHandler::watchSignal(int signalToTrack)
{
    d->m_signalsRegistered.insert(signalToTrack);
#ifndef Q_OS_WIN
    signal(signalToTrack, KSignalHandlerPrivate::signalHandler);
#endif
}

void KSignalHandlerPrivate::signalHandler(int signal)
{
#ifndef Q_OS_WIN
    const int ret = ::write(signalFd[0], &signal, sizeof(signal));
    if (ret != sizeof(signal)) {
        qCWarning(KCOREADDONS_DEBUG) << "signalHandler couldn't write for signal" << strsignal(signal) << " Got error:" << strerror(errno);
    }
#else
    Q_UNUSED(signal);
#endif
}

void KSignalHandlerPrivate::handleSignal()
{
#ifndef Q_OS_WIN
    m_handler->setEnabled(false);
    int signal;
    const int ret = ::read(KSignalHandlerPrivate::signalFd[1], &signal, sizeof(signal));
    if (ret != sizeof(signal)) {
        qCWarning(KCOREADDONS_DEBUG) << "handleSignal couldn't read signal for fd" << KSignalHandlerPrivate::signalFd[1] << " Got error:" << strerror(errno);
        return;
    }
    m_handler->setEnabled(true);

    Q_EMIT q->signalReceived(signal);
#endif
}

KSignalHandler *KSignalHandler::self()
{
    static KSignalHandler s_self;
    return &s_self;
}

#include "moc_ksignalhandler.cpp"
