// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "selectchess.h"
#include "../resultpopup/closepopup.h"
#include "selectinfo.h"
#include "determinebutton.h"
#include "chessselected.h"
#include "constants.h"

#include <QPainter>
#include <QPainterPath>

#include <DHiDPIHelper>

DWIDGET_USE_NAMESPACE
Selectchess::Selectchess(bool compositing, QWidget *parent)
    : QDialog(parent)
    , compositingStatus(compositing)
    , selectLButton(new Selectbutton)
    , selectRButton(new Selectbutton)
{
    setFixedSize(371, 219);
    setAttribute(Qt::WA_TranslucentBackground);
    //设置隐藏边框,以及popup,dialog属性
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);

    initBackgroundPix();
    initUI();
}

/**
 * @brief Selectchess::~Selectchess
 */
Selectchess::~Selectchess()
{
    if (m_mainLayout != nullptr) {
        delete  m_mainLayout;
        m_mainLayout = nullptr;
    }
}

/**
 * @brief Selectchess::initUI 初始化界面
 */
void Selectchess::initUI()
{
    m_mainLayout = new QVBoxLayout();

    QHBoxLayout *m_closeLayout = new QHBoxLayout();
    Closepopup *closeBt = new Closepopup(this);
    connect(closeBt, &Closepopup::signalCloseClicked, this, [ = ] {
        this->close();
        emit signalDialogClose();
    });
    m_closeLayout->addWidget(closeBt, 0, Qt::AlignRight);
    m_closeLayout->addSpacing(10);

    QHBoxLayout *m_seleceInfoLayout = new QHBoxLayout();
    Selectinfo *selectInfo = new Selectinfo(this);
    selectInfo->setMinimumWidth((this->width()));
    m_seleceInfoLayout->addStretch();
    m_seleceInfoLayout->addWidget(selectInfo);
    m_seleceInfoLayout->addStretch();

    QHBoxLayout *m_buttonLayout = new QHBoxLayout();

    Chessselected *LChessSelected = new Chessselected(chess_white);
    Chessselected *RchessSelected = new Chessselected(chess_black);
    selectRButton->setChecked(true);
    //选择的那个颜色棋子, 发送信号
    m_buttonLayout->addStretch(70);
    m_buttonLayout->addWidget(selectLButton);
    m_buttonLayout->addWidget(LChessSelected);
    m_buttonLayout->addStretch(40);
    m_buttonLayout->addWidget(selectRButton);
    m_buttonLayout->addWidget(RchessSelected);
    m_buttonLayout->addStretch(70);

    QHBoxLayout *m_determineLayout = new QHBoxLayout();
    Determinebutton *determineButton = new Determinebutton(this);
    connect(determineButton, &Determinebutton::signalButtonOKClicked, this, &Selectchess::slotButtonOKClicked);
    m_determineLayout->addWidget(determineButton);

    m_mainLayout->addSpacing(5);
    m_mainLayout->addLayout(m_closeLayout);
    m_mainLayout->addLayout(m_seleceInfoLayout);
    m_mainLayout->addLayout(m_buttonLayout);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addLayout(m_determineLayout);
    m_mainLayout->addSpacing(14);
    setLayout(m_mainLayout);
}

/**
 * @brief Selectchess::initBackgroundPix 初始化背景图片
 */
void Selectchess::initBackgroundPix()
{
    if (compositingStatus) {
        backgroundPixmap = DHiDPIHelper::loadNxPixmap(":/resources/chessselected/selectbase.svg");
        setFixedSize(386, 234);
    } else {
        backgroundPixmap = DHiDPIHelper::loadNxPixmap(":/resources/chessselected/selectbase_nshadow.svg");
        setFixedSize(370, 218);
    }
}

/**
 * @brief Selectchess::slotButtonOKClicked 确定按钮点击后发送信号
 */
void Selectchess::slotButtonOKClicked()
{
    //如果选择了棋子颜色, 发送信号, 否则不发送
    emit signalButtonOKClicked();
}

/**
 * @brief Selectchess::setSelectChess 设置选择的棋子颜色
 * @param chessColor 棋子颜色
 */
void Selectchess::setSelectChess(int chessColor)
{
    //用户选择的棋子颜色
    selectChessColor = chessColor;
    //根据用户选择的棋子颜色, 设置不同按钮使能
    if (selectChessColor == chess_white) {
        selectLButton->setChecked(true);
    } else if (selectChessColor == chess_black) {
        selectRButton->setChecked(true);
    }
}

/**
 * @brief Selectchess::getSelsectChess 获取选择的棋子颜色
 * @return 棋子颜色
 */
int Selectchess::getSelsectChess()
{
    //根据按钮选择, 设置棋子颜色
    if (selectLButton->isChecked()) {
        selectChessColor = chess_white;
    } else if (selectRButton->isChecked()) {
        selectChessColor = chess_black;
    }
    return selectChessColor;
}

/**
 * @brief Selectchess::selectClose关闭弹窗 发出信号关闭事件循环
 */
void Selectchess::selectClose()
{
    this->close();
    emit signalDialogClose();
}

/**
 * @brief Selectchess::closeSelectPopup 关闭弹窗
 */
void Selectchess::slotCloseSelectPopup()
{
    this->close();
}

/**
 * @brief Selectchess::slotCompositingChanged 特效窗口
 * @param compositing 是否开启
 */
void Selectchess::slotCompositingChanged(bool compositing)
{
    compositingStatus = compositing;
    initBackgroundPix();
    update();
}

/**
 * @brief Selectchess::paintEvent 绘图函数
 * @param event paintevent
 */
void Selectchess::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.drawPixmap(this->rect(), backgroundPixmap);
    painter.restore();
    QDialog::paintEvent(event);
}
