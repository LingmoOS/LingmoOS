/*
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2023 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "logging.h"

#include <qwayland-wayland.h>

#include <QSize>
#include <QWaylandClientExtensionTemplate>

#include <memory>

class ShmBuffer : public QtWayland::wl_buffer
{
public:
    ShmBuffer(::wl_buffer *buffer);
    ~ShmBuffer();
};

class Shm : public QWaylandClientExtensionTemplate<Shm>, public QtWayland::wl_shm
{
public:
    static Shm *instance();
    ~Shm();
    std::unique_ptr<ShmBuffer> createBuffer(const QImage &image);

private:
    Shm(QObject *parent);
};
