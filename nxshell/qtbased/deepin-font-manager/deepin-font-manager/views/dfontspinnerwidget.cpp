// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontspinnerwidget.h"
#include "dfontspinner.h"

#include <DApplication>

#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

DFontSpinnerWidget::DFontSpinnerWidget(QWidget *parent,  SpinnerStyles styles) : DWidget(parent)
{
    Q_UNUSED(styles)
//    setStyles(styles);
    initUI();
}

/*************************************************************************
 <Function>      initUI
 <Description>   初始化ui界面
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontSpinnerWidget::initUI()
{
    m_spinner = new DFontSpinner(this);
    m_spinner->setFixedSize(QSize(32, 32));
    m_label = new QLabel(this);
    m_label->setText(DApplication::translate("Main", "Loading fonts, please wait..."));

    QHBoxLayout *hLoadingView = new QHBoxLayout;
    hLoadingView->addStretch(1);

    QVBoxLayout *vLoadingView = new QVBoxLayout;
    vLoadingView->addStretch(1);
    vLoadingView->addWidget(m_spinner, 0, Qt::AlignCenter);
    vLoadingView->addWidget(m_label, 0, Qt::AlignCenter);
    vLoadingView->addStretch(1);

    hLoadingView->addLayout(vLoadingView);
    hLoadingView->addStretch(1);

    this->setLayout(hLoadingView);
}

DFontSpinnerWidget::~DFontSpinnerWidget()
{
}

/*************************************************************************
 <Function>      setStyles
 <Description>   设置加载动画样式
 <Author>        null
 <Input>
    <param1>     加载动画样式      Description:styles
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontSpinnerWidget::setStyles(SpinnerStyles styles)
{
    m_Styles = styles;
    if (m_Styles == SpinnerStyles::Load || m_Styles == SpinnerStyles::StartupLoad) {
        m_label->setText(DApplication::translate("Main", "Loading fonts, please wait..."));
    } else if (m_Styles == SpinnerStyles::Delete) {
        m_label->setText(DApplication::translate("Dfuninstalldialog", "Deleting fonts, please wait..."));
    } else {
        m_label->setText(QString());
    }
}

/*************************************************************************
 <Function>      spinnerStart
 <Description>   开始
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontSpinnerWidget::spinnerStart()
{
    m_spinner->start();
}

/*************************************************************************
 <Function>      spinnerStop
 <Description>   停止旋转
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontSpinnerWidget::spinnerStop()
{
    m_spinner->stop();
}
