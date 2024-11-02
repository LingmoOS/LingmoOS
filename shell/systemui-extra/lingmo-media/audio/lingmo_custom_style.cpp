/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "lingmo_custom_style.h"
#include <QStyleOption>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>
#include <QPaintEvent>
#include <QStylePainter>
#include <QCoreApplication>
#include <QDebug>
#include <QToolTip>

FixLabel::FixLabel(QWidget *parent):
    QLabel(parent)
{

}

FixLabel::FixLabel(QString text , QWidget *parent):
    QLabel(parent)
{
    this->setText(text);
}

FixLabel::~FixLabel()
{

}

void FixLabel::paintEvent(QPaintEvent *event)
{
    QFontMetrics fontMetrics(this->font());
    int fontSize = fontMetrics.width(fullText);
    if (fontSize > this->width()) {
        this->setText(fontMetrics.elidedText(fullText, Qt::ElideRight, this->width()), false);
        this->setToolTip(fullText);
    } else {
        this->setText(fullText, false);
        this->setToolTip("");
    }
    QLabel::paintEvent(event);
}

void FixLabel::setText(const QString & text, bool saveTextFlag)
{
    if (saveTextFlag)
        fullText = text;
    QLabel::setText(text);
}


AudioSlider::AudioSlider(QWidget *parent)
    : KSlider(parent)
{
    Q_UNUSED(parent);
    setRange(0,100);
    setSliderType(SmoothSlider);
    setOrientation(Qt::Horizontal);
    setFocusPolicy(Qt::StrongFocus);
    setNodeVisible(false);
}

void AudioSlider::wheelEvent(QWheelEvent *e)
{
    if (this->value() - blueValue >= 10 || blueValue - this->value() >= 10) {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    return KSlider::wheelEvent(e);
}

void AudioSlider::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_VolumeMute) {
        qDebug() << "UkmediaVolumeSlider keyReleaseEvent " << e->key();
        // 过滤掉快捷键的操作
        e->ignore(); // 忽略该事件
        return;
    }

    //start 长按左键或者下键调低音量
    if (e->isAutoRepeat())
    {
        if (this->value() - blueValue >= 10 || blueValue - this->value() >= 10) {
            blueValue = this->value();
            Q_EMIT blueValueChanged(blueValue);
        }
        return;
    }
    //end

    if (this->value() != blueValue)
    {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    return KSlider::keyReleaseEvent(e);
}

void AudioSlider::mouseMoveEvent(QMouseEvent *e)
{
    isMouseWheel = true;

    if (this->value() - blueValue >= 10 || blueValue - this->value() >= 10) {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    return KSlider::mouseMoveEvent(e);
}

void AudioSlider::mouseReleaseEvent(QMouseEvent *e)
{
    isMouseWheel = false;
    if (this->value() != blueValue)
    {
        blueValue = this->value();
        Q_EMIT blueValueChanged(blueValue);
    }

    return KSlider::mouseReleaseEvent(e);
}

AudioSlider::~AudioSlider()
{

}

UkmediaVolumeSlider::UkmediaVolumeSlider(QWidget *parent)
    : KSlider(parent)
{
    Q_UNUSED(parent);
    setRange(-100,100);
    setSingleStep(100);
    setTickInterval(100);
    setSliderType(SmoothSlider);
    setOrientation(Qt::Horizontal);
    setFocusPolicy(Qt::StrongFocus);
    this->installEventFilter(this);
}

void UkmediaVolumeSlider::showTooltip()
{
    QString percent = QString::number(this->value());
    percent.append("%");

    QStyleOptionSlider opt;
    this->initStyleOption(&opt);
    QRect handleRect = this->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
    QPoint point = this->mapToGlobal(handleRect.topLeft());
    QFontMetrics fontMetrics = QFontMetrics(this->font());
    QRect fontRect = fontMetrics.boundingRect(percent);

    QToolTip::showText(point - QPoint(fontRect.width()/2 +3, fontRect.height() + 40), percent);
    return;
}

bool UkmediaVolumeSlider::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        if (event->type() == QEvent::HoverEnter)
             showTooltip();
    }
    return KSlider::eventFilter(watched, event);
}

void UkmediaVolumeSlider::initStyleOption(QStyleOptionSlider *option)
{
    KSlider::initStyleOption(option);
}

UkmediaVolumeSlider::~UkmediaVolumeSlider()
{
}


/**
 * @brief LingmoUIButtonDrawSvg::refreshIcon
 * 刷新图标
 */
void LingmoUIButtonDrawSvg::refreshIcon(QImage img, QColor color)
{
    mImage = img;
    mColor = color;
    this->repaint();
}

/**
 * @brief LingmoUIButtonDrawSvg::paintEvent
 * 绘制按钮背景透明
 */
void LingmoUIButtonDrawSvg::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setBrush(QBrush(QColor(0x13,0x13,0x14,0x00)));
    p.setPen(Qt::NoPen);
    QPainterPath path;
    opt.rect.adjust(0,0,0,0);
    path.addRoundedRect(opt.rect,6,6);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    p.drawRoundedRect(opt.rect,6,6);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

QRect LingmoUIButtonDrawSvg::IconGeometry()
{
    QRect res = QRect(QPoint(0,0),QSize(16,16));
    res.moveCenter(QRect(0,0,width(),height()).center());
    return res;
}

/**
 * @brief LingmoUIButtonDrawSvg::drawIcon
 * 绘制按钮图标
 */
void LingmoUIButtonDrawSvg::drawIcon(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    QRect iconRect = IconGeometry();
    if (mImage.size() != iconRect.size())
    {
        mImage = mImage.scaled(iconRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect r = mImage.rect();
        r.moveCenter(iconRect.center());
        iconRect = r;
    }

    this->setProperty("fillIconSymbolicColor", true);
    filledSymbolicColoredPixmap(mImage,mColor);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform); //抗锯齿 + 高比例模糊处理
    painter.drawImage(iconRect, mImage);
}

//  更新音量图标
void LingmoUIButtonDrawSvg::outputVolumeDarkThemeImage(int value, bool status)
{
    QImage image;
    QColor color = QColor(0,0,0,216);

    if (status) {
        image = QIcon::fromTheme("audio-volume-muted-symbolic").pixmap(24,24).toImage();
    }
    else if (value <= 0) {
        image = QIcon::fromTheme("audio-volume-muted-symbolic").pixmap(24,24).toImage();
    }
    else if (value > 0 && value <= 33) {
        image = QIcon::fromTheme("audio-volume-low-symbolic").pixmap(24,24).toImage();
    }
    else if (value >33 && value <= 66) {
        image = QIcon::fromTheme("audio-volume-medium-symbolic").pixmap(24,24).toImage();
    }
    else {
        image = QIcon::fromTheme("audio-volume-high-symbolic").pixmap(24,24).toImage();
    }

    refreshIcon(image, color);
}

bool LingmoUIButtonDrawSvg::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::Paint:
        drawIcon(static_cast<QPaintEvent*>(event));
        break;

    case QEvent::Move:
    case QEvent::Resize:
    {
        QRect rect = IconGeometry();
    }
        break;

    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
        event->accept();
        break;

    default:
        break;
    }

    return QPushButton::event(event);
}


LingmoUIButtonDrawSvg::LingmoUIButtonDrawSvg(QWidget *parent)
{
    Q_UNUSED(parent);
}
LingmoUIButtonDrawSvg::~LingmoUIButtonDrawSvg()
{

}

QPixmap LingmoUIButtonDrawSvg::filledSymbolicColoredPixmap(QImage &img, QColor &baseColor)
{

    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                int hue = color.hue();
                if (!qAbs(hue - symbolic_color.hue()) < 10) {
                    color.setRed(baseColor.red());
                    color.setGreen(baseColor.green());
                    color.setBlue(baseColor.blue());
                    img.setPixelColor(x, y, color);
                }
            }
        }
    }

    return QPixmap::fromImage(img);
}

TitleLabel::TitleLabel(QWidget *parent):
    QLabel(parent)
{
    this ->setContentsMargins(16,0,0,0);
}

TitleLabel::~TitleLabel()
{

}
