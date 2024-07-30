/*
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#pragma once

#include <QtPlugin>

class Coredump;

class DumpTruckInterface
{
public:
    DumpTruckInterface() = default;
    virtual ~DumpTruckInterface() = default;
    virtual bool handle(const Coredump &dump) = 0;

private:
    Q_DISABLE_COPY_MOVE(DumpTruckInterface)
};

#define DumpTruckInterface_IID "org.kde.drkonqi.DumpTruckInterface/1.0" // NOLINT
Q_DECLARE_INTERFACE(DumpTruckInterface, DumpTruckInterface_IID) // NOLINT
