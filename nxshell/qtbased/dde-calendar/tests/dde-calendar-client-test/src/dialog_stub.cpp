// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dialog_stub.h"

#include "dialog/dcalendarddialog.h"
#include "dialog/myscheduleview.h"

#include <QObject>

int calendarDDialogExecReturn = 0;

int calendar_DDialog_Exec_stub(void *obj)
{
    Q_UNUSED(obj)
    return calendarDDialogExecReturn;
}
void calendarDDialogExecStub(Stub &stub)
{
    typedef int (*fptr)(DDialog *);
    fptr A_foo = (fptr)(&DDialog::exec);
    stub.set(A_foo, calendar_DDialog_Exec_stub);

    typedef int (*fptr2)(QDialog *);
    fptr2 A_foo2 = (fptr2)(&QDialog::exec);
    stub.set(A_foo2, calendar_DDialog_Exec_stub);
}
