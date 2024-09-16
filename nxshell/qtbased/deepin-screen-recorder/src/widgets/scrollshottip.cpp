// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scrollshottip.h"
#include "utils.h"
#include "constant.h"
#include "utils/configsettings.h"
#include "utils/tempfile.h"

#include <DWidget>
#include <DHiDPIHelper>

#include <QTimer>
#include <QApplication>
#include <QPen>
#include <QPainter>
#include <QDebug>
#include <QPainterPath>

const int ScrollShotTip::NUMBER_PADDING_Y = 30;
const int ScrollShotTip::TIP_HEIGHT = 40;

DWIDGET_USE_NAMESPACE

ScrollShotTip::ScrollShotTip(DWidget *parent) : DWidget(parent)
{
    installEventFilter(this);
    QPixmap warmingImg ;
    //警告图片
//    if (Utils::themeType == 1) {
    QString svgName = QString("other/warming.svg");
    warmingImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath(svgName));
//    } else {
//        warmingImg = DHiDPIHelper::loadNxPixmap(Utils::getQrcPath("warming.svg"));
//    }
    this->setMinimumSize(100, TIP_HEIGHT);

    m_warmingIconButton = new DIconButton(this);
    m_warmingIconButton->setIcon(warmingImg);
    m_warmingIconButton->setIconSize(QSize(26, 26));
    m_warmingIconButton->setFlat(true);
    m_warmingIconButton->setFixedSize(QSize(30, 30));

    //提示内容
    m_tipTextLable = new DLabel(this);
    m_tipTextLable->setObjectName("TipText");
    //m_tipTextLable->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    //m_tipTextLable->setStyleSheet("QLabel{background-color:rgb(200,101,102);}");
    //设置背景透明
    //m_tipTextLable->setStyleSheet("QLabel {background-color: transparent;}");

    //帮助文字按钮
    m_scrollShotHelp = new DCommandLinkButton(tr("Get help."), this);
    m_scrollShotHelp->hide();
    QFontMetrics helpFontMetrics(m_scrollShotHelp->font());
    m_scrollShotHelp->resize(helpFontMetrics.width(m_scrollShotHelp->text()), m_scrollShotHelp->height());
    connect(m_scrollShotHelp, &DCommandLinkButton::clicked, this, &ScrollShotTip::openScrollShotHelp);

    //调整捕捉区域文字按钮
    m_scrollShotAdjust = new DCommandLinkButton(tr("adjust the capture area"), this) ;
    m_scrollShotAdjust->hide();
    QFontMetrics adjustFontMetrics(m_scrollShotAdjust->font());
    m_scrollShotAdjust->resize(helpFontMetrics.width(m_scrollShotAdjust->text()), m_scrollShotAdjust->height());
    connect(m_scrollShotAdjust, &DCommandLinkButton::clicked, this, &ScrollShotTip::adjustCaptureArea);

    QHBoxLayout *pHBoxLayout = new QHBoxLayout();
    pHBoxLayout->setAlignment(Qt::AlignCenter);
    pHBoxLayout->addWidget(m_warmingIconButton);
    pHBoxLayout->addStretch();
    pHBoxLayout->addWidget(m_tipTextLable);
    pHBoxLayout->addStretch();
    pHBoxLayout->addWidget(m_scrollShotHelp);
    pHBoxLayout->addStretch();
    pHBoxLayout->addWidget(m_scrollShotAdjust);
    this->setLayout(pHBoxLayout);
    update();

}

//根据提示的类型选取相应的提示方法
void ScrollShotTip::showTip(TipType tipType)
{
    m_tipType = tipType;
    m_scrollShotHelp->hide();
    m_scrollShotAdjust->hide();
    switch (m_tipType) {
    case TipType::StartScrollShotTip:
        showStartScrollShotTip();
        break;
    case TipType::ErrorScrollShotTip:
        showErrorScrollShotTip();
        break;
    case TipType::EndScrollShotTip:
        showEndScrollShotTip();
        break;
    case TipType::QuickScrollShotTip:
        showQuickScrollShotTip();
        break;
    case TipType::MaxLengthScrollShotTip:
        showMaxScrollShotTip();
        break;
    case TipType::InvalidAreaShotTip:
        showInvalidAreaShotTip();
        break;
    }

}

void ScrollShotTip::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    int radius = 20;
    //不同的提示类型其背景的圆角矩形半径不同
    /*switch (m_tipType) {
    case TipType::StartScrollShotTip:
        radius = 20;
        break;
    case TipType::QuickScrollShotTip:
        radius = 20;
        break;
    case TipType::ErrorScrollShotTip:
        radius = 20;
        break;
    case TipType::EndScrollShotTip:
        radius = 20;
        break;
    case TipType::MaxLengthScrollShotTip:
        radius = 20;
        break;
    case TipType::InvalidAreaShotTip:
        radius = 20;
        break;
    }*/
    //获取模糊背景的像素图
    QPixmap blurPixmap = getTooltipBackground();
    if (Utils::themeType == 1) {
        //判断是否获取到当前的模糊背景未获取到直接画一个矩形
        if (!blurPixmap.isNull()) {
            paintRect(painter, blurPixmap, radius);
            drawTooltipBackground(painter, rect(), "#EBEBEB", 0.3, radius);
        }
    } else {
        //判断是否获取到当前的模糊背景未获取到直接画一个矩形
        if (!blurPixmap.isNull()) {
            paintRect(painter, blurPixmap, radius);
            drawTooltipBackground(painter, rect(), "#191919", 0.8, radius);
        }
    }
    painter.setOpacity(1);
}

//获取提示框的模糊背景图
QPixmap ScrollShotTip::getTooltipBackground()
{
    const int radius = 10;
    int imgWidth = 0;
    int imgHeight = 0;
    //qDebug() << "this->x(): " << this->x() << "this->y(): " << this->y() <<"this->width(): " << this->width() <<"this->height(): " << this->height() ;

    //用当前提示窗口的位置和提示框的大小构建一个矩形框
    QRect target(static_cast<int>(this->x()),
                 static_cast<int>(this->y()),
                 static_cast<int>(this->width()),
                 static_cast<int>(this->height()));
    QPixmap tmpImg = m_backgroundPixmap.copy(target);
    //qDebug() << "tmpImg.width1(),tmpImg.height1()" << tmpImg.width() << "," << tmpImg.height();

    if (!tmpImg.isNull()) {
        imgWidth = tmpImg.width();
        imgHeight = tmpImg.height();
        tmpImg = tmpImg.scaled(imgWidth / radius, imgHeight / radius, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        tmpImg = tmpImg.scaled(imgWidth, imgHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        //qDebug() << "tmpImg.width(),tmpImg.height()" << tmpImg.width() << "," << tmpImg.height();
    }
    return  tmpImg;
}

//画矩形
void ScrollShotTip::paintRect(QPainter &painter, QPixmap &blurPixmap, int radius)
{
    QPainterPath rectPath;
    painter.setPen(Qt::transparent);
    rectPath.addRoundedRect(QRectF(rect()), radius, radius);
    painter.drawPath(rectPath);
    painter.setClipPath(rectPath);
    painter.drawPixmap(0, 0,  width(), height(), blurPixmap);
    painter.drawPath(rectPath);
    painter.setClipping(false);
}

//画提示背景
void ScrollShotTip::drawTooltipBackground(QPainter &painter, QRect rect, QString textColor, qreal opacity, int radius)
{
    painter.setOpacity(opacity);
    QPainterPath path;
    path.addRoundedRect(QRectF(rect), radius, radius);
    painter.fillPath(path, QColor(textColor));

    QPen pen(QColor("#000000"));
    painter.setOpacity(0.04);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawPath(path);
}

TipType ScrollShotTip::getTipType()
{
    return m_tipType;
}

void ScrollShotTip::setBackgroundPixmap(QPixmap &backgroundPixmap)
{
    m_backgroundPixmap = backgroundPixmap;

}


//开始滚动截图前的提示
void ScrollShotTip::showStartScrollShotTip()
{
    //m_tipText = "滚动鼠标滚轮或单击滚动区域";
    m_tipText = tr("Scroll your mouse wheel or click to take a scrolling screenshot");
    int width = 0;
    m_tipTextLable->setText(m_tipText);
    QFontMetrics fontMetrics(m_tipTextLable->font());
    m_tipTextLable->resize(fontMetrics.width(m_tipTextLable->text()), m_tipTextLable->height());
    m_warmingIconButton->hide();
    m_scrollShotHelp->hide();
    m_scrollShotAdjust->hide();
    width =  m_tipTextLable->width() + 20;
    setFixedSize(width, TIP_HEIGHT);
    update();

}

//滚动截图出现错误的提示
void ScrollShotTip::showErrorScrollShotTip()
{
    //m_tipText = tr("无法继续截图，查看帮助");
    m_tipText = tr("Failed to take a continuous screenshot.");
    int width = 0;
    m_tipTextLable->setText(m_tipText);
    QFontMetrics labFontMetrics(m_tipTextLable->font());
    m_tipTextLable->resize(labFontMetrics.width(m_tipTextLable->text()), m_tipTextLable->height());
    m_warmingIconButton->show();
    QFontMetrics helpFontMetrics(m_scrollShotHelp->font());
    m_scrollShotHelp->resize(helpFontMetrics.width(m_scrollShotHelp->text()), m_scrollShotHelp->height());
    m_scrollShotHelp->show();
    m_scrollShotAdjust->hide();
    width = m_warmingIconButton->width() + 10 + m_tipTextLable->width() + 30 + m_scrollShotHelp->width();
    setFixedSize(width, TIP_HEIGHT);
    update();
    //qDebug() << "1111 >> m_warmingIconButton->width(): " << m_warmingIconButton->width() <<"m_warmingIconButton->height(): " << m_warmingIconButton->height() ;
    //qDebug() << "1111 >> m_tipTextLable->width(): " << m_tipTextLable->width() <<"m_tipTextLable->height(): " << m_tipTextLable->height() ;
    //qDebug() << "1111 >> m_scrollShotHelp->width(): " << m_scrollShotHelp->width() <<"m_scrollShotHelp->height(): " << m_scrollShotHelp->height() ;
    //qDebug() << "1111 >> this->width(): " << this->width() <<"this->height(): " << this->height() ;
}

//滚动截图到底部出现的提示
void ScrollShotTip::showEndScrollShotTip()
{
    //m_tipText = tr("已到滚动区域底部");
    m_tipText = tr("Reached the bottom of the scroll area");
    int width = 0;
    m_tipTextLable->setText(m_tipText);
    QFontMetrics fontMetrics(m_tipTextLable->font());
    m_tipTextLable->resize(fontMetrics.width(m_tipTextLable->text()), m_tipTextLable->height());
    m_scrollShotHelp->hide();
    m_scrollShotAdjust->hide();
    m_warmingIconButton->show();
    width = m_warmingIconButton->width() + 30 + m_tipTextLable->width();
    setFixedSize(width, TIP_HEIGHT);
    update();

}

//图像拼接长度限制
void ScrollShotTip::showMaxScrollShotTip()
{
    //m_tipText = tr("已到最大长度");
    m_tipText = tr("Reached the maximum length");
    int width = 0;
    m_tipTextLable->setText(m_tipText);
    QFontMetrics fontMetrics(m_tipTextLable->font());
    m_tipTextLable->resize(fontMetrics.width(m_tipTextLable->text()), m_tipTextLable->height());
    m_scrollShotHelp->hide();
    m_scrollShotAdjust->hide();
    m_warmingIconButton->show();
    width = m_warmingIconButton->width() + 30 + m_tipTextLable->width();
    setFixedSize(width, TIP_HEIGHT);
    update();
}

//滚动速度过快
void ScrollShotTip::showQuickScrollShotTip()
{

    //m_tipText = tr("请放慢滚动速度");
    m_tipText = tr("Slow down the scrolling speed");
    int width = 0;
    m_tipTextLable->setText(m_tipText);
    QFontMetrics fontMetrics(m_tipTextLable->font());
    m_tipTextLable->resize(fontMetrics.width(m_tipTextLable->text()), m_tipTextLable->height());
    m_scrollShotHelp->hide();
    m_scrollShotAdjust->hide();
    m_warmingIconButton->show();
    width = m_warmingIconButton->width() + 30 + m_tipTextLable->width();
    setFixedSize(width, TIP_HEIGHT);
    update();
}

void ScrollShotTip::showInvalidAreaShotTip()
{
    //m_tipText = tr("无效区域，点击调整捕捉区域");
    m_tipText = tr("Invalid area, click to ");
    int width = 0;
    m_tipTextLable->setText(m_tipText);
    QFontMetrics labFontMetrics(m_tipTextLable->font());
    m_tipTextLable->resize(labFontMetrics.width(m_tipTextLable->text()), m_tipTextLable->height());
    m_warmingIconButton->show();
    QFontMetrics adjustFontMetrics(m_scrollShotAdjust->font());
    m_scrollShotAdjust->resize(adjustFontMetrics.width(m_scrollShotAdjust->text()), m_scrollShotAdjust->height());
    m_scrollShotAdjust->show();
    m_scrollShotHelp->hide();
    width = m_warmingIconButton->width() + 10 + m_tipTextLable->width() + 30 + m_scrollShotAdjust->width();
    setFixedSize(width, TIP_HEIGHT);
    update();
    //qDebug() << "1111 >> m_warmingIconButton->width(): " << m_warmingIconButton->width() <<"m_warmingIconButton->height(): " << m_warmingIconButton->height() ;
    //qDebug() << "1111 >> m_tipTextLable->width(): " << m_tipTextLable->width() <<"m_tipTextLable->height(): " << m_tipTextLable->height() ;
    //qDebug() << "1111 >> m_scrollShotAdjust->width(): " << m_scrollShotAdjust->width() <<"m_scrollShotAdjust->height(): " << m_scrollShotAdjust->height() ;
    //qDebug() << "1111 >> this->width(): " << this->width() <<"this->height(): " << this->height() ;
}

ScrollShotTip::~ScrollShotTip()
{
    if (m_scrollShotHelp) {
        delete  m_scrollShotHelp;
        m_scrollShotHelp = nullptr;
    }
    if (m_scrollShotAdjust) {
        delete  m_scrollShotAdjust;
        m_scrollShotAdjust = nullptr;
    }
}
