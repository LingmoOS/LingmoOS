// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <DDialog>
#include <DLineEdit>
#include <DComboBox>
#include <DSpinBox>

DWIDGET_USE_NAMESPACE
class SettingDialog : public DDialog
{
    Q_OBJECT
public:
    explicit SettingDialog(QWidget *parent = nullptr);

    QVariantMap searchConfig();

private:
    void initUI();

private:
    DLineEdit *pathEdit { nullptr };
    DLineEdit *indexPathEdit { nullptr };
    DComboBox *modeCheckBox { nullptr };
    DSpinBox *maxResultBox { nullptr };
};

#endif   // SETTINGDIALOG_H
