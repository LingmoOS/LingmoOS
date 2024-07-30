// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

#pragma once

#include "devicenotifier.h"

#include <Solid/Device>

class SolidDeviceNotifier : public DeviceNotifier
{
    Q_OBJECT
public:
    using DeviceNotifier::DeviceNotifier;

    void start() override;
    void loadData() override;

private:
    void checkUDI(const QString &udi);
    void checkSolidDevice(const Solid::Device &device);
};
