// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WARNINGDIALOG_H
#define WARNINGDIALOG_H

#include <ddialog.h>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class WarningDialog : public DDialog
{
    Q_OBJECT

public:
    explicit WarningDialog(QWidget *parent = nullptr);
    ~WarningDialog();
};

#endif // WARNINGDIALOG_H
