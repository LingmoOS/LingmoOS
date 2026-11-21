// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/widgets/batterywidget.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include "base/i18n.h"
#include "base/locale.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"

static const char* tr(const char* text) { return Lingmo::i18n::dtr("lingmo-filemanager", text); }

namespace fm {

namespace {

constexpr const char* kPowerSupply = "/sys/class/power_supply";

struct BatteryState
{
    bool present = false;
    int capacity = -1;
    std::string status;
};

std::string readValue(const std::filesystem::path& path)
{
    std::ifstream file(path);
    std::string value;
    std::getline(file, value);
    while (!value.empty() && (value.back() == '\n' || value.back() == '\r' || value.back() == ' '))
        value.pop_back();
    return value;
}

BatteryState readBattery()
{
    std::error_code error;
    for (const auto& entry : std::filesystem::directory_iterator(kPowerSupply, error)) {
        if (error || !entry.is_directory())
            continue;
        if (readValue(entry.path() / "type") != "Battery")
            continue;
        BatteryState state;
        state.present = true;
        try {
            state.capacity = std::clamp(std::stoi(readValue(entry.path() / "capacity")), 0, 100);
        } catch (...) {
            state.capacity = -1;
        }
        state.status = readValue(entry.path() / "status");
        return state;
    }
    return {};
}

std::string statusText(const std::string& status)
{
    if (status == "Charging")
        return tr("Charging");
    if (status == "Discharging")
        return tr("On battery");
    if (status == "Full")
        return tr("Full");
    if (status == "Not charging")
        return tr("Not charging");
    return tr("Unknown");
}

} // namespace

BatteryWidget::BatteryWidget(WidgetLayer& layer, WidgetSize size)
    : DesktopWidget(layer, WidgetKind::Battery, size)
{
    setName("battery-widget");
    m_title = addChild<ui::Label>();
    m_title->setFontSize(11);
    m_title->setWeight(600);
    m_title->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.58)));
    m_percent = addChild<ui::Label>();
    m_percent->setWeight(300);
    m_percent->setElide(ui::Label::Elide::Right);
    m_percent->setVerticallyCentered(true);
    m_status = addChild<ui::Label>();
    m_status->setFontSize(11);
    m_status->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.62)));

    m_timer.setInterval(std::chrono::seconds(30));
    m_timer.setSingleShot(false);
    m_timer.onTimeout = [this] { refresh(); };
    m_timer.start();
    refresh();
}

BatteryWidget::~BatteryWidget() = default;

void BatteryWidget::refresh()
{
    const BatteryState state = readBattery();
    m_present = state.present;
    m_capacity = state.capacity;
    m_state = state.status;

    const base::Locale& locale = base::Locale::system();
    m_title->setText(tr("Battery"));
    if (!m_present || m_capacity < 0) {
        m_percent->setText(tr("No battery"));
        m_status->setText(std::string());
    } else {
        m_percent->setText(locale.toString(m_capacity, 0) + "%");
        m_status->setText(statusText(m_state));
    }
    requestLayout();
    requestPaint();
}

void BatteryWidget::layoutContent()
{
    const ui::RectF content = contentRect();
    if (content.width <= 0 || content.height <= 0)
        return;
    const bool small = size() == WidgetSize::Small;
    const double titleHeight = std::max(14.0, m_title->sizeHint().height);
    const double statusHeight = std::max(14.0, m_status->sizeHint().height);
    const double barHeight = m_present ? 6 : 0;
    m_title->setFontSize(small ? 10 : 11);
    m_title->setGeometry({content.x, content.y, content.width, titleHeight});

    const double bottom = content.bottom();
    const double statusTop = bottom - statusHeight - (barHeight > 0 ? barHeight + 6 : 0);
    m_status->setGeometry({content.x, statusTop, content.width, statusHeight});
    m_levelBar = m_present ? ui::RectF {content.x, bottom - barHeight, content.width, barHeight}
                           : ui::RectF();

    m_percent->setFontSize(m_present ? (small ? 34.0 : size() == WidgetSize::Large ? 54.0 : 44.0) : 18.0);
    m_percent->setWeight(500);
    m_percent->setVerticallyCentered(true);
    m_title->setColor(widgetTheme(this).text.withAlpha(.65));
    m_status->setColor(widgetTheme(this).text.withAlpha(.65));
    const double percentTop = content.y + titleHeight + 2;
    m_percent->setGeometry({content.x, percentTop, content.width,
                            std::max(0.0, statusTop - percentTop - 4)});
}

void BatteryWidget::paintContent(ui::Painter& painter)
{
    if (!m_present || m_levelBar.width <= 0)
        return;
    const ui::Theme& theme = widgetTheme(this);
    const double level = std::clamp(m_capacity / 100.0, 0.0, 1.0);
    const bool low = m_capacity >= 0 && m_capacity <= 20 && m_state != "Charging";
    const ui::Color fill = low && !widgetMonochrome(this) ? ui::Color {226, 84, 72, 255} : theme.highlight;
    const ui::Color track = theme.text.withAlpha(0.14);

    painter.fillRoundedRect(m_levelBar, m_levelBar.height / 2, track);
    ui::RectF filled = m_levelBar;
    filled.width = std::max(0.0, m_levelBar.width * level);
    if (filled.width > 0)
        painter.fillRoundedRect(filled, filled.height / 2, fill);

    // The little terminal of a battery, so the bar reads as a charge.
    const ui::RectF cap {m_levelBar.right() + 2, m_levelBar.y + m_levelBar.height * 0.25, 2,
                         m_levelBar.height * 0.5};
    painter.fillRoundedRect(cap, 1, track);
}

void BatteryWidget::retranslateContent()
{
    m_title->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.58)));
    m_status->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.62)));
    refresh();
}

} // namespace fm
