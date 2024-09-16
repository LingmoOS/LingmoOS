// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "basewidget.h"

BaseWidget::BaseWidget(QWidget *parent)
    : DFrame(parent)
    , m_borderWidget(new BorderWidget(this))
    , m_layout(new QVBoxLayout(this))
    , m_title(new DLabel(this))
{
    m_layout->setMargin(0);
    m_layout->addWidget(m_borderWidget, 0, Qt::AlignCenter);
    m_layout->setSpacing(5);
    m_layout->addWidget(m_title, 10, Qt::AlignCenter);

    setLineWidth(0);
    setLayout(m_layout);
}

void BaseWidget::setLayoutSpacing(int i)
{
    m_layout->setSpacing(i);
}

/*******************************************************************************
 1. @函数:    updateInterface
 2. @作者:
 3. @日期:    2020-12-16
 4. @说明:    设置边框大小
*******************************************************************************/
void BaseWidget::updateInterface(float f)
{
    m_borderWidget->updateInterface(f);
    m_borderWidget->setFixedSize(m_size * f);
}

/*******************************************************************************
 1. @函数:    setBigPixmap
 2. @作者:
 3. @日期:    2020-12-16
 4. @说明:    设置第一次启动图片函数封装
*******************************************************************************/
void BaseWidget::setBigPixmap(const QString &url)
{
    setBigPixmap(QPixmap(DHiDPIHelper::loadNxPixmap(url)));
}

/*******************************************************************************
 1. @函数:    setSmallPixmap
 2. @作者:
 3. @日期:    2020-12-16
 4. @说明:    设置日常启动图片函数封装
*******************************************************************************/
void BaseWidget::setSmallPixmap(const QString &url)
{
    setSmallPixmap(QPixmap(DHiDPIHelper::loadNxPixmap(url)));
}

/*******************************************************************************
 1. @函数:    setPixmap
 2. @作者:
 3. @日期:    2020-12-16
 4. @说明:    类外设置图片函数
*******************************************************************************/
void BaseWidget::setPixmap(const QPixmap &pixmap)
{
    m_borderWidget->setFixedSize(pixmap.size() / devicePixelRatioF() + QSize(15, 20));
    m_borderWidget->setPixmap(pixmap);

    setFixedWidth(m_borderWidget->width());
}

/*******************************************************************************
 1. @函数:    setBigPixmap
 2. @作者:
 3. @日期:    2020-12-16
 4. @说明:    设置第一次启动图片函数,设置图片到边框界面时，将边框对应的放大，并设置主界面的高度和宽度
*******************************************************************************/
void BaseWidget::setBigPixmap(const QPixmap &pixmap)
{
    QSize scaleSize = QSize(320, 200) * devicePixelRatioF();
    QPixmap scaledPixmap = pixmap.scaled(scaleSize, Qt::KeepAspectRatio);
    m_borderWidget->setFixedSize(QSize(320, 200) + QSize(10, 11));
    m_borderWidget->setPixmap(scaledPixmap);
    m_size = QSize(320, 200) + QSize(10, 11);

    setFixedWidth(m_borderWidget->width());
    setFixedHeight(m_borderWidget->height() + m_title->height());
}

/*******************************************************************************
 1. @函数:    setSmallPixmap
 2. @作者:
 3. @日期:    2020-12-16
 4. @说明:    设置日常启动图片函数,设置图片到边框界面时，将边框对应的放大，并设置主界面的高度和宽度
*******************************************************************************/
void BaseWidget::setSmallPixmap(const QPixmap &pixmap)
{
    QSize scaleSize = QSize(240, 150) * devicePixelRatioF();
    QPixmap scaledPixmap = pixmap.scaled(scaleSize, Qt::KeepAspectRatio);
    m_borderWidget->setFixedSize(QSize(240, 150) + QSize(10, 10));
    m_borderWidget->setPixmap(scaledPixmap);
    m_size = QSize(240, 150) + QSize(10, 10);

    setFixedWidth(m_borderWidget->width());
}

/*******************************************************************************
 1. @函数:    setTitle
 2. @作者:
 3. @日期:    2020-12-16
 4. @说明:    设置图片的标题
*******************************************************************************/
void BaseWidget::setTitle(const QString &title)
{
    m_title->setText(title);
}

/*******************************************************************************
 1. @函数:    setChecked
 2. @作者:
 3. @日期:    2020-12-16
 4. @说明:    设置边框可检查状态
*******************************************************************************/
void BaseWidget::setChecked(bool checked)
{
    m_borderWidget->setChecked(checked);
}

/*******************************************************************************
 1. @函数:    mouseReleaseEvent
 2. @作者:
 3. @日期:    2020-12-16
 4. @说明:    模仿鼠标点击功能
*******************************************************************************/
void BaseWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QFrame::mouseReleaseEvent(event);

    emit clicked();
}

void BaseWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return;
}

void BaseWidget::resizeEvent(QResizeEvent *event)
{
    DFrame::resizeEvent(event);

    emit sizeChanged();
}
