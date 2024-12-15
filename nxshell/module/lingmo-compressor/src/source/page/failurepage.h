// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FAILUREPAGE_H
#define FAILUREPAGE_H

#include "uistruct.h"

#include <DWidget>
#include <DLabel>

DWIDGET_USE_NAMESPACE
class CustomPushButton;
class CustomCommandLinkButton;

// 失败界面
class FailurePage : public DWidget
{
    Q_OBJECT
public:
    explicit FailurePage(QWidget *parent = nullptr);
    ~FailurePage() override;

    /**
     * @brief setFailuerDes     设置失败描述
     * @param strDes            描述内容
     */
    void setFailuerDes(const QString &strDes);

    /**
     * @brief setFailureDetail  设置失败先详细信息
     * @param strDetail         失败详细信息
     * @param strFileName       文件名
     */
    void setFailureDetail(const QString &strDetail, const QString &strFileName = "");

    /**
     * @brief setRetryEnable    设置重试按钮是否可用
     * @param bEnable           是否可用
     */
    void setRetryEnable(bool bEnable);

    /**
     * @brief setFailureInfo  // 设置失败信息
     * @param failureInfo
     */
    void setFailureInfo(const FailureInfo &failureInfo);

    /**
     * @brief getFailureInfo  获取失败信息
     * @return
     */
    FailureInfo getFailureInfo();

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

private:
    DLabel *m_pFailurePixmapLbl; //失败图片显示
    DLabel *m_pFailureLbl; //失败文字显示
    DLabel *m_pFileNameLbl; // 失败的文件名称
    DLabel *m_pDetailLbl; //错误原因
    CustomPushButton *m_pRetrybutton; // 重试按钮
    CustomCommandLinkButton *commandLinkBackButton; //返回按钮
    FailureInfo m_failureInfo = FI_Compress;  // 失败信息

signals:
    void sigBackButtonClickedOnFail();
    void sigFailRetry();
};

#endif // FAILUREPAGE_H
