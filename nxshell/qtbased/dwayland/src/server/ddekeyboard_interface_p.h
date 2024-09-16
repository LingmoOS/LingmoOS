// Copyright 2022  luochaojiang <luochaojiang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

// KWayland
#include "ddekeyboard_interface.h"
// Qt
#include <QHash>
#include <QMap>
#include <QPointer>
#include <QPointF>

#include "qwayland-server-dde-seat.h"

namespace KWaylandServer
{
class DDEKeyboardInterfacePrivate : public QtWaylandServer::dde_keyboard
{
public:
    static DDEKeyboardInterfacePrivate *get(DDEKeyboardInterface *ddekeyboard);

    DDEKeyboardInterfacePrivate(DDEKeyboardInterface *q, DDESeatInterface *seat, wl_resource *resource);
    ~DDEKeyboardInterfacePrivate() override;

    void sendKeymap(int fd, quint32 size);
    void sendModifiers();
    void sendModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial);

    DDEKeyboardInterface *q;
    DDESeatInterface *ddeSeat;
protected:
    void dde_keyboard_release(Resource *resource) override;
};

}