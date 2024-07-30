/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Nicolás Alvarez <nicolas.alvarez@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include <kwindoweffects.h>

class BlurTestWindow : public QWidget
{
public:
    BlurTestWindow();

    void resizeEvent(QResizeEvent *) override;

private:
    QPushButton *m_btnNothing;
    QPushButton *m_btnFullWindow;
    QPushButton *m_btnRect;
    QPushButton *m_btnEllipse;
    QWidget *m_area;

    enum { Nothing, FullWindow, Rect, Ellipse } m_state;

    void setWindowAlpha(int alpha);

    void disableBlur();
    void enableBlur();
    void enableBlurRect();
    void enableBlurEllipse();
};

BlurTestWindow::BlurTestWindow()
{
    m_state = Nothing;
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground, false);
    setWindowAlpha(192);

    m_btnNothing = new QPushButton("Nothing");
    m_btnFullWindow = new QPushButton("Full window");
    m_btnRect = new QPushButton("Rectangle");
    m_btnEllipse = new QPushButton("Ellipse");

    m_area = new QWidget;
    m_area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(m_btnNothing, &QPushButton::clicked, this, &BlurTestWindow::disableBlur);
    connect(m_btnFullWindow, &QPushButton::clicked, this, &BlurTestWindow::enableBlur);
    connect(m_btnRect, &QPushButton::clicked, this, &BlurTestWindow::enableBlurRect);
    connect(m_btnEllipse, &QPushButton::clicked, this, &BlurTestWindow::enableBlurEllipse);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_btnNothing);
    layout->addWidget(m_btnFullWindow);
    layout->addWidget(m_btnRect);
    layout->addWidget(m_btnEllipse);
    layout->addWidget(m_area);

    winId(); // force creation of the associated window
}

void BlurTestWindow::disableBlur()
{
    m_state = Nothing;
    KWindowEffects::enableBlurBehind(windowHandle(), false);
    repaint();
}
void BlurTestWindow::enableBlur()
{
    m_state = FullWindow;
    KWindowEffects::enableBlurBehind(windowHandle(), true);
    repaint();
}
void BlurTestWindow::enableBlurRect()
{
    m_state = Rect;
    QRegion rgn(m_area->geometry());
    KWindowEffects::enableBlurBehind(windowHandle(), true, rgn);
    repaint();
}
void BlurTestWindow::enableBlurEllipse()
{
    m_state = Ellipse;
    QRegion rgn(m_area->geometry(), QRegion::Ellipse);
    KWindowEffects::enableBlurBehind(windowHandle(), true, rgn);
    repaint();
}

void BlurTestWindow::resizeEvent(QResizeEvent *)
{
    if (m_state == Rect) {
        enableBlurRect();
    } else if (m_state == Ellipse) {
        enableBlurEllipse();
    }
}

void BlurTestWindow::setWindowAlpha(int alpha)
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

    BlurTestWindow wnd;
    wnd.show();

    return app.exec();
}
