// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WMMODEMODULE_H
#define WMMODEMODULE_H

#include "../model.h"
#include "../widgets/basewidget.h"
#include "../worker.h"
#include "moduleinterface.h"

#include <QHBoxLayout>
#include <QKeyEvent>

//运行模式模块类
class WMModeModule : public ModuleInterface
{
    Q_OBJECT
public:
    explicit WMModeModule(QWidget* parent = nullptr);

    //首次启动初始化图片
    void updateBigIcon() Q_DECL_OVERRIDE;
    //日常启动初始化图片
    void updateSmallIcon() Q_DECL_OVERRIDE;
    //更新选择按钮位置
    void updateSelectBtnPos() Q_DECL_OVERRIDE;
    //键盘按键事件左右切换特效和普通模式
    void keyPressEvent(QKeyEvent*) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

    //设置首次启动标志位
    void setFirst(bool first);
private slots:
    //特效模式和普通模式切换响应槽函数
    void onWMModeChanged(Model::WMType type);

    //点击普通模式和高效模式响应　ut002764 2021-6-1
    void onSetModeNormal();
    void onSetModeEfficient();

private:
    QHBoxLayout* m_layout;
    //变量命名不规范
    BaseWidget* m_efficientWidget {nullptr};
    //变量命名不规范
    BaseWidget* m_fashionWidget {nullptr};
    bool m_first;
};

#endif  // WMMODEMODULE_H
