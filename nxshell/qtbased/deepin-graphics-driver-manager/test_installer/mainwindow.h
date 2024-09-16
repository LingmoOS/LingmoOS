// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "graphicsdriverproxy.h"
#include "closeButton.h"

#include <QVBoxLayout>
#include <QSettings>
#include <DMainWindow>
#include <DWaterProgress>
#include <QPushButton>
#include <QStackedLayout>
#include <DThemeManager>
#include <DSuggestButton>
#include <DGuiApplicationHelper>
#include <QToolButton>



DWIDGET_USE_NAMESPACE

class QLabel;
class MainWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
private:
    void closeEvent(QCloseEvent *event) override;
    void updateProgress();
    void updateInstallState(bool success);
    void reboot();
private Q_SLOTS:
    void onCancelBtnClicked();
    void onInstall();
    void onThemeChanged(DGuiApplicationHelper::ColorType type);

private:
    QWidget *m_centerWidget;
    QLabel *m_installStateIcon;
    QLabel *m_installState;
    QLabel *m_warningTips;
    DWaterProgress *m_waterProgress;
    QPushButton *m_cancelButton;
    DSuggestButton *m_rebootButton;
    QTimer m_timer;
    int m_rebootDelay;
    int m_process;
    CloseButton *m_closeButton;
    ComDeepinDaemonGraphicsDriverInterface *m_graphicsDriver = nullptr;
};

#endif
