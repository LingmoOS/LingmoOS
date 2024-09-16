// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "timewidget.h"

#include <QVBoxLayout>
#include <QDateTime>
#include <QFontDatabase>
#include <QSettings>

const QStringList weekdayFormat = {"dddd", "ddd"};
const QStringList shortDateFormat = { "yyyy/M/d", "yyyy-M-d", "yyyy.M.d",
                                      "yyyy/MM/dd", "yyyy-MM-dd", "yyyy.MM.dd",
                                      "yy/M/d", "yy-M-d", "yy.M.d" };
const QStringList shortTimeFormat = { "h:mm", "hh:mm"};

TimeWidget::TimeWidget(QWidget *parent)
    : QWidget(parent)
    , m_use24HourFormat(true)
{
    QFont timeFont;
    timeFont.setFamily("Noto Sans CJK SC-Thin");

    m_timeLabel = new QLabel;
    timeFont.setWeight(QFont::ExtraLight);
    m_timeLabel->setFont(timeFont);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    QPalette palette = m_timeLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_timeLabel->setPalette(palette);
    DFontSizeManager::instance()->bind(m_timeLabel, DFontSizeManager::T1);

    m_dateLabel = new QLabel;
    m_dateLabel->setAlignment(Qt::AlignCenter);
    palette = m_dateLabel->palette();
    palette.setColor(QPalette::WindowText, Qt::white);
    m_dateLabel->setPalette(palette);
    DFontSizeManager::instance()->bind(m_dateLabel, DFontSizeManager::T6);

    refreshTime();

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(1000);
    m_refreshTimer->start();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(m_timeLabel);
    vLayout->addWidget(m_dateLabel);
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(vLayout);

    connect(m_refreshTimer, &QTimer::timeout, this, &TimeWidget::refreshTime);
}

void TimeWidget::set24HourFormat(bool use24HourFormat)
{
    m_use24HourFormat = use24HourFormat;
    refreshTime();
}

void TimeWidget::updateLocale(const QString &locale, const QString &shortTimeFormat, const QString &longDateFormat)
{
    m_locale = locale.isEmpty() ? QLocale::system() : QLocale(locale);
    if (!shortTimeFormat.isEmpty())
        m_shortTimeFormat = shortTimeFormat;
    if (!longDateFormat.isEmpty())
        m_longDateFormat = longDateFormat;
    refreshTime();
}

void TimeWidget::refreshTime()
{
    if (m_use24HourFormat) {
        m_timeLabel->setText(m_locale.toString(QDateTime::currentDateTime(), shortTimeFormat.at(m_shortTimeIndex)));
    } else {
        m_timeLabel->setText(m_locale.toString(QDateTime::currentDateTime(), shortTimeFormat.at(m_shortTimeIndex) + " AP"));
    }

    QString date_format = shortDateFormat.at(m_shortDateIndex) + " " + weekdayFormat.at(m_weekdayIndex);
    m_dateLabel->setText(m_locale.toString(QDateTime::currentDateTime(), date_format));

    if (!m_shortTimeFormat.isEmpty() && !m_longDateFormat.isEmpty()) {
        m_timeLabel->setText(m_locale.toString(QTime::currentTime(), m_shortTimeFormat));
        m_dateLabel->setText(m_locale.toString(QDate::currentDate(), m_longDateFormat));
    }
}

/**
 * @brief TimeWidget::setWeekdayFormatType 根据类型来设置周显示格式
 * @param type 自定义类型
 */
void TimeWidget::setWeekdayFormatType(int type)
{
    if(type >= weekdayFormat.size() || type < 0) return;

    m_weekdayIndex = type;
    refreshTime();
}

/**
 * @brief TimeWidget::setShortDateFormat 根据类型来设置短日期显示格式
 * @param type 自定义格式
 */
void TimeWidget::setShortDateFormat(int type)
{
    if(type >= shortDateFormat.size() || type < 0) return;

    m_shortDateIndex = type;
    refreshTime();
}

/**
 * @brief TimeWidget::setShortTimeFormat 根据类型来设置短时间显示格式
 * @param type
 */
void TimeWidget::setShortTimeFormat(int type)
{
    if(type >= shortTimeFormat.size() || type < 0) return;

    m_shortTimeIndex = type;
    refreshTime();
}
