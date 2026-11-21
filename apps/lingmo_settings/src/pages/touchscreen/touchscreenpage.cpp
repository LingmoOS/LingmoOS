// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <string>
#include <vector>

#include "base/i18n.h"
#include "pages/input/availability.h"
#include "pages/input/compositorservice.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

namespace {

// The compositor's org.lingmo.Touchscreen1 service: the touch screens by
// name, and those turned off.
class Touchscreen
{
public:
    Touchscreen()
        : m_service("org.lingmo.Touchscreen1")
    {
    }

    bool isAvailable() const { return !devices().empty(); }
    std::vector<std::string> devices() const { return m_service.stringListValue("devices"); }
    bool isDeviceEnabled(const std::string& name) const
    {
        const std::vector<std::string> disabled = m_service.stringListValue("disabledDevices");
        return std::find(disabled.begin(), disabled.end(), name) == disabled.end();
    }
    void setDeviceEnabled(const std::string& name, bool enabled) { m_service.call("setDeviceEnabled", name, enabled); }

    base::Signal<>& changed() { return m_service.changed; }

private:
    CompositorService m_service;
};

class TouchscreenPage : public Page
{
public:
    explicit TouchscreenPage(SettingsWindow& window)
        : Page(window, tr("Touchscreen"))
    {
        m_card = content()->addChild<Card>(Card::Style::List);
        m_changed = m_touchscreen.changed().connect([this] { update(); });
        update();
    }

private:
    void update()
    {
        const std::vector<std::string> devices = m_touchscreen.devices();
        m_card->setVisible(!devices.empty());
        if (devices != m_devices) {
            while (!m_card->children().empty())
                m_card->takeChild(m_card->children().back().get());
            m_rows.clear();
            for (const std::string& name : devices) {
                SwitchRow* row = m_card->addChild<SwitchRow>(name);
                row->onToggled = [this, name](bool checked) { m_touchscreen.setDeviceEnabled(name, checked); };
                m_rows.push_back(row);
            }
            m_devices = devices;
        }
        for (size_t i = 0; i < m_rows.size(); ++i)
            m_rows[i]->setChecked(m_touchscreen.isDeviceEnabled(m_devices[i]));
    }

    Touchscreen m_touchscreen;
    base::Subscription m_changed;
    Card* m_card;
    std::vector<std::string> m_devices;
    std::vector<SwitchRow*> m_rows;
};

} // namespace

static const PageRegistration registration(
        "touchscreen",
        [](SettingsWindow& window) -> std::unique_ptr<Page> { return std::make_unique<TouchscreenPage>(window); },
        [](SettingsWindow&, AvailabilityCallback callback) {
            return answerOnce(Touchscreen().isAvailable(), std::move(callback));
        });

} // namespace settings
