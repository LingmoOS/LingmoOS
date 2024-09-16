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
class DDESeatInterface;
class DDEKeyboardInterfacePrivate;

/**
 * @brief Resource for the dde_keyboard interface.
 *
 * DDEKeyboardInterface gets created by DDESeatInterface.
 *
 * @since 5.4
 **/
class KWAYLANDSERVER_EXPORT DDEKeyboardInterface : public QObject
{
    Q_OBJECT
public:
    virtual ~DDEKeyboardInterface();

    /**
     * @returns The DDESeatInterface which created this DDEPointerInterface.
     **/
    DDESeatInterface *ddeSeat() const;

    void setKeymap(int fd, quint32 size);
    void updateModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial);
    void keyPressed(quint32 key, quint32 serial);
    void keyReleased(quint32 key, quint32 serial);
    void repeatInfo(qint32 charactersPerSecond, qint32 delay);

private:
    friend class DDESeatInterface;
    friend class DDESeatInterfacePrivate;
    friend class DDEKeyboardInterfacePrivate;
    explicit DDEKeyboardInterface(DDESeatInterface *ddeSeat, wl_resource *resource);
    QScopedPointer<DDEKeyboardInterfacePrivate> d;
};

}