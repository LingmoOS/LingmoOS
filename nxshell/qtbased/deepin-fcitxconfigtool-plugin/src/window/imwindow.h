// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMWIDOW_H
#define IMWIDOW_H

#include <QWidget>
#include <QVBoxLayout>


class IMAddWindow;
class IMSettingWindow;
class QStackedWidget;
class ShortcutKeyWindow;
class IMWindow : public QWidget
{
    enum WindowHendel {
        PopIMSettingWindow = 0,
        PopIMAddWindow,
        PopShortcutKeyWindow
    };
    Q_OBJECT
public:
    explicit IMWindow(QWidget *parent = nullptr);
    virtual ~IMWindow();
signals:
    void requestNextPage(QWidget * const w) const;
private:
    void initUI(); //初始化界面
    void initConnect(); //初始化信号槽
    void initFcitxInterface(); //初始化fcitx接口

private:
    QStackedWidget *m_stackedWidget {nullptr}; //栈窗口
    IMSettingWindow *m_settingWindow {nullptr}; //输入法编辑窗口
    IMAddWindow *m_addWindow {nullptr}; //输入法添加窗口
    ShortcutKeyWindow *m_shortcutKeyWindow {nullptr}; //快捷键冲突提示界面
    QVBoxLayout *m_pLayout {nullptr};
};

#endif // IMWIDOW_H
