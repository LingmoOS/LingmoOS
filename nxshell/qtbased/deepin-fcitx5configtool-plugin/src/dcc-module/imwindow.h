// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 Deepin Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCC_MODULE_IMWINDOW_H
#define DCC_MODULE_IMWINDOW_H

#include <QVBoxLayout>
#include <QWidget>

class IMAddWindow;
class IMSettingWindow;
class QStackedWidget;

class IMWindow : public QWidget
{
    Q_OBJECT

public:
    explicit IMWindow(QWidget *parent = nullptr);
    virtual ~IMWindow();

private:
    void initUI(); // 初始化界面

private:
    QStackedWidget *m_stackedWidget{ nullptr };  // 栈窗口
    IMSettingWindow *m_settingWindow{ nullptr }; // 输入法编辑窗口
    QVBoxLayout *m_pLayout{ nullptr };
};

#endif // !DCC_MODULE_IMWINDOW_H
