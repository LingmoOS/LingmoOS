// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../src/window/namespace.h"
#include "window/modules/common/compixmap.h"

#include <DDialog>

DWIDGET_USE_NAMESPACE

DEF_NAMESPACE_BEGIN
class PwdLevelChangedDlg : public DDialog
{
    Q_OBJECT
public:
    PwdLevelChangedDlg(QWidget *parent = nullptr);
    ~PwdLevelChangedDlg();

private:
};
DEF_NAMESPACE_END
