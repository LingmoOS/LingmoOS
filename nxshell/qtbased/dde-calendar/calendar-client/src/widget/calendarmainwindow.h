// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALENDARMAINWINDOW_H
#define CALENDARMAINWINDOW_H

#include "dschedule.h"
#include "jobtypelistview.h"
#include "settingdialog.h"

#include <DLabel>
#include <DButtonBox>
#include <DFrame>
#include <DMainWindow>
#include <DSearchEdit>
#include <DToolButton>
#include <DMenu>
#include <DSettingsDialog>
#include <DSysInfo>

#include <QStackedLayout>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

DWIDGET_USE_NAMESPACE
class CYearWindow;
class CMonthWindow;
class CWeekWindow;
class CDayWindow;
class SidebarView;
class CScheduleSearchView;
class AnimationStackedWidget;
class CScheduleDataManage;
class CTitleWidget;
class CMyScheduleView;

class Calendarmainwindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit Calendarmainwindow(int index, QWidget *w = nullptr);
    ~Calendarmainwindow() override;
    void viewWindow(int type, const bool showAnimation = false);
    void updateHeight();
    void setSearchWidth(int w);
public slots:
    void slotTheme(int type);
    void slotOpenSchedule(QString job);
    void slotViewShortcut();
    //更新当前时间
    void slotCurrentDateUpdate();
    void slotSetSearchFocus();
private:
    void initUI();
    void initConnection();
    void initData();
    //创建视图
    void createview();
    //重置界面大小
    void resizeView();
    void removeSyncToast();
protected:
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    bool event(QEvent *event) override;
private slots:
    //获取程序状态
    void slotapplicationStateChanged(Qt::ApplicationState state);
    //隐藏提示框
    void setScheduleHide();
    //点击按钮切换视图
    void slotstackWClicked(QAbstractButton *bt);
    void slotWUpdateSchedule();
    void slotSreturnPressed();
    void slotStextChanged();
    //搜索框有焦点时隐藏提示框
    void slotStextfocusChanged(bool onFocus);
    void slotSearchEdit();
    //单击搜索日程动画设置
    void slotSearchSelectSchedule(const DSchedule::Ptr &scheduleInfo);
    //添加视图阴影
    void slotViewtransparentFrame(const bool isShow);
    //启用buttonbox
    void slotSetButtonBox();
    //切换视图
    void slotSwitchView(const int viewIndex);
    //新建日程
    void slotNewSchedule();
    void slotOpenSettingDialog();
    //触发隐藏政策
    void slotShowPrivacy();
    //快捷键删除选中日程
    void slotDeleteitem();
    //设置最大大小
    void slotSetMaxSize(int size = 0);
    void slotSearchFocusSwitch();
    //帐户侧边栏状态发生改变事件
    void slotSidebarStatusChange(bool);

    //显示同步提示
    void slotShowSyncToast(int syncNum);

    void slotAccountUpdate();

private:
    DLabel *m_icon = nullptr;
    AnimationStackedWidget *m_stackWidget = nullptr;
    DButtonBox *m_buttonBox = nullptr;
    DSearchEdit *m_searchEdit = nullptr;
    CYearWindow *m_yearwindow = nullptr;
    CMonthWindow *m_monthWindow = nullptr;
    CWeekWindow *m_weekWindow = nullptr;
    CDayWindow *m_DayWindow = nullptr;
    CTitleWidget *m_titleWidget = nullptr;
    SidebarView *m_sidebarView = nullptr;
    bool m_searchflag = false;
    CScheduleSearchView *m_scheduleSearchView = nullptr;
    DFrame *m_contentBackground = nullptr;
    int m_priindex = 3; //默认打开日视图双击
    DFrame *m_transparentFrame = nullptr;
    bool m_opensearchflag = false;
    int m_scheduleSearchViewMaxWidth;
    QPropertyAnimation *m_animation = nullptr;
    QTimer *m_currentDateUpdateTimer = nullptr;
    DIconButton *m_newScheduleBtn {nullptr}; //全局的新建日程按钮
    CSettingDialog *m_dsdSetting {nullptr};
    JobTypeListView *m_jobTypeListView {nullptr};
    //日历打开默认显示视图
    int m_defaultIndex;
    bool m_setButtonFocus {false};
    bool m_isNormalStateShow {true}; //是否为正常状态显示
    QPoint m_startPos;

    CMyScheduleView *m_dlg = Q_NULLPTR;

};

#endif // CALENDARMAINWINDOW_H
