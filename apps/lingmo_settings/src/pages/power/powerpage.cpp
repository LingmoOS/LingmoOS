// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "base/i18n.h"
#include "base/signal.h"
#include "pages/power/powersettings.h"
#include "lingmo/powerprofiles/power_profiles.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "lingmo/upower/battery_status.h"
#include "widgets/card.h"
#include "widgets/imageselectionitem.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/picture.h"
#include "widgets/rowcombobox.h"
#include "widgets/sections.h"
#include "window/options.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;
using Lingmo::i18n::trn;
using Lingmo::i18n::trNoop;

namespace settings {

namespace {

const std::vector<int> kTimeouts{60, 120, 300, 600, 900, 1200, 1800, -1};

std::vector<std::string> timeoutNames()
{
    std::vector<std::string> names;
    for (const int timeout : kTimeouts)
        names.push_back(timeout < 0 ? tr("Never") : trn("{n} minute", "{n} minutes", uint64_t(timeout / 60)));
    return names;
}

int timeoutToIndex(int timeout)
{
    auto it = std::find(kTimeouts.begin(), kTimeouts.end(), timeout);
    if (it == kTimeouts.end())
        it = std::find(kTimeouts.begin(), kTimeouts.end(), 300);
    return int(it - kTimeouts.begin());
}

struct ProfileTile
{
    const char* profile;
    const char* picture;
    const char* name;
};

constexpr ProfileTile kProfileTiles[] = {
    {"power-saver", "images/powersave.svg", trNoop("Power save")},
    {"balanced", "images/balanced.svg", trNoop("Balanced")},
    {"performance", "images/performance.svg", trNoop("Performance")},
};

class PowerPage : public Page
{
public:
    explicit PowerPage(SettingsWindow& window)
        : Page(window, tr("Power"))
    {
        // Mode.
        m_mode = content()->addChild<Section>(tr("Mode"));
        m_mode->setName("power:mode");
        Card* modes = m_mode->addChild<Card>();
        RowLayout* tiles = modes->addChild<RowLayout>();
        tiles->setSpacing(metrics::tileSpacing);
        for (const ProfileTile& tile : kProfileTiles) {
            ImageSelectionItem* item = tiles->addChild<ImageSelectionItem>(dataPath(tile.picture), tr(tile.name),
                                                                           SizeF{80, 80});
            const std::string profile = tile.profile;
            item->onClicked = [this, profile] { m_profiles.setActiveProfile(profile); };
            m_tiles.push_back(item);
        }
        m_description = m_mode->addChild<Footnote>();
        m_description->setName("power:description");
        m_degraded = m_mode->addChild<Footnote>();
        m_degraded->setName("power:degraded");

        // Display and sleep.
        Card* idle = content()->addChild<Card>(Card::Style::List);
        m_batteryRow = idle->addChild<ListRow>(tr("Turn off display when inactive on battery"));
        m_batteryRow->setName("power:batteryscreenoff");
        m_batteryRow->setInteractive(false);
        RowComboBox* batteryScreenOff = m_batteryRow->addContent<RowComboBox>(timeoutNames());
        batteryScreenOff->setCurrentIndex(timeoutToIndex(m_settings.batteryScreenOff()));
        batteryScreenOff->onActivated = [this](int index) { m_settings.setBatteryScreenOff(kTimeouts[index]); };

        m_acRow = idle->addChild<ListRow>();
        m_acRow->setName("power:acscreenoff");
        m_acRow->setInteractive(false);
        RowComboBox* acScreenOff = m_acRow->addContent<RowComboBox>(timeoutNames());
        acScreenOff->setCurrentIndex(timeoutToIndex(m_settings.acScreenOff()));
        acScreenOff->onActivated = [this](int index) { m_settings.setACScreenOff(kTimeouts[index]); };

        m_preventSleep = idle->addChild<SwitchRow>();
        m_preventSleep->setName("power:preventsleep");
        m_preventSleep->setChecked(m_settings.acPreventSleep());
        m_preventSleep->onToggled = [this](bool checked) { m_settings.setACPreventSleep(checked); };

        if (!options().mockBattery) {
            m_batteryStatus = upower::BatteryStatus::shared();
            m_subscriptions.push_back(m_batteryStatus->availableChanged.connect([this] { updateBattery(); }));
        }

        m_subscriptions.push_back(m_profiles.profilesChanged.connect([this] { updateProfiles(); }));
        m_subscriptions.push_back(m_profiles.activeProfileChanged.connect([this] { updateProfiles(); }));
        m_subscriptions.push_back(m_profiles.performanceDegradedChanged.connect([this] { updateProfiles(); }));
        updateProfiles();
        updateBattery();
    }

private:
    bool hasBattery() const { return options().mockBattery || (m_batteryStatus && m_batteryStatus->isAvailable()); }

    void updateProfiles()
    {
        const std::vector<std::string>& profiles = m_profiles.profiles();
        const std::string& active = m_profiles.activeProfile();
        m_mode->setVisible(!profiles.empty());
        for (size_t i = 0; i < m_tiles.size(); ++i) {
            const std::string profile = kProfileTiles[i].profile;
            m_tiles[i]->setVisible(std::find(profiles.begin(), profiles.end(), profile) != profiles.end());
            m_tiles[i]->setChecked(active == profile);
        }

        std::string description;
        if (active == "power-saver")
            description = tr("Reduces performance and power consumption.");
        else if (active == "balanced")
            description = tr("Standard performance and power consumption.");
        else if (active == "performance")
            description = tr("Higher performance, with more power consumption and heat.");
        m_description->setText(description);
        m_description->setVisible(!description.empty());

        const std::string& degraded = m_profiles.performanceDegraded();
        if (degraded.find("lap-detected") != std::string::npos)
            m_degraded->setText(tr("Performance is limited because the computer is on your lap."));
        else if (degraded.find("high-operating-temperature") != std::string::npos)
            m_degraded->setText(tr("Performance is limited because the computer is too hot."));
        else
            m_degraded->setText(tr("Performance is currently limited."));
        m_degraded->setVisible(active == "performance" && !degraded.empty());
    }

    void updateBattery()
    {
        const bool battery = hasBattery();
        m_batteryRow->setVisible(battery);
        m_acRow->setText(battery ? tr("Turn off display when inactive on power adapter")
                                 : tr("Turn off display when inactive"));
        m_preventSleep->setText(battery ? tr("Prevent automatic sleeping on power adapter when the display is off")
                                        : tr("Prevent automatic sleeping when the display is off"));
    }

    powerprofiles::PowerProfiles m_profiles;
    PowerSettings m_settings;
    std::shared_ptr<upower::BatteryStatus> m_batteryStatus;
    std::vector<base::Subscription> m_subscriptions;

    Section* m_mode;
    std::vector<ImageSelectionItem*> m_tiles;
    Footnote* m_description;
    Footnote* m_degraded;
    ListRow* m_batteryRow;
    ListRow* m_acRow;
    SwitchRow* m_preventSleep;
};

} // namespace

static const PageRegistration registration("power", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<PowerPage>(window);
});

} // namespace settings
