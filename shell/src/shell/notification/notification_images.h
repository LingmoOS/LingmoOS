// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <optional>
#include <string>
#include <vector>

#include "base/dbus_types.h"
#include "ui/bitmap.h"

namespace Lingmo::Shell {

// The spec's (iiibiiay) "image-data" hint, checked but not decoded: the
// checks are cheap and run where the message arrives, the pixels are turned
// into a picture on a worker.
struct ImageHint {
    int32_t width = 0;
    int32_t height = 0;
    int32_t rowStride = 0;
    bool hasAlpha = false;
    std::string pixels;

    // Nothing for another type, a size past 4096 or fewer bytes than the
    // rows need.
    static std::optional<ImageHint> fromVariant(const base::dbus::Variant &hint);
    // Any thread.
    ui::Bitmap toBitmap() const;
};

// The pictures notifications carry, cached as the cards' thumbnails in a
// directory only the user may read. Every function touches the disk and runs
// on the file thread.
namespace notification_images {

// The cards' 44 px thumbnail at up to 3x.
inline constexpr int kThumbnailPixels = 132;

// ~/.cache/lingmo/notification-images
std::string defaultDirectory();
// The square thumbnail cut from the middle of the picture, written as PNG;
// its path, or empty.
std::string save(const std::string &directory, const ui::Bitmap &image);
// Deletes a picture save() wrote; paths the application owns are left alone.
void remove(const std::string &directory, const std::string &path);
// Deletes the pictures save() wrote that none of `keep` names; other files
// are left alone.
void prune(const std::string &directory, const std::vector<std::string> &keep);

} // namespace notification_images

} // namespace Lingmo::Shell

