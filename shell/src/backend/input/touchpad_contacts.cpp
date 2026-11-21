// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "backend/input/touchpad_contacts.h"

#include <linux/input.h>
#include <sys/ioctl.h>

#include <array>

namespace Lingmo::Backend {

namespace {

bool readAbs(int fd, uint32_t code, input_absinfo *info)
{
    return ioctl(fd, EVIOCGABS(code), info) == 0;
}

bool testBit(const uint8_t *bits, unsigned bit)
{
    return bits[bit / 8] & (1u << (bit % 8));
}

} // namespace

std::unique_ptr<TouchpadContacts> TouchpadContacts::create(int fd)
{
    std::array<uint8_t, ABS_MAX / 8 + 1> absBits {};
    if (ioctl(fd, EVIOCGBIT(EV_ABS, absBits.size()), absBits.data()) < 0)
        return nullptr;
    if (!testBit(absBits.data(), ABS_MT_SLOT) || !testBit(absBits.data(), ABS_MT_POSITION_X)
        || !testBit(absBits.data(), ABS_MT_POSITION_Y)
        || !testBit(absBits.data(), ABS_MT_TRACKING_ID))
        return nullptr;

    input_absinfo slot {};
    input_absinfo x {};
    input_absinfo y {};
    if (!readAbs(fd, ABS_MT_SLOT, &slot) || !readAbs(fd, ABS_MT_POSITION_X, &x)
        || !readAbs(fd, ABS_MT_POSITION_Y, &y))
        return nullptr;
    if (x.resolution <= 0 || y.resolution <= 0 || slot.maximum < 0)
        return nullptr;

    std::unique_ptr<TouchpadContacts> contacts(new TouchpadContacts(fd));
    contacts->m_slotCount = slot.maximum + 1;
    contacts->m_minX = x.minimum;
    contacts->m_minY = y.minimum;
    contacts->m_resolutionX = x.resolution;
    contacts->m_resolutionY = y.resolution;
    contacts->m_hasToolType = testBit(absBits.data(), ABS_MT_TOOL_TYPE);
    contacts->m_size = base::SizeF((x.maximum - x.minimum) / double(x.resolution),
                              (y.maximum - y.minimum) / double(y.resolution));
    contacts->m_slots.resize(contacts->m_slotCount);
    return contacts;
}

bool TouchpadContacts::readSlots(uint32_t code, std::vector<int32_t> *values) const
{
    // input_mt_request_layout: the code, then one value per slot.
    values->assign(m_slotCount + 1, 0);
    (*values)[0] = int32_t(code);
    return ioctl(m_fd, EVIOCGMTSLOTS(values->size() * sizeof(int32_t)), values->data()) == 0;
}

base::PointF TouchpadContacts::millimetres(int32_t x, int32_t y) const
{
    return base::PointF((x - m_minX) / m_resolutionX, (y - m_minY) / m_resolutionY);
}

void TouchpadContacts::update(input::Timestamp now)
{
    std::vector<int32_t> ids;
    if (!readSlots(ABS_MT_TRACKING_ID, &ids))
        return;

    std::vector<int32_t> xs;
    std::vector<int32_t> ys;
    for (int i = 0; i < m_slotCount; ++i) {
        Slot &slot = m_slots[i];
        const int32_t id = ids[i + 1];
        if (id == slot.trackingId)
            continue;
        slot.trackingId = id;
        if (id < 0)
            continue;
        if (xs.empty() && (!readSlots(ABS_MT_POSITION_X, &xs) || !readSlots(ABS_MT_POSITION_Y, &ys))) {
            slot.trackingId = -1;
            continue;
        }
        slot.origin = millimetres(xs[i + 1], ys[i + 1]);
        slot.downTime = now;
    }
}

std::vector<input::TouchpadContact> TouchpadContacts::contacts() const
{
    std::vector<input::TouchpadContact> contacts;
    std::vector<int32_t> ids;
    std::vector<int32_t> xs;
    std::vector<int32_t> ys;
    std::vector<int32_t> tools;
    if (!readSlots(ABS_MT_TRACKING_ID, &ids) || !readSlots(ABS_MT_POSITION_X, &xs)
        || !readSlots(ABS_MT_POSITION_Y, &ys))
        return contacts;
    if (m_hasToolType && !readSlots(ABS_MT_TOOL_TYPE, &tools))
        return contacts;

    for (int i = 0; i < m_slotCount; ++i) {
        const Slot &slot = m_slots[i];
        // A finger update() has not seen yet has no touch-down time.
        if (ids[i + 1] < 0 || ids[i + 1] != slot.trackingId)
            continue;
        if (m_hasToolType && tools[i + 1] == MT_TOOL_PALM)
            continue;
        contacts.push_back({slot.origin, millimetres(xs[i + 1], ys[i + 1]), slot.downTime});
    }
    return contacts;
}

bool TouchpadContacts::clicked() const
{
    std::array<uint8_t, KEY_MAX / 8 + 1> keys {};
    if (ioctl(m_fd, EVIOCGKEY(keys.size()), keys.data()) < 0)
        return false;
    return testBit(keys.data(), BTN_LEFT);
}

} // namespace Lingmo::Backend
