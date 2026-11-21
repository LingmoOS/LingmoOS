// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "base/timer.h"
#include "desktop/widgets/widget.h"

namespace Lingmo::ui {
class Label;
class ThemeIcon;
}

namespace fm {

struct WeatherHour
{
    std::string time;
    double temperature = 0;
    int code = -1;
};

struct WeatherData
{
    bool valid = false;
    std::string city;
    double temperature = 0;
    int humidity = -1;
    int code = -1;
    double high = -1;
    double low = -1;
    std::vector<WeatherHour> hourly;
    std::string error;
};

// The weather in the city named in filemanager.conf's Weather/City, fetched
// from Open-Meteo on a worker thread and refreshed slowly.
class WeatherWidget : public DesktopWidget
{
public:
    WeatherWidget(WidgetLayer& layer, WidgetSize size);
    ~WeatherWidget() override;

protected:
    void layoutContent() override;
    void paintContent(ui::Painter& painter) override;
    void retranslateContent() override;

private:
    void refresh();
    void fetch(const std::string& city);
    void apply(const WeatherData& data);
    void updateLabels();

    ui::Label* m_city = nullptr;
    ui::Label* m_temperature = nullptr;
    ui::Label* m_condition = nullptr;
    ui::Label* m_humidity = nullptr;
    ui::Label* m_highLow = nullptr;
    std::array<ui::Label*, 6> m_hourTimes {};
    std::array<ui::Label*, 6> m_hourTemperatures {};
    std::array<ui::ThemeIcon*, 6> m_hourIcons {};
    base::Timer m_timer;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
    std::string m_requestedCity;
    WeatherData m_data;
    bool m_fetching = false;
    std::chrono::steady_clock::time_point m_lastFetch {};
    ui::ThemeIcon* m_weatherIcon = nullptr;
};

} // namespace fm
