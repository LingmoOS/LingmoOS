/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <QObject>
#include <QRect>

// KWayland
#include <KWayland/Server/display.h>
#include <KWayland/Server/output_interface.h>
#include <KWayland/Server/outputdevice_interface.h>

namespace KScreen
{
using namespace KWayland::Server;

class WaylandConfigReader
{
public:
    // static QList<KWayland::Server::OutputInterface*> outputsFromConfig(const QString &configfile, KWayland::Server::Display *display);
    static void outputsFromConfig(const QString &configfile, KWayland::Server::Display *display, QList<KWayland::Server::OutputDeviceInterface *> &outputs);
    static OutputDeviceInterface *createOutputDevice(const QVariantMap &outputConfig, KWayland::Server::Display *display);
    static QList<KWayland::Server::OutputInterface *> createOutputs(KWayland::Server::Display *display,
                                                                    QList<KWayland::Server::OutputDeviceInterface *> &outputdevices);

    static QSize sizeFromJson(const QVariant &data);
    static QRect rectFromJson(const QVariant &data);
    static QPoint pointFromJson(const QVariant &data);
};

} // namespace
