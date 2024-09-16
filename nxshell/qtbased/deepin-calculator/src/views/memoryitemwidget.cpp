// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "memoryitemwidget.h"

#include <QPainter>
#include <QDebug>
#include <QMenu>
#include <QApplication>
#include <QEvent>
#include <QContextMenuEvent>
#include <QClipboard>
#include <DGuiApplicationHelper>

const QSize MEMORYBUTTON_SIZE = QSize(52, 32); //内存按键大小，为画边框比ui大2pix

MemoryItemWidget::MemoryItemWidget(QWidget *parent)
    : QWidget(parent)
    , m_btnplus(new MemoryButton(QString("M+"), true, this))
    , m_btnminus(new MemoryButton(QString("M-"), true, this))
    , m_btnclean(new MemoryButton(QString("MC"), true, this))
    , m_label(new QLabel(this))
{
    setFocusPolicy(Qt::NoFocus);
    QVBoxLayout *layV = new QVBoxLayout(this); //存放四个控件
    QHBoxLayout *lay = new QHBoxLayout(); //存放三个按钮

    lay->addStretch();
    lay->setSpacing(3); //按钮比ui大2pix
    m_btnplus->setFixedSize(MEMORYBUTTON_SIZE);
    m_btnminus->setFixedSize(MEMORYBUTTON_SIZE);
    m_btnclean->setFixedSize(MEMORYBUTTON_SIZE);
    lay->addWidget(m_btnclean);
    lay->addWidget(m_btnplus);
    lay->addWidget(m_btnminus);
    layV->setMargin(0);
    layV->addWidget(m_label);
    layV->addStretch();
    QFont font;
    font.setPixelSize(29);
    m_label->setFont(font);
    m_label->setAttribute(Qt::WA_TranslucentBackground); //label窗体透明

    m_label->setAlignment(Qt::AlignRight | Qt::AlignTop); //label字体居右，居上

    m_label->setAttribute(Qt::WA_TransparentForMouseEvents, true); //label鼠标穿透
    layV->addLayout(lay);
    layV->setContentsMargins(0, 0, 20, 6); //右边距20,下边距6
    this->setLayout(layV);

    connect(m_btnplus, &QPushButton::clicked, this, &MemoryItemWidget::plusbtnclicked);
    connect(m_btnminus, &QPushButton::clicked, this, &MemoryItemWidget::minusbtnclicked);
    connect(m_btnclean, &QPushButton::clicked, this, &MemoryItemWidget::cleanbtnclicked);
    m_btnplus->setFocusPolicy(Qt::NoFocus);
    m_btnminus->setFocusPolicy(Qt::NoFocus);
    m_btnclean->setFocusPolicy(Qt::NoFocus);
    m_btnplus->setHidden(true);
    m_btnminus->setHidden(true);
    m_btnclean->setHidden(true);
//    m_font.setPixelSize(18);
}

void MemoryItemWidget::enterEvent(QEvent *event)
{
    m_ishover = true;
    m_btnplus->setHidden(false); //进入时显示按钮
    m_btnminus->setHidden(false);
    m_btnclean->setHidden(false);
    update();
//    this->setAttribute(Qt::WA_StyledBackground, true);
//    this->setStyleSheet("background-color: rgb(245,245,245)");
    QWidget::enterEvent(event);
}

void MemoryItemWidget::leaveEvent(QEvent *event)
{
//    clearFocus();
    m_ishover = false;
    m_btnplus->setHidden(true); //离开时隐藏按钮
    m_btnminus->setHidden(true);
    m_btnclean->setHidden(true);
    update();
//    this->setAttribute(Qt::WA_StyledBackground, true);
//    this->setStyleSheet("background-color: rgb(255,255,255)");
    QWidget::leaveEvent(event);
}

void MemoryItemWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        return;
//    setFocus();
    m_ispress = true;
    DPalette pl1 = this->palette(); //按下时给label字体设置颜色
    if (m_themetype == 1) {
        pl1.setColor(DPalette::Text, QColor("#303030"));
        pl1.setColor(DPalette::HighlightedText, QColor("#303030"));
    } else {
        pl1.setColor(DPalette::Text, QColor("#FFFFFF"));
        pl1.setColor(DPalette::HighlightedText, QColor("#FFFFFF"));
    }

    m_label->setPalette(pl1);
    update();
    QWidget::mousePressEvent(event);
}

void MemoryItemWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        return;
//    clearFocus();
    m_ispress = false;
    DPalette pl1 = this->palette(); //松开鼠标时给label字体重新设置颜色
    if (m_themetype == 1) {
        pl1.setColor(DPalette::Text, QColor("#303030"));
        pl1.setColor(DPalette::HighlightedText, QColor("#303030"));
    } else {
        pl1.setColor(DPalette::Text, QColor("#B4B4B4"));
        pl1.setColor(DPalette::HighlightedText, QColor("#B4B4B4"));
    }

    m_label->setPalette(pl1);
    QWidget::mouseReleaseEvent(event);
}

void MemoryItemWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this); //添加各item的菜单项
    QAction *copy = new QAction(tr("Copy"), menu); //复制
    QAction *clean = new QAction(tr("Clear memory item"), menu); //MC
    QAction *plus = new QAction(tr("Add to memory item"), menu); //M+
    QAction *minus = new QAction(tr("Subtract from memory item"), menu); //M-
    menu->addAction(copy);
    menu->addAction(clean);
    menu->addAction(plus);
    menu->addAction(minus);
    if (m_isExpressionEmpty) {
        plus->setEnabled(false);
        minus->setEnabled(false);
    } else {
        plus->setEnabled(true);
        minus->setEnabled(true);
    }
    connect(copy, &QAction::triggered, this, &MemoryItemWidget::menucopy);
    connect(clean, &QAction::triggered, this, &MemoryItemWidget::menuclean);
    connect(plus, &QAction::triggered, this, &MemoryItemWidget::menuplus);
    connect(minus, &QAction::triggered, this, &MemoryItemWidget::menuminus);
    menu->exec(event->globalPos()); //当前鼠标位置显示菜单
    delete menu;
}

/**
 * @brief 当做出内存操作时对label->settext
 * @param s-text
 */
void MemoryItemWidget::setTextLabel(QString s)
{
    m_label->setText(s);
}

/**
 * @brief 获取当前label的text
 */
QString MemoryItemWidget::textLabel()
{
    return m_label->text();
}

void MemoryItemWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QRectF rect = this->rect();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true); //防走样
    painter.setPen(Qt::NoPen);
    QColor press, hover;
    int type = DGuiApplicationHelper::instance()->themeType();
    if (type == 1) {
        press = QColor(0, 0, 0);
        press.setAlphaF(0.2);
        hover = QColor(0, 0, 0);
        hover.setAlphaF(0.05);
    } else {
        press = QColor(255, 255, 255);
        press.setAlphaF(0.2);
        hover = QColor(255, 255, 255);
        hover.setAlphaF(0.05);
    }
    if (m_ishover) {
        painter.setPen(Qt::NoPen);
//        painter.setFont(m_font);
        painter.setBrush(hover);
        painter.drawRect(rect); //hover状态下对item进行颜色填充
        if (m_ispress) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(press);
            painter.drawRect(rect); //press状态下对item进行颜色填充

        }
    }
}

void MemoryItemWidget::setLineHight(int line, int height)
{
    m_label->setFixedHeight(height * line);
    m_line = line;
}

int MemoryItemWidget::getLine()
{
    return m_line;
}

void MemoryItemWidget::showTextEditMenuByAltM()
{
    QMenu *menu = new QMenu(this); //添加各item的菜单项
    QAction *copy = new QAction(tr("Copy"), menu); //复制
    QAction *clean = new QAction(tr("Clear memory item"), menu); //MC
    QAction *plus = new QAction(tr("Add to memory item"), menu); //M+
    QAction *minus = new QAction(tr("Subtract from memory item"), menu); //M-
    menu->addAction(copy);
    menu->addAction(clean);
    menu->addAction(plus);
    menu->addAction(minus);
    if (m_isExpressionEmpty) {
        plus->setEnabled(false);
        minus->setEnabled(false);
    } else {
        plus->setEnabled(true);
        minus->setEnabled(true);
    }
    connect(copy, &QAction::triggered, this, &MemoryItemWidget::menucopy);
    connect(clean, &QAction::triggered, this, &MemoryItemWidget::menuclean);
    connect(plus, &QAction::triggered, this, &MemoryItemWidget::menuplus);
    connect(minus, &QAction::triggered, this, &MemoryItemWidget::menuminus);
    QPoint menupoint;
    menupoint.setX(mapToGlobal(m_label->rect().center()).x());
    menupoint.setY(mapToGlobal(m_label->rect().bottomLeft()).y());
    menu->exec(menupoint); //当前鼠标位置显示菜单
    delete menu;
}

void MemoryItemWidget::themetypechanged(int type)
{
    m_themetype = type;
    DPalette pl1 = this->palette();
    //itemwidget字体颜色设置
    if (m_themetype == 1) {
        pl1.setColor(DPalette::Text, QColor("#303030"));
    } else {
        pl1.setColor(DPalette::Text, QColor("#B4B4B4"));
    }

    m_label->setPalette(pl1);
}

/**
 * @brief 当输入栏为空时，内存列表中的加减按钮置灰
 */
void MemoryItemWidget::isexpressionempty(bool b)
{
    m_isExpressionEmpty = b;
    if (m_isExpressionEmpty) {
        m_btnplus->setEnabled(false);
        m_btnminus->setEnabled(false);
    } else {
        m_btnplus->setEnabled(true);
        m_btnminus->setEnabled(true);
    }
}

/**
 * @brief MemoryItemWidget::setNextItemHover
 * 上一个item清除后，鼠标自动到下一个item上，此时为hover状态
 */
void MemoryItemWidget::setNextItemHover()
{
    m_ishover = true;
    m_btnplus->setHidden(false); //进入时显示按钮
    m_btnminus->setHidden(false);
    m_btnclean->setHidden(false);
}

MemoryItemWidget::~MemoryItemWidget()
{
}
