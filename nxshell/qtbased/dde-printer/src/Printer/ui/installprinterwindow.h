// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INSTALLPRINTERWINDOW_H
#define INSTALLPRINTERWINDOW_H
#include "addprinter.h"

#include <DMainWindow>
#include <DImageButton>

DWIDGET_USE_NAMESPACE
DWIDGET_BEGIN_NAMESPACE
class DIconButton;
class DSpinner;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QComboBox;
class QPushButton;
class QWidget;
class QLabel;
class QStackedWidget;
QT_END_NAMESPACE
enum InstallationStatus {
    Installing = 0,
    Installed,
    Printing,
    Printed,
    PrintFailed,
    Reinstall,
    Unknown
};

class PrinterTestJob;

class InstallPrinterWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit InstallPrinterWindow(QWidget *parent = nullptr);
    virtual ~InstallPrinterWindow() override;

    void setTask(AddPrinterTask *task);
    void setDefaultPrinterName(const QString &name);
    void setDevice(const TDeviceInfo &device);
    void copyDriverData(QComboBox *source) Q_DECL_DEPRECATED;
    /**
    * @projectName   Printer
    * @brief         拷贝上一级界面的备选驱动
    * @author        liurui
    * @date          2019-11-01
    */
    void copyDriverData(const QMap<QString, QVariant> &itemDataMap);

private:
    void initUI();
    void initConnections();

    void setStatus(InstallationStatus status);

    void feedbackPrintTestPage(bool success);
    void showHplipSetupInstruction();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 响应取消按钮
    void cancelBtnClickedSlot();
    // 响应左侧按钮
    void leftBtnClickedSlot();
    // 响应右侧按钮
    void rightBtnClickedSlot();
public slots:
    // 响应后台安装打印机线程的状态消息
    void receiveInstallationStatusSlot(int status);

signals:
    /**
    * @projectName   Printer
    * @brief         通知主界面刷新打印机列表，参数是新
    * 添加的打印机名称，方便后续选中操作
    * @author        liurui
    * @date          2019-11-07
    */
    void updatePrinterList(const QString &newPrinterName);
    void showParentWindows();

private:
    DSpinner *m_pSpinner;

    QLabel *m_pStatusImageLabel;
    QLabel *m_pStatusLabel;
    QLabel *m_pTipLabel;
    QLabel *m_pHplipTipLabel;
    QPushButton *m_pCancelInstallBtn;

    // 这两个按钮需要在不同的阶段切换文案，实现不同功能，实现复用
    QPushButton *m_pCheckPrinterListBtn;
    QPushButton *m_pPrinterTestPageBtn;

    QComboBox *m_pDriverCombo;

    // 用于安装失败之后，返回上级界面，有两种情况
    QWidget *m_pParentWidget;
private:
    InstallationStatus m_status;
    QString m_printerName;

    TDeviceInfo m_device;

    bool m_bInstallFail;

    PrinterTestJob *m_testJob;

    AddPrinterTask *m_pAddPrinterTask;
};

#endif // INSTALLPRINTERWINDOW_H
