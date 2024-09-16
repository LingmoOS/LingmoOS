// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin.h"
#include "common/helper.hpp"

#include <QMouseEvent>
#include <QVBoxLayout>
#include <QPainter>

#include <DBlurEffectWidget>
#include <DFrame>
#include <DDBusSender>
#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE

namespace dwclock {
QString CalendarWidgetPlugin::title() const
{
    return tr("Calendar");
}

QString CalendarWidgetPlugin::description() const
{
    return tr("Display calendar");
}

QString CalendarWidgetPlugin::aboutDescription() const
{
    return tr("Viewing calendar");
}

IWidget *CalendarWidgetPlugin::createWidget()
{
    return new CalendarWidget();
}

QWidget *CalendarWidget::view()
{
    auto frame = new CalendarContainer();
    frame->setFixedSize(handler()->size());
    frame->setRoundedCornerRadius(handler()->roundedCornerRadius());
    auto layout = new QVBoxLayout(frame);
    layout->setMargin(0);
    DCalendarWidget *calendar = new DCalendarWidget();
    calendar->setAutoFillBackground(false);
    DFontSizeManager::instance()->bind(calendar, DFontSizeManager::T7);
    layout->addWidget(calendar, 0, Qt::AlignCenter);
    frame->installEventFilter(this);
    m_calendar = calendar;
    return frame;
}

bool CalendarWidget::initialize(const QStringList &arguments) {
    Q_UNUSED(arguments);
    static bool hasLoaded = false;
    if (!hasLoaded)
        hasLoaded = BuildinWidgetsHelper::instance()->loadTranslator("dde-widgets-calendar_");

    return true;
}

void CalendarWidget::showWidgets()
{
    if (m_calendar) {
        m_calendar->setSelectedDate(QDate::currentDate());
    }
}

bool CalendarWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (qobject_cast<CalendarContainer *>(watched) &&
        event->type() == QEvent::MouseButtonDblClick)
    {
        showDDECalendar();
        return true;
    }
    return QObject::eventFilter(watched, event);
}

void CalendarWidget::showDDECalendar()
{
    qDebug(dwLog()) << "showDDECalendar()";
    DDBusSender().service("com.deepin.Calendar")
        .interface("com.deepin.Calendar")
        .path("/com/deepin/Calendar")
        .method("RaiseWindow")
        .call();
}

QIcon CalendarWidgetPlugin::logo() const
{
    return QIcon::fromTheme("dde-calendar");
}

QStringList CalendarWidgetPlugin::contributors() const
{
    return {BuildinWidgetsHelper::instance()->contributor()};
}

CalendarContainer::CalendarContainer(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(false);
}

int CalendarContainer::roundedCornerRadius() const
{
    return m_roundedCornerRadius;
}

void CalendarContainer::setRoundedCornerRadius(int newRadius)
{
    if (m_roundedCornerRadius == newRadius)
        return;
    m_roundedCornerRadius = newRadius;
    emit roundedCornerRadiusChanged();
}

void CalendarContainer::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(palette().base());
    p.setPen(Qt::NoPen);
    auto radius = roundedCornerRadius();
    p.drawRoundedRect(rect(), radius, radius);
}
}
