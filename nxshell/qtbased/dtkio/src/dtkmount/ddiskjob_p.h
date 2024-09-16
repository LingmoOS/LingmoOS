// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDISKJOB_P_H
#define DDISKJOB_P_H

#include <DDiskJob>
#include "udisks2_interface.h"

DMOUNT_BEGIN_NAMESPACE

class DDiskJobPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DDiskJob)

public:
    explicit DDiskJobPrivate(DDiskJob *qq)
        : QObject { qq }, q_ptr { qq } { }
    ~DDiskJobPrivate() override { }

private:
    DDiskJob *q_ptr { nullptr };

    OrgFreedesktopUDisks2JobInterface *iface { nullptr };
};

DMOUNT_END_NAMESPACE

#endif   // DDISKJOB_P_H
