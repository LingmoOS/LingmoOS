// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/osd/osd.h"

#include <algorithm>
#include <cmath>

#include "base/dbus.h"
#include "base/logging.h"
#include "core/main_loop.h"
#include "shell/osd/level_track.h"
#include "shell/shell_data.h"
#include "shell/shell_window.h"
#include "ui/embedded_window.h"
#include "ui/theme.h"
#include "wm/window.h"

namespace Lingmo::Shell {

namespace dbus = base::dbus;

static const base::LogCategory lingmoOsd("lingmo.shell.osd");

// Ash's volume step, on PipeWire's scale where 1 is normal.
static constexpr double kNormalVolume = 1.0;
static constexpr double kVolumeStep = kNormalVolume * 0.04;
// powerd's sixteen brightness steps.
static constexpr double kBrightnessStep = 100.0 / 16;

Osd::Osd(Shell *shell, std::unique_ptr<OsdLevels> levels)
    : m_shell(shell)
    , m_levels(std::move(levels))
    , m_hideTimer(core::mainLoop())
    , m_settleTimer(core::mainLoop())
{
    m_window = std::make_unique<ShellWindow>(shell, wm::WindowRole::Osd);
    m_window->setFocusPolicy(wm::FocusPolicy::NoFocus);
    // Transparent for input: clicks go to whatever is underneath.
    m_window->setInputRegion(base::Region());
    auto track = std::make_unique<LevelTrack>();
    m_track = track.get();
    auto content = std::make_unique<ui::View>();
    content->addChild(std::move(track));
    m_track->setGeometry({double(kPadding), double(kPadding), double(kWidth - 2 * kPadding),
                          double(kTrackHeight)});
    m_window->setContentView(std::move(content));
    m_window->setSize(base::Size(kWidth, kHeight));
    applyGlass();

    m_hideTimer.setSingleShot(true);
    m_hideTimer.setInterval(kHideDelay);
    m_hideTimer.onTimeout = [this] { hide(); };
    m_settleTimer.setSingleShot(true);
    m_settleTimer.setInterval(kSettle);
    m_settleTimer.onTimeout = [this] {
        m_pendingVolume = -1;
        m_pendingMuted = -1;
        m_pendingMicrophoneMuted = -1;
        m_pendingBrightness = -1;
        m_pendingKeyboardBrightness = -1;
        update();
    };

    m_subscriptions.push_back(m_levels->changed.connect([this] { update(); }));
    m_subscriptions.push_back(m_levels->keyboardChangedByFirmware.connect([this] {
        present(Kind::KeyboardBrightness);
    }));
    m_subscriptions.push_back(m_shell->outputsChanged.connect([this] {
        if (isShown())
            place();
    }));
    update();
}

Osd::~Osd()
{
    m_subscriptions.clear();
    if (m_service && m_shell->bus())
        m_shell->bus()->releaseName("org.lingmo.Osd");
    m_service.reset();
    m_window.reset();
}

bool Osd::registerService()
{
    if (m_service)
        return true;
    dbus::Connection *bus = m_shell->bus();
    if (!bus)
        return false;
    auto object = std::make_unique<dbus::Object>(bus, "/Osd", "org.lingmo.Osd");
    object->addMethod("volumeDown", [this] { volumeDown(); });
    object->addMethod("volumeUp", [this] { volumeUp(); });
    object->addMethod("toggleMute", [this] { toggleMute(); });
    object->addMethod("toggleMicrophoneMute", [this] { toggleMicrophoneMute(); });
    object->addMethod("brightnessDown", [this] { brightnessDown(); });
    object->addMethod("brightnessUp", [this] { brightnessUp(); });
    object->addMethod("keyboardBrightnessDown", [this] { keyboardBrightnessDown(); });
    object->addMethod("keyboardBrightnessUp", [this] { keyboardBrightnessUp(); });
    if (!object->publish() || !bus->requestName("org.lingmo.Osd", dbus::Connection::DoNotQueue)) {
        LM_WARNING(lingmoOsd) << "Cannot own org.lingmo.Osd";
        return false;
    }
    m_service = std::move(object);
    return true;
}

bool Osd::isShown() const
{
    return m_window->isShown();
}

base::Rect Osd::geometry() const
{
    const Shell::Output *output = m_shell->output({});
    if (!output)
        return base::Rect(0, 0, kWidth, kHeight);
    const base::Rect area = m_shell->workArea(*output);
    return base::Rect(int(std::lround(area.x() + (area.width() - kWidth) / 2.0)),
                      area.y() + kTopMargin, kWidth, kHeight);
}

void Osd::place()
{
    m_window->setGeometry(geometry());
}

void Osd::applyGlass()
{
    const bool dark = ui::Theme::current().darkMode;
    // PanelStyle.glassTint on PopupGlass.
    const render::GlassParams glass = dark ? popupGlass(0.10, 0.10, 0.11, 0.75)
                                           : popupGlass(0.90, 0.90, 0.92, 0.72);
    m_window->setBackgroundBlur(base::Rect(0, 0, kWidth, kHeight), int(kRadius), glass);
}

void Osd::present(Kind kind)
{
    m_kind = kind;
    update();
    place();
    applyGlass();
    m_window->show();
    m_hideTimer.start();
}

void Osd::hide()
{
    m_window->hide();
}

void Osd::settle()
{
    m_settleTimer.start();
}

double Osd::volume() const
{
    return m_pendingVolume >= 0 ? m_pendingVolume : m_levels->hasSink() ? m_levels->volume() : 0;
}

bool Osd::muted() const
{
    return m_pendingMuted >= 0 ? m_pendingMuted == 1
                               : m_levels->hasSink() && m_levels->muted();
}

bool Osd::microphoneMuted() const
{
    return m_pendingMicrophoneMuted >= 0 ? m_pendingMicrophoneMuted == 1
                                         : m_levels->hasMicrophone() && m_levels->microphoneMuted();
}

double Osd::brightness() const
{
    return m_pendingBrightness >= 0 ? m_pendingBrightness : m_levels->brightness();
}

int Osd::keyboardBrightness() const
{
    return m_pendingKeyboardBrightness >= 0 ? m_pendingKeyboardBrightness
                                            : m_levels->keyboardBrightness();
}

double Osd::position() const
{
    switch (m_kind) {
    case Kind::Brightness:
        return std::clamp(brightness() / 100, 0.0, 1.0);
    case Kind::KeyboardBrightness:
        return m_levels->keyboardMaximum() > 0
                ? double(keyboardBrightness()) / m_levels->keyboardMaximum()
                : 0;
    case Kind::Microphone:
        if (!m_levels->hasMicrophone() || microphoneMuted())
            return 0;
        return std::clamp(m_levels->microphoneVolume() / kNormalVolume, 0.0, 1.0);
    case Kind::Volume:
        break;
    }
    return muted() ? 0 : std::clamp(volume() / kNormalVolume, 0.0, 1.0);
}

std::string Osd::iconFile() const
{
    switch (m_kind) {
    case Kind::Brightness:
        return shellDataFile("brightness.svg");
    case Kind::KeyboardBrightness:
        return shellDataFile("keyboard-brightness.svg");
    case Kind::Microphone:
        return shellDataFile(microphoneMuted() ? "microphone-muted.svg" : "microphone.svg");
    case Kind::Volume:
        break;
    }
    const double percent = volume() / kNormalVolume * 100;
    if (muted() || percent <= 0)
        return shellDataFile("audio-volume-muted-symbolic.svg");
    if (percent <= 25)
        return shellDataFile("audio-volume-low-symbolic.svg");
    if (percent <= 75)
        return shellDataFile("audio-volume-medium-symbolic.svg");
    return shellDataFile("audio-volume-high-symbolic.svg");
}

void Osd::update()
{
    // A held key glides; the first level after showing snaps.
    m_track->setAnimated(isShown());
    m_track->setPosition(position());
    m_track->setIcon(iconFile());
}

void Osd::volumeUp()
{
    if (!m_levels->hasSink())
        return;
    if (muted()) {
        m_pendingMuted = 0;
        m_levels->setMuted(false);
    }
    // Another application can raise it past the normal volume; up never
    // lowers it.
    const double current = volume();
    m_pendingVolume = std::max(current,
                               std::min(kNormalVolume, (std::round(current / kVolumeStep) + 1) * kVolumeStep));
    settle();
    m_levels->setVolume(m_pendingVolume);
    present(Kind::Volume);
}

void Osd::volumeDown()
{
    if (!m_levels->hasSink())
        return;
    // Muted stays muted; the key only shows it.
    if (!muted()) {
        const double value = std::max(0.0, (std::round(volume() / kVolumeStep) - 1) * kVolumeStep);
        m_pendingVolume = value;
        settle();
        m_levels->setVolume(value);
        if (value == 0) {
            m_pendingMuted = 1;
            m_levels->setMuted(true);
        }
    }
    present(Kind::Volume);
}

void Osd::toggleMute()
{
    if (!m_levels->hasSink())
        return;
    const bool mute = !muted();
    m_pendingMuted = mute ? 1 : 0;
    settle();
    m_levels->setMuted(mute);
    present(Kind::Volume);
}

void Osd::toggleMicrophoneMute()
{
    if (!m_levels->hasMicrophone())
        return;
    const bool mute = !microphoneMuted();
    m_pendingMicrophoneMuted = mute ? 1 : 0;
    settle();
    m_levels->setMicrophoneMuted(mute);
    present(Kind::Microphone);
}

void Osd::stepBrightness(int steps)
{
    if (!m_levels->brightnessEnabled())
        return;
    const double index = std::round(brightness() / kBrightnessStep) + steps;
    const int value = int(std::clamp(std::round(index * kBrightnessStep), 0.0, 100.0));
    m_pendingBrightness = value;
    settle();
    m_levels->setBrightness(value);
    present(Kind::Brightness);
}

void Osd::brightnessUp()
{
    stepBrightness(1);
}

void Osd::brightnessDown()
{
    stepBrightness(-1);
}

void Osd::stepKeyboardBrightness(int steps)
{
    const int maximum = m_levels->keyboardMaximum();
    if (maximum <= 0)
        return;
    // A backlight has a few levels or many; at most ten steps either way.
    const int step = std::max(1, int(std::lround(maximum / 10.0)));
    const int index = int(std::lround(double(keyboardBrightness()) / step)) + steps;
    const int value = std::clamp(index * step, 0, maximum);
    m_pendingKeyboardBrightness = value;
    settle();
    m_levels->setKeyboardBrightness(value);
    present(Kind::KeyboardBrightness);
}

void Osd::keyboardBrightnessUp()
{
    stepKeyboardBrightness(1);
}

void Osd::keyboardBrightnessDown()
{
    stepKeyboardBrightness(-1);
}

} // namespace Lingmo::Shell
