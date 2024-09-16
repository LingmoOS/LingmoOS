// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRAWDIALOG_H
#define DRAWDIALOG_H

#include "config.h"

#ifdef USE_DTK
#include <DDialog>
DWIDGET_USE_NAMESPACE
class DrawDialog : public DDialog
#else
#include "dialog.h"
class DrawDialog : public MessageDlg
#endif
{
    Q_OBJECT
public:
    explicit DrawDialog(QWidget *parent = nullptr);


signals:
    void signalSaveToDDF();
    void singalDoNotSaveToDDF();


protected:
    void keyPressEvent(QKeyEvent *e) override;
    void showEvent(QShowEvent *event) override;

};

#endif // DRAWDIALOG_H
