// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBLOCKPARTITION_P_H
#define DBLOCKPARTITION_P_H

#include <DBlockPartition>
#include "dblockdevice_p.h"
#include "udisks2_interface.h"

DMOUNT_BEGIN_NAMESPACE

class DBlockPartitionPrivate : public DBlockDevicePrivate
{
public:
    DBlockPartitionPrivate(DBlockPartition *qq);

    OrgFreedesktopUDisks2PartitionInterface *dbus;
};

DMOUNT_END_NAMESPACE

#endif   // DBLOCKPARTITION_P_H
