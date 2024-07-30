/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_FAKE_LOGIND_H
#define SOLID_FAKE_LOGIND_H

#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>
#include <QString>

class FakeLogind : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.login1.Manager")
public:
    explicit FakeLogind(QObject *parent);

public Q_SLOTS:
    QDBusUnixFileDescriptor Inhibit(const QString &what, const QString &who, const QString &why, const QString &mode);

    void checkFd();

Q_SIGNALS:
    void inhibitionRemoved();
    void newInhibition(const QString &what, const QString &who, const QString &why, const QString &mode);

private:
    int m_fd;
};

#endif // SOLID_FAKE_LOGIND_H
