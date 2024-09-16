// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMDBUTTONWIDGET_H
#define CMDBUTTONWIDGET_H

#include <QObject>

#include <DWidget>
#include <DCommandLinkButton>

DWIDGET_USE_NAMESPACE

class DetailTreeView;

/**
 * @brief The CmdButtonWidget class
 * 展开详细信息的按钮
 */
class CmdButtonWidget : public DWidget
{
    Q_OBJECT
public:
    explicit CmdButtonWidget(DetailTreeView *parent = nullptr);

signals:
    /**
     * @brief cmdButtonClicked:Button点击信号
     */
    void cmdButtonClicked();

protected:
    /**
     * @brief paintEvent:重绘事件
     * @param event:事件
     */
    void paintEvent(QPaintEvent *event) override;

private:
    /**
     * @brief initUI:舒适化UI界面
     */
    void initUI();

private:
    DCommandLinkButton *mp_cmdButton;             // 展开/收起按钮

};

#endif // CMDBUTTONWIDGET_H
