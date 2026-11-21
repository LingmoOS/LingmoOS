// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/widgets/weatherwidget.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <mutex>
#include <numbers>
#include <optional>
#include <sstream>
#include <string>

#include <curl/curl.h>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/json.h"
#include "base/locale.h"
#include "base/settings.h"
#include "base/thread_pool.h"
#include "core/config.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/path.h"
#include "ui/theme.h"

using Lingmo::base::json::Value;
using Lingmo::i18n::format;
static const char* tr(const char* text) { return Lingmo::i18n::dtr("lingmo-filemanager", text); }

namespace fm {

namespace {

constexpr const char* kGeocoding = "https://geocoding-api.open-meteo.com/v1/search";
constexpr const char* kForecast = "https://api.open-meteo.com/v1/forecast";
constexpr auto kRefreshInterval = std::chrono::minutes(15);
constexpr auto kCacheExpiry = std::chrono::minutes(15);
constexpr size_t kMaxCacheEntries = 10;

// Weather cache: shared across all WeatherWidget instances
struct CachedWeather
{
    WeatherData data;
    std::chrono::steady_clock::time_point fetchTime;
};

static std::map<std::string, CachedWeather> s_weatherCache;
static std::mutex s_cacheMutex;

std::optional<WeatherData> getCachedWeather(const std::string& city)
{
    std::lock_guard lock(s_cacheMutex);
    auto it = s_weatherCache.find(city);
    if (it == s_weatherCache.end())
        return std::nullopt;

    const auto age = std::chrono::steady_clock::now() - it->second.fetchTime;
    if (age > kCacheExpiry) {
        s_weatherCache.erase(it);
        return std::nullopt;
    }
    return it->second.data;
}

void setCachedWeather(const std::string& city, const WeatherData& data)
{
    std::lock_guard lock(s_cacheMutex);
    // LRU eviction: remove oldest entry if cache is full
    if (s_weatherCache.size() >= kMaxCacheEntries) {
        auto oldest = std::min_element(s_weatherCache.begin(), s_weatherCache.end(),
            [](const auto& a, const auto& b) { return a.second.fetchTime < b.second.fetchTime; });
        s_weatherCache.erase(oldest);
    }
    s_weatherCache[city] = {data, std::chrono::steady_clock::now()};
}

struct HttpResponse
{
    bool ok = false;
    std::string body;
};

size_t appendBody(char* data, size_t size, size_t count, void* user)
{
    auto* body = static_cast<std::string*>(user);
    body->append(data, size * count);
    return size * count;
}

std::optional<std::string> urlEncode(CURL* curl, const std::string& value)
{
    char* escaped = curl_easy_escape(curl, value.c_str(), int(value.size()));
    if (!escaped)
        return std::nullopt;
    std::string result = escaped;
    curl_free(escaped);
    return result;
}

std::optional<HttpResponse> get(const std::string& url)
{
    CURL* curl = curl_easy_init();
    if (!curl)
        return std::nullopt;
    HttpResponse response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 8L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "lingmo-filemanager/5.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, appendBody);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
    const CURLcode result = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);
    if (result != CURLE_OK || status < 200 || status >= 300)
        return std::nullopt;
    response.ok = true;
    return response;
}

bool fetchWeather(const std::string& city, WeatherData& data)
{
    CURL* escape = curl_easy_init();
    if (!escape)
        return false;
    const std::optional<std::string> encoded = urlEncode(escape, city);
    curl_easy_cleanup(escape);
    if (!encoded)
        return false;

    const std::optional<HttpResponse> search = get(
            std::string(kGeocoding) + "?name=" + *encoded + "&count=1&format=json&language=en");
    if (!search || !search->ok)
        return false;
    const std::optional<Value> searchJson = Value::parse(search->body);
    if (!searchJson || !searchJson->isObject())
        return false;
    const Value& results = (*searchJson)["results"];
    if (!results.isArray() || results.size() == 0)
        return false;
    const Value& place = results[0];
    const double latitude = place["latitude"].toDouble();
    const double longitude = place["longitude"].toDouble();
    if (!std::isfinite(latitude) || !std::isfinite(longitude))
        return false;

    std::ostringstream url;
    url << kForecast << "?latitude=" << latitude << "&longitude=" << longitude
        << "&current=temperature_2m,relative_humidity_2m,weather_code"
        << "&daily=temperature_2m_max,temperature_2m_min"
        << "&hourly=temperature_2m,weather_code&forecast_days=1&timezone=auto";
    const std::optional<HttpResponse> forecast = get(url.str());
    if (!forecast || !forecast->ok)
        return false;
    const std::optional<Value> json = Value::parse(forecast->body);
    if (!json || !json->isObject())
        return false;
    const Value& current = (*json)["current"];
    if (!current.isObject())
        return false;

    data.valid = true;
    data.city = place["name"].toString(city);
    data.temperature = current["temperature_2m"].toDouble();
    data.humidity = int(current["relative_humidity_2m"].toDouble(-1));
    data.code = int(current["weather_code"].toDouble(-1));

    const Value& daily = (*json)["daily"];
    if (daily.isObject()) {
        const Value& highs = daily["temperature_2m_max"];
        const Value& lows = daily["temperature_2m_min"];
        if (highs.isArray() && highs.size() > 0)
            data.high = highs[0].toDouble(-1);
        if (lows.isArray() && lows.size() > 0)
            data.low = lows[0].toDouble(-1);
    }

    const Value& hourly = (*json)["hourly"];
    if (hourly.isObject() && hourly["time"].isArray()
        && hourly["temperature_2m"].isArray() && hourly["weather_code"].isArray()) {
        const Value& times = hourly["time"];
        const Value& temperatures = hourly["temperature_2m"];
        const Value& codes = hourly["weather_code"];
        const size_t count = std::min({times.size(), temperatures.size(), codes.size()});
        size_t first = 0;
        const std::string currentTime = current["time"].toString();
        for (size_t i = 0; i < count; ++i) {
            if (times[i].toString() >= currentTime) {
                first = i;
                break;
            }
            first = i;
        }
        const size_t end = std::min(count, first + 6);
        data.hourly.reserve(end - first);
        for (size_t i = first; i < end; ++i) {
            WeatherHour hour;
            hour.time = times[i].toString();
            hour.temperature = temperatures[i].toDouble();
            hour.code = int(codes[i].toDouble(-1));
            data.hourly.push_back(std::move(hour));
        }
    }
    return true;
}

std::string conditionText(int code)
{
    if (code == 0)
        return tr("Clear");
    if (code == 1)
        return tr("Mostly clear");
    if (code == 2)
        return tr("Partly cloudy");
    if (code == 3)
        return tr("Cloudy");
    if (code == 45 || code == 48)
        return tr("Fog");
    if (code >= 51 && code <= 57)
        return tr("Drizzle");
    if ((code >= 61 && code <= 67) || (code >= 80 && code <= 82))
        return tr("Rain");
    if ((code >= 71 && code <= 77) || code == 85 || code == 86)
        return tr("Snow");
    if (code >= 95)
        return tr("Thunderstorm");
    return tr("Weather");
}

bool isRain(int code)
{
    return (code >= 51 && code <= 67) || (code >= 80 && code <= 82) || code >= 95;
}

bool isSnow(int code)
{
    return (code >= 71 && code <= 77) || code == 85 || code == 86;
}

const char* weatherIconName(int code)
{
    if (code == 0)
        return "weather-clear";
    if (code <= 2)
        return "weather-few-clouds";
    if (code == 3)
        return "weather-overcast";
    if (code == 45 || code == 48)
        return "weather-fog";
    if (code >= 51 && code <= 55)
        return "weather-showers-scattered";
    if (code == 56 || code == 57)
        return "weather-freezing-rain";
    if (code >= 61 && code <= 65)
        return "weather-showers";
    if (code == 66 || code == 67)
        return "weather-freezing-rain";
    if (code >= 71 && code <= 75)
        return "weather-snow";
    if (code == 77)
        return "weather-snow-scattered";
    if (code >= 80 && code <= 82)
        return "weather-showers";
    if (code == 85 || code == 86)
        return "weather-snow";
    if (code >= 95 && code <= 99)
        return "weather-storm";
    return "weather-none-available";
}

std::string hourText(const std::string& isoTime)
{
    const size_t separator = isoTime.find('T');
    if (separator == std::string::npos || separator + 3 > isoTime.size())
        return {};
    int hour = 0;
    try {
        hour = std::stoi(isoTime.substr(separator + 1, 2));
    } catch (...) {
        return {};
    }
    if (hour < 0 || hour > 23)
        return {};
    base::ClockTime time;
    time.hour = hour;
    return base::Locale::system().toString(time, "h AP");
}

void drawWeatherGlyph(ui::Painter& painter, const ui::RectF& rect, int code, const ui::Theme& theme)
{
    const double size = std::min(rect.width, rect.height);
    const double cx = rect.x + rect.width / 2;
    const double cy = rect.y + rect.height / 2;
    const ui::Color ink = theme.text.withAlpha(0.62);
    const ui::Color accent = theme.highlight;
    if (code <= 2) {
        const double radius = size * 0.18;
        painter.fillEllipse({cx - radius, cy - radius, radius * 2, radius * 2}, accent);
        for (int i = 0; i < 8; ++i) {
            const double angle = i * std::numbers::pi / 4;
            painter.drawLine({cx + std::cos(angle) * radius * 1.45, cy + std::sin(angle) * radius * 1.45},
                             {cx + std::cos(angle) * radius * 1.95, cy + std::sin(angle) * radius * 1.95},
                             ink, std::max(1.0, size * 0.025));
        }
        if (code == 2)
            painter.fillRoundedRect({cx - size * 0.18, cy + size * 0.12, size * 0.42, size * 0.2},
                                    size * 0.1, theme.text.withAlpha(0.24));
        return;
    }
    painter.fillRoundedRect({cx - size * 0.28, cy + size * 0.02, size * 0.56, size * 0.22},
                            size * 0.11, theme.text.withAlpha(0.24));
    painter.fillRoundedRect({cx - size * 0.04, cy - size * 0.14, size * 0.3, size * 0.24},
                            size * 0.1, theme.text.withAlpha(0.24));
    if (isRain(code)) {
        for (int i = 0; i < 3; ++i)
            painter.drawLine({cx - size * 0.2 + i * size * 0.18, cy + size * 0.28},
                             {cx - size * 0.25 + i * size * 0.18, cy + size * 0.44}, accent,
                             std::max(1.0, size * 0.03));
    } else if (isSnow(code)) {
        for (int i = 0; i < 3; ++i)
            painter.fillEllipse({cx - size * 0.2 + i * size * 0.2, cy + size * 0.34, size * 0.06, size * 0.06},
                                accent);
    }
}

} // namespace

WeatherWidget::WeatherWidget(WidgetLayer& layer, WidgetSize size)
    : DesktopWidget(layer, WidgetKind::Weather, size)
{
    setName("weather-widget");
    m_city = addChild<ui::Label>();
    m_city->setFontSize(11);
    m_city->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.58)));
    m_city->setElide(ui::Label::Elide::Right);
    m_temperature = addChild<ui::Label>();
    m_temperature->setWeight(300);
    m_condition = addChild<ui::Label>();
    m_condition->setFontSize(11);
    m_condition->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.68)));
    m_condition->setElide(ui::Label::Elide::Right);
    m_humidity = addChild<ui::Label>();
    m_humidity->setFontSize(10);
    m_humidity->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.5)));
    m_highLow = addChild<ui::Label>();
    m_highLow->setFontSize(11);
    m_highLow->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.68)));
    m_highLow->setElide(ui::Label::Elide::Right);

    m_weatherIcon = addChild<ui::ThemeIcon>();
    m_weatherIcon->setVisible(false);

    for (size_t i = 0; i < m_hourTimes.size(); ++i) {
        m_hourTimes[i] = addChild<ui::Label>();
        m_hourTimes[i]->setFontSize(9);
        m_hourTimes[i]->setTextAlignment(ui::Label::TextAlignment::Center);
        m_hourTimes[i]->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.58)));
        m_hourTimes[i]->setVisible(false);
        m_hourIcons[i] = addChild<ui::ThemeIcon>();
        m_hourIcons[i]->setSize(22);
        m_hourIcons[i]->setVisible(false);
        m_hourTemperatures[i] = addChild<ui::Label>();
        m_hourTemperatures[i]->setFontSize(10);
        m_hourTemperatures[i]->setWeight(600);
        m_hourTemperatures[i]->setTextAlignment(ui::Label::TextAlignment::Center);
        m_hourTemperatures[i]->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.82)));
        m_hourTemperatures[i]->setVisible(false);
    }

    m_timer.setInterval(kRefreshInterval);
    m_timer.setSingleShot(false);
    m_timer.onTimeout = [this] { refresh(); };
    m_timer.start();
    refresh();
}

WeatherWidget::~WeatherWidget()
{
    *m_alive = false;
    m_timer.stop();
}

void WeatherWidget::refresh()
{
    const base::Settings settings(settingsFile());
    const std::string city = settings.stringValue("Weather/City");
    if (city.empty()) {
        m_requestedCity.clear();
        m_data = {};
        updateLabels();
        return;
    }

    // Check cache first: avoid network request if fresh data exists
    if (city == m_requestedCity) {
        const auto cached = getCachedWeather(city);
        if (cached) {
            m_data = *cached;
            m_lastFetch = std::chrono::steady_clock::now();
            updateLabels();
            return;
        }
    }

    const auto now = std::chrono::steady_clock::now();
    const bool stale = !m_data.valid || m_lastFetch.time_since_epoch().count() == 0
            || now - m_lastFetch >= kRefreshInterval;
    if (city != m_requestedCity || (stale && !m_fetching)) {
        m_requestedCity = city;
        if (!m_fetching)
            fetch(city);
        else
            updateLabels();
        return;
    }
    updateLabels();
}

void WeatherWidget::fetch(const std::string& city)
{
    base::EventLoop* loop = base::EventLoop::current();
    if (!loop)
        return;
    m_fetching = true;
    updateLabels();
    base::ThreadPool::global().post([this, alive = m_alive, loop, city] {
        WeatherData result;
        if (!fetchWeather(city, result))
            result.error = "unavailable";
        loop->post([this, alive, result, city] {
            if (!*alive)
                return;
            m_fetching = false;
            m_lastFetch = std::chrono::steady_clock::now();
            if (city != m_requestedCity) {
                // The city changed while this was in flight; the next refresh
                // fetches the new one.
                refresh();
                return;
            }
            if (result.valid)
                apply(result);
            else {
                m_data = {};
                m_data.error = result.error;
                updateLabels();
            }
        });
    });
}

void WeatherWidget::apply(const WeatherData& data)
{
    m_data = data;
    setCachedWeather(m_requestedCity, data);
    updateLabels();
}

void WeatherWidget::updateLabels()
{
    const bool valid = m_data.valid;
    m_city->setText(m_requestedCity.empty() ? tr("Weather") : m_requestedCity);
    m_temperature->setText(valid ? base::Locale::system().toString(std::lround(m_data.temperature), 0) + "°"
                                : "--");
    if (valid)
        m_condition->setText(conditionText(m_data.code));
    else if (m_requestedCity.empty())
        m_condition->setText(tr("Set a city in Settings"));
    else if (m_fetching)
        m_condition->setText(tr("Updating…"));
    else
        m_condition->setText(tr("Weather unavailable"));
    m_humidity->setText(valid && m_data.humidity >= 0
                                ? std::string(tr("Humidity")) + " "
                                        + base::Locale::system().toString(m_data.humidity, 0) + "%"
                                : std::string());
    m_humidity->setVisible(size() == WidgetSize::Small);
    if (valid && m_data.high >= 0 && m_data.low >= 0) {
        m_highLow->setText(format(tr("H:{high}° L:{low}°"),
                                   {{"high", base::Locale::system().toString(std::lround(m_data.high), 0)},
                                    {"low", base::Locale::system().toString(std::lround(m_data.low), 0)}}));
    } else {
        m_highLow->setText({});
    }

    for (size_t i = 0; i < m_hourTimes.size(); ++i) {
        const bool show = valid && i < m_data.hourly.size() && size() != WidgetSize::Small;
        m_hourTimes[i]->setVisible(show);
        m_hourIcons[i]->setVisible(show);
        m_hourTemperatures[i]->setVisible(show);
        if (show) {
            const WeatherHour& hour = m_data.hourly[i];
            m_hourTimes[i]->setText(hourText(hour.time));
            m_hourIcons[i]->setIconName(weatherIconName(hour.code));
            m_hourTemperatures[i]->setText(base::Locale::system().toString(std::lround(hour.temperature), 0) + "°");
        }
    }

    // Load weather icon via ThemeIcon (self-drawing View)
    if (m_weatherIcon) {
        if (valid) {
            const double iconSize = size() == WidgetSize::Small ? 36
                                   : size() == WidgetSize::Large ? 64 : 48;
            m_weatherIcon->setIconName(weatherIconName(m_data.code));
            m_weatherIcon->setSize(iconSize);
            m_weatherIcon->setVisible(size() != WidgetSize::Small);
        } else {
            m_weatherIcon->setVisible(false);
        }
    }

    requestLayout();
    requestPaint();
}

void WeatherWidget::layoutContent()
{
    const ui::RectF content = contentRect();
    if (content.width <= 0 || content.height <= 0)
        return;
    const bool small = size() == WidgetSize::Small;
    const bool large = size() == WidgetSize::Large;
    const double cityHeight = std::max(14.0, m_city->sizeHint().height);
    const double conditionHeight = std::max(14.0, m_condition->sizeHint().height);
    const double humidityHeight = std::max(12.0, m_humidity->sizeHint().height);
    const ui::Theme& theme = widgetTheme(this);
    m_humidity->setVisible(small);
    m_highLow->setVisible(!small);
    const double textWidth = small ? content.width : std::max(0.0, content.width * 0.52);
    m_city->setGeometry({content.x, content.y, textWidth, cityHeight});
    m_temperature->setFontSize(small ? 34 : large ? 54 : 44);
    m_temperature->setWeight(500);
    m_temperature->setVerticallyCentered(true);
    m_city->setWeight(600);
    m_condition->setColor(theme.text.withAlpha(.7));
    m_humidity->setColor(theme.text.withAlpha(.6));
    m_highLow->setColor(theme.text.withAlpha(.68));
    m_temperature->setGeometry({content.x, content.y + cityHeight + 2, textWidth,
                                small ? std::max(0.0, content.height - cityHeight - conditionHeight - humidityHeight - 8)
                                      : std::max(0.0, content.height * .42)});
    m_condition->setGeometry({content.x, content.bottom() - conditionHeight - humidityHeight - 2,
                              textWidth, conditionHeight});
    m_humidity->setGeometry({content.x, content.bottom() - humidityHeight, textWidth, humidityHeight});

    if (!small) {
        const double sideX = content.right() - std::max(96.0, content.width * .34);
        const double sideWidth = content.right() - sideX;
        const double iconSize = m_weatherIcon && m_weatherIcon->isVisible()
                ? m_weatherIcon->sizeHint().width : (large ? 58 : 46);
        if (m_weatherIcon && m_weatherIcon->isVisible())
            m_weatherIcon->setGeometry({content.right() - iconSize, content.y + 2, iconSize, iconSize});
        m_condition->setGeometry({sideX, content.y + iconSize + 2, sideWidth, conditionHeight});
        m_highLow->setGeometry({sideX, content.y + iconSize + conditionHeight + 2, sideWidth, 16});
        m_condition->setTextAlignment(ui::Label::TextAlignment::Right);
        m_highLow->setTextAlignment(ui::Label::TextAlignment::Right);

        const double rowTop = large ? content.y + content.height * .62 : content.bottom() - 54;
        const double rowHeight = std::max(42.0, content.bottom() - rowTop);
        const double cellWidth = content.width / double(m_hourTimes.size());
        for (size_t i = 0; i < m_hourTimes.size(); ++i) {
            const bool show = m_data.valid && i < m_data.hourly.size();
            m_hourTimes[i]->setVisible(show);
            m_hourIcons[i]->setVisible(show);
            m_hourTemperatures[i]->setVisible(show);
            if (show) {
                const WeatherHour& hour = m_data.hourly[i];
                m_hourTimes[i]->setText(hourText(hour.time));
                m_hourIcons[i]->setIconName(weatherIconName(hour.code));
                m_hourTemperatures[i]->setText(base::Locale::system().toString(std::lround(hour.temperature), 0) + "°");
            }
            const double x = content.x + cellWidth * double(i);
            m_hourTimes[i]->setGeometry({x, rowTop, cellWidth, 14});
            m_hourIcons[i]->setGeometry({x + (cellWidth - 22) / 2, rowTop + 15, 22, 22});
            m_hourTemperatures[i]->setGeometry({x, rowTop + rowHeight - 14, cellWidth, 14});
        }
    } else {
        m_condition->setTextAlignment(ui::Label::TextAlignment::Left);
        m_highLow->setGeometry({0, 0, 0, 0});
        m_humidity->setGeometry({0, 0, 0, 0});
        for (size_t i = 0; i < m_hourTimes.size(); ++i) {
            m_hourTimes[i]->setVisible(false);
            m_hourIcons[i]->setVisible(false);
            m_hourTemperatures[i]->setVisible(false);
        }
    }

    // Position the weather icon at the top-right of the content area for the compact card.
    if (m_weatherIcon && m_weatherIcon->isVisible()) {
        const double iconSize = m_weatherIcon->sizeHint().width;
        if (small)
            m_weatherIcon->setGeometry({content.right() - iconSize - 4, content.y + 4, iconSize, iconSize});
    }
}

void WeatherWidget::paintContent(ui::Painter& painter)
{
    if (size() == WidgetSize::Small)
        return;

    const ui::Theme& theme = widgetTheme(this);
    const ui::RectF content = contentRect();
    if (m_data.valid) {
        if (!m_weatherIcon || m_weatherIcon->path().empty()) {
            const double iconSize = std::min(64.0, content.width * 0.24);
            drawWeatherGlyph(painter, {content.right() - iconSize - 4, content.y + 4, iconSize, iconSize},
                             m_data.code, theme);
        }
        const double cellWidth = content.width / double(m_hourIcons.size());
        const double rowTop = size() == WidgetSize::Large ? content.y + content.height * .62
                                                           : content.bottom() - 54;
        for (size_t i = 0; i < m_hourIcons.size() && i < m_data.hourly.size(); ++i) {
            if (m_hourIcons[i]->path().empty()) {
                drawWeatherGlyph(painter,
                                 {content.x + cellWidth * double(i) + (cellWidth - 22) / 2,
                                  rowTop + 15, 22, 22},
                                 m_data.hourly[i].code, theme);
            }
        }
    }
}

void WeatherWidget::retranslateContent()
{
    m_city->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.58)));
    m_condition->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.68)));
    m_humidity->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.5)));
    m_highLow->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.68)));
    for (size_t i = 0; i < m_hourTimes.size(); ++i) {
        m_hourTimes[i]->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.58)));
        m_hourTemperatures[i]->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.82)));
    }
    refresh();
}

} // namespace fm
