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

#include <kwindoweffects.h>

class BlurTestWindow: public QWidget
{
public:
    BlurTestWindow();

    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;

private:
    QPushButton *m_btnNothing;
    QPushButton *m_btnFullWindow;
    QPushButton *m_btnRect;
    QPushButton *m_btnEllipse;
    QWidget *m_area;

    enum {
        Nothing, FullWindow, Rect, Ellipse
    } m_state;

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

    m_btnNothing    = new QPushButton("Nothing");
    m_btnFullWindow = new QPushButton("Full window");
    m_btnRect       = new QPushButton("Rectangle");
    m_btnEllipse    = new QPushButton("Ellipse");

    m_area = new QWidget;
    m_area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(m_btnNothing,    &QPushButton::clicked, this, &BlurTestWindow::disableBlur);
    connect(m_btnFullWindow, &QPushButton::clicked, this, &BlurTestWindow::enableBlur);
    connect(m_btnRect,       &QPushButton::clicked, this, &BlurTestWindow::enableBlurRect);
    connect(m_btnEllipse,    &QPushButton::clicked, this, &BlurTestWindow::enableBlurEllipse);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_btnNothing);
    layout->addWidget(m_btnFullWindow);
    layout->addWidget(m_btnRect);
    layout->addWidget(m_btnEllipse);
    layout->addWidget(m_area);
    setLayout(layout);
}

void BlurTestWindow::disableBlur()
{
    m_state = Nothing;
    KWindowEffects::enableBlurBehind(winId(), false);
    repaint();
}
void BlurTestWindow::enableBlur()
{
    m_state = FullWindow;
    KWindowEffects::enableBlurBehind(winId(), true);
    repaint();
}
void BlurTestWindow::enableBlurRect()
{
    m_state = Rect;
    QRegion rgn(m_area->geometry());
    KWindowEffects::enableBlurBehind(winId(), true, rgn);
    repaint();
}
void BlurTestWindow::enableBlurEllipse()
{
    m_state = Ellipse;
    QRegion rgn(m_area->geometry(), QRegion::Ellipse);
    KWindowEffects::enableBlurBehind(winId(), true, rgn);
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
