// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "graphicsdriverproxy.h"
#include "device.h"

#include <QVBoxLayout>
#include <QSettings>
#include <DMainWindow>
#include <DWaterProgress>
#include <QPushButton>
#include <QStackedLayout>
#include <DThemeManager>
#include <DSuggestButton>
#include <DGuiApplicationHelper>
#include <DSpinner>



DWIDGET_USE_NAMESPACE


class QLabel;
class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    enum SolutionType {
        NoResolution = 0,
        AMD,
        NVIDIA,
        INTEL,
        INTEL_NVIDIA,
        INTEL_NVIDIA_USE_INTEL,
        INTEL_NVIDIA_USE_NVIDIA
    };
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    void keyPressEvent(QKeyEvent *e);
    void closeEvent(QCloseEvent *event);
    void noResolutions();
    void loadDevice();
    void setVendorIcon();

private Q_SLOTS:
    void loadResolutions();
    void onResolutionSelected();
    void onUpdateBtnClicked();
    void onToggleBtnClicked();
    void onRebootBtnClicked();
    void onCancelBtnClicked();
    void onTryClicked();
    void onPreInstallProgress(int progress);
    void onThemeChanged(DGuiApplicationHelper::ColorType type);



private:
    int m_usedIndex = 0;
    int m_selectedIndex = 0;

    QWidget *m_centerWidget;
    QVBoxLayout *m_resolutionsLayout;
    QLabel *m_vendorIcon;
    QLabel *m_vendorName;
    QLabel *m_tipsIcon;
    QLabel *m_warningTips;
    QLabel *m_installTips;
    DSpinner *m_spinner;
    QWidget *m_resolutionsWidget;
    QPushButton *m_toggleButton;
    QPushButton *m_updateButton;
    DSuggestButton *m_rebootButton;
    QPushButton *m_cancelButton;
    QPushButton *m_tryButton;
    QPushButton *m_okButton;
    ComDeepinDaemonGraphicsDriverInterface *m_graphicsDriver = nullptr;
    DeviceList m_devices;
    bool m_startPreInstall = false;

};

#endif
