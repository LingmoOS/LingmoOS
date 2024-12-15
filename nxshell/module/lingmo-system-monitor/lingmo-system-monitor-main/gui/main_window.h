// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H
#include "user_page_widget.h"
#include <DMainWindow>
#include <DShadowLine>
#include <DStackedWidget>
#include "dbus/dbusforsystemomonitorpluginservce.h"

DWIDGET_USE_NAMESPACE

class Toolbar;
class SystemServicePageWidget;
class ProcessPageWidget;
class Settings;
class UserPageWidget;
class MainWindow : public DMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)

public:
    /**
     * @brief MainWindow Constructor
     * @param parent Parent Object
     */
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    /**
     * @brief toolbar Get toolbar instance
     * @return Toolbar instance
     */
    inline Toolbar *toolbar() const { return m_toolbar; }

    //!
    //! \brief raiseWindow 窗口置顶显示
    //!
    void raiseWindow();

Q_SIGNALS:
    /**
     * @brief Kill application operation requested signal
     */
    void killProcessPerformed();
    /**
     * @brief Display mode changed signal
     * @param mode Display mode
     */
    void displayModeChanged(int mode);
    /**
     * @brief Loading status changed signal
     * @param loading Loading state flag
     */
    void loadingStatusChanged(bool loading);

public Q_SLOTS:
    /**
     * @brief Initialize ui components
     */
    void initDisplay();

    /**
     * @brief onLoadStatusChanged
     */
    void onLoadStatusChanged(bool loading);

    /**
     * @brief onStartMonitorJob
     */
    void onStartMonitorJob();

    //!
    //! \brief onDetailInfoByDbus 收到dbus传来的需要切换界面的操作
    //! \param msgCode
    //!
    void onDetailInfoByDbus(QString msgCode);

    /**
     * @brief popupSettingsDialog
     */
    void popupSettingsDialog();

    /**
     * @brief onKillProcess
     */
    void onKillProcess();

protected:
    /**
     * @brief Initialize ui components
     */
    void initUI();
    /**
     * @brief Initialize connections
     */
    void initConnections();

    /**
     * @brief resizeEvent Resize event handler
     * @param event Resize event
     */
    void resizeEvent(QResizeEvent *event) override;
    /**
     * @brief closeEvent Close event handler
     * @param event Close event
     */
    void closeEvent(QCloseEvent *event) override;
    /**
     * @brief eventFilter Filters events if this object has been installed as an event filter for the watched object.
     * @param obj Object being watched
     * @param event Event to be filtered
     * @return To filter the event out, return true; otherwise return false
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

    /**
     * @brief showEvent Show event handler
     * @param event Show event
     */
    void showEvent(QShowEvent *event) override;

private:
    Settings *m_settings = nullptr;

    Toolbar *m_toolbar = nullptr;
    DStackedWidget *m_pages = nullptr;
    ProcessPageWidget *m_procPage = nullptr;
    SystemServicePageWidget *m_svcPage = nullptr;
    UserPageWidget *m_accountProcPage = nullptr;
    bool m_initLoad = false;
    DShadowLine *m_tbShadow  = nullptr;
    QWidget *m_focusedWidget = nullptr;

    DBusForSystemoMonitorPluginServce *m_pDbusService;
};

#endif  // MAIN_WINDOW_H
