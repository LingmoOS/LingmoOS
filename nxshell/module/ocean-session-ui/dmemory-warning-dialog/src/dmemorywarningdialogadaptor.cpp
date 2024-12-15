// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dmemorywarningdialogadaptor.h"

DMemoryWarningDialogAdaptor::DMemoryWarningDialogAdaptor(DMemoryWarningDialog *dialog)
    : QDBusAbstractAdaptor(dialog)

    , dialog(dialog)
{

}

DMemoryWarningDialogAdaptor::~DMemoryWarningDialogAdaptor()
{

}

void DMemoryWarningDialogAdaptor::Show(const QString &launchInfo)
{
    qDebug() << Q_FUNC_INFO << launchInfo;

    dialog->show();
    dialog->updateAppInfo(launchInfo);
}

void DMemoryWarningDialogAdaptor::Hide()
{
    qDebug() << Q_FUNC_INFO;

    dialog->hide();
}
