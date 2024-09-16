// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTASKBARCONTROL_P_H
#define DTASKBARCONTROL_P_H

#include <dtkgui_global.h>
#include <DObjectPrivate>
#include <DObject>

#include "dtaskbarcontrol.h"

DCORE_USE_NAMESPACE
DGUI_BEGIN_NAMESPACE

class DTaskbarControlPrivate : public DObjectPrivate
{
    D_DECLARE_PUBLIC(DTaskbarControl)

public:
    DTaskbarControlPrivate(DTaskbarControl *q);
    ~DTaskbarControlPrivate();

    int m_counter;
    bool m_counterVisible;
    double m_progress;
    bool m_progressVisible;
};

DGUI_END_NAMESPACE

#endif // DTASKBARCONTROL_P_H
