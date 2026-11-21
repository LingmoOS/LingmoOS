// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "base/i18n.h"
#include "base/signal.h"
#include "lingmo/bluez/devices_model.h"
#include "lingmo/bluez/manager.h"
#include "pages/bluetooth/bluetoothdialogs.h"
#include "ui/button.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/cell_layout.h"
#include "ui/progress_bar.h"
#include "ui/switch.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/iconbutton.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo;
using namespace Lingmo::bluez;
using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace settings {

namespace {

// A label in the theme's secondary text colour.
class SecondaryLabel : public Label
{
public:
    using Label::Label;

protected:
    void layout() override
    {
        setColor(Theme::current().disabledText);
        Label::layout();
    }
};

// One Bluetooth device. Clicking pairs or connects; on a connected device, or
// one still pairing, the disconnect or cancel action shows on the right while
// the row is hovered, or after a tap where there is no hover.
class DeviceRow : public ListRow
{
public:
    DeviceRow(std::shared_ptr<BluetoothObject> device, std::function<void(std::shared_ptr<BluetoothObject>)> showDetails)
        : m_device(std::move(device))
    {
        m_icon = setLeading<ThemeIcon>("bluetooth", 24);
        m_icon->setImageProviderLookup(true);
        ColumnLayout* texts = addContent<ColumnLayout>();
        texts->setSpacing(metrics::textSpacing);
        texts->setLayoutHints({.fillWidth = true, .verticalAlignment = Alignment::Center});
        m_name = texts->addChild<Label>();
        m_name->setElide(Label::Elide::Right);
        m_name->setLayoutHints({.fillWidth = true});
        m_status = texts->addChild<SecondaryLabel>();
        m_status->setElide(Label::Elide::Right);
        m_status->setLayoutHints({.fillWidth = true});
        m_busy = addContent<BusyIndicator>();
        m_busy->setLayoutHints({.preferredWidth = 18, .preferredHeight = 18});
        m_action = addContent<Button>();
        m_action->onClicked = [this] {
            if (pairing())
                m_device->cancelPairing();
            else
                m_device->disconnectFromDevice();
        };
        m_info = addContent<IconButton>("info", 20);
        m_info->onClicked = [this, showDetails = std::move(showDetails)] { showDetails(m_device); };
        onHoveredChanged = [this] {
            if (!isHovered())
                m_revealed = false;
            update();
        };
        onClicked = [this] {
            if (connectable()) {
                if (m_device->paired())
                    m_device->connectToDevice();
                else
                    m_device->pair();
            } else {
                m_revealed = true;
                update();
            }
        };
        update();
    }

    const std::shared_ptr<BluetoothObject>& device() const { return m_device; }

    void update()
    {
        setName(m_device->path());
        m_icon->setIconName(m_device->icon().empty() ? "bluetooth" : m_device->icon());
        m_name->setText(m_device->name());
        std::string status = m_device->blocked()  ? tr("Blocked")
                             : linked()           ? tr("Connected")
                             : m_device->paired() ? tr("Not connected")
                                                  : "";
        if (linked() && m_device->batteryPercentage() >= 0)
            // TRANSLATORS: a device's connection and its battery level, such as "Connected · 80%".
            status = format(tr("{status} · {percent}%"),
                            {{"status", status}, {"percent", std::to_string(m_device->batteryPercentage())}});
        m_status->setText(status);
        m_status->setVisible(!status.empty());
        setMinimumRowHeight(status.empty() ? metrics::rowHeight : metrics::twoLineRowHeight);
        setHighlightOnPress(connectable());
        m_busy->setVisible(m_device->busy());
        // The busy indicator stands in for the action, except that pairing can be cancelled.
        m_action->setVisible(!m_device->blocked() && (pairing() || (linked() && !m_device->busy()))
                             && (isHovered() || m_revealed));
        m_action->setText(pairing() ? tr("Cancel pairing") : tr("Disconnect"));
        m_info->setVisible(m_device->paired());
    }

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        m_status->setFontSize(std::max(1.0, theme.fontSize() - 1));
        m_info->setColor(theme.highlight);
        ListRow::layout();
    }

private:
    // BlueZ reports Connected as soon as the link for pairing is up, before
    // authentication finishes.
    bool pairing() const { return m_device->busy() && !m_device->paired(); }
    bool linked() const { return m_device->connected() && !pairing(); }
    bool connectable() const { return !linked() && !pairing() && !m_device->busy() && !m_device->blocked(); }

    std::shared_ptr<BluetoothObject> m_device;
    ThemeIcon* m_icon;
    Label* m_name;
    Label* m_status;
    BusyIndicator* m_busy;
    Button* m_action;
    IconButton* m_info;
    bool m_revealed = false;
};

// A titled card of the devices in a model. Empty, it hides, or shows its
// placeholder in the card's place when one is set; while busy only the title
// and its indicator show.
class DeviceSection : public ColumnLayout
{
public:
    DeviceSection(std::string title, std::shared_ptr<Manager> manager, DevicesModel::Filter filter, bool namedOnly,
                  std::function<void(std::shared_ptr<BluetoothObject>)> showDetails)
        : m_model(std::move(manager))
        , m_showDetails(std::move(showDetails))
    {
        m_model.setFilter(filter);
        m_model.setNamedOnly(namedOnly);
        setSpacing(metrics::sectionLabelSpacing);
        setLayoutHints({.fillWidth = true});
        RowLayout* header = addChild<RowLayout>();
        header->setSpacing(0);
        header->setLayoutHints({.fillWidth = true});
        SectionLabel* label = header->addChild<SectionLabel>(std::move(title));
        label->setMargins({metrics::cardHorizontalPadding, 0, metrics::m, 0});
        label->setLayoutHints({});
        m_busy = header->addChild<BusyIndicator>();
        m_busy->setLayoutHints({.preferredWidth = 14, .preferredHeight = 14});
        m_busy->setDuration(std::chrono::milliseconds(1600));
        header->addChild<View>()->setLayoutHints({.fillWidth = true});
        m_placeholder = addChild<Footnote>();
        m_placeholder->setName("placeholder");
        m_card = addChild<Card>(Card::Style::List);
        m_subscriptions.push_back(m_model.rowsChanged.connect([this] {
            rebuild();
            update();
        }));
        m_subscriptions.push_back(m_model.dataChanged.connect([this] { update(); }));
        rebuild();
        update();
    }

    void setBusy(bool busy)
    {
        m_isBusy = busy;
        update();
    }

    void setPlaceholder(std::string text)
    {
        m_placeholderText = std::move(text);
        update();
    }

    void update()
    {
        for (DeviceRow* row : m_rows)
            row->update();
        const bool shown = !m_rows.empty();
        setVisible(shown || m_isBusy || !m_placeholderText.empty());
        m_busy->setVisible(m_isBusy);
        m_busy->setRunning(m_isBusy);
        m_placeholder->setText(m_placeholderText);
        m_placeholder->setVisible(!shown && !m_placeholderText.empty());
        m_card->setVisible(shown);
    }

private:
    void rebuild()
    {
        for (DeviceRow* row : m_rows)
            m_card->takeChild(row);
        m_rows.clear();
        for (const auto& device : m_model.devices())
            m_rows.push_back(m_card->addChild<DeviceRow>(device, m_showDetails));
    }

    DevicesModel m_model;
    std::function<void(std::shared_ptr<BluetoothObject>)> m_showDetails;
    BusyIndicator* m_busy;
    Footnote* m_placeholder;
    Card* m_card;
    std::vector<DeviceRow*> m_rows;
    bool m_isBusy = false;
    std::string m_placeholderText;
    std::vector<base::Subscription> m_subscriptions;
};

// An adapter in the first card: its name and whether others can find it.
class AdapterRow : public ListRow
{
public:
    explicit AdapterRow(std::shared_ptr<BluetoothObject> adapter)
        : m_adapter(std::move(adapter))
    {
        setName("adapter:" + m_adapter->path());
        m_state = addContent<SecondaryLabel>();
        m_info = addContent<IconButton>("info", 20);
        update();
    }

    const std::shared_ptr<BluetoothObject>& adapter() const { return m_adapter; }
    IconButton* info() const { return m_info; }

    void update()
    {
        setText(m_adapter->name());
        m_state->setText(m_adapter->discoverable() ? tr("Discoverable") : tr("Not discoverable"));
    }

protected:
    void layout() override
    {
        m_info->setColor(Theme::current().highlight);
        ListRow::layout();
    }

private:
    std::shared_ptr<BluetoothObject> m_adapter;
    Label* m_state;
    IconButton* m_info;
};

class BluetoothPage : public Page
{
public:
    explicit BluetoothPage(SettingsWindow& window)
        : Page(window, tr("Bluetooth"))
        , m_manager(Manager::shared())
    {
        ColumnLayout* top = content()->addChild<ColumnLayout>();
        top->setSpacing(metrics::sectionLabelSpacing);
        top->setLayoutHints({.fillWidth = true});
        m_firstCard = top->addChild<Card>(Card::Style::List);
        m_switch = m_firstCard->addChild<SwitchRow>(tr("Bluetooth"));
        m_switch->onToggled = [this](bool checked) { m_manager->setEnabled(checked); };
        m_message = top->addChild<SectionLabel>(std::string(), true);
        m_message->setName("bluetooth:message");

        m_devices = content()->addChild<ColumnLayout>();
        m_devices->setSpacing(metrics::sectionSpacing);
        m_devices->setLayoutHints({.fillWidth = true});
        const auto details = [this](std::shared_ptr<BluetoothObject> device) { showDeviceDetails(std::move(device)); };
        m_paired = m_devices->addChild<DeviceSection>(tr("My devices"), m_manager, DevicesModel::PairedDevices, false,
                                                      details);
        m_paired->setName("bluetooth:paired");
        m_other = m_devices->addChild<DeviceSection>(tr("Other devices"), m_manager, DevicesModel::UnpairedDevices,
                                                     true, details);
        m_other->setName("bluetooth:other");

        m_subscriptions.push_back(m_manager->changed.connect([this] { update(); }));
        m_subscriptions.push_back(m_manager->adaptersChanged.connect([this] {
            rebuildAdapters();
            update();
        }));
        // Pairing prompts come from the shell's agent, which BlueZ falls back to.
        m_subscriptions.push_back(m_manager->enabledChanged.connect(
            [this] { m_manager->setDiscoveryRequested(m_manager->enabled()); }));
        m_manager->setDiscoveryRequested(m_manager->enabled());
        rebuildAdapters();
        update();
    }

    ~BluetoothPage() override
    {
        m_subscriptions.clear();
        m_manager->setDiscoveryRequested(false);
    }

private:
    void rebuildAdapters()
    {
        for (AdapterRow* row : m_adapterRows)
            m_firstCard->takeChild(row);
        m_adapterRows.clear();
        for (const auto& adapter : m_manager->adapters()) {
            AdapterRow* row = m_firstCard->addChild<AdapterRow>(adapter);
            const std::weak_ptr<BluetoothObject> target = adapter;
            const auto open = [this, target] {
                if (const auto object = target.lock())
                    showAdapterDetails(object);
            };
            row->onClicked = open;
            row->info()->onClicked = open;
            m_adapterRows.push_back(row);
        }
    }

    void update()
    {
        const bool enabled = m_manager->enabled();
        m_switch->setChecked(enabled);
        m_switch->control()->setEnabled(m_manager->operational() && !m_manager->adapters().empty());
        for (AdapterRow* row : m_adapterRows) {
            row->update();
            row->setVisible(enabled);
        }
        const std::string message = !m_manager->errorText().empty() ? m_manager->errorText()
                                    : !m_manager->operational()     ? tr("Bluetooth service is unavailable")
                                    : m_manager->hardwareBlocked() ? tr("Bluetooth is blocked by a hardware switch")
                                    : m_manager->adapters().empty() ? tr("No Bluetooth adapters found")
                                                                    : "";
        m_message->setText(message);
        m_message->setVisible(!message.empty());
        m_devices->setVisible(enabled);
        const bool scanning = m_manager->scanning();
        m_other->setBusy(scanning);
        m_other->setPlaceholder(scanning ? "" : tr("No devices found. Make sure your device is discoverable."));
    }

    void showAdapterDetails(std::shared_ptr<BluetoothObject> adapter)
    {
        m_deviceDialog.reset();
        m_adapterDialog = std::make_unique<AdapterDetailsDialog>(&settingsWindow().window(), std::move(adapter));
        m_adapterDialog->show();
    }

    void showDeviceDetails(std::shared_ptr<BluetoothObject> device)
    {
        m_adapterDialog.reset();
        m_deviceDialog = std::make_unique<DeviceDetailsDialog>(&settingsWindow().window(), std::move(device));
        m_deviceDialog->show();
    }

    std::shared_ptr<Manager> m_manager;
    Card* m_firstCard;
    SwitchRow* m_switch;
    std::vector<AdapterRow*> m_adapterRows;
    SectionLabel* m_message;
    ColumnLayout* m_devices;
    DeviceSection* m_paired;
    DeviceSection* m_other;
    std::unique_ptr<AdapterDetailsDialog> m_adapterDialog;
    std::unique_ptr<DeviceDetailsDialog> m_deviceDialog;
    std::vector<base::Subscription> m_subscriptions;
};

// Reports whether there is an adapter, from BlueZ's first answer on.
class BluetoothAvailability : public AvailabilityWatch
{
public:
    explicit BluetoothAvailability(AvailabilityCallback callback)
        : m_manager(Manager::shared())
        , m_callback(std::move(callback))
    {
        m_subscriptions.push_back(m_manager->readyChanged.connect([this] { report(); }));
        m_subscriptions.push_back(m_manager->availableChanged.connect([this] { report(); }));
        report();
    }

private:
    void report()
    {
        if (m_manager->ready())
            m_callback(m_manager->available());
    }

    std::shared_ptr<Manager> m_manager;
    AvailabilityCallback m_callback;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace

static const PageRegistration registration(
    "bluetooth",
    [](SettingsWindow& window) -> std::unique_ptr<Page> { return std::make_unique<BluetoothPage>(window); },
    [](SettingsWindow&, AvailabilityCallback callback) -> std::unique_ptr<AvailabilityWatch> {
        return std::make_unique<BluetoothAvailability>(std::move(callback));
    });

} // namespace settings
