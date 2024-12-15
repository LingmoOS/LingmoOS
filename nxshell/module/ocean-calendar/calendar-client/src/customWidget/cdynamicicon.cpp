// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cdynamicicon.h"


#include <DAboutDialog>
#include <DLog>

#include <QSize>
#include <QPainter>

#define  BackgroundRatio     0.9394

#define  Month_X_Ratio       0.3203
#define  Month_Y_Ratio       0.1690
#define  Month_W_Ratio       0.3125
#define  Month_H_Ratio       0.15625

#define  Day_X_Ratio        0.28125
#define  Day_Y_Ratio        0.33333
#define  Day_W_Ratio        0.4375
#define  Day_H_Ratio        0.4160

#define  Week_X_Ratio       0.4375
#define  Week_Y_Ratio       0.728515625
#define  Week_W_Ratio       0.21875
#define  Week_H_Ratio       0.09375


CDynamicIcon *CDynamicIcon::m_Icon = nullptr;
CDynamicIcon::CDynamicIcon(int width, int height)
    : m_pixmap(new QPixmap(QSize(width, height)))
    , m_Date()
    , m_Dayrenderer(new QSvgRenderer())
    , m_Weekrenderer(new QSvgRenderer())
    , m_Monthrenderer(new QSvgRenderer())
    , m_backgroundrenderer(new QSvgRenderer())

{
    m_pixmap->fill(Qt::transparent);
    m_Date = QDate::currentDate();
    setDate(m_Date);
}

CDynamicIcon::~CDynamicIcon()
{
    delete m_pixmap;
    m_pixmap = nullptr;

    delete  m_Dayrenderer;
    m_Dayrenderer = nullptr;

    delete  m_Weekrenderer;
    m_Weekrenderer = nullptr;

    delete  m_Monthrenderer;
    m_Monthrenderer = nullptr;

    delete  m_backgroundrenderer;
    m_backgroundrenderer = nullptr;

}

CDynamicIcon *CDynamicIcon::getInstance()
{
    if (nullptr == m_Icon) {
        m_Icon = new CDynamicIcon(512, 512);
    }
    return m_Icon;
}

void CDynamicIcon::releaseInstance()
{
    if (m_Icon != nullptr) {
        delete m_Icon;
        m_Icon = nullptr;
    }
}

void CDynamicIcon::setDate(const QDate &date)
{
    m_Date = date;
    QString dayfile = QString(":/resources/DynamicIcon/day%1.svg").arg(date.day());

    m_Dayrenderer->load(dayfile);

    QString weekfile = QString(":/resources/DynamicIcon/week%1.svg").arg(date.dayOfWeek());
    m_Weekrenderer->load(weekfile);

    QString monthfile = QString(":/resources/DynamicIcon/month%1.svg").arg(date.month());
    m_Monthrenderer->load(monthfile);

    QString backgroundfile(":/resources/DynamicIcon/calendar_bg.svg");
    m_backgroundrenderer->load(backgroundfile);

    //draw pixmap
    paintPixmap(m_pixmap);

}

void CDynamicIcon::setIcon()
{
    QIcon icon(this->getPixmap());
    qApp->setProductIcon(icon);
    qApp->setWindowIcon(icon);

    if (qApp->aboutDialog() != nullptr)
        qApp->aboutDialog()->setProductIcon(icon);

    m_Titlebar->setIcon(icon);
}

void CDynamicIcon::setTitlebar(DTitlebar *titlebar)
{
    m_Titlebar = titlebar;
}

void CDynamicIcon::paintPixmap(QPixmap *pixmap)
{
    QPainter painter(pixmap);

    //draw background
    qreal backgroundwidget = pixmap->size().width() * BackgroundRatio;
    qreal backgroundHeight = pixmap->size().height() * BackgroundRatio;
    QRectF backgroundRect = QRectF((pixmap->size().width() - backgroundwidget) / 2,
                                   (pixmap->size().height() - backgroundHeight) / 2,
                                   backgroundwidget,
                                   backgroundHeight);
    m_backgroundrenderer->render(&painter, backgroundRect);

    //draw month
    qreal month_x = pixmap->size().width() * Month_X_Ratio;
    qreal month_y = pixmap->size().width() * Month_Y_Ratio;
    qreal month_w = pixmap->size().width() * Month_W_Ratio;
    qreal month_h = pixmap->size().width() * Month_H_Ratio;
    m_Monthrenderer->render(&painter, QRectF(month_x, month_y, month_w, month_h));

    //draw day
    qreal day_x = pixmap->size().width() * Day_X_Ratio;
    qreal day_y = pixmap->size().width() * Day_Y_Ratio;
    qreal day_w = pixmap->size().width() * Day_W_Ratio;
    qreal day_h = pixmap->size().width() * Day_H_Ratio;
    m_Dayrenderer->render(&painter, QRectF(day_x, day_y, day_w, day_h));

    //draw week
    qreal week_x = pixmap->size().width() * Week_X_Ratio;
    qreal week_y = pixmap->size().width() * Week_Y_Ratio;
    qreal week_w = pixmap->size().width() * Week_W_Ratio;
    qreal week_h = pixmap->size().width() * Week_H_Ratio;
    m_Weekrenderer->render(&painter, QRectF(week_x, week_y, week_w, week_h));

    painter.end();
}
