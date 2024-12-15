// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CCUTDIALOG_H
#define CCUTDIALOG_H

#include "dialog.h"

class DRAWLIB_EXPORT CCutDialog : public MessageDlg
{
    Q_OBJECT
public:
    explicit CCutDialog(QWidget *parent = nullptr);

    enum CutStatus {
        Save = 0,  // 确认裁剪
        Cancel, // 保持现状
        Discard// 取消裁剪
    };

    CutStatus getCutStatus();
    int exec() override;

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void showEvent(QShowEvent *event) override;
private:
    CutStatus m_cutStatus;
};

#endif // CCUTDIALOG_H
