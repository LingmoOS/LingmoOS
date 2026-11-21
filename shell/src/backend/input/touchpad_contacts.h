// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_BACKEND_INPUT_TOUCHPAD_CONTACTS_H_
#define LINGMO_BACKEND_INPUT_TOUCHPAD_CONTACTS_H_

#include "base/geometry.h"
#include "input/touchpad_click.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace Lingmo::Backend {

// The fingers on a touchpad, read from the evdev descriptor libinput already
// has open. libinput does not hand out a touchpad's contacts; the kernel's
// multitouch slot state is queried instead, which leaves libinput's event
// stream untouched.
class TouchpadContacts
{
public:
    // nullptr when the device has no multitouch slots or no resolution.
    static std::unique_ptr<TouchpadContacts> create(int fd);

    // Notes fingers that came down since the last call.
    void update(input::Timestamp now);

    // The fingers down now, palms left out; empty if the device is revoked.
    std::vector<input::TouchpadContact> contacts() const;
    // Whether the pad itself is pressed down, which a tap is not.
    bool clicked() const;

    base::SizeF size() const { return m_size; }
    int fd() const { return m_fd; }

private:
    explicit TouchpadContacts(int fd) : m_fd(fd) { }

    bool readSlots(uint32_t code, std::vector<int32_t> *values) const;
    base::PointF millimetres(int32_t x, int32_t y) const;

    int m_fd = -1;
    int m_slotCount = 0;
    int32_t m_minX = 0;
    int32_t m_minY = 0;
    double m_resolutionX = 0;
    double m_resolutionY = 0;
    bool m_hasToolType = false;
    base::SizeF m_size;

    struct Slot {
        int32_t trackingId = -1;
        base::PointF origin;
        input::Timestamp downTime;
    };
    std::vector<Slot> m_slots;
};

} // namespace Lingmo::Backend

#endif // LINGMO_BACKEND_INPUT_TOUCHPAD_CONTACTS_H_
