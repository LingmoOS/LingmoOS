// Copyright (C) 2019 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DFrame>
#include <DLabel>
#include <DSpinner>

enum CheckStatus {
    NoCheck = 0,
    CheckFailed,
    CheckSucess
};

DWIDGET_USE_NAMESPACE
class NetCheckItem : public DFrame
{
    Q_OBJECT
public:
    explicit NetCheckItem(QString title, QWidget *parent = nullptr);
    ~NetCheckItem();

    // 重置
    void resetNetCheckInfo();
    // 开始检测
    void startNetChecking();
    // 结束检测
    void stopNetCheck(int status, QString result);

private:
    QString m_titleName;
    DLabel *m_title;
    DLabel *m_tipTitle;
    DSpinner *m_spinner;

    QString m_currentColor;
};
