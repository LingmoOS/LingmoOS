/*
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#pragma once

#include "DumpTruckInterface.h"

#include <QObject>

// Plugin for coredump helper daemon. It issues KNotifications with
// the option of opening konsole with gdb.
// This is separated as a plugin so the core always-on daemon does not
// need anything besides QtCore and lower level libraries, slightly
// reducing the footprint.
// It also sports no i18n because it must be opted into and is only intended
// for developers.
class NotifyTruck : public QObject, public DumpTruckInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DumpTruckInterface_IID)
    Q_INTERFACES(DumpTruckInterface)
public:
    NotifyTruck() = default;
    ~NotifyTruck() override = default;

    bool handle(const Coredump &dump) override;

private:
    Q_DISABLE_COPY_MOVE(NotifyTruck)
};
