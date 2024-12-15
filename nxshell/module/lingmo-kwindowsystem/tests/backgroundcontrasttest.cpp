/* This file is part of the KDE libraries

    Copyright 2013 Nicolás Alvarez <nicolas.alvarez@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSlider>

#include <kwindoweffects.h>

class ContrastTestWindow: public QWidget
{
public:
    ContrastTestWindow();

    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;

private:
    QPushButton *m_btnNothing;
    QPushButton *m_btnFullWindow;
    QPushButton *m_btnRect;
    QPushButton *m_btnEllipse;

    QSlider  *m_contSlider;
    QSlider  *m_intSlider;
    QSlider  *m_satSlider;
    QWidget *m_area;

    qreal m_contrast;
    qreal m_intensity;
    qreal m_saturation;

    enum {
        Nothing, FullWindow, Rect, Ellipse
    } m_state;

    void setWindowAlpha(int alpha);

    void disableContrast();
    void enableContrast();
    void enableContrastRect();
    void enableContrastEllipse();
    void updateContrast(int contrast);
    void updateIntensity(int contrast);
    void updateSaturation(int contrast);
};

ContrastTestWindow::ContrastTestWindow()
{
    m_state = Nothing;
    m_contrast = 1;
    m_intensity = 1;
    m_saturation = 1;
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground, false);
    setWindowAlpha(92);

    m_btnNothing    = new QPushButton("Nothing");
    m_btnFullWindow = new QPushButton("Full window");
    m_btnRect       = new QPushButton("Rectangle");
    m_btnEllipse    = new QPushButton("Ellipse");

    m_contSlider       = new QSlider();
    m_contSlider->setMaximum(200);
    m_contSlider->setValue(100);
    m_contSlider->setOrientation(Qt::Horizontal);

    m_intSlider       = new QSlider();
    m_intSlider->setMaximum(200);
    m_intSlider->setValue(100);
    m_intSlider->setOrientation(Qt::Horizontal);

    m_satSlider       = new QSlider();
    m_satSlider->setMaximum(200);
    m_satSlider->setValue(100);
    m_satSlider->setOrientation(Qt::Horizontal);

    m_area = new QWidget;
    m_area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(m_btnNothing,    &QPushButton::clicked, this, &ContrastTestWindow::disableContrast);
    connect(m_btnFullWindow, &QPushButton::clicked, this, &ContrastTestWindow::enableContrast);
    connect(m_btnRect,       &QPushButton::clicked, this, &ContrastTestWindow::enableContrastRect);
    connect(m_btnEllipse,    &QPushButton::clicked, this, &ContrastTestWindow::enableContrastEllipse);

    connect(m_contSlider,    &QSlider::valueChanged, this, &ContrastTestWindow::updateContrast);
    connect(m_intSlider,     &QSlider::valueChanged, this, &ContrastTestWindow::updateIntensity);
    connect(m_satSlider,     &QSlider::valueChanged, this, &ContrastTestWindow::updateSaturation);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_btnNothing);
    layout->addWidget(m_btnFullWindow);
    layout->addWidget(m_btnRect);
    layout->addWidget(m_btnEllipse);
    layout->addWidget(m_contSlider);
    layout->addWidget(m_intSlider);
    layout->addWidget(m_satSlider);
    layout->addWidget(m_area);
    setLayout(layout);
}

void ContrastTestWindow::disableContrast()
{
    m_state = Nothing;
    KWindowEffects::enableBackgroundContrast(winId(), false);
    repaint();
}
void ContrastTestWindow::enableContrast()
{
    m_state = FullWindow;
    KWindowEffects::enableBackgroundContrast(winId(), true, m_contrast, m_intensity, m_saturation);
    repaint();
}
void ContrastTestWindow::enableContrastRect()
{
    m_state = Rect;
    QRegion rgn(m_area->geometry());
    KWindowEffects::enableBackgroundContrast(winId(), true, m_contrast, m_intensity, m_saturation, rgn);
    repaint();
}
void ContrastTestWindow::enableContrastEllipse()
{
    m_state = Ellipse;
    QRegion rgn(m_area->geometry(), QRegion::Ellipse);
    KWindowEffects::enableBackgroundContrast(winId(), true, m_contrast, m_intensity, m_saturation, rgn);
    repaint();
}

void ContrastTestWindow::updateContrast(int contrast)
{
    m_contrast = (qreal)contrast/100;

    switch(m_state) {
    case FullWindow:
        enableContrast();
        break;
    case Rect:
        enableContrastRect();
        break;
    case Ellipse:
        enableContrastEllipse();
        break;
    default:
        break;
    }
}

void ContrastTestWindow::updateIntensity(int contrast)
{
    m_intensity = (qreal)contrast/100;

    switch(m_state) {
    case FullWindow:
        enableContrast();
        break;
    case Rect:
        enableContrastRect();
        break;
    case Ellipse:
        enableContrastEllipse();
        break;
    default:
        break;
    }
}

void ContrastTestWindow::updateSaturation(int contrast)
{
    m_saturation = (qreal)contrast/100;

    switch(m_state) {
    case FullWindow:
        enableContrast();
        break;
    case Rect:
        enableContrastRect();
        break;
    case Ellipse:
        enableContrastEllipse();
        break;
    default:
        break;
    }
}

void ContrastTestWindow::resizeEvent(QResizeEvent *)
{
    if (m_state == Rect) {
        enableContrastRect();
    } else if (m_state == Ellipse) {
        enableContrastEllipse();
    }
}

void ContrastTestWindow::setWindowAlpha(int alpha)
{
    QPalette pal = this->palette();
    QColor windowColor = pal.color(QPalette::Window);
    windowColor.setAlpha(alpha);
    pal.setColor(QPalette::Window, windowColor);
    this->setPalette(pal);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    ContrastTestWindow wnd;
    wnd.show();

    return app.exec();
}
