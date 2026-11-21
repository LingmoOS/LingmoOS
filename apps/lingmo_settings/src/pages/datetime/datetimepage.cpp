// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/event_loop.h"
#include "base/i18n.h"
#include "lingmo/timedate/time_date.h"
#include "pages/datetime/time.h"
#include "pages/datetime/timezonedialog.h"
#include "pages/datetime/timezonemap.h"
#include "ui/cell_layout.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

namespace {

class DateTimePage : public Page
{
public:
    explicit DateTimePage(SettingsWindow& window)
        : Page(window, tr("Date & Time"))
        , m_time(m_timeDate)
        , m_map(m_timeDate)
    {
        ColumnLayout* section = content()->addChild<ColumnLayout>();
        section->setSpacing(metrics::sectionLabelSpacing);
        section->setLayoutHints({.fillWidth = true});

        Card* card = section->addChild<Card>(Card::Style::List);
        m_autoSync = card->addChild<SwitchRow>(tr("Auto sync"));
        // Greyed out, with the note below, when no NTP service is installed.
        m_autoSync->setEnabled(m_time.canNtp());
        m_autoSync->onToggled = [this](bool checked) { m_time.setUseNtp(checked); };
        m_twentyFour = card->addChild<SwitchRow>(tr("24-hour time"));
        m_twentyFour->onToggled = [this](bool checked) { m_time.setTwentyFour(checked); };
        m_timeZone = card->addChild<NavigationItem>(tr("Time zone"));
        m_timeZone->onClicked = [this] {
            if (m_dialog)
                return;
            m_dialog = std::make_unique<TimeZoneDialog>(settingsWindow(), m_map);
            m_dialog->onClosed = [this] {
                std::shared_ptr<TimeZoneDialog> closing(m_dialog.release());
                base::EventLoop::current()->post([closing] {});
            };
            m_dialog->show();
        };

        Footnote* note = section->addChild<Footnote>(
                tr("Unable to use auto sync. Please ensure your NTP service is installed."));
        note->setVisible(!m_time.canNtp());

        m_subscriptions.push_back(m_time.useNtpChanged.connect([this] { update(); }));
        m_subscriptions.push_back(m_time.twentyFourChanged.connect([this] { update(); }));
        m_subscriptions.push_back(m_map.currentTimeZoneChanged.connect([this] { update(); }));
        update();
    }

private:
    void update()
    {
        m_autoSync->setChecked(m_time.useNtp());
        m_twentyFour->setChecked(m_time.twentyFour());
        m_timeZone->setValue(m_map.currentTimeZone());
    }

    timedate::TimeDate m_timeDate;
    TimeSettings m_time;
    TimeZoneMap m_map;
    std::vector<base::Subscription> m_subscriptions;
    SwitchRow* m_autoSync;
    SwitchRow* m_twentyFour;
    NavigationItem* m_timeZone;
    std::unique_ptr<TimeZoneDialog> m_dialog;
};

} // namespace

static const PageRegistration registration("datetime", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<DateTimePage>(window);
});

} // namespace settings
