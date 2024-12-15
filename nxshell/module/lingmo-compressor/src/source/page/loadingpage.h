// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOADINGPAGE_H
#define LOADINGPAGE_H

#include <DWidget>
#include <DSpinner>
#include <DLabel>

DWIDGET_USE_NAMESPACE

// 加载界面
class LoadingPage : public DWidget
{
public:
    explicit LoadingPage(QWidget *parent = nullptr);
    ~LoadingPage() override;

    /**
     * @brief startLoading  开始加载
     */
    void startLoading();

    /**
     * @brief stopLoading   结束加载
     */
    void stopLoading();

    /**
     * @brief setDes    设置描述
     * @param strDes    描述内容
     */
    void setDes(const QString &strDes);

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

private:
    DSpinner *m_pSpinner;   // 加载效果
    DLabel *m_pTextLbl;
};

#endif // LOADINGPAGE_H
