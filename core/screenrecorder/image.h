// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINGMO_RECORDER_IMAGE_H
#define LINGMO_RECORDER_IMAGE_H

#include "base/geometry.h"

#include <cstdint>
#include <memory>
#include <vector>

// Premultiplied ARGB32 pixels, one uint32_t each in native byte order. A view
// of memory someone else owns, or pixels of its own that copies share.
struct Image
{
    const uint8_t *bits = nullptr;
    int width = 0;
    int height = 0;
    int stride = 0;
    std::shared_ptr<std::vector<uint8_t>> storage;

    static Image view(const uint8_t *bits, int width, int height, int stride);
    static Image allocate(int width, int height);

    bool isNull() const { return !bits; }
    Lingmo::Base::Size size() const { return {width, height}; }
    Lingmo::Base::Rect rect() const { return Lingmo::Base::Rect(0, 0, width, height); }
    // Only for an image that owns its pixels.
    uint8_t *mutableBits() const { return storage ? storage->data() : nullptr; }
};

// One single-plane DMA-BUF; the capture session keeps the file descriptor.
struct DmaBuf
{
    int fd = -1;
    uint32_t format = 0;
    uint64_t modifier = 0;
    uint32_t offset = 0;
    uint32_t stride = 0;
    Lingmo::Base::Size size;

    bool isValid() const { return fd >= 0; }
};

//! @p image, which is in wl_output @p transform, the way it is shown.
Image upright(const Image &image, uint32_t transform);

#endif // LINGMO_RECORDER_IMAGE_H
