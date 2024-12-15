// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clock.h"

#include <QPainter>
#include <QTimeZone>
#include <QIcon>
#include <QDebug>
#include <QResizeEvent>
#include <DDciIcon>
#include <DFontSizeManager>
#include <DFontManager>
#include <QApplication>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
namespace dwclock {
static const QSize baseClockSize = QSize(224, 224);
static const QSize basePointSize = QSize(224, 224);

Clock::Clock()
{
    updateLocationFont();
}

Clock::~Clock()
{

}

QPixmap Clock::getPixmap(const QString &name, const QSize &size)
{
    const QIcon &icon = QIcon::fromTheme(name);
    const auto ratio = qApp->devicePixelRatio();
    QPixmap pixmap = icon.pixmap(size * ratio).scaled(size * ratio, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QPainter p(&pixmap);
    p.setRenderHints(QPainter::Antialiasing);
    p.drawPixmap(0, 0, pixmap);
    pixmap.setDevicePixelRatio(ratio);
    return pixmap;
}

QPixmap Clock::getPixmap(const QString &name, const int size, const bool isDark)
{
    const auto &icon = DDciIcon::fromTheme(name);
    return icon.pixmap(1, size, isDark ? DDciIcon::Dark : DDciIcon::Light);
}

void Clock::paint(QPainter *painter, const QRect &rect)
{
    m_clockPlatSize = rect.size();
    const auto ratio = m_clockPlatSize.width() * 1.0 / baseClockSize.width();
    m_clockPointSize = basePointSize * ratio;

    QDateTime datetime(QDateTime::currentDateTime());
    datetime = datetime.addSecs(timeDiff(m_utcOffset, datetime));
    const QTime time(datetime.time());

    const bool nightMode = !(time.hour() >= 6 && time.hour() < 18);
    updateClockPixmap(nightMode);

    const auto center = rect.center();

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    // draw plate
    QRect platRect = m_plat.rect();
    platRect.moveCenter(center);
    painter->drawPixmap(platRect, m_plat);

    int nHour = (time.hour() >= 12) ? (time.hour() - 12) : time.hour();
    int nStartAngle = 0; //The image from 0 start

    const QPointF clockPointPosition(-m_clockPointSize.width() / 2.0, -m_clockPointSize.height() / 2.0);
    // draw hour hand
    const qreal hourAngle = qreal(nHour * 30 + time.minute() * 30 / 60 + time.second() * 30 / 60 / 60 - nStartAngle);
    painter->save();
    painter->translate(center);
    painter->rotate(hourAngle);
    painter->drawPixmap(clockPointPosition, m_hour);
    painter->restore();

    // draw minute hand
    const qreal minuteAngle = qreal(time.minute() * 6 + time.second() * 6 / 60 - nStartAngle);
    painter->save();
    painter->translate(center);
    painter->rotate(minuteAngle);
    painter->drawPixmap(clockPointPosition, m_min);
    painter->restore();

    // draw second hand
    const qreal secondAngle = qreal(time.second() * 6 - nStartAngle);
    painter->save();
    painter->translate(center);
    painter->rotate(secondAngle);
    painter->drawPixmap(clockPointPosition, m_sec);
    painter->restore();

    if (!m_location.isEmpty()) {
        // draw location
        painter->save();
        painter->translate(center);
        painter->setPen(nightMode ? Qt::white : Qt::black);
        painter->setFont(m_locationTxtFont);

        QFontMetrics fmTxt(m_locationTxtFont);
        // we assume height is equal width, so maxWidth == r - height.
        auto txt = fmTxt.elidedText(m_location, Qt::ElideRight, m_clockPlatSize.width() - fmTxt.height());
        const QSize textSize = fmTxt.size(Qt::TextSingleLine, txt);
        painter->drawText(QRect(-textSize.width() / 2, -m_clockPlatSize.height() / 3, textSize.width(), textSize.height()),
                         Qt::AlignHCenter | Qt::AlignVCenter,
                         txt);
        painter->restore();
    }
}

int Clock::timeDiff(int utcOffset,  const QDateTime &localTime)
{
    if (!localTime.isValid()) {
        QDateTime *currentTime = const_cast<QDateTime *>(&localTime);
        *currentTime = QDateTime::currentDateTime();
    }
    return utcOffset - localTime.offsetFromUtc();
}

void Clock::setUTCOffset(const int utcOffset)
{
    if (m_utcOffset == utcOffset)
        return;

    m_utcOffset = utcOffset;
}

void Clock::setLocation(const QString &location)
{
    if (m_location == location)
        return;

    m_location = location;
}

void Clock::updateClockPixmap(const bool isNightMode)
{
    if (isNightMode != m_isBlack || m_plat.size() != m_clockPlatSize) {
        m_hour = getPixmap("clock_small_hours", m_clockPointSize.width(), isNightMode);
        m_min = getPixmap("clock_small_minute", m_clockPointSize.width(), isNightMode);
        m_sec = getPixmap("clock_small_sec", m_clockPointSize.width(), isNightMode);

        m_plat = getPixmap("clock_small", m_clockPlatSize.width(), isNightMode);

        m_isBlack = isNightMode;
    }
}

void Clock::updateLocationFont()
{
    m_locationTxtFont = DFontSizeManager::instance()->t8();
    m_locationTxtFont.setFamily("SourceHanSansSC");
    m_locationTxtFont.setWeight(QFont::ExtraLight);
}
}
