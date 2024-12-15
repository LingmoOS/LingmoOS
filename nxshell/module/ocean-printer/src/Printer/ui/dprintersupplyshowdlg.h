// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DPRINTERSUPPLYSHOWDLG_H
#define DPRINTERSUPPLYSHOWDLG_H

#include <cupssnmp.h>

#include <DDialog>
#include <DPushButton>
#include <DSpinner>

#include <QStackedWidget>
#include <QMap>
#include <refreshsnmpbackendtask.h>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class DPrinterSupplyShowDlg : public DDialog
{
    Q_OBJECT
public:
    DPrinterSupplyShowDlg(RefreshSnmpBackendTask *task, QWidget *parent = nullptr);
    ~DPrinterSupplyShowDlg();

protected:
    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    void initUI();
    void initConnection();
    void moveToParentCenter();
    QWidget* initColorSupplyItem(const SUPPLYSDATA& info, bool bColor);
    bool isColorPrinter();
    void initColorTrans();
    QString getTranslatedColor(const QString& strColor);

private slots:
    void supplyFreshed(const QString&, bool);

private:
    QStackedWidget* m_pStackedWidget;
    DPushButton* m_pConfirmBtn;
    QMap<QString,QString> m_mapColorTrans;
    QWidget* m_pContentWidget;
    RefreshSnmpBackendTask* m_pFreshTask;
    DSpinner* m_pFreshSpinner;
};

#endif // DPRINTERSUPPLYSHOWDLG_H
