// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOPMODEMODULE_H
#define DESKTOPMODEMODULE_H

#include "../model.h"
#include "../widgets/basewidget.h"
#include "../worker.h"
#include "moduleinterface.h"

#include <QHBoxLayout>
#include <QKeyEvent>

//桌面样式模块类
class DesktopModeModule : public ModuleInterface
{
    Q_OBJECT
public:
    explicit DesktopModeModule(QWidget* parent = nullptr);

    //首次启动初始化
    void updateBigIcon() Q_DECL_OVERRIDE;
    //日常启动初始化图片
    void updateSmallIcon() Q_DECL_OVERRIDE;
    //更新选中按钮的位置
    void updateSelectBtnPos() Q_DECL_OVERRIDE;
    //键盘按键事件左右切换特效和普通模式
    void keyPressEvent(QKeyEvent*) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

    //设置是否是第一次启动标志
    void setFirst(bool first);
    //更新界面大小
    void updateInterface(float);

private slots:
    //修改选中框状态
    void onDesktopTypeChanged(Model::DesktopMode mode);

    //点击时尚模式和高效模式响应　ut002764 2021-5-31
    void onSetModeFashion();
    void onSetModeEfficient();

private:
    //水平布局
    QHBoxLayout* m_layout;
    //高效模式界面
    BaseWidget* m_efficientWidget;
    //时尚模式界面
    BaseWidget* m_fashionWidget;
    QSize m_size;
    bool isfirst;
};

#endif  // DESKTOPMODEMODULE_H
