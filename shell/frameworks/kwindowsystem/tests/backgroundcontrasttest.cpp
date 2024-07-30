/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Nicolás Alvarez <nicolas.alvarez@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <optional>

#include <QApplication>
#include <QColorDialog>
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QScopeGuard>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>

#include <kwindoweffects.h>

class ContrastTestWindow : public QWidget
{
public:
    ContrastTestWindow();

    void resizeEvent(QResizeEvent *) override;

private:
    QPushButton *m_btnNothing;
    QPushButton *m_btnFullWindow;
    QPushButton *m_btnRect;
    QPushButton *m_btnEllipse;

    QPushButton *m_blur;
    bool m_doBlur;

    QPushButton *m_bg;
    QColor m_bgColour;

    QSlider *m_contSlider;
    QSlider *m_intSlider;
    QSlider *m_satSlider;
    QWidget *m_area;

    qreal m_contrast;
    qreal m_intensity;
    qreal m_saturation;

    enum { Nothing, FullWindow, Rect, Ellipse } m_state;

    void disableContrast();
    void enableContrast();
    void enableContrastRect();
    void enableContrastEllipse();
    void updateContrast(int contrast);
    void updateIntensity(int contrast);
    void updateSaturation(int contrast);
    void update();

    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)

        QPainter p(this);
        p.setPen(Qt::transparent);
        p.setBrush(m_bgColour);
        p.drawRect(this->rect());

        QWidget::paintEvent(event);
    }
};

ContrastTestWindow::ContrastTestWindow()
{
    m_state = Nothing;
    m_contrast = 1;
    m_intensity = 1;
    m_saturation = 1;
    m_bgColour = Qt::transparent;
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground, true);

    m_btnNothing = new QPushButton("Nothing");
    m_btnFullWindow = new QPushButton("Full window");
    m_btnRect = new QPushButton("Rectangle");
    m_btnEllipse = new QPushButton("Ellipse");
    m_blur = new QPushButton("Enable Blur");
    m_blur->setCheckable(true);
    m_bg = new QPushButton("Set Background Colour...");
    connect(m_bg, &QPushButton::pressed, this, [this]() {
        m_bgColour = QColorDialog::getColor(Qt::white, nullptr, "pick colour", QColorDialog::ShowAlphaChannel);

        repaint();
    });

    connect(m_blur, &QPushButton::toggled, this, [this](bool checked) {
        m_blur->setText(checked ? "Disable Blur" : "Enable Blur");
        m_doBlur = checked;

        update();
    });

    m_contSlider = new QSlider();
    m_contSlider->setMaximum(200);
    m_contSlider->setValue(100);
    m_contSlider->setOrientation(Qt::Horizontal);

    m_intSlider = new QSlider();
    m_intSlider->setMaximum(200);
    m_intSlider->setValue(100);
    m_intSlider->setOrientation(Qt::Horizontal);

    m_satSlider = new QSlider();
    m_satSlider->setMaximum(200);
    m_satSlider->setValue(100);
    m_satSlider->setOrientation(Qt::Horizontal);

    m_area = new QWidget;
    m_area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(m_btnNothing, &QPushButton::clicked, this, &ContrastTestWindow::disableContrast);
    connect(m_btnFullWindow, &QPushButton::clicked, this, &ContrastTestWindow::enableContrast);
    connect(m_btnRect, &QPushButton::clicked, this, &ContrastTestWindow::enableContrastRect);
    connect(m_btnEllipse, &QPushButton::clicked, this, &ContrastTestWindow::enableContrastEllipse);

    connect(m_contSlider, &QSlider::valueChanged, this, &ContrastTestWindow::updateContrast);
    connect(m_intSlider, &QSlider::valueChanged, this, &ContrastTestWindow::updateIntensity);
    connect(m_satSlider, &QSlider::valueChanged, this, &ContrastTestWindow::updateSaturation);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_btnNothing);
    layout->addWidget(m_btnFullWindow);
    layout->addWidget(m_btnRect);
    layout->addWidget(m_btnEllipse);
    layout->addWidget(m_contSlider);
    layout->addWidget(m_intSlider);
    layout->addWidget(m_satSlider);
    layout->addWidget(m_area);
    layout->addWidget(m_blur);
    layout->addWidget(m_bg);

    winId(); // force creation of the associated window
}

void ContrastTestWindow::update()
{
    const auto s = qScopeGuard([this]() {
        repaint();
    });

    if (m_state == Nothing) {
        KWindowEffects::enableBackgroundContrast(windowHandle(), false);
        KWindowEffects::enableBlurBehind(windowHandle(), false);
    }

    auto region = QRegion();
    switch (m_state) {
    case Nothing:
    case FullWindow:
        break;
    case Rect:
        region = m_area->geometry();
        break;
    case Ellipse:
        region = QRegion(m_area->geometry(), QRegion::Ellipse);
        break;
    }

    KWindowEffects::enableBlurBehind(windowHandle(), m_doBlur, region);

    KWindowEffects::enableBackgroundContrast(windowHandle(), true, m_contrast, m_intensity, m_saturation, region);

    repaint();
}

void ContrastTestWindow::disableContrast()
{
    m_state = Nothing;

    update();
}
void ContrastTestWindow::enableContrast()
{
    m_state = FullWindow;

    update();
}
void ContrastTestWindow::enableContrastRect()
{
    m_state = Rect;

    update();
}
void ContrastTestWindow::enableContrastEllipse()
{
    m_state = Ellipse;

    update();
}

void ContrastTestWindow::updateContrast(int contrast)
{
    m_contrast = (qreal)contrast / 100;

    update();
}

void ContrastTestWindow::updateIntensity(int contrast)
{
    m_intensity = (qreal)contrast / 100;

    update();
}

void ContrastTestWindow::updateSaturation(int contrast)
{
    m_saturation = (qreal)contrast / 100;

    update();
}

void ContrastTestWindow::resizeEvent(QResizeEvent *)
{
    update();
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    ContrastTestWindow wnd;
    wnd.show();

    return app.exec();
}
