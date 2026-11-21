// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/widgets/countdownwidget.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <numbers>
#include <string>

#include "base/i18n.h"
#include "base/paths.h"
#include "base/settings.h"
#include "core/config.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/path.h"
#include "ui/theme.h"

static const char* tr(const char* text) { return Lingmo::i18n::dtr("lingmo-filemanager", text); }

namespace fm {

namespace {

// A short helper to format an integer with at least two digits, so the
// hours/minutes/seconds columns line up while they tick down.
std::string zeroPad(long value)
{
    char buffer[16];
    if (value < 10)
        std::snprintf(buffer, sizeof(buffer), "0%ld", value);
    else
        std::snprintf(buffer, sizeof(buffer), "%ld", value);
    return buffer;
}

} // namespace

CountdownWidget::CountdownWidget(WidgetLayer& layer, WidgetSize size)
    : DesktopWidget(layer, WidgetKind::Countdown, size)
{
    setName("countdown-widget");

    m_eventLabel = addChild<ui::Label>();
    m_eventLabel->setElide(ui::Label::Elide::Right);

    m_daysVal    = addChild<ui::Label>();
    m_hoursVal   = addChild<ui::Label>();
    m_minsVal    = addChild<ui::Label>();
    m_secsVal    = addChild<ui::Label>();
    for (ui::Label* v : {m_daysVal, m_hoursVal, m_minsVal, m_secsVal}) {
        v->setWeight(300);
        v->setColor(ui::ColorRecipe(widgetTheme(this).text));
    }

    m_daysUnit   = addChild<ui::Label>();
    m_hoursUnit  = addChild<ui::Label>();
    m_minsUnit   = addChild<ui::Label>();
    m_secsUnit   = addChild<ui::Label>();
    for (ui::Label* u : {m_daysUnit, m_hoursUnit, m_minsUnit, m_secsUnit}) {
        u->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.55)));
    }

    m_expired = addChild<ui::Label>();
    m_expired->setWeight(500);
    m_expired->setColor(ui::ColorRecipe(widgetTheme(this).highlight));

    loadConfig();

    m_timer.setInterval(std::chrono::seconds(1));
    m_timer.setSingleShot(false);
    m_timer.onTimeout = [this] { refresh(); };
    m_timer.start();

    // Set unit labels immediately so the first frame is not blank.
    // retranslateContent() will keep them in sync on language changes.
    m_daysUnit ->setText(tr("days"));
    m_hoursUnit->setText(tr("hours"));
    m_minsUnit ->setText(tr("min"));
    m_secsUnit ->setText(tr("sec"));

    refresh();
}

CountdownWidget::~CountdownWidget()
{
    m_timer.stop();
}

std::string CountdownWidget::settingsFile()
{
    return fm::settingsFile();
}

void CountdownWidget::loadConfig()
{
    const base::Settings settings(settingsFile());
    m_eventName = settings.stringValue("Countdown/Name");
    if (m_eventName.empty())
        m_eventName = tr("Event");

    m_eventLabel->setText(m_eventName);
    const std::string dateStr = settings.stringValue("Countdown/TargetDate");
    bool parsed = false;
    if (!dateStr.empty()) {
        std::tm tm = {};
        int y = 0, mo = 0, d = 0;
        if (std::sscanf(dateStr.c_str(), "%d-%d-%d", &y, &mo, &d) == 3) {
            tm.tm_year  = y - 1900;
            tm.tm_mon   = mo - 1;
            tm.tm_mday  = d;
            tm.tm_hour  = 0;
            tm.tm_min   = 0;
            tm.tm_sec   = 0;
            tm.tm_isdst = -1;
            const std::time_t t = std::mktime(&tm);
            if (t != std::time_t(-1)) {
                m_targetTime = t;
                parsed = true;
            }
        }
    }

    if (!parsed) {
        // Default: one year ahead from today.
        const std::time_t now = std::time(nullptr);
        std::tm future = {};
        localtime_r(&now, &future);
        future.tm_year  += 1;
        future.tm_isdst  = -1;
        m_targetTime = std::mktime(&future);
    }
}

void CountdownWidget::refresh()
{
    const std::time_t now  = std::time(nullptr);
    const std::time_t diff = m_targetTime - now;

    if (diff <= 0) {
        m_isExpired = true;
        m_expired->setText(tr("Event reached!"));
    } else {
        m_isExpired = false;
        const long days  =  diff / 86400L;
        const long hours = (diff % 86400L) / 3600L;
        const long mins  = (diff % 3600L)  / 60L;
        const long secs  =  diff % 60L;

        m_daysVal ->setText(std::to_string(days));
        m_hoursVal->setText(zeroPad(hours));
        m_minsVal ->setText(zeroPad(mins));
        m_secsVal ->setText(zeroPad(secs));
    }

    requestLayout();
    requestPaint();
}

void CountdownWidget::layoutContent()
{
    m_expired->setColor(widgetTheme(this).highlight);
    const ui::RectF content = contentRect();
    if (content.width <= 0 || content.height <= 0)
        return;

    // Hide everything first by collapsing to zero; each branch then shows what
    // it needs.
    const ui::RectF zero {0, 0, 0, 0};
    for (ui::Label* w : {m_eventLabel, m_daysVal, m_hoursVal, m_minsVal, m_secsVal,
                         m_daysUnit, m_hoursUnit, m_minsUnit, m_secsUnit, m_expired})
        w->setGeometry(zero);

    if (m_isExpired) {
        m_expired->setFontSize(size() == WidgetSize::Small ? 13 : 15);
        const double h = std::max(20.0, m_expired->sizeHint().height);
        m_expired->setGeometry({content.x, content.y + (content.height - h) / 2.0,
                                content.width, h});
        return;
    }

    if (size() == WidgetSize::Small) {
        // Event label at top (11 px).
        m_eventLabel->setFontSize(11);
        m_eventLabel->setWeight(400);
        const double evH = std::max(14.0, m_eventLabel->sizeHint().height);
        m_eventLabel->setGeometry({content.x, content.y, content.width, evH});

        // Days value: large, weight 300, centred between event label and unit.
        m_daysVal->setFontSize(42);
        const double unitH = 14.0;
        const double valH  = std::max(0.0, content.height - evH - 4 - unitH - 2);
        const double valY  = content.y + evH + 4;
        m_daysVal->setGeometry({content.x, valY, content.width, valH});

        // Unit label immediately below the value.
        m_daysUnit->setFontSize(11);
        m_daysUnit->setGeometry({content.x, valY + valH + 2, content.width, unitH});

    } else {
        // Medium and Large share the four-column strip layout.
        const double valSize  = (size() == WidgetSize::Large) ? 44.0 : 32.0;
        const double unitSize = 10.0;
        const double unitH    = 13.0;

        double stripY      = content.y;
        double stripHeight = content.height;

        {
            // Name the event at every size rather than presenting anonymous digits.
            m_eventLabel->setFontSize(14);
            m_eventLabel->setWeight(600);
            const double evH = std::max(18.0, m_eventLabel->sizeHint().height);
            m_eventLabel->setGeometry({content.x, content.y, content.width, evH});
            stripY      = content.y + evH + 8;
            stripHeight = content.height - evH - 8;
        }

        const double colW = content.width / 4.0;
        ui::Label* vals[]  = {m_daysVal,  m_hoursVal,  m_minsVal,  m_secsVal};
        ui::Label* units[] = {m_daysUnit, m_hoursUnit, m_minsUnit, m_secsUnit};

        for (int i = 0; i < 4; ++i) {
            const double cx = content.x + colW * i;
            vals[i]->setFontSize(valSize);
            vals[i]->setVerticallyCentered(true);
            vals[i]->setTextAlignment(ui::Label::TextAlignment::Center);
            units[i]->setTextAlignment(ui::Label::TextAlignment::Center);
            // Value label occupies the upper portion of the strip.
            const double valH = std::max(0.0, stripHeight - unitH - 4);
            vals[i]->setGeometry({cx, stripY, colW, valH});
            // Unit label sits below it.
            units[i]->setFontSize(unitSize);
            units[i]->setGeometry({cx, stripY + valH + 4, colW, unitH});
        }
    }
}

void CountdownWidget::paintContent(ui::Painter& painter)
{
    if (size() != WidgetSize::Large)
        return;
    if (m_isExpired || m_targetTime <= 0)
        return;

    const ui::Theme& theme   = widgetTheme(this);
    const ui::RectF  content = contentRect();

    // Fraction of the current 24-hour cycle that has elapsed, used as the
    // visual sweep so the arc ticks visibly as seconds pass.
    const std::time_t now  = std::time(nullptr);
    const std::time_t diff = m_targetTime - now;
    const double daySeconds = double(diff % 86400L);
    // elapsed within the day: from 0 (just ticked over) to ~1 (almost done)
    const double fraction = 1.0 - daySeconds / 86400.0;
    if (fraction <= 0.001)
        return;

    // Position: top-right corner of the content area.
    const double r  = 26.0;
    const double cx = content.right() - r - 6;
    const double cy = content.y + r + 6;

    // Subtle background ring.
    painter.fillEllipse({cx - r, cy - r, r * 2.0, r * 2.0},
                        theme.text.withAlpha(0.07));

    // Filled sector approximated with 20 quadratic Bezier segments.
    // Arc starts at 12 o'clock (−π/2) and sweeps clockwise.
    const double startAngle = -std::numbers::pi / 2.0;
    const double sweepAngle = fraction * 2.0 * std::numbers::pi;
    const int    steps      = 20;
    const double dtheta     = sweepAngle / steps;

    ui::Path path;
    path.moveTo(cx, cy);
    // First rim point.
    path.lineTo(cx + r * std::cos(startAngle),
                cy + r * std::sin(startAngle));
    for (int i = 0; i < steps; ++i) {
        const double t1   = startAngle + dtheta * i;
        const double t2   = t1 + dtheta;
        const double tmid = (t1 + t2) * 0.5;
        // Quadratic Bezier control point on the tangent intersection.
        const double rctrl = r / std::cos(dtheta * 0.5);
        path.quadTo(cx + rctrl * std::cos(tmid),
                    cy + rctrl * std::sin(tmid),
                    cx + r * std::cos(t2),
                    cy + r * std::sin(t2));
    }
    path.close();
    painter.fillPath(path, theme.highlight.withAlpha(0.7));
}

void CountdownWidget::retranslateContent()
{
    m_daysUnit ->setText(tr("days"));
    m_hoursUnit->setText(tr("hours"));
    m_minsUnit ->setText(tr("min"));
    m_secsUnit ->setText(tr("sec"));
    m_expired  ->setText(tr("Event reached!"));
    loadConfig();
    refresh();
}

} // namespace fm
