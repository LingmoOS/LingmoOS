// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEDRAGCOMMON_H
#define DFILEDRAGCOMMON_H

#include <dtkgui_global.h>

DGUI_BEGIN_NAMESPACE

enum DFileDragState
{
    Failed = -1,
    Stalled,
    Paused,
    Running,
    Finished,
    CustomState = 0x100
};

DGUI_END_NAMESPACE

#endif // DFILEDRAGCOMMON_H
