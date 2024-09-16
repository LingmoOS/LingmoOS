// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clockpanel.h"
#include "clock.h"
#include "global.h"
#include "timezonemodel.h"

#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include <DFontSizeManager>
#include <QTimer>
#include <QPainter>
#include <QDateTime>
#include <QtMath>

#include <DPaletteHelper>
#include <DStyleOption>
DWIDGET_USE_NAMESPACE

namespace dwclock {
ClockDelegate::ClockDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

QSize ClockDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto view = qobject_cast<const ClockView *>(option.widget);
    Q_ASSERT(view);

    QSize cellSize;
    auto size = view->contentsRect().size();
    // TODO it mabey a bug, QListView's width == item's width * itemCount + (spacing + 1) * itemCount,
    // but it can't place all item in line, and QListView's width must be greater sum of them.
    // so cellWidth is set to `cellWidth - 1` temporarily.
    if (view->isSmallType()) {
        const int columnCount = 2;
        auto cellWidth = (size.width() - (columnCount + 1) * (view->spacing())) / columnCount;
        cellSize = QSize(cellWidth - 1, cellWidth);
    } else {
        auto cellWidth = (size.width() - (index.model()->rowCount() + 1) * (view->spacing())) / qMax(index.model()->rowCount(), 1);
        cellSize = QSize(cellWidth - 1, size.height() - view->spacing() * 2);
    }
    return cellSize;
}

void ClockDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto view = qobject_cast<const ClockView *>(option.widget);
    Q_ASSERT(view);

    QPoint offsetPoint(0, 0);
    // draw background
    QRect contentRect(option.rect);
    if (!view->isSmallType()) {
        painter->save();
        painter->setBrush(UI::clock::middleClockBackground);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(contentRect, contentRect.width() / 2, contentRect.width() / 2);
        painter->restore();
    }

    const int utcOffset = index.data(TimezoneModel::ZoneOffset).toInt();
    m_clock.setUTCOffset(utcOffset);
    const QString &location = index.data().toString();

    m_clock.setLocation(view->isSmallType() ? location : QString());

    // draw clock
    {
        QRect clockRect;
        if (view->isSmallType()) {
            clockRect = option.rect;
        } else {
            clockRect = QRect(contentRect.x(), contentRect.y(), contentRect.width(), contentRect.width());
            clockRect.adjust(UI::clock::spacingBackgroundAndClock, UI::clock::spacingBackgroundAndClock,
                             -UI::clock::spacingBackgroundAndClock, -UI::clock::spacingBackgroundAndClock);
        }
        m_clock.paint(painter, clockRect);
        offsetPoint.ry() = clockRect.bottom() + UI::clock::spacingCityNameAndClock;
    }

    if (!view->isSmallType()) {
        // draw location
        painter->save();
        {
            QFont txtFont = DFontSizeManager::instance()->t8();
            txtFont.setWeight(QFont::Bold);
            QFontMetrics fmTxt(txtFont);
            auto txt = fmTxt.elidedText(location, Qt::ElideRight, contentRect.width());
            const QSize textSize = fmTxt.size(Qt::TextSingleLine, txt);
            QRect locationRect(QPoint(0, 0), textSize);
            locationRect.moveTo(contentRect.center().x() - qCeil(textSize.width() / 2.0), offsetPoint.y());
            painter->setFont(txtFont);
            painter->drawText(locationRect, Qt::AlignCenter, txt);
            offsetPoint.ry() = locationRect.bottom() + UI::clock::spacingCityNameAndTimezoneOffset;
        }
        painter->restore();

        // draw prompt of offset
        painter->save();
        {
            QFont txtFont = DFontSizeManager::instance()->t9();
            txtFont.setWeight(QFont::Normal);
            const QString offsetTxt(promptOfUtcOffsetText(utcOffset));
            QFontMetrics fmTxt(txtFont);
            const QSize textSize = fmTxt.size(Qt::TextSingleLine, offsetTxt);
            QRect offsetRect(QPoint(0, 0), textSize);
            offsetRect.moveTo(contentRect.center().x() - qCeil(textSize.width() / 2.0), offsetPoint.y());
            painter->setFont(txtFont);
            painter->drawText(offsetRect, Qt::AlignCenter, offsetTxt);
            offsetPoint.ry() = offsetRect.bottom() + UI::clock::spacingCityNameAndTimezoneOffset;
        }
        painter->restore();

        // draw offset
        painter->save();
        {
            QFont txtFont = DFontSizeManager::instance()->t9();
            txtFont.setWeight(QFont::Normal);
            const QString offsetTxt(utcOffsetText(utcOffset));
            QFontMetrics fmTxt(txtFont);
            const QSize textSize = fmTxt.size(Qt::TextSingleLine, offsetTxt);
            QRect offsetRect(QPoint(0, 0), textSize);
            offsetRect.moveTo(contentRect.center().x() - qCeil(textSize.width() / 2.0), offsetPoint.y());
            painter->setFont(txtFont);
            painter->drawText(offsetRect, Qt::AlignCenter, offsetTxt);
            offsetPoint.ry() = offsetRect.bottom() + UI::clock::spacingCityNameAndTimezoneOffset;
        }
        painter->restore();
    }
}

QString ClockDelegate::utcOffsetText(const int offset) const
{
    const int time = (Clock::timeDiff(offset)) / 3600;
    return QString("%1%2h").arg(time > 0 ? "+" : "").arg(time);
}

QString ClockDelegate::promptOfUtcOffsetText(const int offset) const
{
    const QDateTime localTime(QDateTime::currentDateTime());

    const double timeDelta = (Clock::timeDiff(offset, localTime)) / 3600.0;

    QString dateLiteral;
    if (localTime.time().hour() + timeDelta >= 24) {
        dateLiteral = tr("Tomorrow");
    } else if (localTime.time().hour() + timeDelta <= 0) {
        dateLiteral = tr("Yesterday");
    } else {
        dateLiteral = tr("Today");
    }
    return dateLiteral;
}

ClockView::ClockView(QWidget *parent)
    : QListView (parent)
{
    setFrameShape(QFrame::NoFrame);
    setFlow(QListView::LeftToRight);
    setItemDelegate(new ClockDelegate(this));

    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setSelectionMode(QAbstractItemView::NoSelection);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Update per second.
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this](){ update(); });
    timer->start(1000);
}

ClockView::~ClockView()
{
}

bool ClockView::isSmallType() const
{
    return m_isSmallType;
}

void ClockView::setSmallType(const bool small)
{
    if (m_isSmallType == small)
        return;

    m_isSmallType = small;
    update();
}

ClockPanel::ClockPanel(QWidget *parent)
    : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(UI::clock::defaultMargins);
    m_layout->setSpacing(0);

    m_view = new ClockView();
    m_layout->addWidget(m_view);

    QPalette pt = palette();
    // QFrame doesn't draw Background using QPalette::Window.
    pt.setBrush(QPalette::Base, Qt::transparent);
    setPalette(pt);
    setAutoFillBackground(true);
}

ClockView *ClockPanel::view() const
{
    return m_view;
}

void ClockPanel::setSmallType(const bool isSmallType)
{
    if (isSmallType) {
        m_layout->setContentsMargins(6, 6, 6, 6);
        m_view->setSpacing(UI::clock::smallSpacing);
    } else {
        m_layout->setContentsMargins(UI::clock::defaultMargins);
        m_view->setSpacing(UI::clock::middleSpacing);
    }
    m_view->setSmallType(isSmallType);
}

void ClockPanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QColor color = DPaletteHelper::instance()->palette(this->topLevelWidget()).color(QPalette::Base);
    color.setAlpha(100);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    auto radius = roundedCornerRadius();
    painter.drawRoundedRect(rect(), radius, radius);
}

int ClockPanel::roundedCornerRadius() const
{
    return m_roundedCornerRadius;
}

void ClockPanel::setRoundedCornerRadius(int newRoundedCornerRadius)
{
    if (m_roundedCornerRadius == newRoundedCornerRadius)
        return;
    m_roundedCornerRadius = newRoundedCornerRadius;
    emit roundedCornerRadiusChanged();
}

}
