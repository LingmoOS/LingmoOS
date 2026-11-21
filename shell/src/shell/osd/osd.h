// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <memory>
#include <string>

#include "base/signal.h"
#include "base/timer.h"
#include "shell/shell.h"

namespace Lingmo::base::dbus {
class Object;
}

namespace Lingmo::Shell {

class LevelTrack;
class ShellWindow;

// What the on-screen display shows the levels of and sets: the default sink
// and source, the backlight and the keyboard backlight. Setters answer
// asynchronously; changed() follows what the services report.
class OsdLevels
{
public:
    virtual ~OsdLevels() = default;

    // Volumes on PipeWire's cubic scale, 1 being normal.
    virtual bool hasSink() const = 0;
    virtual double volume() const = 0;
    virtual bool muted() const = 0;
    virtual void setVolume(double volume) = 0;
    virtual void setMuted(bool muted) = 0;

    virtual bool hasMicrophone() const = 0;
    virtual double microphoneVolume() const = 0;
    virtual bool microphoneMuted() const = 0;
    virtual void setMicrophoneMuted(bool muted) = 0;

    // Percent.
    virtual bool brightnessEnabled() const = 0;
    virtual int brightness() const = 0;
    virtual void setBrightness(int value) = 0;

    // Zero without a keyboard backlight.
    virtual int keyboardMaximum() const = 0;
    virtual int keyboardBrightness() const = 0;
    virtual void setKeyboardBrightness(int value) = 0;

    base::Signal<> changed;
    // The keyboard backlight's own hotkey changed it.
    base::Signal<> keyboardChangedByFirmware;
};

// The session's: PipeWire, org.lingmo.Services.Brightness and UPower.
std::unique_ptr<OsdLevels> createSessionOsdLevels(base::dbus::Connection *sessionBus);

// The on-screen display: the level a volume,
// microphone or brightness key set, a glass panel with the control centre's
// track, top centre below the status bar, gone 1.5 s after the last key.
// Serves org.lingmo.Osd at /Osd, which the compositor's media keys call.
class Osd : public Component
{
public:
    enum class Kind {
        Volume,
        Microphone,
        Brightness,
        KeyboardBrightness,
    };

    static constexpr int kWidth = 280;
    static constexpr int kTrackHeight = 22;
    static constexpr int kPadding = 16;
    static constexpr int kHeight = kTrackHeight + 2 * kPadding;
    // PanelStyle.popupMargin: the gap below the status bar.
    static constexpr int kTopMargin = 6;
    // FishUI's bigRadius * 1.5.
    static constexpr double kRadius = 18;
    static constexpr std::chrono::milliseconds kHideDelay {1500};
    // How long a requested value stands for the reported one.
    static constexpr std::chrono::milliseconds kSettle {500};

    Osd(Shell *shell, std::unique_ptr<OsdLevels> levels);
    ~Osd() override;

    bool registerService();

    void volumeUp();
    void volumeDown();
    void toggleMute();
    void toggleMicrophoneMute();
    void brightnessUp();
    void brightnessDown();
    void keyboardBrightnessUp();
    void keyboardBrightnessDown();

    Kind kind() const { return m_kind; }
    ShellWindow *window() const { return m_window.get(); }
    LevelTrack *track() const { return m_track; }
    bool isShown() const;
    // Where the OSD goes on the primary output's work area.
    base::Rect geometry() const;

private:
    void present(Kind kind);
    void hide();
    void place();
    void update();
    void applyGlass();
    void settle();
    double position() const;
    std::string iconFile() const;

    double volume() const;
    bool muted() const;
    bool microphoneMuted() const;
    double brightness() const;
    int keyboardBrightness() const;
    void stepBrightness(int steps);
    void stepKeyboardBrightness(int steps);

    Shell *m_shell;
    std::unique_ptr<OsdLevels> m_levels;
    std::unique_ptr<ShellWindow> m_window;
    LevelTrack *m_track = nullptr;
    Kind m_kind = Kind::Volume;
    // Requested and not reported yet; negative for none.
    double m_pendingVolume = -1;
    int m_pendingMuted = -1;
    int m_pendingMicrophoneMuted = -1;
    double m_pendingBrightness = -1;
    int m_pendingKeyboardBrightness = -1;
    base::Timer m_hideTimer;
    base::Timer m_settleTimer;
    std::unique_ptr<base::dbus::Object> m_service;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

