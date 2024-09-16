// Copyright (C) 2020 ~ 2022 Uniontech Software Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFSECURITYTOOLSFRONTITEM_H
#define DEFSECURITYTOOLSFRONTITEM_H

#include "defsecuritytoolinfo.h"

#include <DFrame>
#include <DLabel>
#include <DPushButton>
#include <DSpinner>
#include <DSuggestButton>

#include <QMap>

DWIDGET_USE_NAMESPACE

#define UNITWIDTH 345
#define UNITHEIGHT 80

class DefSecurityToolsFrontUnit : public DFrame
{
    Q_OBJECT
public:
    explicit DefSecurityToolsFrontUnit(DWidget *parent = nullptr);
    ~DefSecurityToolsFrontUnit() override;

public:
    // 设置第三方应用信息
    void setToolsInfo(const DEFSECURITYTOOLINFO &info);
    void setAppStatus(DEFSECURITYTOOLSTATUS status);
    void setDataModel(QObject *pModel);

private:
    void installPackage();
    void unInstallPackage();
    void updatePackage();
    void showtool();
    void updateAppStatus();
    void initUi();
    void initConnection();
    DLabel *createLabel(int iPixSize);
    DPushButton *creatorBtn(const QString &strText, bool bSuggest);
    void setShowString(DLabel *pLabel, const QString &strText, int iLen);
    void setShowString(DPushButton *pBtn, const QString &strText, int iLen);
    void initTranslation();
    QString getTranslation(const QString &strKey);
    void reportStatus(const QString &strMsg);
signals:

public slots:

private:
    DEFSECURITYTOOLINFO m_toolInfo;
    DEFSECURITYTOOLSTATUS m_status;
    DLabel *m_pImageLabel;
    DLabel *m_pAppNameLabel;
    DLabel *m_pDesLabel;
    DLabel *m_pCompanyNameLabel;
    DSuggestButton *m_pOpenBtn;
    DSuggestButton *m_pUpdateBtn;
    DPushButton *m_pInstallBtn;
    DPushButton *m_pUnInstallBtn;
    DSpinner *m_pSpinner;
    QObject *m_pDataModel;
    QMap<QString, QString> transMap;

private:
    const QString m_strInstallFailed = tr("Failed to install the tool. Please try later.");
    const QString m_strUpdateFailed = tr("Failed to update the tool. Please try later.");
    const QString m_strUninstallFailed = tr("Failed to uninstall the tool. Please try later.");
};

#endif // DEFSECURITYTOOLSFRONTITEM_H
