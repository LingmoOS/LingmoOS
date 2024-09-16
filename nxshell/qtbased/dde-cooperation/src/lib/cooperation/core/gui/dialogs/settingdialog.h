// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include "global_defines.h"

#include <QMouseEvent>

namespace cooperation_core {

class SettingDialogPrivate;
class SettingDialog : public CooperationAbstractDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog() override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void loadConfig();

private:
    QScopedPointer<SettingDialogPrivate> d;
};

}   // namespace cooperation_core

#endif   // SETTINGDIALOG_H
