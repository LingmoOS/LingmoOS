// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/widgets/calendarwidget.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <optional>

#include "base/i18n.h"
#include "base/locale.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"

static const char* tr(const char* text) { return Lingmo::i18n::dtr("lingmo-filemanager", text); }

namespace fm {

namespace {

int firstWeekdayOfMonth(int year, int month)
{
    const std::chrono::year_month_day first {std::chrono::year {year},
                                             std::chrono::month {unsigned(month)},
                                             std::chrono::day {1}};
    return int(std::chrono::weekday {std::chrono::sys_days {first}}.iso_encoding());
}

int daysInMonth(int year, int month)
{
    const std::chrono::year_month_day_last last {
            std::chrono::year {year}, std::chrono::month_day_last {std::chrono::month {unsigned(month)}}};
    return int(unsigned(last.day()));
}

} // namespace

// A small painted chevron: the month arrows.
class MonthArrow final : public ui::View
{
public:
    explicit MonthArrow(bool forward)
        : m_forward(forward)
    {
        setCursorShape(ui::CursorShape::Pointer);
    }

    bool acceptsPointer() const override { return true; }

    void paint(ui::Painter& painter) override
    {
        const ui::Theme& theme = widgetTheme(this);
        const ui::Color color = theme.text.withAlpha(m_hovered ? 0.9 : 0.55);
        const ui::RectF rect = localRect();
        const double cx = rect.width / 2;
        const double cy = rect.height / 2;
        const double dx = m_forward ? 2.5 : -2.5;
        painter.drawLine({cx - dx, cy - 4}, {cx + dx, cy}, color, 1.5);
        painter.drawLine({cx + dx, cy}, {cx - dx, cy + 4}, color, 1.5);
    }

    std::function<void()> onClicked;

protected:
    void pointerEnter() override
    {
        m_hovered = true;
        requestPaint();
    }
    void pointerLeave() override
    {
        m_hovered = false;
        requestPaint();
    }
    void pointerRelease(const ui::PointerEvent& event) override
    {
        if (localRect().contains(event.position) && onClicked)
            onClicked();
    }

private:
    bool m_forward;
    bool m_hovered = false;
};

CalendarWidget::CalendarWidget(WidgetLayer& layer, WidgetSize size)
    : DesktopWidget(layer, WidgetKind::Calendar, size)
{
    setName("calendar-widget");
    m_title = addChild<ui::Label>();
    m_title->setWeight(600);
    m_title->setElide(ui::Label::Elide::Right);

    m_previous = addChild<MonthArrow>(false);
    m_next = addChild<MonthArrow>(true);
    m_previous->setToolTip(tr("Previous Month"));
    m_next->setToolTip(tr("Next Month"));
    m_previous->onClicked = [this] { shiftMonth(-1); };
    m_next->onClicked = [this] { shiftMonth(1); };

    for (int i = 0; i < 7; ++i) {
        ui::Label* weekday = addChild<ui::Label>();
        weekday->setTextAlignment(ui::Label::TextAlignment::Center);
        weekday->setVerticallyCentered(true);
        m_weekdays.push_back(weekday);
    }
    for (int i = 0; i < 42; ++i) {
        ui::Label* day = addChild<ui::Label>();
        day->setTextAlignment(ui::Label::TextAlignment::Center);
        day->setVerticallyCentered(true);
        m_days.push_back(day);
    }

    const base::DateTime now = base::DateTime::fromSecondsSinceEpoch(std::time(nullptr));
    showMonth(now.date.year, now.date.month);
    m_lastDay=now.date.day;
    m_dayTimer.setInterval(std::chrono::minutes(1));
    m_dayTimer.onTimeout=[this]{const auto today=base::DateTime::fromSecondsSinceEpoch(std::time(nullptr));
        if(today.date.day!=m_lastDay){m_lastDay=today.date.day;updateDayLabels();requestLayout();requestPaint();}};
    m_dayTimer.start();
}

CalendarWidget::~CalendarWidget() = default;

void CalendarWidget::showMonth(int year, int month)
{
    while (month < 1) {
        month += 12;
        --year;
    }
    while (month > 12) {
        month -= 12;
        ++year;
    }
    m_year = std::clamp(year, 1900, 2200);
    m_month = month;

    const base::Locale& locale = base::Locale::system();
    m_title->setText(locale.toString(base::CalendarDate {m_year, m_month, 1}, size()==WidgetSize::Small ? "MMM" : "MMMM yyyy"));
    m_title->setToolTip(locale.toString(base::CalendarDate {m_year, m_month, 1}, "MMMM yyyy"));

    const int firstDay = locale.firstDayOfWeek();
    for (int i = 0; i < 7; ++i) {
        const int day = ((firstDay - 1 + i) % 7) + 1;
        m_weekdays[size_t(i)]->setText(locale.standaloneDayName(day, base::Locale::FormatType::Narrow));
    }
    updateDayLabels();
    requestLayout();
    requestPaint();
}

void CalendarWidget::updateDayLabels()
{
    const base::Locale& locale = base::Locale::system();
    const int firstDay = locale.firstDayOfWeek();
    const int weekday = firstWeekdayOfMonth(m_year, m_month);
    const int offset = (weekday - firstDay + 7) % 7;
    const int days = daysInMonth(m_year, m_month);

    const base::DateTime now = base::DateTime::fromSecondsSinceEpoch(std::time(nullptr));
    const bool current = now.date.year == m_year && now.date.month == m_month;
    m_todayIndex = -1;
    for (int i = 0; i < 42; ++i) {
        ui::Label* label = m_days[size_t(i)];
        const int day = i - offset + 1;
        if (day < 1 || day > days) {
            label->setVisible(false);
            continue;
        }
        label->setVisible(true);
        label->setText(locale.toString(day));
        if (current && day == now.date.day)
            m_todayIndex = i;
    }
}

void CalendarWidget::shiftMonth(int delta)
{
    int month = m_month + delta;
    int year = m_year;
    if (month < 1) {
        month = 12;
        --year;
    } else if (month > 12) {
        month = 1;
        ++year;
    }
    showMonth(year, month);
}

void CalendarWidget::layoutContent()
{
    const ui::RectF content = contentRect();
    if (content.width <= 0 || content.height <= 0)
        return;
    const double titleHeight = 22;
    const double weekdayHeight = 16;
    const double arrowsWidth = 20;
    m_title->setFontSize(12);
    m_title->setColor(widgetTheme(this).highlight);
    m_title->setGeometry({content.x, content.y, std::max(0.0, content.width - arrowsWidth * 2), titleHeight});
    m_previous->setGeometry({content.right() - arrowsWidth * 2, content.y, arrowsWidth, titleHeight});
    m_next->setGeometry({content.right() - arrowsWidth, content.y, arrowsWidth / 2 + 4, titleHeight});

    const double cellWidth = content.width / 7;
    const double weekdayY = content.y + titleHeight + 4;
    for (int i = 0; i < 7; ++i) {
        ui::Label* weekday = m_weekdays[size_t(i)];
        weekday->setFontSize(10);
        weekday->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.65)));
        weekday->setGeometry({content.x + i * cellWidth, weekdayY, cellWidth, weekdayHeight});
    }

    const double gridY = weekdayY + weekdayHeight + 2;
    const int offset=(firstWeekdayOfMonth(m_year,m_month)-base::Locale::system().firstDayOfWeek()+7)%7;
    const int rows=(offset+daysInMonth(m_year,m_month)+6)/7;
    const double cellHeight = std::max(0.0, (content.bottom() - gridY) / rows);
    for (int i = 0; i < 42; ++i) {
        ui::Label* day = m_days[size_t(i)];
        const int row = i / 7;
        const int column = i % 7;
        day->setFontSize(std::clamp(cellWidth * 0.42, 9.0, 12.0));
        day->setWeight(i == m_todayIndex ? 700 : 400);
        day->setColor(i == m_todayIndex ? std::optional<ui::ColorRecipe>(widgetTheme(this).highlightedText)
                                        : std::nullopt);
        day->setGeometry({content.x + column * cellWidth, gridY + row * cellHeight, cellWidth, cellHeight});
    }
}

void CalendarWidget::paintContent(ui::Painter& painter)
{
    const ui::Theme& theme = widgetTheme(this);
    const ui::RectF content = contentRect();
    if (content.width <= 0)
        return;
    // Today's day gets a faint disc behind its label.
    if (m_todayIndex >= 0 && size_t(m_todayIndex) < m_days.size()) {
        const ui::RectF cell = m_days[size_t(m_todayIndex)]->geometry();
        const double radius = std::min(cell.width, cell.height) * 0.46;
        const double cx = cell.x + cell.width / 2;
        const double cy = cell.y + cell.height / 2;
        painter.fillEllipse({cx - radius, cy - radius, radius * 2, radius * 2},
                            theme.highlight);
    }

}

void CalendarWidget::retranslateContent()
{
    m_previous->setToolTip(tr("Previous Month"));
    m_next->setToolTip(tr("Next Month"));
    showMonth(m_year, m_month);
}

} // namespace fm
