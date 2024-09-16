// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHOOSEFILEBUTTON_H
#define CHOOSEFILEBUTTON_H

#include <DLabel>
#include <DCommandLinkButton>

DWIDGET_USE_NAMESPACE

/**
 * @brief The ChooseFileButton class
 * 文件选择对话框中的文件选择按钮
 * 一开始使用的控件为DPushButton ,后UI与测试提出按钮需要跟随活动色变化。后修改为DCommandLinkButton.
 */
class ChooseFileButton : public DCommandLinkButton
{
    Q_OBJECT
public:
    explicit ChooseFileButton(QString text, QWidget *parent = nullptr);

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif  // CHOOSEFILEBUTTON_H
