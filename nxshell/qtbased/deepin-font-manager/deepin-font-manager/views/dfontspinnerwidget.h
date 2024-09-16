// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DWidget>

#include <QLabel>

class DFontSpinner;

class DFontSpinnerWidget : public Dtk::Widget::DWidget
{
public:
    /*************************************************************************
     <Enum>      SpinnerStyles
     <Description>   加载动画样式
     <Author>
     <Value>
        <Value1>   StartupLoad      Description:启动应用加载
        <Value2>   Load             Description:安装后加载
        <Value3>   Delete           Description:删除后加载
        <Value4>   NoLabel          Description:无加载提示信息
     <Note>          null
    *************************************************************************/
    enum SpinnerStyles {
        StartupLoad,
        Load,
        Delete,
        NoLabel,
    };


public:
    explicit DFontSpinnerWidget(QWidget *parent = nullptr,  SpinnerStyles styles = SpinnerStyles::Load);
    void initUI();

    void spinnerStart();
    void spinnerStop();
    ~DFontSpinnerWidget();

    void setStyles(SpinnerStyles styles);

private:

    SpinnerStyles m_Styles;
    DFontSpinner *m_spinner = nullptr;
    QLabel *m_label = nullptr;
};
