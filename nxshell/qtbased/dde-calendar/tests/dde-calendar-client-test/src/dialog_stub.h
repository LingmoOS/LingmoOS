// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIALOG_STUB_H
#define DIALOG_STUB_H

#include "../third-party_stub/stub.h"

//DCalendarDDialog
//对话框 exec 打桩
int calendar_DDialog_Exec_stub(void *obj);

void calendarDDialogExecStub(Stub &stub);

extern int calendarDDialogExecReturn;

#endif // DIALOG_STUB_H
