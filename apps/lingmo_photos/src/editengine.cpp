// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "editengine.h"

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <algorithm>
#include <cctype>
#include <cmath>

namespace Photos {
namespace {
void fail(std::string* error, const std::string& message)
{
    if (error)
        *error = message;
}
}

bool saveEditedCopy(const std::string& source, const std::string& target, const EditParams& params, std::string* error)
{
    GError* loadError = nullptr;
    GdkPixbuf* loaded = gdk_pixbuf_new_from_file(source.c_str(), &loadError);
    if (!loaded) {
        fail(error, loadError && loadError->message ? loadError->message : "Unable to decode image");
        if (loadError)
            g_error_free(loadError);
        return false;
    }

    const int width = gdk_pixbuf_get_width(loaded);
    const int height = gdk_pixbuf_get_height(loaded);
    const int channels = gdk_pixbuf_get_n_channels(loaded);
    const int rowStride = gdk_pixbuf_get_rowstride(loaded);
    guchar* pixels = gdk_pixbuf_get_pixels(loaded);
    const double contrast = 1.0 + params.contrast;
    const double saturation = 1.0 + params.saturation;
    for (int y = 0; y < height; ++y) {
        guchar* row = pixels + y * rowStride;
        for (int x = 0; x < width; ++x) {
            guchar* pixel = row + x * channels;
            double r = pixel[0] / 255.0;
            double g = pixel[1] / 255.0;
            double b = pixel[2] / 255.0;
            const double luminance = 0.2126 * r + 0.7152 * g + 0.0722 * b;
            r = luminance + (r - luminance) * saturation;
            g = luminance + (g - luminance) * saturation;
            b = luminance + (b - luminance) * saturation;
            r = (r - .5) * contrast + .5 + params.brightness * .35 + params.temperature * .10;
            g = (g - .5) * contrast + .5 + params.brightness * .35;
            b = (b - .5) * contrast + .5 + params.brightness * .35 - params.temperature * .10;
            pixel[0] = static_cast<guchar>(std::clamp(r, 0.0, 1.0) * 255.0 + .5);
            pixel[1] = static_cast<guchar>(std::clamp(g, 0.0, 1.0) * 255.0 + .5);
            pixel[2] = static_cast<guchar>(std::clamp(b, 0.0, 1.0) * 255.0 + .5);
        }
    }

    GdkPixbuf* transformed = loaded;
    const int normalizedRotation = ((params.rotation % 4) + 4) % 4;
    if (normalizedRotation != 0) {
        transformed = gdk_pixbuf_rotate_simple(loaded, static_cast<GdkPixbufRotation>(normalizedRotation * 90));
        g_object_unref(loaded);
        if (!transformed) {
            fail(error, "Unable to rotate image");
            return false;
        }
    }

    const auto extension = target.find_last_of('.');
    std::string format = extension == std::string::npos ? "png" : target.substr(extension + 1);
    std::transform(format.begin(), format.end(), format.begin(), [](unsigned char value) { return char(std::tolower(value)); });
    if (format == "jpg")
        format = "jpeg";
    if (format != "png" && format != "jpeg" && format != "webp")
        format = "png";
    GError* saveError = nullptr;
    const bool saved = gdk_pixbuf_save(transformed, target.c_str(), format.c_str(), &saveError, "quality", "95", nullptr);
    if (!saved)
        fail(error, saveError && saveError->message ? saveError->message : "Unable to save edited copy");
    if (saveError)
        g_error_free(saveError);
    g_object_unref(transformed);
    return saved;
}

} // namespace Photos
