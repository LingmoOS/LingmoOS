// Copyright 2022  luochaojiang <luochaojiang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

// KWayland
#include "oceankeyboard_interface.h"
// Qt
#include <QHash>
#include <QMap>
#include <QPointer>
#include <QPointF>

#include "qwayland-server-ocean-seat.h"

namespace KWaylandServer
{
class OCEANKeyboardInterfacePrivate : public QtWaylandServer::ocean_keyboard
{
public:
    static OCEANKeyboardInterfacePrivate *get(OCEANKeyboardInterface *oceankeyboard);

    OCEANKeyboardInterfacePrivate(OCEANKeyboardInterface *q, OCEANSeatInterface *seat, wl_resource *resource);
    ~OCEANKeyboardInterfacePrivate() override;

    void sendKeymap(int fd, quint32 size);
    void sendModifiers();
    void sendModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial);

    OCEANKeyboardInterface *q;
    OCEANSeatInterface *oceanSeat;
protected:
    void ocean_keyboard_release(Resource *resource) override;
};

}