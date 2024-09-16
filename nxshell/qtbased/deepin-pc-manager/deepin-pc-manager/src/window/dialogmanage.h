// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "modules/common/comdata.h"

#include <DDialog>

#include <QObject>

DWIDGET_USE_NAMESPACE

///
/// \brief 安全中心弹窗管理类
///
class DialogManage : public QObject
{
    Q_OBJECT
public:
    explicit DialogManage(QObject *parent = nullptr);

public:
    // 扫描时更新提示框
    static void showUpdateWhenScanningDialog();

    // 常规的提示框   左上角图标和文本信息  底下的按钮都是确定
    /**
     * @brief showNormalTipDialog
     * @param icontype   图标类型
     * @param textInfo   提示信息
     */
    void showNormalTipDialog(TipIcon icontype, TipTextInfo textInfo);

    // 更新服务地址提示窗
    DDialog *creatChangeAddressTipDialog();

signals:
    // 关闭信号
    void closed();
    // 按钮点击
    void buttonClicked();
};
