// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESS_PAGE_WIDGET_H
#define PROCESS_PAGE_WIDGET_H

#include "ui_common.h"
#include "model/cpu_info_model.h"

#include <DApplicationHelper>
#include <DButtonBox>
#include <DFrame>
#include <DStackedWidget>
#include <DLabel>
#include <DSpinner>
DWIDGET_USE_NAMESPACE

class MainWindow;
class MonitorCompactView;
class MonitorExpandView;
class ProcessTableView;
class Settings;
class XWinKillPreviewWidget;
class DetailViewStackedWidget;

/**
 * @brief Process & performance monitor view frame
 */
class ProcessPageWidget : public DFrame
{
    Q_OBJECT
    //记录进程列表按钮选中状态
    typedef enum _ProcessButtonCheckedType {
        MY_APPS = 0,
        USER_PROCESS = 1,
        ALL_PROCESSS = 2
    } ProcessButtonCheckedType;
public:
    /**
     * @brief Default constructor
     * @param parent Parent object
     */
    explicit ProcessPageWidget(DWidget *parent = nullptr);
    /**
     * @brief Destructor
     */
    ~ProcessPageWidget();

    /**
     * @brief Initialize ui components
     */
    void initUI();
    /**
     * @brief Initialize connections
     */
    void initConnections();

    /**
     * @brief Filters events if this object has been installed as an event filter for the watched object
     * @param obj Object being watched
     * @param event Event to be filtered
     * @return To filter this event out, return true; otherwise return false
     */
    bool eventFilter(QObject *obj, QEvent *event);

    void switchProcessPage();

    //!
    //! \brief switchCurrentNoFilterPage 切换到总进程页面
    //!
    void switchCurrentNoFilterPage();

public Q_SLOTS:
    /**
     * @brief Switch performance display mode when required
     * @param mode Display mode
     */
    void switchDisplayMode(int mode);

protected:
    /**
     * @brief Paint event handler
     */
    void paintEvent(QPaintEvent *);

private Q_SLOTS:
    /**
     * @brief Popup up process kill confirm dialog
     * @param pid Process id
     */
    void popupKillConfirmDialog(int pid);
    /**
     * @brief Show kill application preview widget
     */
    void showWindowKiller();
    /**
     * @brief Create application preview widget
     */
    void createWindowKiller();

    /**
     * @brief Change icons when theme changed
     * @param themeType Theme currently applied
     */
    void changeIconTheme(DApplicationHelper::ColorType themeType);

    /**
     * @brief 列表数据刷新，更新应用和进程统计
     */
    void onAppAndProcCountUpdated(int appCount, int procCount);

    /**
     * @brief 详情页切换
     */
    void onDetailWidgetChanged(int index);

    /**
     * @brief 延迟加载左侧数据控件
     */
    void onLoadLeftDataWidgetDelay();

    /**
     * @brief 应用程序视图响应槽函数
     */
    void onAppButtonClicked();
    /**
     * @brief 我的进程视图响应槽函数
     */
    void onUserProcButtonClicked();
    /**
     * @brief 所有进程视图响应槽函数
     */
    void onAllProcButtonClicked();
private:
    // global setttings instance
    Settings *m_settings = nullptr;

    // stacked widget to hold expand & compact monitor view
    DStackedWidget *m_views = nullptr;

    DetailViewStackedWidget *m_rightStackView = nullptr;
    // expand monitor view
    MonitorExpandView *m_expandView = nullptr;
    // compact monitor view
    MonitorCompactView *m_compactView = nullptr;

    // process view context text
    DLabel *m_procViewMode = nullptr;
    // process view context summary
    DLabel *m_procViewModeSummary = nullptr;
    // show app view mode button
    DButtonBoxButton *m_appButton = nullptr;
    // show my proc view mode button
    DButtonBoxButton *m_myProcButton = nullptr;
    // show all proc mode button
    DButtonBoxButton *m_allProcButton = nullptr;

    // process table view
    ProcessTableView *m_procTable = nullptr;
    QWidget *m_processWidget = nullptr;

    //loading spinner
    DSpinner *m_spinner = nullptr;
    DLabel *m_loadingLabel = nullptr;
    QWidget *m_spinnerWidget = nullptr;

    DStackedWidget *m_loadingAndProcessTB = nullptr;
    ProcessButtonCheckedType m_procBtnCheckedType;
    //paint event 计数
    int m_ipaintDelayTimes = 0;
    //所有进程数量
    int m_iallProcNum = 0;

    // kill process by window selection preview widget
    XWinKillPreviewWidget *m_xwkillPreview = nullptr;
    CPUInfoModel *m_model;
};

#endif  // PROCESS_PAGE_WIDGET_H
