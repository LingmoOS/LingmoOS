// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DMEMORYWARNINGDIALOG_H
#define DMEMORYWARNINGDIALOG_H

#include "processinfomodel.h"

#include <QDialog>
#include <QPushButton>
#include <QLabel>

#include <dabstractdialog.h>

class ProcessInfoModel;

DWIDGET_USE_NAMESPACE

bool confirm(const QPixmap &icon);

class DMemoryWarningDialog : public DAbstractDialog
{
    Q_OBJECT

public:
    explicit DMemoryWarningDialog(QWidget *parent = nullptr);
    ~DMemoryWarningDialog();

    enum TipsType
    {
        LaunchApp,
        OpenChromeTab,
        ExecuteCommand,
    };

public slots:
    void updateAppInfo(const QString &appInfo);

private:
    void keyPressEvent(QKeyEvent *e);
    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);

private slots:
    void onNeedMemChanged(const qulonglong needed);
    void onContinueClicked();
    void onCancelClicked();
    void updateTips();

private:
    int m_needed;
    TipsType m_tipsType;
    QString m_appName;
    QPointer<StartManagerInter> m_startManagerInter;
    ProcessInfoModel *m_infoModel;
    QLabel *m_icon;
    QLabel *m_memNeeded;
    QPushButton *m_cancelButton;
    QPushButton *m_continueButton;
};

#endif // DMEMORYWARNINGDIALOG_H
