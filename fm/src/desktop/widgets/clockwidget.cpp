// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/widgets/clockwidget.h"

#include <algorithm>
#include <chrono>
#include <ctime>

#include "base/locale.h"
#include "ui/label.h"
#include "ui/theme.h"

namespace fm {

namespace {

std::string clockText(const base::Locale& locale, const base::DateTime& now)
{
    return locale.toString(now.time, base::Locale::FormatType::Short);
}

std::string dateText(const base::Locale& locale, const base::DateTime& now)
{
    return locale.toString(now.date, "dddd, d MMMM");
}

} // namespace

ClockWidget::ClockWidget(WidgetLayer& layer, WidgetSize size)
    : DesktopWidget(layer, WidgetKind::Clock, size)
{
    setName("clock-widget");
    m_time = addChild<ui::Label>();
    m_time->setElide(ui::Label::Elide::Right);
    m_date = addChild<ui::Label>();
    m_date->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.62)));
    m_date->setElide(ui::Label::Elide::Right);

    m_timer.setInterval(std::chrono::seconds(1));
    m_timer.setSingleShot(false);
    m_timer.onTimeout = [this] { refresh(); };
    m_timer.start();
    refresh();
}

ClockWidget::~ClockWidget() = default;

void ClockWidget::refresh()
{
    const std::time_t seconds = std::time(nullptr);
    const base::DateTime now = base::DateTime::fromSecondsSinceEpoch(seconds);
    const base::Locale& locale = base::Locale::system();
    const std::string time = clockText(locale, now);
    const std::string date = dateText(locale, now);
    if (time != m_lastTime) {
        m_lastTime = time;
        m_time->setText(time);
        requestLayout();
    }
    if (date != m_lastDate) {
        m_lastDate = date;
        m_date->setText(date);
        requestLayout();
    }
}

void ClockWidget::layoutContent()
{
    const ui::RectF content = contentRect();
    if (content.width <= 0 || content.height <= 0)
        return;
    double timeSize = 40;
    if (size() == WidgetSize::Small)
        timeSize = 30;
    else if (size() == WidgetSize::Large)
        timeSize = 48;
    m_time->setFontSize(timeSize);
    m_time->setWeight(500);
    m_time->setVerticallyCentered(true);
    m_time->setTextAlignment(ui::Label::TextAlignment::Center);
    m_date->setTextAlignment(ui::Label::TextAlignment::Center);
    m_date->setFontSize(size() == WidgetSize::Small ? 11 : 13);
    m_date->setWeight(500);
    m_date->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.62)));

    const double dateHeight = std::max(16.0, m_date->sizeHint().height);
    m_time->setGeometry({content.x, content.y, content.width,
                         std::max(0.0, content.height - dateHeight - 2)});
    m_date->setGeometry({content.x, content.y + content.height - dateHeight, content.width, dateHeight});
}

void ClockWidget::retranslateContent()
{
    m_lastTime.clear();
    m_lastDate.clear();
    refresh();
}

} // namespace fm
