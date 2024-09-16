// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SUCCESSPAGE_H
#define SUCCESSPAGE_H

#include "uistruct.h"

#include <DWidget>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class CustomPushButton;
class CustomCommandLinkButton;

// 成功界面
class SuccessPage : public DWidget
{
    Q_OBJECT
public:
    explicit SuccessPage(QWidget *parent = nullptr);
    ~SuccessPage() override;

    /**
     * @brief setCompressFullPath   设置压缩路径
     * @param strFullPath   压缩路径（含名称）
     */
    void setCompressFullPath(const QString &strFullPath);

    /**
     * @brief setDes    设置成功信息展示
     * @param strDes    描述内容
     */
    void setSuccessDes(const QString &strDes);

    /**
     * @brief setSuccessType 设置成功界面类型
     * @param successInfo
     */
    void setSuccessType(const SuccessInfo &successInfo);

    /**
     * @brief getSuccessType 获取成功界面类型
     * @return
     */
    SuccessInfo getSuccessType();

    /**
     * @brief setDetail  设置失败先详细信息
     * @param strDetail         失败详细信息
     */
    void setDetail(const QString &strDetail);

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

signals:
    /**
     * @brief signalViewFile    查看信号
     */
    void signalViewFile();

    /**
     * @brief sigBackButtonClicked  返回信号
     */
    void sigBackButtonClicked();

private:
    DLabel *m_pSuccessPixmapLbl = nullptr; //成功图片显示
    DLabel *m_pSuccessLbl = nullptr;      // 成功文字显示
    DLabel *m_pDetailLbl = nullptr; // 描述信息
    CustomPushButton *m_pShowFileBtn = nullptr; // 查看文件按钮
    CustomCommandLinkButton *m_pReturnBtn = nullptr; // 返回按钮
    QString m_strFullPath;  // 压缩地址
    SuccessInfo m_successInfoType = SI_Compress; // 成功界面类型
};

#endif //SUCCESSPAGE_H
