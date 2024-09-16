// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DDialog>

DWIDGET_USE_NAMESPACE
class AlertDialog : public DDialog
{
    Q_OBJECT
public:
    explicit AlertDialog(QWidget *parent, const QString title, const QString acceptText = tr("Confirm"), ButtonType type=DDialog::ButtonType::ButtonRecommend);
    explicit inline AlertDialog(QWidget *parent):DDialog (parent){}
    int execWarningDialog(const QString &title, const QString &message, const QString &buttonText);

private:
    void initUI(const QString &title, const QString &acceptText, DDialog::ButtonType type);

};
