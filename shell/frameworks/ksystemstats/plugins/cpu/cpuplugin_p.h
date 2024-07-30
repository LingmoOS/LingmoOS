/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CPUPLUGIN_P_H
#define CPUPLUGIN_P_H

class CpuPlugin;
namespace KSysGuard
{
    class SensorContainer;
}

class CpuPluginPrivate {
public:
    CpuPluginPrivate(CpuPlugin *q);
    virtual ~CpuPluginPrivate() =  default;
    virtual void update() {};

    KSysGuard::SensorContainer *m_container;
};

#endif
