// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cnumbersliderwidget.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QPicture>
#include <QDebug>
class NumberSlider::NumberSlider_private
{
public:
    explicit NumberSlider_private(NumberSlider *qq): q(qq) {}

    NumberSlider *q;
    QLabel  *m_label = nullptr;
    DSlider *m_slider = nullptr;
    DLineEdit *m_text = nullptr;

    uint m_diameter = 0;
    bool m_bClicked = false;

    void initUI()
    {
        q->setMinimumSize(324, 36);
        QHBoxLayout *layout = new QHBoxLayout(q);
        layout->setMargin(0);
        layout->setSpacing(0);

        m_label = new QLabel(q);

        m_label->setText(tr("Percentage"));

        QFont nameLabelFont = m_label->font();
        nameLabelFont.setPixelSize(13);
        m_label->setFixedSize(QSize(56, 36));
        m_label->setFont(nameLabelFont);

        m_text = new DLineEdit(q);
        m_text->setFixedSize(QSize(65, 36));
        m_text->setClearButtonEnabled(false);
        QLineEdit *lineedit = nullptr;
#ifdef USE_DTK
        lineedit = m_text->lineEdit();
#else
        lineedit = m_text;
#endif
        lineedit->setReadOnly(true);
        m_text->setText("0%");
        QFont alphaLabelFont = m_text->font();
        lineedit->setTextMargins(0, 0, 0, 0);
        alphaLabelFont.setPixelSize(14);
        m_text->setFont(alphaLabelFont);

        m_slider = new DSlider(Qt::Horizontal, q);
        setWgtAccesibleName(m_slider, "Color slider");
#ifdef USE_DTK
        m_slider->slider()->setFocusPolicy(Qt::NoFocus);
#endif
        m_slider->setMinimum(0);
        m_slider->setMaximum(255);

        //m_slider->setFixedWidth(180);
        layout->addSpacing(5);
        layout->addWidget(m_label, Qt::AlignLeft);
        layout->addSpacing(5);
        layout->addWidget(m_slider, Qt::AlignLeft);
        layout->addSpacing(5);
        layout->addWidget(m_text, Qt::AlignLeft);

        q->setLayout(layout);
        q->setFlagDiameter(1);
        q->setRange(0, 99);
    }

    void initConnection()
    {
        connect(m_slider, &DSlider::sliderPressed, q, [ = ]() {
            emit q->valueChanged(m_slider->value(), EChangedBegin);
            m_bClicked = true;
        });

        connect(m_slider, &DSlider::valueChanged, q, [ = ](int value) {
            value = m_slider->value();
            m_text->setText(QString("%1%").arg(value * 100 / 255));
            if (m_bClicked) {
                emit q->valueChanged(value, EChangedUpdate);
            }
        });

        connect(m_slider, &DSlider::sliderReleased, q, [ = ]() {
            emit q->valueChanged(m_slider->value(), EChangedFinished);
            m_bClicked = false;
        });
    }

    QPicture getCirclePicture(int radius)
    {
        QPicture pic;
        auto r = QRect(0, 0, 40, 40);
        pic.setBoundingRect(r);
        QPainter painter(&pic);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        painter.setBrush(QBrush(Qt::black));
        QPen pen(Qt::black);
        pen.setWidth(radius);
        pen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
        pen.setCapStyle(Qt::PenCapStyle::RoundCap);
        painter.setPen(pen);
        painter.drawPoint(pic.boundingRect().center());
        painter.end();
        return pic;
    }
};
NumberSlider::NumberSlider(QWidget *parent)
    : QWidget(parent), NumberSlider_d(new NumberSlider_private(this))
{
    d_NumberSlider()->initUI();
    d_NumberSlider()->initConnection();
}

NumberSlider::NumberSlider(const QString &text, QWidget *parent): NumberSlider(parent)
{
    setText(text);
}

void NumberSlider::setValue(int value)
{
    d_NumberSlider()->m_slider->setValue(value);
    d_NumberSlider()->m_text->setText(QString("%1%").arg(value));
}

int NumberSlider::value() const
{
    return d_NumberSlider()->m_slider->value();
}

void NumberSlider::setRange(int min, int max)
{
#ifdef USE_DTK
    d_NumberSlider()->m_slider->setMinimum(min);
    d_NumberSlider()->m_slider->setMaximum(max);
#else
    d_NumberSlider()->m_slider->setRange(min, max);
#endif
}

int NumberSlider::min() const
{
    return d_NumberSlider()->m_slider->minimum();
}

int NumberSlider::max() const
{
    return d_NumberSlider()->m_slider->maximum();
}

void NumberSlider::setText(const QString &str)
{
    d_NumberSlider()->m_label->setText(str);
}

QString NumberSlider::text() const
{
    return d_NumberSlider()->m_label->text();
}

void NumberSlider::setTextVisiable(bool b)
{
    d_NumberSlider()->m_text->setVisible(b);
}

bool NumberSlider::isTextVisiable() const
{
    return d_NumberSlider()->m_text->isVisible();
}

void NumberSlider::setFlagDiameter(uint diameter)
{
    if (diameter > 30)
        return;

    if (d_NumberSlider()->m_diameter == diameter)
        return;

    d_NumberSlider()->m_diameter = diameter;

    int tDiameter = static_cast<int>(diameter);
    auto pic = d_NumberSlider()->getCirclePicture(tDiameter);
    if (!pic.isNull()) {
        d_NumberSlider()->m_label->setFixedWidth(30);
        d_NumberSlider()->m_label->setPicture(pic);
    }
}

uint NumberSlider::flagDiameter() const
{
    return d_NumberSlider()->m_diameter;
}

void NumberSlider::setTickInterval(int t)
{
#ifdef USE_DTK
    d_NumberSlider()->m_slider->slider()->setTickInterval(t);
#else
    d_NumberSlider()->m_slider->setTickInterval(t);
#endif
}

void NumberSlider::setPix(QPixmap pix)
{
    d_NumberSlider()->m_label->setPixmap(pix);
}


