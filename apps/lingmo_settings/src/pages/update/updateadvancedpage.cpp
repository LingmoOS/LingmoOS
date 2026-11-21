// SPDX-FileCopyrightText: 2026 Lingmo OS Team
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <cstdio>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "base/i18n.h"
#include "base/signal.h"
#include "lingmo/update/update_status.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/rowcombobox.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo;
using Lingmo::i18n::tr;
using Lingmo::i18n::trNoop;

namespace settings {

namespace {

struct Frequency
{
    int mode;
    int minutes;
    const char* name;
};

constexpr Frequency kFrequencies[] = {
    {0, 60, trNoop("Every hour")},
    {0, 180, trNoop("Every 3 hours")},
    {0, 360, trNoop("Every 6 hours")},
    {0, 720, trNoop("Every 12 hours")},
    {1, 1440, trNoop("Daily")},
};

std::vector<std::string> frequencyNames()
{
    std::vector<std::string> names;
    for (const Frequency& frequency : kFrequencies)
        names.push_back(tr(frequency.name));
    return names;
}

std::vector<std::string> timeNames()
{
    std::vector<std::string> names;
    for (int hour = 0; hour < 24; ++hour) {
        for (int minute = 0; minute < 60; minute += 30) {
            char text[6];
            std::snprintf(text, sizeof(text), "%02d:%02d", hour, minute);
            names.push_back(text);
        }
    }
    return names;
}

int timeIndex(int hour, int minute)
{
    return std::clamp(hour, 0, 23) * 2 + (minute >= 30 ? 1 : 0);
}

int frequencyIndex(const update::UpdateStatus& status)
{
    for (size_t i = 0; i < std::size(kFrequencies); ++i) {
        if (kFrequencies[i].mode != status.checkMode())
            continue;
        if (status.checkMode() == 1 || kFrequencies[i].minutes == status.checkInterval())
            return int(i);
    }
    return 2;
}

class UpdateAdvancedPage : public Page
{
public:
    explicit UpdateAdvancedPage(SettingsWindow& window)
        : Page(window, tr("Advanced Settings"))
    {
        m_status = update::UpdateStatus::shared();

        Section* section = content()->addChild<Section>(tr("Automatic checks"));
        section->setName("update-advanced:section");
        Card* card = section->addChild<Card>(Card::Style::List);

        m_autoCheck = card->addChild<SwitchRow>(tr("Automatically check for updates"));
        m_autoCheck->setName("update-advanced:auto-check");
        m_autoCheck->onToggled = [this](bool checked) { m_status->setAutoCheck(checked); };

        m_frequencyRow = card->addChild<ListRow>(tr("Check frequency"));
        m_frequencyRow->setName("update-advanced:frequency-row");
        m_frequencyRow->setInteractive(false);
        m_frequency = m_frequencyRow->addContent<RowComboBox>(frequencyNames());
        m_frequency->setName("update-advanced:frequency");
        m_frequency->onActivated = [this](int index) {
            const Frequency& frequency = kFrequencies[index];
            m_status->setCheckSchedule(frequency.mode, frequency.minutes);
        };

        m_timeRow = card->addChild<ListRow>(tr("Check time"));
        m_timeRow->setName("update-advanced:time-row");
        m_timeRow->setInteractive(false);
        m_time = m_timeRow->addContent<RowComboBox>(timeNames());
        m_time->setName("update-advanced:time");
        m_time->onActivated = [this](int index) {
            m_status->setCheckTime(index / 2, (index % 2) * 30);
        };

        content()->addChild<Footnote>(
                tr("Checks run in the background. When updates are available, a notification tells you."));

        m_subscriptions.push_back(m_status->changed.connect([this] { refresh(); }));
        refresh();
    }

private:
    void refresh()
    {
        const bool available = m_status->isAvailable();
        const bool pending = m_status->isRequestPending();
        const bool enabled = available && !pending && m_status->autoCheck();

        m_autoCheck->setEnabled(available && !pending);
        m_autoCheck->setChecked(m_status->autoCheck());

        m_frequencyRow->setEnabled(enabled);
        m_frequency->setEnabled(enabled);
        m_frequency->setCurrentIndex(frequencyIndex(*m_status));

        // The time of day matters only for a daily check.
        m_timeRow->setVisible(m_status->checkMode() == 1);
        m_timeRow->setEnabled(enabled);
        m_time->setEnabled(enabled);
        m_time->setCurrentIndex(timeIndex(m_status->checkHour(), m_status->checkMinute()));
    }

    std::shared_ptr<update::UpdateStatus> m_status;
    SwitchRow* m_autoCheck;
    ListRow* m_frequencyRow;
    RowComboBox* m_frequency;
    ListRow* m_timeRow;
    RowComboBox* m_time;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace

static const PageRegistration registration("update-advanced", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<UpdateAdvancedPage>(window);
});

} // namespace settings
