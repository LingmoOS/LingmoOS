#include "datetimeedit.h"
#include <QPainter>
#include <QBrush>
#include <QDebug>
#include <QRect>
#include <QApplication>
#include <QSvgRenderer>
#include <QLineEdit>
#include <QCalendarWidget>
#include <QGraphicsBlurEffect>

QGraphicsDropShadowEffect *shadow_effect;
DateTimeEdit::DateTimeEdit(QWidget *parent) : QDateTimeEdit(parent){
    this->setButtonSymbols(QAbstractSpinBox::NoButtons);
    installEventFilter(this);

//    this->setStyleSheet("\
//                        QCalendarWidget QWidget#qt_calendar_navigationbar { \
//                            background-color: palette(base);\
//                        } \
//                        QCalendarWidget QSpinBox { \
//                            width:70px; \
//                            selection-background-color: palette(highlight); \
//                        } \
//                        QCalendarWidget QWidget {alternate-background-color: palette(base);} \
//                        QCalendarWidget QTableView { \
//                            selection-background-color: palette(highlight); \
//                        }\
//                        QCalendarWidget QToolButton { \
//                            color: palette(text);\
//                        } \
//                        ");
}

DateTimeEdit::~DateTimeEdit() {

}

void DateTimeEdit::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);  // 反锯齿;
    QBrush brush = QBrush(palette().color(QPalette::Button));
    painter.setPen(Qt::NoPen);
    painter.setBrush(brush);
    painter.drawRoundedRect(this->rect(),6,6);
    QPixmap pix = loadSvg("/usr/share/lingmo-user-guide/data/lingmo-down-symbolic.svg",64);
    QRect rect = QRect(113,10,16,12);
    painter.drawPixmap(rect, pix);

    QRect rectBoxt = this->rect();
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);  // 反锯齿;
    if (this->calendarWidget() && this->calendarWidget()->isVisible()) {
        focusFlag = true;
    }
    if (focusFlag == true) {
        QPen pen(palette().brush(QPalette::Active, QPalette::Highlight), 2);
        pen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.translate(1, 1);
        painter.drawRoundedRect(rectBoxt.adjusted(0, 0, -2, -2), 6, 6);
    } else if (hoverFlag == true) {
        painter.setPen(palette().color(QPalette::Active, QPalette::Highlight));
        painter.setBrush(Qt::NoBrush);
        painter.translate(0.5, 0.5);
        painter.drawRoundedRect(rectBoxt.adjusted(0, 0, -1, -1), 6, 6);
    }
}


QPixmap DateTimeEdit::loadSvg(const QString &path, int size) {
    int origSize = size;
    const auto ratio = qApp->devicePixelRatio();
    if ( 2 == ratio) {
        size += origSize;
    } else if (3 == ratio) {
        size += origSize;
    }
    QPixmap pixmap(size, size);
    QSvgRenderer renderer(path);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return drawSymbolicColoredPixmap(pixmap);
}

QPixmap DateTimeEdit::drawSymbolicColoredPixmap(const QPixmap &source) {
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            QColor color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                QColor colorSet = palette().color(QPalette::ButtonText);
                color.setRed(colorSet.red());
                color.setGreen(colorSet.green());
                color.setBlue(colorSet.blue());
                img.setPixelColor(x, y, color);
            }
        }
    }
    return QPixmap::fromImage(img);
}


bool DateTimeEdit::eventFilter(QObject *obj, QEvent *event) {
    if (QEvent::HoverEnter == event->type()) {
        hoverFlag = true;
        repaint();
    } else if (QEvent::HoverLeave == event->type()){
        hoverFlag = false;
        repaint();
    } else if (QEvent::FocusIn == event->type()) {
        focusFlag = true;
        repaint();
    } else if (QEvent::FocusOut == event->type()) {
        focusFlag = false;
        hoverFlag = false;
        repaint();
        if (this->date() != QDateTime::currentDateTime().date() && !this->calendarWidget()->isVisible()) {
            Q_EMIT changeDate();
        }
    }


    return QObject::eventFilter(obj,event);
}
