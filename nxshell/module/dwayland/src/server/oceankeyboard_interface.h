// Copyright 2022  luochaojiang <luochaojiang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QObject>

#include <DWayland/Server/kwaylandserver_export.h>

struct wl_resource;

namespace KWaylandServer
{
class OCEANSeatInterface;
class OCEANKeyboardInterfacePrivate;

/**
 * @brief Resource for the ocean_keyboard interface.
 *
 * OCEANKeyboardInterface gets created by OCEANSeatInterface.
 *
 * @since 5.4
 **/
class KWAYLANDSERVER_EXPORT OCEANKeyboardInterface : public QObject
{
    Q_OBJECT
public:
    virtual ~OCEANKeyboardInterface();

    /**
     * @returns The OCEANSeatInterface which created this OCEANPointerInterface.
     **/
    OCEANSeatInterface *oceanSeat() const;

    void setKeymap(int fd, quint32 size);
    void updateModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial);
    void keyPressed(quint32 key, quint32 serial);
    void keyReleased(quint32 key, quint32 serial);
    void repeatInfo(qint32 charactersPerSecond, qint32 delay);

private:
    friend class OCEANSeatInterface;
    friend class OCEANSeatInterfacePrivate;
    friend class OCEANKeyboardInterfacePrivate;
    explicit OCEANKeyboardInterface(OCEANSeatInterface *oceanSeat, wl_resource *resource);
    QScopedPointer<OCEANKeyboardInterfacePrivate> d;
};

}