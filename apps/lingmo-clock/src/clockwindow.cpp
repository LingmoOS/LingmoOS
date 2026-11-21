// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "clockwindow.h"
#include "base/event_loop.h"
#include "base/i18n.h"
#include "clockcore.h"
#include "globalmenu.h"
#include "ui/application.h"
#include "ui/about_dialog.h"
#include "ui/button.h"
#include "ui/check_box.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/list_view.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/popup_glass.h"
#include "ui/scroll_view.h"
#include "ui/sidebar.h"
#include "ui/switch.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/view.h"
#include "ui/window_header.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <numbers>
#include <spawn.h>
#include <sstream>
#include <sys/wait.h>
#include <thread>
extern char** environ;
using namespace Lingmo::ui;
using Lingmo::I18n::tr;
namespace Clock {
namespace {
class Page;
std::string wallTime(const std::string& zone) {
    return localTime(zone, std::chrono::system_clock::now());
}
std::string civilDate() {
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm{};
    localtime_r(&now, &tm);
    std::ostringstream s;
    s << std::put_time(&tm, "%A, %B %d");
    return s.str();
}
std::string pageTitle(const std::string& id) {
    if (id == "world")
        return tr("World Clock");
    if (id == "alarms")
        return tr("Alarms");
    if (id == "stopwatch")
        return tr("Stopwatch");
    return tr("Timer");
}
std::string pageDescription(const std::string& id) {
    if (id == "world")
        return tr("Keep the local time of the places that matter to you.");
    if (id == "alarms")
        return tr("Set one-time or repeating alarms for your day.");
    if (id == "stopwatch")
        return tr("Measure elapsed time with precise split laps.");
    return tr("Run multiple countdowns independently.");
}

class AnalogClock final : public View {
  public:
    explicit AnalogClock(std::string zone) : m_zone(std::move(zone)) {}
    SizeF sizeHint() const override {
        return {72, 72};
    }

  protected:
    void paint(Painter& painter) override {
        const auto& theme = Theme::current();
        const double cx = geometry().width / 2, cy = geometry().height / 2, r = std::min(cx, cy) - 3;
        painter.fillEllipse({cx - r, cy - r, r * 2, r * 2}, theme.secondBackground);
        painter.strokeRoundedRect({cx - r, cy - r, r * 2, r * 2}, r, theme.text.withAlpha(.28), 1);
        const auto now = localTimeParts(m_zone, std::chrono::system_clock::now());
        for (int tick = 0; tick < 60; ++tick) {
            const double a = (tick * 6 - 90) * std::numbers::pi / 180.0;
            const double outer = r - 5;
            const double inner = outer - (tick % 5 == 0 ? 7 : 3);
            const Color color = theme.text.withAlpha(tick % 5 == 0 ? .8 : .32);
            painter.drawLine({cx + std::cos(a) * inner, cy + std::sin(a) * inner},
                             {cx + std::cos(a) * outer, cy + std::sin(a) * outer}, color, tick % 5 == 0 ? 1.5 : 1);
        }
        const double minute = (now[1] + now[2] / 60.0) * 6 - 90;
        const double hour = ((now[0] % 12) + now[1] / 60.0) * 30 - 90;
        auto hand = [&](double angle, double length, double width, Color color) {
            const double a = angle * std::numbers::pi / 180.0;
            painter.drawLine({cx, cy}, {cx + std::cos(a) * length, cy + std::sin(a) * length}, color, width);
        };
        hand(hour, r * .48, 3.2, theme.text);
        hand(minute, r * .72, 2, theme.highlight);
        painter.fillEllipse({cx - 2.5, cy - 2.5, 5, 5}, theme.highlight);
    }

  private:
    std::string m_zone;
};

class TimerProgress final : public View {
  public:
    explicit TimerProgress(double progress) : m_progress(std::clamp(progress, 0.0, 1.0)) {}
    void setProgress(double progress) {
        const double next = std::clamp(progress, 0.0, 1.0);
        if (next == m_progress)
            return;
        m_progress = next;
        requestPaint();
    }
    SizeF sizeHint() const override {
        return {64, 64};
    }

  protected:
    void paint(Painter& painter) override {
        const auto& theme = Theme::current();
        const double cx = 32, cy = 32;
        for (int i = 0; i < 60; ++i) {
            const double a = (i * 6 - 90) * std::numbers::pi / 180.0;
            const bool active = i < m_progress * 60;
            painter.drawLine({cx + std::cos(a) * 25, cy + std::sin(a) * 25},
                             {cx + std::cos(a) * 29, cy + std::sin(a) * 29},
                             active ? theme.highlight : theme.text.withAlpha(.18), 2);
        }
    }

  private:
    double m_progress;
};

class ClockRoot final : public View {
  public:
    ClockRoot(ClockWindow& owner, Store& store, std::unique_ptr<GlobalMenu>& menu)
        : m_owner(owner), m_store(store), m_menu(menu) {
        setName("clock-root");
        m_sidebar = addChild<SidebarList>(std::vector<SidebarList::Entry>{{"world", tr("World Clock"),
                                                                            "clock-app-symbolic"},
                                                                          {"alarms", tr("Alarms"), "appointment-new"},
                                                                          {"stopwatch", tr("Stopwatch"), "chronometer"},
                                                                          {"timer", tr("Timer"), "appointment-new"}});
        m_sidebar->onActivated = [this](const std::string& id) { showPage(id); };
        m_header = addChild<WindowHeader>(46);
        m_title = m_header->setContent<Label>(std::string());
        m_title->setVerticallyCentered(true);
        m_title->setFontSize(18);
        m_appTitle = addChild<Label>(tr("Clock"));
        m_appTitle->setVerticallyCentered(true);
        m_appTitle->setFontSize(18);
        m_pages = addChild<View>();
        m_pages->setName("clock-page-host");
        m_tick = Lingmo::ui::Application::instance()->loop().startTimer(std::chrono::milliseconds(100), true,
                                                                        [this] { refresh(); });
        m_menuAvailability = m_menu->availabilityChanged.connect([this](bool) { refreshMenu(); });
        refreshMenu();
        showPage("world");
    }
    ~ClockRoot() override {
        if (m_tick)
            Lingmo::ui::Application::instance()->loop().stopTimer(m_tick);
    }
    void setMenu(GlobalMenu* menu) {
        m_global = menu;
        refreshMenu();
    }
    void openPage(const std::string& id) {
        showPage(id);
    }

  protected:
    void layout() override;
    void paint(Painter& p) override {
        const auto& t = Theme::current();
        p.fillRect({220, 0, std::max(0.0, geometry().width - 220), geometry().height}, t.background);
        const RectF side{0, 0, std::min(220.0, geometry().width), geometry().height};
        p.fillRect(side, t.secondBackground.withAlpha(0.48));
        p.fillRect({side.width, 0, 1, geometry().height}, t.text.withAlpha(.08));
    }

  private:
    void refreshMenu() {
        if (!m_global)
            return;
        requestLayout();
    }
    void buildMenu() {
        m_localMenu.clear();
        auto* clock = m_localMenu.addSubmenu(tr("Clock"));
        clock->addItem(tr("World Clock"), [this] { showPage("world"); });
        clock->addItem(tr("Alarms"), [this] { showPage("alarms"); });
        clock->addItem(tr("Stopwatch"), [this] { showPage("stopwatch"); });
        clock->addItem(tr("Timer"), [this] { showPage("timer"); });
        clock->addSeparator();
        clock->addItem(tr("Quit"), [] { Application::instance()->quit(); });
        auto* help = m_localMenu.addSubmenu(tr("Help"));
        help->addItem(tr("About Clock"), [this] { m_owner.showAbout(); });
    }
    void showPage(const std::string& id);
    void refresh();
    ClockWindow& m_owner;
    Store& m_store;
    std::unique_ptr<GlobalMenu>& m_menu;
    GlobalMenu* m_global = nullptr;
    SidebarList* m_sidebar = nullptr;
    WindowHeader* m_header = nullptr;
    Label* m_title = nullptr;
    Label* m_appTitle = nullptr;
    View* m_pages = nullptr;
    Page* m_page = nullptr;
    std::map<std::string, Page*> m_pageById;
    Menu m_localMenu;
    int m_tick = 0;
    std::string m_current = "world";
    Lingmo::base::Subscription m_menuAvailability;
};

class Page final : public View {
  public:
    Page(ClockRoot& root, Store& store, std::string id) : m_root(root), m_store(store), m_id(std::move(id)) {
        setName("page:" + m_id);
        m_scroll = addChild<ScrollView>();
        m_content = m_scroll->setContent<Column>();
        m_content->setMargins({28, 26, 28, 24});
        m_content->setSpacing(16);
        m_date = m_content->addChild<Label>(civilDate());
        m_date->setFontSize(14);
        m_date->setColor(ColorRecipe{ColorId::DisabledText});
        m_heading = m_content->addChild<Label>(pageTitle(m_id));
        m_heading->setFontSize(26);
        m_heading->setWeight(600);
        m_description = m_content->addChild<Label>(pageDescription(m_id));
        m_description->setFontSize(14);
        m_description->setColor(ColorRecipe{ColorId::DisabledText});
        m_body = m_content->addChild<Column>();
        m_body->setSpacing(14);
        build();
    }
    void refresh() {
        if (m_date)
            m_date->setText(civilDate());
        if (m_id == "world")
            refreshWorld();
        if (m_id == "stopwatch")
            refreshStopwatch();
        if (m_id == "timer")
            refreshTimers();
    }

  protected:
    void layout() override {
        m_scroll->setGeometry(localRect());
    }

  private:
    Button* action(std::string label, std::function<void()> cb) {
        auto* b = m_body->addChild<Button>(std::move(label));
        b->onClicked = std::move(cb);
        return b;
    }
    Label* text(std::string value, double size = 16) {
        auto* l = m_body->addChild<Label>(std::move(value));
        l->setFontSize(size);
        return l;
    }
    void build() {
        if (m_id == "world")
            buildWorld();
        else if (m_id == "alarms")
            buildAlarms();
        else if (m_id == "stopwatch")
            buildStopwatch();
        else
            buildTimers();
    }
    void buildWorld() {
        auto* searchRow = m_body->addChild<Row>();
        searchRow->setSpacing(10);
        m_search = searchRow->addChild<TextField>();
        m_search->setName("city-search");
        m_search->setPlaceholder(tr("Search city or time zone, e.g. Tokyo"));
        m_search->onTextChanged = [this] { refreshZoneMatches(); };
        searchRow->addStretch();
        auto* add = searchRow->addChild<Button>(tr("Add city"));
        add->setName("add-city");
        add->onClicked = [this] {
            const auto matches = searchTimeZones(m_search->text());
            if (!matches.empty())
                addZone(matches.front());
        };
        m_matches = m_body->addChild<Column>();
        m_matches->setSpacing(2);
        m_world = m_body->addChild<Column>();
        m_world->setSpacing(8);
        if (m_store.worldClocks().empty()) {
            m_store.addWorldClock({"local", tr("Local time"), ""});
            m_store.addWorldClock({"New_York", tr("New York"), "America/New_York"});
            m_store.addWorldClock({"Tokyo", tr("Tokyo"), "Asia/Tokyo"});
        }
        refreshWorld();
    }
    void addZone(const ZoneMatch& match) {
        for (const auto& clock : m_store.worldClocks()) {
            if (clock.zone == match.zone)
                return;
        }
        m_store.addWorldClock({match.zone, match.city, match.zone});
        m_search->clear();
        refreshZoneMatches();
        refreshWorld();
    }
    void refreshZoneMatches() {
        if (!m_matches)
            return;
        while (!m_matches->children().empty())
            m_matches->takeChild(m_matches->children().front().get());
        for (const auto& match : searchTimeZones(m_search->text())) {
            auto* choice = m_matches->addChild<Button>(match.city + "   " + match.zone);
            choice->onClicked = [this, match] { addZone(match); };
        }
    }
    void refreshWorld() {
        if (!m_world)
            return;
        const auto clocks = m_store.worldClocks();
        if (clocks.size() == m_worldTimes.size()) {
            const auto now = std::chrono::system_clock::now();
            for (size_t i = 0; i < clocks.size(); ++i) {
                m_worldTimes[i]->setText(wallTime(clocks[i].zone));
                m_worldDiffs[i]->setText(clocks[i].zone + " · " + utcOffset(clocks[i].zone, now) + " · " +
                                         timeDifference(clocks[i].zone, now));
                m_worldFaces[i]->requestPaint();
            }
            return;
        }
        while (!m_world->children().empty())
            m_world->takeChild(m_world->children().front().get());
        m_worldTimes.clear();
        m_worldDiffs.clear();
        m_worldFaces.clear();
        for (const auto& c : clocks) {
            auto* row = m_world->addChild<Row>();
            row->setSpacing(14);
            auto* face = row->addChild<AnalogClock>(c.zone);
            m_worldFaces.push_back(face);
            auto* names = row->addChild<Column>();
            names->setSpacing(2);
            names->setLayoutHints({.preferredWidth = 300});
            auto* city = names->addChild<Label>(c.city);
            city->setFontSize(18);
            city->setElide(Label::Elide::Right);
            auto* diff = names->addChild<Label>(c.zone + " · " + utcOffset(c.zone, std::chrono::system_clock::now()) +
                                                " · " + timeDifference(c.zone, std::chrono::system_clock::now()));
            diff->setFontSize(12);
            diff->setElide(Label::Elide::Right);
            diff->setColor(ColorRecipe{ColorId::DisabledText});
            m_worldDiffs.push_back(diff);
            auto* time = row->addChild<Label>(wallTime(c.zone));
            time->setFontSize(30);
            time->setFontFeatures({{"tnum", 1}});
            time->setLayoutHints({.preferredWidth = 122, .horizontalAlignment = Alignment::End});
            m_worldTimes.push_back(time);
            auto* remove = row->addChild<Button>("×");
            remove->setLayoutHints({.preferredWidth = 48, .preferredHeight = 40});
            remove->setToolTip(tr("Remove city"));
            remove->onClicked = [this, id = c.id] {
                m_store.removeWorldClock(id);
                refreshWorld();
            };
        }
    }
    void buildAlarms() {
        auto* fieldLabels = m_body->addChild<Row>();
        fieldLabels->setSpacing(10);
        auto* nameLabel = fieldLabels->addChild<Label>(tr("Name"));
        nameLabel->setFontSize(12);
        nameLabel->setColor(ColorRecipe{ColorId::DisabledText});
        fieldLabels->addStretch();
        auto* soundLabel = fieldLabels->addChild<Label>(tr("Sound"));
        soundLabel->setFontSize(12);
        soundLabel->setColor(ColorRecipe{ColorId::DisabledText});
        auto* details = m_body->addChild<Row>();
        details->setSpacing(10);
        m_alarmName = details->addChild<TextField>(tr("Alarm"));
        m_alarmName->setName("alarm-name");
        details->addStretch();
        m_alarmSound = details->addChild<TextField>();
        m_alarmSound->setName("alarm-sound");
        m_alarmSound->setPlaceholder(tr("Sound file path (optional)"));
        auto* timeHint = m_body->addChild<Label>(tr("Choose a time and repeat days, or leave all days off for a one-time alarm."));
        timeHint->setFontSize(12);
        timeHint->setColor(ColorRecipe{ColorId::DisabledText});
        auto* timeLabels = m_body->addChild<Row>();
        timeLabels->setSpacing(8);
        auto* hourLabel = timeLabels->addChild<Label>(tr("Hour"));
        hourLabel->setFontSize(12);
        hourLabel->setColor(ColorRecipe{ColorId::DisabledText});
        auto* minuteLabel = timeLabels->addChild<Label>(tr("Minute"));
        minuteLabel->setFontSize(12);
        minuteLabel->setColor(ColorRecipe{ColorId::DisabledText});
        auto* clockInput = m_body->addChild<Row>();
        clockInput->setSpacing(8);
        m_alarmHour = clockInput->addChild<TextField>("07");
        m_alarmHour->setName("alarm-hour");
        m_alarmHour->setPlaceholder(tr("Hours"));
        m_alarmHour->setMaximumLength(2);
        m_alarmHour->setValidator([](const std::string& v) {
            return v.empty() ||
                   (v.size() <= 2 && std::all_of(v.begin(), v.end(), [](unsigned char c) { return std::isdigit(c); }) &&
                    std::stoi(v) <= 23);
        });
        m_alarmMinute = clockInput->addChild<TextField>("00");
        m_alarmMinute->setName("alarm-minute");
        m_alarmMinute->setPlaceholder(tr("Minutes"));
        m_alarmMinute->setMaximumLength(2);
        m_alarmMinute->setValidator([](const std::string& v) {
            return v.empty() ||
                   (v.size() <= 2 && std::all_of(v.begin(), v.end(), [](unsigned char c) { return std::isdigit(c); }) &&
                    std::stoi(v) <= 59);
        });
        auto* days = m_body->addChild<Row>();
        days->setSpacing(8);
        const std::array<const char*, 7> labels = {tr("Mon"), tr("Tue"), tr("Wed"), tr("Thu"),
                                                   tr("Fri"), tr("Sat"), tr("Sun")};
        for (int i = 0; i < 7; ++i) {
            auto* day = days->addChild<CheckBox>(labels[size_t(i)]);
            day->setChecked(false);
            m_alarmDays.push_back(day);
        }
        auto* add = action(tr("Add alarm"), [this] {
            int hour = 7, minute = 0;
            try {
                hour = std::stoi(m_alarmHour->text());
                minute = std::stoi(m_alarmMinute->text());
            } catch (...) {
                return;
            }
            uint8_t repeat = 0;
            for (int i = 0; i < 7; ++i)
                if (m_alarmDays[size_t(i)]->isChecked())
                    repeat |= uint8_t(1u << i);
            Alarm a;
            a.name = m_alarmName->text().empty() ? tr("Alarm") : m_alarmName->text();
            a.hour = hour;
            a.minute = minute;
            a.weekdays = repeat;
            a.recurring = repeat != 0;
            a.sound = m_alarmSound->text();
            const auto now = std::chrono::system_clock::now();
            if (a.recurring)
                a.nextOccurrenceUtc = nextAlarmOccurrenceUtc(a.hour, a.minute, a.weekdays, now);
            else {
                a.weekdays = 0x7f;
                a.nextOccurrenceUtc = nextAlarmOccurrenceUtc(a.hour, a.minute, 0x7f, now);
            }
            m_store.saveAlarm(a);
            reschedule();
            refreshAlarms();
        });
        add->setName("add-alarm");
        auto* scheduled = m_body->addChild<Label>(tr("Scheduled alarms"));
        scheduled->setFontSize(15);
        scheduled->setWeight(600);
        m_alarms = m_body->addChild<Column>();
        m_alarms->setSpacing(8);
        refreshAlarms();
    }
    void refreshAlarms() {
        if (!m_alarms)
            return;
        while (!m_alarms->children().empty())
            m_alarms->takeChild(m_alarms->children().front().get());
        auto alarms = m_store.alarms();
        if (alarms.empty()) {
            auto* empty = m_alarms->addChild<Label>(tr("No alarms yet. Add one above."));
            empty->setColor(ColorRecipe{ColorId::DisabledText});
        }
        for (const auto& a : alarms) {
            auto* row = m_alarms->addChild<Row>();
            row->setSpacing(12);
            const std::string timeText = (a.hour < 10 ? "0" : "") + std::to_string(a.hour) + ":" +
                                         (a.minute < 10 ? "0" : "") + std::to_string(a.minute);
            auto* time = row->addChild<Label>(timeText);
            time->setFontSize(26);
            time->setFontFeatures({{"tnum", 1}});
            const std::string kind = a.recurring ? std::string(tr("Repeats")) : std::string(tr("Once"));
            auto* name = row->addChild<Label>(a.name + " · " + kind);
            name->setFontSize(14);
            if (a.recurring) {
                static const std::array<const char*, 7> weekdays = {tr("Mon"), tr("Tue"), tr("Wed"), tr("Thu"),
                                                                    tr("Fri"), tr("Sat"), tr("Sun")};
                std::string days;
                for (size_t day = 0; day < weekdays.size(); ++day) {
                    if (a.weekdays & (1u << day)) {
                        if (!days.empty())
                            days += "  ";
                        days += weekdays[day];
                    }
                }
                auto* repeat = row->addChild<Label>(days);
                repeat->setFontSize(12);
                repeat->setColor(ColorRecipe{ColorId::DisabledText});
            }
            row->addStretch();
            auto* enabled = row->addChild<Switch>();
            enabled->setChecked(a.enabled);
            enabled->setToolTip(tr("Enable alarm"));
            enabled->onToggled = [this, id = a.id](bool on) {
                m_store.setAlarmEnabled(id, on);
                reschedule();
            };
            auto* remove = row->addChild<Button>("×");
            remove->setToolTip(tr("Delete alarm"));
            remove->onClicked = [this, id = a.id] {
                m_store.removeAlarm(id);
                reschedule();
                refreshAlarms();
            };
        }
    }
    void buildStopwatch() {
        m_stopwatchDisplay = text("00:00:00.00", 56);
        m_stopwatchDisplay->setFontFeatures({{"tnum", 1}});
        m_stopwatchDisplay->setName("stopwatch-display");
        auto* controls = m_body->addChild<Row>();
        controls->setSpacing(10);
        auto* toggle = controls->addChild<Button>(tr("Start"));
        toggle->setName("stopwatch-toggle");
        toggle->onClicked = [this, toggle] {
            if (m_stopwatch.running()) {
                m_stopwatch.pause();
                toggle->setText(tr("Resume"));
            } else {
                m_stopwatch.start();
                toggle->setText(tr("Pause"));
            }
        };
        auto* lap = controls->addChild<Button>(tr("Lap"));
        lap->setName("stopwatch-lap");
        lap->onClicked = [this] {
            auto cumulative = m_stopwatch.lap();
            auto split = cumulative - m_lastLap;
            m_lastLap = cumulative;
            auto* l = m_laps->addChild<Label>(std::string(tr("Lap")) + " " + std::to_string(m_stopwatch.laps().size()) +
                                              "    " + formatDuration(split) + "    " + tr("Total") + " " +
                                              formatDuration(cumulative));
            l->setFontFeatures({{"tnum", 1}});
        };
        auto* reset = controls->addChild<Button>(tr("Reset"));
        reset->onClicked = [this, toggle] {
            m_stopwatch.reset();
            m_lastLap = {};
            toggle->setText(tr("Start"));
            while (!m_laps->children().empty())
                m_laps->takeChild(m_laps->children().front().get());
        };
        auto* lapTitle = m_body->addChild<Label>(tr("Lap history"));
        lapTitle->setFontSize(15);
        lapTitle->setWeight(600);
        m_laps = m_body->addChild<Column>();
        m_laps->setSpacing(4);
    }
    void refreshStopwatch() {
        if (m_stopwatchDisplay)
            m_stopwatchDisplay->setText(formatDuration(m_stopwatch.elapsed()));
    }
    void buildTimers() {
        auto* duration = m_body->addChild<Row>();
        duration->setSpacing(10);
        m_hours = duration->addChild<TextField>("0");
        m_hours->setName("timer-hours");
        m_hours->setPlaceholder(tr("Hours"));
        m_hours->setMaximumLength(3);
        m_hours->setValidator([](const std::string& v) {
            return v.empty() || (v.size() <= 3 && std::all_of(v.begin(), v.end(), [](unsigned char c) { return std::isdigit(c); }) &&
                                 std::stoi(v) <= 99);
        });
        m_minutes = duration->addChild<TextField>("5");
        m_minutes->setName("timer-minutes");
        m_minutes->setPlaceholder(tr("Minutes"));
        m_minutes->setMaximumLength(2);
        m_minutes->setValidator([](const std::string& v) {
            return v.empty() || (v.size() <= 2 && std::all_of(v.begin(), v.end(), [](unsigned char c) { return std::isdigit(c); }) &&
                                 std::stoi(v) <= 59);
        });
        m_seconds = duration->addChild<TextField>("0");
        m_seconds->setName("timer-seconds");
        m_seconds->setPlaceholder(tr("Seconds"));
        m_seconds->setMaximumLength(2);
        m_seconds->setValidator([](const std::string& v) {
            return v.empty() || (v.size() <= 2 && std::all_of(v.begin(), v.end(), [](unsigned char c) { return std::isdigit(c); }) &&
                                 std::stoi(v) <= 59);
        });
        duration->addStretch();
        auto* add = duration->addChild<Button>(tr("Add timer"));
        add->setName("add-timer");
        add->onClicked = [this] {
            auto parse = [](const std::string& s) -> int64_t {
                try {
                    return static_cast<int64_t>(std::stoll(s));
                } catch (...) {
                    return 0;
                }
            };
            auto h = parse(m_hours->text()), mi = parse(m_minutes->text()), s = parse(m_seconds->text());
            auto ns = std::chrono::seconds(std::max<int64_t>(0, h * 3600 + mi * 60 + s));
            if (ns.count() == 0)
                return;
            const int64_t id = ++m_timerId;
            const auto total = std::chrono::duration_cast<std::chrono::nanoseconds>(ns);
            m_runningTimers[id] = {std::chrono::steady_clock::now(), total, total, true, m_timerSound->text()};
            refreshTimers();
        };
        m_timerSound = m_body->addChild<TextField>();
        m_timerSound->setName("timer-sound");
        m_timerSound->setPlaceholder(tr("Sound file path (optional)"));
        auto* activeTitle = m_body->addChild<Label>(tr("Active timers"));
        activeTitle->setFontSize(15);
        activeTitle->setWeight(600);
        m_timers = m_body->addChild<Column>();
        m_timers->setSpacing(8);
        m_timersEmpty = m_timers->addChild<Label>(tr("No active timers."));
        m_timersEmpty->setColor(ColorRecipe{ColorId::DisabledText});
        refreshTimers();
    }
    void refreshTimers() {
        if (!m_timers)
            return;
        for (auto it = m_runningTimers.begin(); it != m_runningTimers.end();) {
            auto& [id, t] = *it;
            auto remaining = t.remaining;
            if (t.running)
                remaining -= std::chrono::steady_clock::now() - t.started;
            if (remaining.count() <= 0) {
                const auto sound = t.sound;
                if (t.row)
                    m_timers->takeChild(t.row);
                it = m_runningTimers.erase(it);
                notify(tr("Timer finished"), sound);
                continue;
            }
            const double progress =
                1.0 - double(std::max<int64_t>(0, remaining.count())) / std::max<int64_t>(1, t.total.count());
            if (!t.row) {
                if (m_timersEmpty) {
                    m_timers->takeChild(m_timersEmpty);
                    m_timersEmpty = nullptr;
                }
                t.row = m_timers->addChild<Row>();
                t.row->setSpacing(10);
                t.progressView = t.row->addChild<TimerProgress>(progress);
                t.timeLabel = t.row->addChild<Label>();
                t.timeLabel->setFontSize(28);
                t.timeLabel->setFontFeatures({{"tnum", 1}});
                t.row->addStretch();
                t.toggle = t.row->addChild<Button>();
                t.toggle->onClicked = [this, id] {
                    auto& timer = m_runningTimers.at(id);
                    if (timer.running) {
                        timer.remaining -= std::chrono::steady_clock::now() - timer.started;
                        timer.running = false;
                    } else {
                        timer.started = std::chrono::steady_clock::now();
                        timer.running = true;
                    }
                    refreshTimers();
                };
                auto* reset = t.row->addChild<Button>(tr("Reset"));
                reset->onClicked = [this, id] {
                    auto& timer = m_runningTimers.at(id);
                    timer.remaining = timer.total;
                    timer.running = false;
                    refreshTimers();
                };
                auto* remove = t.row->addChild<Button>("×");
                remove->setToolTip(tr("Remove timer"));
                remove->onClicked = [this, id] {
                    auto found = m_runningTimers.find(id);
                    if (found != m_runningTimers.end()) {
                        if (found->second.row)
                            m_timers->takeChild(found->second.row);
                        m_runningTimers.erase(found);
                    }
                };
            }
            t.timeLabel->setText(formatDuration(remaining, false));
            t.progressView->setProgress(progress);
            t.toggle->setText(t.running ? tr("Pause") : tr("Resume"));
            ++it;
        }
    }
    void reschedule() {
        if (!Lingmo::ui::Application::instance()->sessionBus())
            return;
        auto* bus = Lingmo::ui::Application::instance()->sessionBus();
        auto call = bus->newMethodCall("org.lingmo.Clock", "/org/lingmo/Clock", "org.lingmo.Clock", "Reschedule");
        bus->send(call);
    }
    void notify(const std::string& title, const std::string& sound = {}) {
        auto* bus = Application::instance()->sessionBus();
        if (bus) {
            auto call = bus->newMethodCall("org.freedesktop.Notifications", "/org/freedesktop/Notifications",
                                           "org.freedesktop.Notifications", "Notify");
            call.append(std::string("Lingmo Clock"), uint32_t(0), std::string("clock"), title, std::string(),
                        std::vector<std::string>{}, std::map<std::string, Lingmo::base::dbus::Variant>{}, int32_t(0));
            bus->send(call);
        }
        const std::string file = sound.empty() ? "/usr/share/sounds/freedesktop/stereo/alarm-clock-elapsed.oga" : sound;
        pid_t pid = 0;
        char* argv[] = {const_cast<char*>("paplay"), const_cast<char*>(file.c_str()), nullptr};
        if (posix_spawnp(&pid, "paplay", nullptr, nullptr, argv, environ) == 0)
            std::thread([pid] {
                int status = 0;
                while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
                }
            }).detach();
    }
    struct RunningTimer {
        std::chrono::steady_clock::time_point started;
        std::chrono::nanoseconds total;
        std::chrono::nanoseconds remaining;
        bool running;
        std::string sound;
        Row* row = nullptr;
        TimerProgress* progressView = nullptr;
        Label* timeLabel = nullptr;
        Button* toggle = nullptr;
    };
    ClockRoot& m_root;
    Store& m_store;
    std::string m_id;
    ScrollView* m_scroll = nullptr;
    Column* m_content = nullptr;
    Label *m_date = nullptr, *m_heading = nullptr;
    Label* m_description = nullptr;
    Column* m_body = nullptr;
    TextField* m_search = nullptr;
    TextField *m_alarmName = nullptr, *m_alarmSound = nullptr, *m_alarmHour = nullptr, *m_alarmMinute = nullptr;
    TextField *m_hours = nullptr, *m_minutes = nullptr, *m_seconds = nullptr, *m_timerSound = nullptr;
    std::vector<CheckBox*> m_alarmDays;
    std::vector<Label*> m_worldTimes;
    std::vector<Label*> m_worldDiffs;
    std::vector<AnalogClock*> m_worldFaces;
    Column *m_world = nullptr, *m_matches = nullptr, *m_alarms = nullptr, *m_laps = nullptr, *m_timers = nullptr;
    Label *m_stopwatchDisplay = nullptr, *m_timersEmpty = nullptr;
    Stopwatch m_stopwatch;
    std::chrono::nanoseconds m_lastLap{};
    int64_t m_timerId = 0;
    std::map<int64_t, RunningTimer> m_runningTimers;
};
void ClockRoot::layout() {
    const double side = 220;
    m_appTitle->setGeometry({20, 0, side - 40, 46});
    m_sidebar->setGeometry({0, 46, side, std::max(0.0, geometry().height - 46)});
    m_header->setGeometry({0, 0, geometry().width, 46});
    m_pages->setGeometry({side, 46, std::max(0.0, geometry().width - side), std::max(0.0, geometry().height - 46)});
    if (Window* surface = window())
        surface->setBackgroundBlur({0, 0, geometry().width, geometry().height}, Theme::current().windowRadius);
    for (const auto& [id, page] : m_pageById) {
        (void)id;
        page->setGeometry(m_pages->localRect());
    }
}
void ClockRoot::showPage(const std::string& id) {
    m_current = id;
    m_sidebar->setCurrent(id);
    auto found = m_pageById.find(id);
    if (found == m_pageById.end()) {
        auto* page = m_pages->addChild<Page>(*this, m_store, id);
        page->setGeometry(m_pages->localRect());
        m_pageById[id] = page;
        m_pages->requestLayout();
        found = m_pageById.find(id);
    }
    for (const auto& [pageId, page] : m_pageById)
        page->setVisible(pageId == id);
    m_page = found->second;
    buildMenu();
}
void ClockRoot::refresh() {
    for (const auto& [id, page] : m_pageById) {
        (void)id;
        page->refresh();
    }
}
} // namespace

ClockWindow::ClockWindow(std::string storePath) : m_store(std::move(storePath)), m_menu(), m_window() {
    m_window.setTitle(tr("Clock"));
    m_window.setRestoresSize("clock");
    m_window.setMinimumSize({740, 540});
    m_window.resize({1040, 700});
    m_about = std::make_unique<AboutDialog>(&m_window);
    m_about->setName(tr("Clock"));
    m_about->setIconSource("lingmo-clock");
    m_about->setDescription(tr("World clock, alarms, stopwatch and timers."));
    m_menu = std::make_unique<GlobalMenu>(Application::instance()->sessionBus(), [this](int32_t id) {
        if (id == 8) {
            showAbout();
            return;
        }
        if (id == 6) {
            Application::instance()->quit();
            return;
        }
        if (m_root) {
            const char* page = id == 2 ? "world" : id == 3 ? "alarms" : id == 4 ? "stopwatch" : "timer";
            auto* root = dynamic_cast<ClockRoot*>(m_root);
            if (root)
                root->openPage(page);
        }
    });
    auto* root = new ClockRoot(*this, m_store, m_menu);
    root->setMenu(m_menu.get());
    m_root = root;
    const auto& theme = Theme::current();
    const bool glass = m_window.backgroundGlassAvailable();
    m_window.setBackgroundColor(glass ? Color{0, 0, 0, 0} : theme.background);
    if (glass) {
        m_window.setBackgroundGlass(popupGlass(theme.darkMode));
        m_window.setBackgroundBlur({0, 0, 1040, 700}, theme.windowRadius);
    } else {
        m_window.setBackgroundGlass(std::nullopt);
        m_window.setBackgroundBlur({}, 0);
    }
    m_window.setContentView(std::unique_ptr<View>(root));
}
ClockWindow::~ClockWindow() = default;
void ClockWindow::showAbout() { m_about->show(); }
void ClockWindow::show() {
    m_window.show();
}
void ClockWindow::showPage(const std::string& id) {
    (void)id;
}
} // namespace Clock
