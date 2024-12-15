// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kill_process_confirm_dialog.h"

#include "common/common.h"

#include <DApplication>
#include <DHiDPIHelper>
#include <DWidget>

#include <QMessageBox>

// constructor
KillProcessConfirmDialog::KillProcessConfirmDialog(QWidget *parent)
    : DDialog(parent)
{
    // dialog icon
    setIcon(QIcon::fromTheme("dialog-warning"));

    // button click event
    connect(this, &KillProcessConfirmDialog::buttonClicked, this,
            &KillProcessConfirmDialog::onButtonClicked);
}

// button click event handler
void KillProcessConfirmDialog::onButtonClicked(int index, const QString &)
{
    if (index == 1) {
        // ok button clicked
        setResult(QMessageBox::Ok);
    } else {
        // cancel button clicked
        setResult(QMessageBox::Cancel);
    }
}
