/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef WAYLAND_SERVER_ABSTRACT_DATA_DEVICE_H
#define WAYLAND_SERVER_ABSTRACT_DATA_DEVICE_H

#include "deepin-kwin_export.h"

#include <QObject>

namespace KWaylandServer
{

class AbstractDataSource;

/**
 * @brief The AbstractDataDevice class abstracts the data that
 * can be transferred to another client.
 *
 * It loosely maps to DataDeviceInterface
 */

// Anything related to selections are pure virtual, content relating
// to drag and drop has a default implementation

class KWIN_EXPORT AbstractDataDevice : public QObject
{
    Q_OBJECT
public:
    virtual void sendSelection(AbstractDataSource *other) {};
    virtual void sendPrimarySelection(AbstractDataSource *other){}

    enum DeviceType {
        DeviceType_Unknown,
        DeviceType_Data,
        DeviceType_DataControl,
        DeviceType_Primary,
        DeviceType_X11,
    };

    DeviceType m_deviceType = DeviceType_Unknown;
    virtual int deviceType()
    {
        return m_deviceType;
    }

    /**
     * The pid of the Source endpoint.
     *
     * Please note: if the Source got created with @link Display::createClient @endlink
     * the pid will be identical to the process running the KWayland::Server::Display.
     *
     * @returns The pid of the Source.
     **/
    virtual pid_t processId()
    {
        return pid;
    }

    // The pid of the Source
    pid_t pid = 0;

protected:
    explicit AbstractDataDevice(QObject *parent = nullptr);
};

}

#endif
