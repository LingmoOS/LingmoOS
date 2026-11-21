// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_BACKEND_INPUT_LIBINPUT_BACKEND_H_
#define LINGMO_BACKEND_INPUT_LIBINPUT_BACKEND_H_

#include "input/mouse_settings.h"
#include "input/seat_observer.h"
#include "input/touchpad_settings.h"
#include "input/touchscreen_settings.h"
#include "input/types.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

struct libinput;
struct libinput_event;
struct libinput_device;
struct udev;

namespace Lingmo::Input {
class InputManager;
}

namespace Lingmo::Backend {

class Session;
class TouchpadContacts;

// Opens the seat's input devices through libinput and pumps them from the main
// loop, feeding an input::InputManager.
//
// Devices are opened through the Session, so the compositor never needs
// permission to /dev/input itself and the seat manager can revoke access across
// VT switches.
class LibinputBackend : public input::SeatObserver
{
public:
    ~LibinputBackend() override;

    // Creates a udev-backed context assigned to the session's seat. Returns
    // nullptr if the context or the seat could not be acquired.
    static std::unique_ptr<LibinputBackend>
    create(Session *session, input::InputManager *inputManager);

    // Devices are suspended while another session owns the seat; libinput is
    // told so that it re-reads device state on resume.
    void setActive(bool active);

    void onSurfaceFocused(compositor::Surface *, compositor::Surface *) override { }
    // Every keyboard's lock lights follow the lock state the clients see.
    void onKeyboardModifierUpdated() override;
    void onKeyboardLayoutUpdated() override { onKeyboardModifierUpdated(); }

private:
    LibinputBackend(Session *session, input::InputManager *inputManager);

    bool open();
    void dispatch();
    void processEvent(libinput_event *event);
    // Sorts a new device into the mice or the touchpads, by udev's
    // classification, and applies their preferences.
    void addPointingDevice(libinput_device *device);
    void removePointingDevice(libinput_device *device);
    void applyMouseSettings(libinput_device *device);
    void applyTouchpadSettings(libinput_device *device);
    // The button a touchpad's press or release is sent as: a press libinput's
    // clickfinger counted as one finger may be two by isTwoFingerClick.
    uint32_t touchpadButton(libinput_device *device, uint32_t button, bool pressed,
                            input::Timestamp time);
    void addTouchscreen(libinput_device *device);
    void removeTouchscreen(libinput_device *device);
    void updateTouchscreenList();
    void applyTouchscreenSettings(libinput_device *device);

    static int openRestricted(const char *path, int flags, void *userData);
    static void closeRestricted(int fd, void *userData);

    Session *m_session = nullptr;
    input::InputManager *m_inputManager = nullptr;

    udev *m_udev = nullptr;
    libinput *m_libinput = nullptr;
    int m_watch = 0;

    // Session device handles, keyed by the fd libinput knows them as.
    std::map<int, int> m_deviceIds;
    std::map<std::string, int> m_deviceFds;
    bool m_active = true;
    input::MouseSettings m_mouseSettings;
    input::TouchpadSettings m_touchpadSettings;
    input::TouchscreenSettings m_touchscreenSettings;
    std::vector<base::Subscription> m_settingsSubscriptions;
    std::set<libinput_device *> m_mice;
    std::set<libinput_device *> m_touchpads;
    std::set<libinput_device *> m_touchscreens;
    std::map<libinput_device *, std::unique_ptr<TouchpadContacts>> m_touchpadContacts;
    // Touchpad presses sent as another button, so their releases follow.
    std::map<std::pair<libinput_device *, uint32_t>, uint32_t> m_touchpadButtons;
    std::map<libinput_device *, input::KeyboardModifierLayout> m_keyboardLayouts;
    uint32_t m_leds = 0;
};

} // namespace Lingmo::Backend

#endif // LINGMO_BACKEND_INPUT_LIBINPUT_BACKEND_H_
