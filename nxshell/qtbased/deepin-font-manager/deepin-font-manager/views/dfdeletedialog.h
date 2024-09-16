// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFDELETEDIALOG_H
#define DFDELETEDIALOG_H

#include "dfontbasedialog.h"
#include "signalmanager.h"

#include <DDialog>
#include <DLabel>
#include <DPushButton>
#include <DWarningButton>

DWIDGET_USE_NAMESPACE

class DFontMgrMainWindow;

/*************************************************************************
 <Class>         DFontBaseDialog
 <Description>   此类为字体删除确认页面，主要功能为确认删除字体和取消删除字体
 <Author>
 <Note>          null
*************************************************************************/
class DFDeleteDialog : public DDialog
{
    Q_OBJECT
public:
    DFDeleteDialog(DFontMgrMainWindow *win, int deleteCnt = 0, int systemCnt = 0, bool hasCurrent = false, QWidget *parent = nullptr);

    static constexpr int DEFAULT_WINDOW_W = 380;
    static constexpr int DEFAULT_WINDOW_H = 160;

public slots:
    //适应系统字体变化
    void onFontChanged(const QFont &font);

protected:
    //响应键盘press事件中的esc按键
    void keyPressEvent(QKeyEvent *event) override;
signals:

private slots:
    //根据主题设置页面颜色
    void setTheme();
private:
    //初始化字体删除待确认页面
    void initUI();
    //初始化用于判断删除或取消操作的信号和槽的链接函数
    void initConnections();
    //初始化页面提示信息标题
    void initMessageTitle();
    //初始化页面提示信息内容
    void initMessageDetail();

    DLabel *messageTitle;
    DLabel *messageDetail;

    DFontMgrMainWindow *m_mainWindow;
    SignalManager *m_signalManager = SignalManager::instance();
    int m_deleteCnt;
    int m_systemCnt;
    int m_old_width {0};
    int m_old_height {0};
    int m_w_wd {0};
    int m_w_ht {0};
    int m_count {0};
    bool m_deleting;
    bool m_hasCurFont;
};

/*************************************************************************
 <Class>         DFHandleTTCDialog
 <Description>   此类为DFDeleteTTCDialog和DFDisableTTCDialog的基类
 <Author>
 <Note>          null
*************************************************************************/
class DFHandleTTCDialog : public DFontBaseDialog
{
    Q_OBJECT
public:
    DFHandleTTCDialog(DFontMgrMainWindow *win, QString &file, QWidget *parent = nullptr);

    static constexpr int DEFAULT_WINDOW_W = 380;
    static constexpr int DEFAULT_WINDOW_H = 160;
    bool getDeleting();
    bool getAapplyToAll();
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
    /**
     * @brief execDialog 优化后的exec
     * @return
     */
    int execDialog();

protected:
    //响应键盘press事件中的esc按键
    void keyPressEvent(QKeyEvent *event) override;
    //变化事件处理，比如字体、字号变化
    void changeEvent(QEvent *event) override;

    //初始化字体删除待确认页面
    void initUI();
    //初始化用于判断删除或取消操作的信号和槽的链接函数
    void initConnections();
    //初始化页面提示信息标题
    void initMessageTitle();

    // 部分按钮内容由子类设置
    virtual void setConfirmBtnText() = 0;
    virtual void setMessageTitleText() = 0;

    //初始化页面提示信息内容
    void initMessageDetail();
    //初始化页面按钮
    QLayout *initBottomButtons();
    //自动换行处理，根据字符串宽度适当添加'\n'
    void autoFeed(DLabel *label);

    DLabel *messageTitle = nullptr;
    DCheckBox *applyAllCkb = nullptr;

    QString m_messageTitleText;
    int m_iLabelOldHeight = 0;
    int m_iDialogOldHeight = 0;

    DPushButton *m_cancelBtn = nullptr;
    DWarningButton *m_confirmBtn = nullptr;
    DFontMgrMainWindow *m_mainWindow = nullptr; //父窗口
    bool m_confirm = false; //是否删除
    bool m_bAapplyToAll = false; //是否应用全部
    QString fontset; // 字体集内所有字体名称
};

/*************************************************************************
 <Class>         DFDeleteTTCDialog
 <Description>   此类为ttc字体删除确认页面，主要功能为确认删除字体集和取消删除字体集
 <Author>
 <Note>          null
*************************************************************************/
class DFDeleteTTCDialog : public DFHandleTTCDialog
{
    Q_OBJECT
public:
    DFDeleteTTCDialog(DFontMgrMainWindow *win, QString &file, QWidget *parent = nullptr);

    void setConfirmBtnText() override;
    void setMessageTitleText() override;

};

/*************************************************************************
 <Class>         DFDisableTTCDialog
 <Description>   此类为ttc字体禁用/启用确认页面，主要功能为确认禁用字体集和取启用字体集
 <Author>
 <Note>          null
*************************************************************************/
class DFDisableTTCDialog : public DFHandleTTCDialog
{
    Q_OBJECT
public:
    DFDisableTTCDialog(DFontMgrMainWindow *win, QString &file, bool &isEnable, QWidget *parent = nullptr);

protected:
    void setConfirmBtnText() override;
    void setMessageTitleText() override;

private:
    bool m_isEnable; // true:启用;false:禁用
};
#endif  // DFDELETEDIALOG_H
