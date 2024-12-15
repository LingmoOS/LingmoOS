// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTREELANDWINDOWMANAGERHELPER_H
#define DTREELANDWINDOWMANAGERHELPER_H

#include "dtkgui_global.h"
#include "dwindowmanagerhelper.h"
#include <QObject>

DGUI_BEGIN_NAMESPACE

class TreelandWindowManagerHelper : public DWindowManagerHelper
{
public:
    explicit TreelandWindowManagerHelper(QObject *parent = 0);
    bool hasBlurWindow() const;
    bool hasComposite() const;
    bool hasNoTitlebar() const;
};

DGUI_END_NAMESPACE

#endif // DTREELANDWINDOWMANAGERHELPER_H
