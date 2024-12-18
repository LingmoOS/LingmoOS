// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cweekwidget.h"
#include "calendarmanager.h"
#include "constants.h"
#include <QLocale>
#include <QPainter>

static QStringList weekStr;

CWeekWidget::CWeekWidget(QWidget *parent) : QPushButton(parent)
  , m_autoFirstDay(true)
  , m_autoFontSizeByWindow(true)
  , m_firstDay(CalendarManager::instance()->firstDayOfWeek())
{
    setMinimumHeight(10);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setFocusPolicy(Qt::NoFocus);
    weekStr << tr("Sun") << tr("Mon") << tr("Tue") << tr("Wed") << tr("Thu") << tr("Fri") << tr("Sat");
}

void CWeekWidget::setFirstDay(Qt::DayOfWeek first)
{
    if(m_firstDay == first)
        return;

    m_firstDay = first;
    setAutoFirstDay(true);
    update();
}

void CWeekWidget::setAutoFirstDay(bool is)
{
    if(m_autoFirstDay == is)
        return;

    m_autoFirstDay = is;
    update();
}

void CWeekWidget::setAutoFontSizeByWindow(bool is)
{
    if(m_autoFontSizeByWindow == is)
        return;

    m_autoFontSizeByWindow = is;
    update();
}

void CWeekWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font;

    if (m_autoFontSizeByWindow) {
        //字体跟随界面大小
        qreal w = this->width() / 7;
        qreal h = this->height();
        qreal r = w > h ? h : w ;

        if (QLocale::system().language() == QLocale::English) {
            r*=0.8;
        }

        //根据高度和宽度设置时间字体的大小

        font.setPixelSize(int(r/20.0*12));

    } else {
        // font.setPixelSize(OCEANCalendar::FontSizeTwelve);
    }

    painter.setFont(font);

    QLocale locale;
    qreal setp = (width())/7.0;
    // 获取一周首日
    int firstDay = m_firstDay;
    if (m_autoFirstDay) {
        firstDay = CalendarManager::instance()->firstDayOfWeek();
    }

    // 绘制周一到周日
    for (int i = Qt::Monday; i <= Qt::Sunday; ++i) {
        int index = (firstDay + i - Qt::Monday) % Qt::Sunday;

        const QString &text = weekStr[index];
        QRectF rect((i-Qt::Monday)*setp, 0, setp, height());
        painter.drawText(rect, Qt::AlignCenter, text);
    }
}
