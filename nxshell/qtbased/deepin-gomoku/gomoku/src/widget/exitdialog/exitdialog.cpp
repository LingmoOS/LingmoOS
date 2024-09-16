// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exitdialog.h"
#include "cancelbutton.h"
#include "exitbutton.h"
#include "exitlabel.h"
#include "resultpopup/closepopup.h"

#include <QPushButton>
#include <DHiDPIHelper>
#include <DApplication>
#include <QPainter>
#include <QPainterPath>

ExitDialog::ExitDialog(bool compositing, QWidget *parent)
    : QDialog(parent)
    , result(BTType::BTCancel)
    , compositingStatus(compositing)
{
    setFixedSize(372, 219);
    setAttribute(Qt::WA_TranslucentBackground); //背景透明
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint); //取消标题栏

    initBackgroundPix();

    initUI();
}

/**
 * @brief ExitDialog::~ExitDialog
 */
ExitDialog::~ExitDialog()
{
    if (m_mainLayout != nullptr) {
        delete  m_mainLayout;
        m_mainLayout = nullptr;
    }

}

/**
 * @brief ExitDialog::initUI  初始化ui界面
 */
void ExitDialog::initUI()
{
    //上层关闭按钮布局
    QHBoxLayout *m_titleLayout = new QHBoxLayout;
    Closepopup *closeButton = new Closepopup(this);
    connect(closeButton, &Closepopup::signalCloseClicked, this, &ExitDialog::soltDialogClose);

    m_titleLayout->addStretch();
    m_titleLayout->addWidget(closeButton);
    m_titleLayout->addSpacing(10); //关闭按钮距离右边界的距离


    //中层标签布局
    QHBoxLayout *m_textLayout = new QHBoxLayout;
    m_textLayout->addStretch();

    ExitLabel *exitLabel = new ExitLabel(this);
    m_textLayout->addWidget(exitLabel);
    m_textLayout->addStretch();

    //下层按钮布局
    QHBoxLayout *m_BTLayout = new QHBoxLayout;
    m_BTLayout->addSpacing(14); //按钮距离左边界的距离

    CancelButton *cancelButton = new CancelButton(this);
    ExitButton *exitButton = new ExitButton(this);
    connect(cancelButton, &CancelButton::signalButtonOKClicked, this, &ExitDialog::soltDialogClose);
    connect(exitButton, &ExitButton::signalButtonOKClicked, this, &ExitDialog::soltGameExit);

    m_BTLayout->addWidget(cancelButton);
    m_BTLayout->addWidget(exitButton);
    m_BTLayout->addSpacing(16);//按钮距离右边界的距离

    //主布局
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->addSpacing(5);//上层布局离中间布局的距离
    m_mainLayout->addLayout(m_titleLayout);
    m_mainLayout->addLayout(m_textLayout);
    m_mainLayout->addStretch();
    m_mainLayout->addLayout(m_BTLayout);
    m_mainLayout->addSpacing(20); //按钮距离下边界的距离
    setLayout(m_mainLayout);
}

/**
 * @brief ExitDialog::initBackgroundPix 初始化背景图片
 */
void ExitDialog::initBackgroundPix()
{
    if (compositingStatus) {
        backgroundQPixmap = DHiDPIHelper::loadNxPixmap(":/resources/exitdialog/close-dialog.svg");
        setFixedSize(386, 234);
    } else {
        backgroundQPixmap = DHiDPIHelper::loadNxPixmap(":/resources/exitdialog/close-dialog-nshadow.svg");
        setFixedSize(370, 218);
    }
}

/**
 * @brief ExitDialog::soltDialogClose 槽函数：处理取消按钮和关闭dialog的信号
 */
void ExitDialog::soltDialogClose()
{
    setResult(BTType::BTCancel); //将状态设置为取消
    emit signalClicked();
    this->done(0);
}

/**
 * @brief ExitDialog::soltGameExit 槽函数：处理退出游戏的信号
 */
void ExitDialog::soltGameExit()
{
    setResult(BTType::BTExit); //将状态设置为退出
    emit signalClicked();
    this->done(0);
}

/**
 * @brief ExitDialog::slotCompositingChanged 特效窗口
 * @param compositing 是否开启
 */
void ExitDialog::slotCompositingChanged(bool compositing)
{
    compositingStatus = compositing;
    initBackgroundPix();
    update();
}


/**
 * @brief ExitDialog::paintEvent 重写绘画事件
 * @param event
 */
void ExitDialog::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.drawPixmap(this->rect(), backgroundQPixmap);
    painter.restore();
    QDialog::paintEvent(event);
}
