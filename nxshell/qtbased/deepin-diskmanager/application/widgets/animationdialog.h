// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef ANIMATIONDIALOG_H
#define ANIMATIONDIALOG_H

#include "customcontrol/ddbase.h"

#include <DSpinner>
#include <DLabel>

#include <QWidget>

DWIDGET_USE_NAMESPACE

/**
 * @class AnimationDialog
 * @brief 动画窗口类
 */

class AnimationDialog : public DDialog
{
    Q_OBJECT
public:
    explicit AnimationDialog(QWidget *parent = nullptr);

    /**
     * @brief 设置动画是否可见
     * @param isShow ture显示，false不显示
     * @param title 提示文本
     */
    void setShowSpinner(bool isShow, const QString &title = "");

signals:

public slots:

protected:
    /**
     * @brief 键盘按下事件
     * @param event事件
     */
    void keyPressEvent(QKeyEvent *event) override;

private:

    /**
     * @brief 初始化界面
     */
    void initUi();

    DSpinner *m_spinner;
    DLabel *m_label;
};

#endif // ANIMATIONDIALOG_H
