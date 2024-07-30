/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakelogind.h"

#include <fcntl.h>
#include <qtemporaryfile.h>
#include <sys/stat.h>
#include <unistd.h>

#include <QDebug>
#include <QTemporaryFile>
#include <QTimer>

FakeLogind::FakeLogind(QObject *parent)
    : QObject(parent)
{
}

QDBusUnixFileDescriptor FakeLogind::Inhibit(const QString &what, const QString &who, const QString &why, const QString &mode)
{
    Q_EMIT newInhibition(what, who, why, mode);

    QDBusUnixFileDescriptor foo;
    foo.setFileDescriptor(-1);

    QTemporaryFile file;
    if (!file.open()) {
        qDebug() << "Could not open a temporary file";
        return foo;
    }

    m_fd = file.handle();
    foo.giveFileDescriptor(m_fd);

    // We could use epoll for this, but it will make the code harder to read for a test.
    auto t = new QTimer();
    t->setInterval(100);
    connect(t, SIGNAL(timeout()), SLOT(checkFd()));
    t->start();

    return foo;
}

void FakeLogind::checkFd()
{
    int e = fcntl(m_fd, F_GETFD) != -1 || errno != EBADF;

    if (e == 0) {
        Q_EMIT inhibitionRemoved();
        delete sender();
    }
}

#include "moc_fakelogind.cpp"
