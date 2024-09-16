// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DMEMORYWARNINGDIALOGADAPTOR_H
#define DMEMORYWARNINGDIALOGADAPTOR_H

#include "dmemorywarningdialog.h"

#include <QDBusAbstractAdaptor>

class DMemoryWarningDialogAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.MemoryWarningDialog1")

public:
    explicit DMemoryWarningDialogAdaptor(DMemoryWarningDialog *dialog);
    ~DMemoryWarningDialogAdaptor();

public Q_SLOTS:
    void Show(const QString &launchInfo);
    void Hide();

private:
    DMemoryWarningDialog *dialog;
};

#endif // DMEMORYWARNINGDIALOGADAPTOR_H
