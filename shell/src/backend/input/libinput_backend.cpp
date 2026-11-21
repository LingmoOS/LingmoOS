// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "backend/input/libinput_backend.h"

#include <cerrno>
#include <fcntl.h>
#include <libinput.h>
#include <libudev.h>
#include <linux/input-event-codes.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <string>
#include <vector>

#include "backend/input/touchpad_contacts.h"
#include "base/file.h"
#include "base/event_loop.h"
#include "base/strings.h"
#include "backend/session/session.h"
#include "core/logging.h"
#include "core/main_loop.h"
#include "input/input_manager.h"
#include "input/pointer_acceleration.h"
#include "input/seat.h"
#include "input/touchpad_click.h"
#include "input/xkb_tracker.h"

namespace Lingmo::Backend {

namespace {

input::Timestamp timestampFromUsec(uint64_t usec)
{
    return input::Timestamp(std::chrono::microseconds(usec));
}

// libinput reports wheel movement in 1/120 of a click.
constexpr double kV120PerClick = 120.0;

input::GestureType gestureType(libinput_event_type type)
{
    switch (type) {
    case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
    case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
    case LIBINPUT_EVENT_GESTURE_PINCH_END:
        return input::GestureType::Pinch;
    case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
    case LIBINPUT_EVENT_GESTURE_HOLD_END:
        return input::GestureType::Hold;
    default:
        return input::GestureType::Swipe;
    }
}

constexpr uint32_t kSecondaryClickMethods =
        LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER | LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS;

bool secondaryClickConfigurable(libinput_device *device)
{
    return (libinput_device_config_click_get_methods(device) & kSecondaryClickMethods)
            == kSecondaryClickMethods;
}

// hid_apple's swap_opt_cmd makes the kernel report Command as Alt already.
bool kernelSwapsAppleModifiers()
{
    const auto contents = base::file::read("/sys/module/hid_apple/parameters/swap_opt_cmd");
    return contents && base::strings::trimmed(*contents) != "0";
}

// KeyboardCapability's rule takes a device named "Apple ... Keyboard" for an
// Apple keyboard; the vendor ids also catch the Magic Keyboard, whose name
// does not say Apple.
input::KeyboardModifierLayout modifierLayout(libinput_device *device)
{
    constexpr unsigned kAppleUsbVendor = 0x05ac;
    constexpr unsigned kAppleBluetoothVendor = 0x004c;
    const unsigned vendor = libinput_device_get_id_vendor(device);
    bool apple = vendor == kAppleUsbVendor || vendor == kAppleBluetoothVendor;
    if (!apple) {
        using base::strings::CaseSensitivity;
        using base::strings::SplitBehavior;
        std::vector<std::string> tokens;
        for (const auto &word : base::strings::split(libinput_device_get_name(device), ' ',
                                                     SplitBehavior::SkipEmptyParts)) {
            for (auto &token : base::strings::split(word, '.', SplitBehavior::SkipEmptyParts))
                tokens.push_back(std::move(token));
        }
        apple = base::strings::contains(tokens, "apple", CaseSensitivity::Insensitive)
                && base::strings::contains(tokens, "keyboard", CaseSensitivity::Insensitive);
    }
    return apple && !kernelSwapsAppleModifiers() ? input::KeyboardModifierLayout::Apple
                                                 : input::KeyboardModifierLayout::Pc;
}

} // namespace

LibinputBackend::LibinputBackend(Session *session, input::InputManager *inputManager)
    : m_session(session)
    , m_inputManager(inputManager)
{
    if (input::Seat *seat = m_inputManager->seat()) {
        seat->addObserver(this);
        m_leds = seat->clientXkbTracker()->leds();
    }
    m_settingsSubscriptions.push_back(m_mouseSettings.settingsChanged.connect([this] {
        for (auto *device : m_mice)
            applyMouseSettings(device);
    }));
    // Disabling a device queues the release of whatever it held without
    // waking libinput's fd, so the queue is drained here.
    m_settingsSubscriptions.push_back(m_touchpadSettings.settingsChanged.connect([this] {
        for (auto *device : m_touchpads)
            applyTouchpadSettings(device);
        dispatch();
    }));
    m_settingsSubscriptions.push_back(m_touchscreenSettings.settingsChanged.connect([this] {
        for (auto *device : m_touchscreens)
            applyTouchscreenSettings(device);
        dispatch();
    }));
}

LibinputBackend::~LibinputBackend()
{
    if (input::Seat *seat = m_inputManager->seat())
        seat->removeObserver(this);
    if (m_watch)
        core::mainLoop()->unwatch(m_watch);
    for (auto *device : m_mice)
        libinput_device_unref(device);
    for (auto *device : m_touchpads)
        libinput_device_unref(device);
    for (auto *device : m_touchscreens)
        libinput_device_unref(device);
    // Destroying the context runs close_restricted for every open device, so
    // the session handles are released here rather than by hand.
    if (m_libinput)
        libinput_unref(m_libinput);
    if (m_udev)
        udev_unref(m_udev);
}

std::unique_ptr<LibinputBackend>
LibinputBackend::create(Session *session, input::InputManager *inputManager)
{
    std::unique_ptr<LibinputBackend> backend(new LibinputBackend(session, inputManager));
    if (!backend->open())
        return nullptr;
    backend->m_mouseSettings.registerService();
    backend->m_touchpadSettings.registerService();
    backend->m_touchscreenSettings.registerService();
    return backend;
}

int LibinputBackend::openRestricted(const char *path, int flags, void *userData)
{
    auto *self = static_cast<LibinputBackend *>(userData);
    int deviceId = -1;
    const int fd = self->m_session->openDevice(path, &deviceId);
    if (fd < 0)
        return errno ? -errno : -1;

    // libseat hands back a descriptor it opened itself, so the flags libinput
    // asked for have to be applied afterwards.
    const int current = fcntl(fd, F_GETFL);
    if (current >= 0)
        fcntl(fd, F_SETFL, (current & ~O_ACCMODE) | (flags & O_ACCMODE) | (flags & O_NONBLOCK));

    self->m_deviceIds[fd] = deviceId;
    self->m_deviceFds[path] = fd;
    return fd;
}

void LibinputBackend::closeRestricted(int fd, void *userData)
{
    auto *self = static_cast<LibinputBackend *>(userData);
    std::erase_if(self->m_deviceFds, [fd](const auto &entry) { return entry.second == fd; });
    // libinput closes devices on suspend before it reports them removed.
    std::erase_if(self->m_touchpadContacts,
                  [fd](const auto &entry) { return entry.second->fd() == fd; });
    auto it = self->m_deviceIds.find(fd);
    if (it == self->m_deviceIds.end()) {
        close(fd);
        return;
    }
    self->m_session->closeDevice(it->second);
    self->m_deviceIds.erase(it);
}

bool LibinputBackend::open()
{
    static const libinput_interface interface = {&LibinputBackend::openRestricted,
                                                 &LibinputBackend::closeRestricted};

    m_udev = udev_new();
    if (!m_udev) {
        LM_CRITICAL(lingmoInput) << "Failed to create a udev context";
        return false;
    }

    m_libinput = libinput_udev_create_context(&interface, this, m_udev);
    if (!m_libinput) {
        LM_CRITICAL(lingmoInput) << "Failed to create a libinput context";
        return false;
    }

    const std::string seatName = m_session->seatName();
    if (libinput_udev_assign_seat(m_libinput, seatName.c_str()) != 0) {
        LM_CRITICAL(lingmoInput) << "Failed to assign libinput to seat" << m_session->seatName();
        return false;
    }

    m_watch = core::mainLoop()->watch(libinput_get_fd(m_libinput), EPOLLIN,
                                      [this](uint32_t) { dispatch(); });

    // The seat's devices are announced immediately; drain them so the initial
    // device set is known before the first frame.
    dispatch();

    LM_INFO(lingmoInput) << "libinput opened on seat" << m_session->seatName();
    return true;
}

void LibinputBackend::onKeyboardModifierUpdated()
{
    const input::Seat *seat = m_inputManager->seat();
    const uint32_t leds = seat ? seat->clientXkbTracker()->leds() : 0;
    if (leds == m_leds)
        return;
    m_leds = leds;
    for (const auto &[device, layout] : m_keyboardLayouts)
        libinput_device_led_update(device, static_cast<libinput_led>(m_leds));
}

void LibinputBackend::setActive(bool active)
{
    if (m_active == active || !m_libinput)
        return;
    LM_DEBUG(lingmoInputTransitions) << "libinput active" << active;
    m_active = active;
    if (active) {
        libinput_resume(m_libinput);
        dispatch();
    } else {
        libinput_suspend(m_libinput);
        // Suspending queues the releases of whatever the devices still hold;
        // left queued, clients repeat a held key until the session returns.
        dispatch();
    }
}

void LibinputBackend::dispatch()
{
    if (libinput_dispatch(m_libinput) != 0) {
        LM_WARNING(lingmoInput) << "libinput_dispatch failed";
        return;
    }

    // Fingers are stamped as they arrive, ahead of the clicks that follow.
    const auto now = std::chrono::steady_clock::now();
    for (const auto &[device, contacts] : m_touchpadContacts)
        contacts->update(now);

    while (libinput_event *event = libinput_get_event(m_libinput)) {
        processEvent(event);
        libinput_event_destroy(event);
    }
}

void LibinputBackend::addPointingDevice(libinput_device *device)
{
    if (!libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_POINTER))
        return;
    auto *udevDevice = libinput_device_get_udev_device(device);
    if (!udevDevice)
        return;
    const auto tagged = [udevDevice](const char *name) {
        const char *value = udev_device_get_property_value(udevDevice, name);
        return value && std::strcmp(value, "1") == 0;
    };
    const bool touchpad = tagged("ID_INPUT_TOUCHPAD");
    const bool mouse = tagged("ID_INPUT_MOUSE") && !touchpad
            && !tagged("ID_INPUT_POINTINGSTICK") && !tagged("ID_INPUT_TABLET")
            && !tagged("ID_INPUT_TOUCHSCREEN");
    const char *devnode = udev_device_get_devnode(udevDevice);
    const auto fd = devnode ? m_deviceFds.find(devnode) : m_deviceFds.end();
    udev_device_unref(udevDevice);
    if (touchpad) {
        if (m_touchpads.insert(device).second)
            libinput_device_ref(device);
        if (fd != m_deviceFds.end()) {
            if (auto contacts = TouchpadContacts::create(fd->second))
                m_touchpadContacts[device] = std::move(contacts);
        }
        applyTouchpadSettings(device);
        m_touchpadSettings.setAvailable(true);
        m_touchpadSettings.setSecondaryClickConfigurable(
                std::any_of(m_touchpads.begin(), m_touchpads.end(),
                            secondaryClickConfigurable));
    } else if (mouse) {
        if (m_mice.insert(device).second)
            libinput_device_ref(device);
        applyMouseSettings(device);
        m_mouseSettings.setAvailable(true);
    }
}

void LibinputBackend::removePointingDevice(libinput_device *device)
{
    if (m_mice.erase(device)) {
        libinput_device_unref(device);
        m_mouseSettings.setAvailable(!m_mice.empty());
    }
    m_touchpadContacts.erase(device);
    std::erase_if(m_touchpadButtons,
                  [device](const auto &entry) { return entry.first.first == device; });
    if (m_touchpads.erase(device)) {
        libinput_device_unref(device);
        m_touchpadSettings.setAvailable(!m_touchpads.empty());
        m_touchpadSettings.setSecondaryClickConfigurable(
                std::any_of(m_touchpads.begin(), m_touchpads.end(),
                            secondaryClickConfigurable));
    }
}

void LibinputBackend::applyMouseSettings(libinput_device *device)
{
    const auto check = [device](libinput_config_status status, const char *setting) {
        if (status != LIBINPUT_CONFIG_STATUS_SUCCESS)
            LM_WARNING(lingmoInput) << "Cannot apply mouse" << setting << "to"
                                << libinput_device_get_name(device)
                                << libinput_config_status_to_str(status);
    };
    if (libinput_device_config_left_handed_is_available(device))
        check(libinput_device_config_left_handed_set(device, m_mouseSettings.leftHanded()),
              "left-handed mode");
    if (libinput_device_config_scroll_has_natural_scroll(device))
        check(libinput_device_config_scroll_set_natural_scroll_enabled(
                      device, m_mouseSettings.naturalScroll()),
              "natural scrolling");
    if (!libinput_device_config_accel_is_available(device))
        return;
    const uint32_t profiles = libinput_device_config_accel_get_profiles(device);
    if (m_mouseSettings.acceleration() && (profiles & LIBINPUT_CONFIG_ACCEL_PROFILE_CUSTOM)) {
        // The custom curve takes the pointer speed itself; libinput's speed
        // setting does not apply to it.
        int dpi = 0;
        if (auto *udevDevice = libinput_device_get_udev_device(device)) {
            dpi = input::mouseDpi(udev_device_get_property_value(udevDevice, "MOUSE_DPI"));
            udev_device_unref(udevDevice);
        }
        auto curve = input::accelerationPoints(dpi, m_mouseSettings.pointerAcceleration());
        libinput_config_accel *config =
                libinput_config_accel_create(LIBINPUT_CONFIG_ACCEL_PROFILE_CUSTOM);
        if (config) {
            check(libinput_config_accel_set_points(config, LIBINPUT_ACCEL_TYPE_MOTION, curve.step,
                                                   curve.points.size(), curve.points.data()),
                  "acceleration curve");
            check(libinput_device_config_accel_apply(device, config), "acceleration curve");
            libinput_config_accel_destroy(config);
            return;
        }
    }
    const auto profile = m_mouseSettings.acceleration() ? LIBINPUT_CONFIG_ACCEL_PROFILE_ADAPTIVE
                                                        : LIBINPUT_CONFIG_ACCEL_PROFILE_FLAT;
    if (profiles & profile)
        check(libinput_device_config_accel_set_profile(device, profile), "acceleration profile");
    check(libinput_device_config_accel_set_speed(device, m_mouseSettings.pointerAcceleration()),
          "pointer speed");
}

void LibinputBackend::applyTouchpadSettings(libinput_device *device)
{
    const auto check = [device](libinput_config_status status, const char *setting) {
        if (status != LIBINPUT_CONFIG_STATUS_SUCCESS)
            LM_WARNING(lingmoInput) << "Cannot apply touchpad" << setting << "to"
                                << libinput_device_get_name(device)
                                << libinput_config_status_to_str(status);
    };
    const uint32_t modes = libinput_device_config_send_events_get_modes(device);
    uint32_t mode = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
    if (!m_touchpadSettings.enabled())
        mode = LIBINPUT_CONFIG_SEND_EVENTS_DISABLED;
    else if (m_touchpadSettings.disableWithMouse())
        mode = LIBINPUT_CONFIG_SEND_EVENTS_DISABLED_ON_EXTERNAL_MOUSE;
    // Enabled is the one mode every device has; it stands in for one this
    // device lacks.
    if (!(modes & mode))
        mode = LIBINPUT_CONFIG_SEND_EVENTS_ENABLED;
    check(libinput_device_config_send_events_set_mode(device, mode), "send-events mode");

    if (libinput_device_config_dwt_is_available(device))
        check(libinput_device_config_dwt_set_enabled(device,
                                                     m_touchpadSettings.disableWhileTyping()
                                                             ? LIBINPUT_CONFIG_DWT_ENABLED
                                                             : LIBINPUT_CONFIG_DWT_DISABLED),
              "disable-while-typing");
    if (libinput_device_config_tap_get_finger_count(device) > 0) {
        check(libinput_device_config_tap_set_enabled(device, m_touchpadSettings.tapToClick()
                                                                     ? LIBINPUT_CONFIG_TAP_ENABLED
                                                                     : LIBINPUT_CONFIG_TAP_DISABLED),
              "tap-to-click");
        check(libinput_device_config_tap_set_drag_enabled(device,
                                                          m_touchpadSettings.tapDragging()
                                                                  ? LIBINPUT_CONFIG_DRAG_ENABLED
                                                                  : LIBINPUT_CONFIG_DRAG_DISABLED),
              "tap-and-drag");
    }
    // libinput defaults most clickpads to bottom-edge button areas, where a
    // two-finger press is still a left click.
    const auto clickMethod = m_touchpadSettings.twoFingerSecondaryClick()
            ? LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER
            : LIBINPUT_CONFIG_CLICK_METHOD_BUTTON_AREAS;
    if (libinput_device_config_click_get_methods(device) & clickMethod)
        check(libinput_device_config_click_set_method(device, clickMethod), "click method");
    if (libinput_device_config_scroll_has_natural_scroll(device))
        check(libinput_device_config_scroll_set_natural_scroll_enabled(
                      device, m_touchpadSettings.naturalScroll()),
              "natural scrolling");
    // Touchpads keep libinput's own adaptive profile, which is built for
    // fingers on a pad; the mouse curve is not.
    if (libinput_device_config_accel_is_available(device))
        check(libinput_device_config_accel_set_speed(device,
                                                     m_touchpadSettings.pointerAcceleration()),
              "pointer speed");
}

uint32_t LibinputBackend::touchpadButton(libinput_device *device, uint32_t button, bool pressed,
                                         input::Timestamp time)
{
    const auto key = std::make_pair(device, button);
    if (!pressed) {
        const auto sent = m_touchpadButtons.find(key);
        if (sent == m_touchpadButtons.end())
            return button;
        const uint32_t result = sent->second;
        m_touchpadButtons.erase(sent);
        return result;
    }

    const auto contacts = m_touchpadContacts.find(device);
    if (button != BTN_LEFT || contacts == m_touchpadContacts.end()
        || libinput_device_config_click_get_method(device)
                != LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER
        || !contacts->second->clicked())
        return button;
    const auto fingers = contacts->second->contacts();
    if (fingers.size() != 2
        || !input::isTwoFingerClick(fingers[0], fingers[1], time,
                                     contacts->second->size()))
        return button;
    m_touchpadButtons[key] = BTN_RIGHT;
    return BTN_RIGHT;
}

void LibinputBackend::addTouchscreen(libinput_device *device)
{
    if (!libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_TOUCH))
        return;
    if (m_touchscreens.insert(device).second)
        libinput_device_ref(device);
    applyTouchscreenSettings(device);
    updateTouchscreenList();
}

void LibinputBackend::removeTouchscreen(libinput_device *device)
{
    if (!m_touchscreens.erase(device))
        return;
    libinput_device_unref(device);
    updateTouchscreenList();
}

void LibinputBackend::updateTouchscreenList()
{
    std::vector<std::string> names;
    for (auto *device : m_touchscreens) {
        std::string name = libinput_device_get_name(device);
        // Identical panels share a name, and so one switch.
        if (std::find(names.begin(), names.end(), name) == names.end())
            names.push_back(std::move(name));
    }
    std::sort(names.begin(), names.end(), [](const std::string &a, const std::string &b) {
        return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
                                            [](unsigned char x, unsigned char y) {
                                                return std::tolower(x) < std::tolower(y);
                                            });
    });
    m_touchscreenSettings.setDevices(names);
}

void LibinputBackend::applyTouchscreenSettings(libinput_device *device)
{
    // Suspending the device makes libinput cancel any finger still down.
    const auto mode = m_touchscreenSettings.deviceEnabled(libinput_device_get_name(device))
            ? LIBINPUT_CONFIG_SEND_EVENTS_ENABLED
            : LIBINPUT_CONFIG_SEND_EVENTS_DISABLED;
    const auto status = libinput_device_config_send_events_set_mode(device, mode);
    if (status != LIBINPUT_CONFIG_STATUS_SUCCESS)
        LM_WARNING(lingmoInput) << "Cannot apply touchscreen send-events mode to"
                            << libinput_device_get_name(device)
                            << libinput_config_status_to_str(status);
}

void LibinputBackend::processEvent(libinput_event *event)
{
    const libinput_event_type type = libinput_event_get_type(event);
    switch (type) {
    case LIBINPUT_EVENT_DEVICE_ADDED:
        if (libinput_device_has_capability(libinput_event_get_device(event),
                                           LIBINPUT_DEVICE_CAP_KEYBOARD)) {
            m_keyboardLayouts[libinput_event_get_device(event)] =
                    modifierLayout(libinput_event_get_device(event));
            // A keyboard plugged in, or reopened after a VT switch, shows the
            // locks as they are, not as its firmware left them.
            libinput_device_led_update(libinput_event_get_device(event),
                                       static_cast<libinput_led>(m_leds));
        }
        addPointingDevice(libinput_event_get_device(event));
        addTouchscreen(libinput_event_get_device(event));
        LM_DEBUG(lingmoInput) << "input device added:"
                          << libinput_device_get_name(libinput_event_get_device(event));
        break;
    case LIBINPUT_EVENT_DEVICE_REMOVED:
        m_keyboardLayouts.erase(libinput_event_get_device(event));
        removePointingDevice(libinput_event_get_device(event));
        removeTouchscreen(libinput_event_get_device(event));
        LM_DEBUG(lingmoInputTransitions)
                << "libinput removed"
                << libinput_device_get_sysname(libinput_event_get_device(event));
        LM_DEBUG(lingmoInput) << "input device removed:"
                          << libinput_device_get_name(libinput_event_get_device(event));
        break;
    case LIBINPUT_EVENT_KEYBOARD_KEY: {
        libinput_event_keyboard *keyEvent = libinput_event_get_keyboard_event(event);
        const auto layout = m_keyboardLayouts.find(libinput_event_get_device(event));
        m_inputManager->onKey(timestampFromUsec(libinput_event_keyboard_get_time_usec(keyEvent)),
                              libinput_event_keyboard_get_key(keyEvent),
                              libinput_event_keyboard_get_key_state(keyEvent)
                                      == LIBINPUT_KEY_STATE_PRESSED,
                              layout == m_keyboardLayouts.end()
                                      ? input::KeyboardModifierLayout::Pc
                                      : layout->second);
        break;
    }
    case LIBINPUT_EVENT_POINTER_MOTION: {
        libinput_event_pointer *motion = libinput_event_get_pointer_event(event);
        m_inputManager->onPointerMotion(
                timestampFromUsec(libinput_event_pointer_get_time_usec(motion)),
                base::PointF(libinput_event_pointer_get_dx(motion),
                        libinput_event_pointer_get_dy(motion)),
                base::PointF(libinput_event_pointer_get_dx_unaccelerated(motion),
                        libinput_event_pointer_get_dy_unaccelerated(motion)));
        break;
    }
    case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE: {
        libinput_event_pointer *motion = libinput_event_get_pointer_event(event);
        m_inputManager->onPointerMotionNormalized(
                timestampFromUsec(libinput_event_pointer_get_time_usec(motion)),
                base::PointF(libinput_event_pointer_get_absolute_x_transformed(motion, 1),
                        libinput_event_pointer_get_absolute_y_transformed(motion, 1)));
        break;
    }
    case LIBINPUT_EVENT_TOUCH_DOWN:
    case LIBINPUT_EVENT_TOUCH_MOTION: {
        libinput_event_touch *touch = libinput_event_get_touch_event(event);
        const auto timestamp = timestampFromUsec(libinput_event_touch_get_time_usec(touch));
        // The seat slot is unique across every touchscreen on the seat.
        const int32_t id = libinput_event_touch_get_seat_slot(touch);
        const base::PointF normalized(libinput_event_touch_get_x_transformed(touch, 1),
                                 libinput_event_touch_get_y_transformed(touch, 1));
        if (type == LIBINPUT_EVENT_TOUCH_DOWN)
            m_inputManager->onTouchDown(timestamp, id, normalized);
        else
            m_inputManager->onTouchMotion(timestamp, id, normalized);
        break;
    }
    case LIBINPUT_EVENT_TOUCH_UP: {
        libinput_event_touch *touch = libinput_event_get_touch_event(event);
        m_inputManager->onTouchUp(timestampFromUsec(libinput_event_touch_get_time_usec(touch)),
                                  libinput_event_touch_get_seat_slot(touch));
        break;
    }
    case LIBINPUT_EVENT_TOUCH_CANCEL: {
        // libinput cancels the whole touch session at once.
        libinput_event_touch *touch = libinput_event_get_touch_event(event);
        m_inputManager->onTouchCancel(timestampFromUsec(libinput_event_touch_get_time_usec(touch)));
        break;
    }
    case LIBINPUT_EVENT_TOUCH_FRAME: {
        libinput_event_touch *touch = libinput_event_get_touch_event(event);
        m_inputManager->onTouchFrame(timestampFromUsec(libinput_event_touch_get_time_usec(touch)));
        break;
    }
    case LIBINPUT_EVENT_POINTER_BUTTON: {
        libinput_event_pointer *button = libinput_event_get_pointer_event(event);
        LM_DEBUG(lingmoInputTransitions)
                << "libinput device"
                << libinput_device_get_sysname(libinput_event_get_device(event)) << "code"
                << libinput_event_pointer_get_button(button)
                << (libinput_event_pointer_get_button_state(button) == LIBINPUT_BUTTON_STATE_PRESSED
                            ? "press"
                            : "release")
                << "seat-count-after" << libinput_event_pointer_get_seat_button_count(button);
        const auto timestamp = timestampFromUsec(libinput_event_pointer_get_time_usec(button));
        const bool pressed =
                libinput_event_pointer_get_button_state(button) == LIBINPUT_BUTTON_STATE_PRESSED;
        m_inputManager->onPointerButton(
                timestamp,
                touchpadButton(libinput_event_get_device(event),
                               libinput_event_pointer_get_button(button), pressed, timestamp),
                pressed);
        break;
    }
    case LIBINPUT_EVENT_POINTER_SCROLL_WHEEL: {
        libinput_event_pointer *scroll = libinput_event_get_pointer_event(event);
        const auto value = [scroll](libinput_pointer_axis axis) {
            if (!libinput_event_pointer_has_axis(scroll, axis))
                return 0.0;
            return libinput_event_pointer_get_scroll_value_v120(scroll, axis) / kV120PerClick
                    * input::kAxisUnitsPerClick;
        };
        m_inputManager->onPointerAxis(
                timestampFromUsec(libinput_event_pointer_get_time_usec(scroll)),
                base::PointF(value(LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL),
                        value(LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL)),
                true);
        break;
    }
    case LIBINPUT_EVENT_POINTER_SCROLL_FINGER:
    case LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS: {
        libinput_event_pointer *scroll = libinput_event_get_pointer_event(event);
        // Fingers on a touchpad scroll at the user's speed; a trackball or
        // button scrolling keeps libinput's.
        const double factor = type == LIBINPUT_EVENT_POINTER_SCROLL_FINGER
                        && m_touchpads.contains(libinput_event_get_device(event))
                ? m_touchpadSettings.scrollSpeed()
                : 1.0;
        const auto value = [scroll, factor](libinput_pointer_axis axis) {
            if (!libinput_event_pointer_has_axis(scroll, axis))
                return 0.0;
            return libinput_event_pointer_get_scroll_value(scroll, axis) * factor;
        };
        const base::PointF offset(value(LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL),
                             value(LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL));
        const input::Timestamp timestamp =
                timestampFromUsec(libinput_event_pointer_get_time_usec(scroll));
        // A finger scroll of zero is libinput's way of saying the fingers were
        // lifted, which is wl_pointer.axis_stop.
        if (offset.isOrigin())
            m_inputManager->onPointerAxisStop(timestamp);
        else
            m_inputManager->onPointerAxis(timestamp, offset, false);
        break;
    }
    case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
    case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
    case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN: {
        libinput_event_gesture *gesture = libinput_event_get_gesture_event(event);
        m_inputManager->onPointerGestureBegin(
                timestampFromUsec(libinput_event_gesture_get_time_usec(gesture)),
                gestureType(type),
                static_cast<uint32_t>(libinput_event_gesture_get_finger_count(gesture)));
        break;
    }
    case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
    case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE: {
        libinput_event_gesture *gesture = libinput_event_get_gesture_event(event);
        const bool pinch = type == LIBINPUT_EVENT_GESTURE_PINCH_UPDATE;
        m_inputManager->onPointerGestureUpdate(
                timestampFromUsec(libinput_event_gesture_get_time_usec(gesture)),
                gestureType(type),
                base::PointF(libinput_event_gesture_get_dx(gesture),
                        libinput_event_gesture_get_dy(gesture)),
                base::PointF(libinput_event_gesture_get_dx_unaccelerated(gesture),
                        libinput_event_gesture_get_dy_unaccelerated(gesture)),
                pinch ? libinput_event_gesture_get_scale(gesture) : 1.0,
                pinch ? libinput_event_gesture_get_angle_delta(gesture) : 0.0);
        break;
    }
    case LIBINPUT_EVENT_GESTURE_SWIPE_END:
    case LIBINPUT_EVENT_GESTURE_PINCH_END:
    case LIBINPUT_EVENT_GESTURE_HOLD_END: {
        libinput_event_gesture *gesture = libinput_event_get_gesture_event(event);
        m_inputManager->onPointerGestureEnd(
                timestampFromUsec(libinput_event_gesture_get_time_usec(gesture)),
                gestureType(type), libinput_event_gesture_get_cancelled(gesture));
        break;
    }
    default:
        break;
    }
}

} // namespace Lingmo::Backend
