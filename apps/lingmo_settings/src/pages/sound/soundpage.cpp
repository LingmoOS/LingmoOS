// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/i18n.h"
#include "base/dbus.h"
#include "lingmo/audio/audio_model.h"
#include "lingmo/audio/backend.h"
#include "lingmo/audio/volume_monitor.h"
#include "base/signal.h"
#include "ui/button.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/cell_layout.h"
#include "ui/segmented_control.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/sliderrow.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace Lingmo;
using namespace Lingmo::Audio;
using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trc;

namespace settings {

// What kind of device a row is, as people tell them apart.
static std::string deviceType(const AudioObject& device)
{
    if (device.isVirtualDevice())
        return tr("Virtual");
    const std::string bus = device.bus();
    if (bus == "bluetooth")
        return tr("Bluetooth");
    if (bus == "usb")
        return "USB";
    const std::vector<AudioObject::Choice> ports = device.ports();
    const int active = device.activePortIndex();
    if (active >= 0 && active < int(ports.size()) && ports[active].name.starts_with("hdmi"))
        return "HDMI";
    return tr("Built-in");
}

static bool hasFrontPair(const AudioObject& device)
{
    const std::vector<std::string> channels = device.channels();
    return std::find(channels.begin(), channels.end(), "FL") != channels.end()
           && std::find(channels.begin(), channels.end(), "FR") != channels.end();
}

// -1 is all left, 1 all right. Only a device with both front channels has one.
static double balance(const AudioObject& device)
{
    const std::vector<std::string> channels = device.channels();
    const auto left = std::find(channels.begin(), channels.end(), "FL");
    const auto right = std::find(channels.begin(), channels.end(), "FR");
    const std::vector<double> volumes = device.channelVolumes();
    if (left == channels.end() || right == channels.end() || volumes.size() != channels.size())
        return 0;
    const double leftVolume = volumes[left - channels.begin()];
    const double rightVolume = volumes[right - channels.begin()];
    const double high = std::max(leftVolume, rightVolume);
    if (high <= 0)
        return 0;
    return leftVolume >= rightVolume ? rightVolume / high - 1 : 1 - leftVolume / high;
}

static void setBalance(AudioObject& device, double value)
{
    const std::vector<std::string> channels = device.channels();
    std::vector<double> volumes = device.channelVolumes();
    const double volume = device.volume();
    for (size_t i = 0; i < channels.size() && i < volumes.size(); ++i) {
        if (channels[i] == "FL")
            volumes[i] = volume * std::min(1.0, 1 - value);
        else if (channels[i] == "FR")
            volumes[i] = volume * std::min(1.0, 1 + value);
    }
    device.setChannelVolumes(volumes);
}

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

// A sink or source: its name, its type and a check mark on the default one.
class DeviceRow : public ListRow
{
public:
    explicit DeviceRow(std::shared_ptr<AudioObject> device)
        : m_device(std::move(device))
    {
        setName("device-" + std::to_string(m_device->index()));
        m_type = addContent<SecondaryLabel>();
        m_check = addContent<TrailingIcon>("check", 18);
        update();
    }

    const std::shared_ptr<AudioObject>& device() const { return m_device; }

    void update()
    {
        setText(m_device->displayName());
        m_type->setText(deviceType(*m_device));
        m_check->icon()->setVisible(m_device->isDefault());
    }

protected:
    void layout() override
    {
        m_check->icon()->setColor(Theme::current().highlight);
        ListRow::layout();
    }

private:
    std::shared_ptr<AudioObject> m_device;
    Label* m_type;
    TrailingIcon* m_check;
};

class SoundPage : public Page
{
public:
    explicit SoundPage(SettingsWindow& window)
        : Page(window, tr("Sound"))
        , m_backend(Backend::shared())
        , m_outputs(AudioObject::Sink, m_backend)
        , m_inputs(AudioObject::Source, m_backend)
        , m_monitor(m_backend)
    {
        auto* eventCard = content()->addChild<Card>(Card::Style::List);
        m_eventSounds = eventCard->addChild<SwitchRow>(tr("System event sounds"));
        m_eventSounds->setName("event-sounds");
        m_eventSounds->setEnabled(false);
        m_eventSounds->onToggled = [this](bool enabled) {
            m_soundChange = m_sounds.callAsync("setEnabled", [this](const base::dbus::Reply&) {
                refreshSounds();
            }, enabled);
        };
        m_previewSound = eventCard->addChild<Button>(tr("Test sound"));
        m_previewSound->setEnabled(false);
        m_previewSound->onClicked = [this] { m_sounds.send("playEvent", std::string("message-new-instant")); };
        m_soundProperties = m_sounds.onPropertiesChanged(
                [this](const base::dbus::VariantMap&, const std::vector<std::string>&) { refreshSounds(); });
        if (auto* bus = base::dbus::Connection::session()) {
            m_soundOwner = bus->watchName("org.lingmo.Services", [this](const std::string& owner) {
                m_soundRead = {};
                m_eventSounds->setEnabled(false);
                m_previewSound->setEnabled(false);
                if (!owner.empty())
                    refreshSounds();
            });
        }

        m_connecting = content()->addChild<Card>();
        Label* connecting = m_connecting->addChild<Label>(tr("Connecting to the audio service…"));
        connecting->setWrap(true);
        connecting->setLayoutHints({.fillWidth = true});

        m_errorCard = content()->addChild<Card>();
        m_error = m_errorCard->addChild<Label>();
        m_error->setWrap(true);
        m_error->setLayoutHints({.fillWidth = true});
        Button* dismiss = m_errorCard->addChild<Button>(tr("Dismiss"));
        dismiss->onClicked = [this] { setOperationError({}); };

        ColumnLayout* devices = content()->addChild<ColumnLayout>();
        devices->setSpacing(metrics::l);
        devices->setLayoutHints({.fillWidth = true});
        m_tabs = devices->addChild<SegmentedControl>(std::vector<std::string>{tr("Output"), tr("Input")});
        m_tabs->setName("tabs");
        m_tabs->setLayoutHints({.fillWidth = true});
        m_tabs->onCurrentChanged = [this](int) {
            rebuildDevices();
            update();
        };

        m_deviceCard = devices->addChild<Card>(Card::Style::List);
        m_noDevices = m_deviceCard->addChild<ListRow>();
        m_noDevices->setInteractive(false);
        m_noDevices->setEnabled(false);

        m_sliders = devices->addChild<Card>(Card::Style::List);
        m_outputVolume = m_sliders->addChild<SliderRow>(tr("Output volume"));
        m_outputVolume->setIcons("audio-volume-zero", "audio-volume-high");
        m_outputVolume->setRange(AudioObject::MinimalVolume, AudioObject::NormalVolume);
        m_outputVolume->setStepSize(0.01);
        m_outputVolume->onMoved = [this](double value) {
            if (const auto device = m_outputs.defaultNode()) {
                device->setVolume(value);
                // With no mute switch, muting shows as zero here.
                device->setMuted(value == 0);
            }
        };
        m_balance = m_sliders->addChild<SliderRow>(tr("Balance"));
        m_balance->setName("balance");
        m_balance->setCaptions(trc("balance", "Left"), trc("balance", "Right"));
        m_balance->setRange(-1, 1);
        m_balance->setStepSize(0.02);
        m_balance->onMoved = [this](double value) {
            if (const auto device = m_outputs.defaultNode())
                setBalance(*device, value);
        };
        m_inputVolume = m_sliders->addChild<SliderRow>(tr("Input volume"));
        m_inputVolume->setRange(AudioObject::MinimalVolume, AudioObject::NormalVolume);
        m_inputVolume->setStepSize(0.01);
        m_inputVolume->onMoved = [this](double value) {
            if (const auto device = m_inputs.defaultNode()) {
                device->setVolume(value);
                device->setMuted(false);
            }
        };
        m_inputLevel = m_sliders->addChild<SliderRow>(tr("Input level"));
        m_inputLevel->setMeterLevel(0);

        m_subscriptions.push_back(m_backend->connectedChanged.connect([this] {
            setOperationError({});
            update();
        }));
        m_subscriptions.push_back(
            m_backend->operationFailed.connect([this](std::string message) { setOperationError(message); }));
        for (AudioModel* model : {&m_outputs, &m_inputs}) {
            m_subscriptions.push_back(model->rowsChanged.connect([this] {
                rebuildDevices();
                update();
            }));
            m_subscriptions.push_back(model->dataChanged.connect([this] { update(); }));
            m_subscriptions.push_back(model->defaultNodeChanged.connect([this] { update(); }));
        }
        m_subscriptions.push_back(m_monitor.volumeChanged.connect([this] { updateMeter(); }));
        rebuildDevices();
        setOperationError({});
        update();
    }

private:
    void refreshSounds()
    {
        m_soundRead = m_sounds.getPropertyAsync("enabled", [this](const base::dbus::Reply& reply) {
            const auto value = reply.value<base::dbus::Variant>();
            const auto enabled = value ? value->get<bool>() : std::optional<bool>();
            m_eventSounds->setEnabled(enabled.has_value());
            m_eventSounds->setChecked(enabled.value_or(false));
            m_previewSound->setEnabled(enabled.value_or(false));
        });
    }

    bool showInputs() const { return m_tabs->currentIndex() == 1; }
    AudioModel& model() { return showInputs() ? m_inputs : m_outputs; }

    void setOperationError(const std::string& message)
    {
        m_operationError = message;
        m_errorCard->setVisible(!message.empty());
        m_error->setText(format(tr("Could not apply the audio change: {error}"), {{"error", message}}));
    }

    void rebuildDevices()
    {
        for (DeviceRow* row : m_rows)
            m_deviceCard->takeChild(row);
        m_rows.clear();
        // The rows go before the placeholder, which stays last.
        std::unique_ptr<View> placeholder = m_deviceCard->takeChild(m_noDevices);
        for (const auto& device : model().objects()) {
            DeviceRow* row = m_deviceCard->addChild<DeviceRow>(device);
            const std::weak_ptr<AudioObject> target = device;
            row->onClicked = [this, target] {
                const auto object = target.lock();
                if (!object || object->isDefault())
                    return;
                setOperationError({});
                object->setDefault(true);
            };
            m_rows.push_back(row);
        }
        m_deviceCard->addChild(std::move(placeholder));
    }

    void update()
    {
        for (DeviceRow* row : m_rows)
            row->update();
        const bool inputs = showInputs();
        const bool connected = m_backend->isConnected();
        m_connecting->setVisible(!connected);
        m_noDevices->setText(inputs ? tr("No input devices") : tr("No output devices"));
        m_noDevices->setVisible(connected && model().count() == 0);

        const std::shared_ptr<AudioObject> output = m_outputs.defaultNode();
        const std::shared_ptr<AudioObject> input = m_inputs.defaultNode();
        m_sliders->setVisible(inputs ? input != nullptr : output && output->hasVolume());

        m_outputVolume->setVisible(!inputs && output && output->hasVolume());
        m_outputVolume->setName(output ? "volume-" + std::to_string(output->index()) : std::string());
        m_outputVolume->setValue(output && !output->muted() ? output->volume() : 0);
        m_outputVolume->setSliderEnabled(output && output->volumeWritable());

        m_balance->setVisible(!inputs && output && hasFrontPair(*output));
        m_balance->setValue(output ? balance(*output) : 0);
        m_balance->setSliderEnabled(output && output->volumeWritable());

        m_inputVolume->setVisible(inputs && input && input->hasVolume());
        m_inputVolume->setName(input ? "volume-" + std::to_string(input->index()) : std::string());
        m_inputVolume->setValue(input ? input->volume() : 0);
        m_inputVolume->setSliderEnabled(input && input->volumeWritable());

        m_inputLevel->setVisible(inputs && input);
        m_monitor.setTarget(inputs ? input : nullptr);
        updateMeter();
    }

    void updateMeter()
    {
        // Peaks in dB over the bottom 50 dB, so speech fills about half.
        const double volume = m_monitor.volume();
        m_inputLevel->setMeterLevel(volume > 0 ? std::max(0.0, 1 + std::log10(volume) * 20 / 50) : 0);
    }

    base::dbus::Proxy m_sounds{base::dbus::Connection::session(), "org.lingmo.Services",
                               "/org/lingmo/Services/Sound", "org.lingmo.Services.Sound"};
    SwitchRow* m_eventSounds = nullptr;
    Button* m_previewSound = nullptr;
    base::dbus::Match m_soundProperties;
    base::dbus::NameWatch m_soundOwner;
    base::dbus::PendingCall m_soundRead;
    base::dbus::PendingCall m_soundChange;
    std::shared_ptr<Backend> m_backend;
    AudioModel m_outputs;
    AudioModel m_inputs;
    VolumeMonitor m_monitor;
    std::string m_operationError;
    Card* m_connecting;
    Card* m_errorCard;
    Label* m_error;
    SegmentedControl* m_tabs;
    Card* m_deviceCard;
    ListRow* m_noDevices;
    std::vector<DeviceRow*> m_rows;
    Card* m_sliders;
    SliderRow* m_outputVolume;
    SliderRow* m_balance;
    SliderRow* m_inputVolume;
    SliderRow* m_inputLevel;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace

static const PageRegistration registration("sound", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<SoundPage>(window);
});

} // namespace settings
