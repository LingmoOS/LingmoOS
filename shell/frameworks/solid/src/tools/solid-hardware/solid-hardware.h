/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef SOLID_HARDWARE_H
#define SOLID_HARDWARE_H

#include <QCoreApplication>
#include <QEventLoop>

#include <solid/storageaccess.h>

class QCommandLineParser;
class SolidHardware : public QCoreApplication
{
    Q_OBJECT
public:
    SolidHardware(int &argc, char **argv)
        : QCoreApplication(argc, argv)
        , m_error(0)
    {
    }

    bool hwList(bool interfaces, bool system);
    bool hwCapabilities(const QString &udi);
    bool hwProperties(const QString &udi);
    bool hwQuery(const QString &parentUdi, const QString &query);
    bool listen();
    bool monitor(const QString &udi);

    enum VolumeCallType { Mount, Unmount, Eject, CanCheck, Check, CanRepair, Repair };
    bool hwVolumeCall(VolumeCallType type, const QString &udi);

private:
    QEventLoop m_loop;
    int m_error;
    QString m_errorString;

private Q_SLOTS:
    void slotStorageResult(Solid::ErrorType error, const QVariant &errorData);
    void deviceAdded(const QString &udi);
    void deviceRemoved(const QString &udi);
};

#endif // SOLID_HARDWARE_H
