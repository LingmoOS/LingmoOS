// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "animationdialog.h"

#include <DMainWindow>
#include <DSpinner>
#include <DPushButton>
#include <DDialog>

DWIDGET_USE_NAMESPACE

class CenterWidget;
class BufferWin;
class DMDbusHandler;

/**
 * @class MainWindow
 * @brief 主窗口类
 */

class MainWindow : public Dtk::Widget::DMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    static MainWindow *instance();

    /**
     * @brief 获取登录结果
     * @return 返回登录认证结果
     */
    QString getRootLoginResult();

protected:

    /**
     * @brief 窗口关闭事件
     */
    void closeEvent(QCloseEvent *event) override;

private slots:

    /**
     * @brief 刷新按钮点击响应的槽函数
     */
    void onRefreshButtonClicked();

    /**
     * @brief 获取磁盘信息
     */
    void getDeviceInfo();

private:

    /**
     * @brief 初始化界面
     */
    void initUi();

    /**
     * @brief 紧凑模式按钮设置
     */
    void setSizebyMode(DPushButton *button);
    void resetButtonsSize();

    /**
     * @brief 初始化连接
     */
    void initConnection();

signals:

public slots:

    /**
     * @brief Handle退出槽函数
     */
    void onHandleQuitAction();

    /**
     * @brief 接收是否显示动画的槽函数
     * @param isShow ture显示，false不显示
     */
    void onShowSpinerWindow(bool isShow = false, const QString &title = "");

private:
    CenterWidget *m_central;
    DMDbusHandler *m_handler;
    DPushButton *m_btnRefresh; // 刷新按钮
    AnimationDialog *m_dialog;
};

#endif // MAINWINDOW_H
