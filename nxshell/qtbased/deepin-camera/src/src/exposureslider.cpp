// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exposureslider.h"

#include <QVBoxLayout>
#include <QPalette>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

#include <DFontSizeManager>
#include <DApplication>
#include <QParallelAnimationGroup>

#define WIDTH 40
#define HEIGHT 192
#define ANIMATION_DURATION 50

ExposureSlider::ExposureSlider(QWidget *parent) : QWidget(parent)
  ,m_pLabShowValue(nullptr)
  ,m_slider(nullptr)
  ,m_valueMax(100)
  ,m_valueMin(-100)
  ,m_curValue(0)
  ,m_opacity(102)  //UI默认
{
    resize(WIDTH, HEIGHT);
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(vLayout);
    setContentsMargins(0, 0, 0, 0);

    QFont font;
    font.setFamily("SourceHanSansSC");
    font.setPixelSize(15);
    font.setWeight(QFont::Medium);

    m_pLabShowValue = new DLabel(this);
    m_pLabShowValue->setForegroundRole(QPalette::BrightText);
    m_pLabShowValue->setFont(font);
    m_pLabShowValue->setAlignment(Qt::AlignCenter);
    m_pLabShowValue->setText("0");
    m_pLabShowValue->setFixedWidth(30);
    //颜色不随主题变化
    QPalette pa = m_pLabShowValue->palette();
    pa.setColor(QPalette::BrightText, Qt::white);
    m_pLabShowValue->setPalette(pa);
    //字体大小不随系统变化
    connect(qApp, &QGuiApplication::fontChanged, this, [=](){
        QFont font("SourceHanSansSC");
        font.setPixelSize(15);
        font.setWeight(QFont::Medium);
        m_pLabShowValue->setFont(font);
    });

    vLayout->addStretch(5);
    vLayout->addWidget(m_pLabShowValue, 0, Qt::AlignHCenter);
    vLayout->setSpacing(2);

    m_slider = new DSlider(Qt::Vertical, this);
    m_slider->setFixedHeight(150);
    m_slider->setIconSize(QSize(15, 15));
    m_slider->slider()->setRange(-100, 100);
    m_slider->slider()->installEventFilter(this);
    m_slider->setValue(m_curValue);
    m_slider->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(m_slider, 0, Qt::AlignCenter);
    vLayout->addStretch(5);

    connect(m_slider, &DSlider::valueChanged, this, &ExposureSlider::onValueChanged);
    connect(m_slider, &DSlider::valueChanged, this, &ExposureSlider::valueChanged);

    m_pLabShowValue->hide();
    m_slider->hide();
}

void ExposureSlider::setOpacity(int opacity)
{
    m_opacity = opacity;
    update();
}

void ExposureSlider::showContent(bool show, bool isShortCut)
{
    QGraphicsOpacityEffect *effectSlider = new QGraphicsOpacityEffect(m_slider);
    m_slider->setGraphicsEffect(effectSlider);
    QPropertyAnimation *opacity = new QPropertyAnimation(effectSlider, "opacity", this);
    opacity->setDuration(ANIMATION_DURATION);
    opacity->setEasingCurve(QEasingCurve::OutSine);

    QGraphicsOpacityEffect *effectLabel = new QGraphicsOpacityEffect(m_pLabShowValue);
    m_pLabShowValue->setGraphicsEffect(effectLabel);
    QPropertyAnimation *opacity1 = new QPropertyAnimation(effectLabel, "opacity", this);
    opacity1->setDuration(ANIMATION_DURATION);
    opacity1->setEasingCurve(QEasingCurve::OutSine);

    QParallelAnimationGroup *pGroup = new QParallelAnimationGroup(this);
    pGroup->addAnimation(opacity);
    pGroup->addAnimation(opacity1);

    if (show) {
        opacity->setStartValue(0);
        opacity->setEndValue(1);
        opacity1->setStartValue(0);
        opacity1->setEndValue(1);

        connect(pGroup, &QParallelAnimationGroup::finished, this, [=](){
            if (isShortCut) //键盘触发，需在此处理焦点
                m_slider->slider()->setFocus();
            pGroup->deleteLater();
        });

        m_pLabShowValue->show();
        m_slider->show();
        pGroup->start();
    } else {
        opacity->setStartValue(1);
        opacity->setEndValue(0);
        opacity1->setStartValue(1);
        opacity1->setEndValue(0);

        connect(pGroup, &QParallelAnimationGroup::finished, this, &ExposureSlider::contentHided);
        connect(pGroup, &QParallelAnimationGroup::finished, this, [=](){
            m_pLabShowValue->hide();
            m_slider->hide();
            pGroup->deleteLater();
        });

        pGroup->start();
    }

}

void ExposureSlider::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);

    QRectF topRect(rect().topLeft(), QSize(WIDTH, WIDTH));
    QRectF centerRect(rect().topLeft() + QPoint(0, WIDTH / 2.0), QSize(WIDTH, HEIGHT - WIDTH));
    QRectF bottomRect(rect().topLeft() + QPoint(0, HEIGHT - WIDTH), QSize(WIDTH, WIDTH));

    QPainterPath path;
    path.moveTo(topRect.bottomRight());
    path.arcTo(topRect, 0, 180);
    path.lineTo(centerRect.bottomLeft());
    path.arcTo(bottomRect, 180, 180);
    path.lineTo(centerRect.topRight());

    painter.setPen(Qt::NoPen);
    painter.fillPath(path, QBrush(QColor(0, 0, 0, m_opacity)));

    if (m_curValue) {
        QRectF rect = this->rect();
        rect.setTopLeft(QPoint(29, rect.height() / 2 + 10));
        rect.setSize(QSize(4, 4));
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(255, 255, 255, 0.8 * 255)));
        painter.drawEllipse(rect);
    }
}

void ExposureSlider::wheelEvent(QWheelEvent *event)
{
    if (m_curValue >= m_valueMax)
        m_curValue = m_valueMax;

    if (m_curValue <= m_valueMin)
        m_curValue = m_valueMin;

    m_curValue += event->angleDelta().y() / 120;
    m_slider->setValue(m_curValue);
}

void ExposureSlider::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Down: {
        if (m_curValue <= m_valueMin)
            return;
        m_curValue -= 1;
        m_slider->setValue(m_curValue);
        break;
    }
    case Qt::Key_Up: {
        if (m_curValue >= m_valueMax)
            return;
        m_curValue += 1;
        m_slider->setValue(m_curValue);
        break;
    }
    case Qt::Key_Return: {
        if (!m_slider->slider()->hasFocus())
            return;
        emit enterBtnClicked(true);
        break;
    }
    }
    QWidget::keyReleaseEvent(event);
}

bool ExposureSlider::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == m_slider->slider() && e->type() == QEvent::Hide) {
        QWidget* fw = QApplication::focusWidget();
        if (fw == m_slider->slider())
            m_slider->slider()->clearFocus();
    }
    return QWidget::eventFilter(obj, e);
}

void ExposureSlider::onValueChanged(int value)
{
    if (value > m_valueMax || value < m_valueMin)
        return;

    m_curValue = value;

    m_pLabShowValue->setText(QString("%1").arg(value));

    update();
}

ExposureSlider::~ExposureSlider()
{

}
