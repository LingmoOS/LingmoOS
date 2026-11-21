// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "heifhandler.h"

#include <QColorSpace>
#include <QIODevice>
#include <QImage>
#include <QImageReader>
#include <QVariant>

#include <libheif/heif.h>

#include <cstring>
#include <vector>

HeifHandler::~HeifHandler()
{
    if (m_handle)
        heif_image_handle_release(m_handle);
    if (m_context)
        heif_context_free(m_context);
}

bool HeifHandler::canRead(QIODevice *device)
{
    const QByteArray head = device->peek(64);
    if (head.size() < 12)
        return false;
    return heif_check_filetype(reinterpret_cast<const uint8_t *>(head.constData()), head.size())
            == heif_filetype_yes_supported;
}

bool HeifHandler::canRead() const
{
    if (m_read || !device())
        return false;
    // Once parsed, the device has been read to its end.
    if (m_opened)
        return m_handle != nullptr;
    if (!canRead(device()))
        return false;
    setFormat(device()->peek(12).mid(8, 4) == "avif" ? QByteArrayLiteral("avif") : QByteArrayLiteral("heif"));
    return true;
}

// Parses the container once; the pixels are decoded only by read().
bool HeifHandler::open() const
{
    if (m_opened)
        return m_handle != nullptr;
    m_opened = true;
    if (!device())
        return false;

    m_data = device()->readAll();
    m_context = heif_context_alloc();
    heif_error error = heif_context_read_from_memory_without_copy(m_context, m_data.constData(),
                                                                   size_t(m_data.size()), nullptr);
    if (error.code != heif_error_Ok)
        return false;
    error = heif_context_get_primary_image_handle(m_context, &m_handle);
    return error.code == heif_error_Ok && m_handle;
}

bool HeifHandler::read(QImage *image)
{
    if (m_read || !open())
        return false;
    m_read = true;

    // The decoder allocates the pixels itself; a forged size is refused first.
    const qint64 limit = qint64(QImageReader::allocationLimit()) * 1024 * 1024;
    if (limit > 0 && qint64(heif_image_handle_get_width(m_handle)) * heif_image_handle_get_height(m_handle) * 4 > limit)
        return false;

    const bool alpha = heif_image_handle_has_alpha_channel(m_handle);
    heif_image *decoded = nullptr;
    const heif_error error = heif_decode_image(m_handle, &decoded, heif_colorspace_RGB,
                                               alpha ? heif_chroma_interleaved_RGBA
                                                     : heif_chroma_interleaved_RGB,
                                               nullptr);
    if (error.code != heif_error_Ok || !decoded)
        return false;

    const int width = heif_image_get_width(decoded, heif_channel_interleaved);
    const int height = heif_image_get_height(decoded, heif_channel_interleaved);
    int stride = 0;
    const uint8_t *pixels = heif_image_get_plane_readonly(decoded, heif_channel_interleaved, &stride);
    if (!pixels || width <= 0 || height <= 0) {
        heif_image_release(decoded);
        return false;
    }

    // Asked for less, averaged straight from the decoder's plane: no copy at full size.
    QImage result;
    if (m_scaledSize.isValid() && m_scaledSize.width() < width && m_scaledSize.height() < height) {
        result = shrink(pixels, width, height, stride, alpha, m_scaledSize);
    } else {
        // Converting to the same format only wraps the decoder's plane, which is released below.
        const QImage wrapped(pixels, width, height, stride,
                             alpha ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
        result = alpha ? wrapped.copy() : wrapped.convertToFormat(QImage::Format_RGB32);
    }
    heif_image_release(decoded);
    if (result.isNull())
        return false;

    const size_t profileSize = heif_image_handle_get_raw_color_profile_size(m_handle);
    if (profileSize > 0) {
        QByteArray profile(qsizetype(profileSize), Qt::Uninitialized);
        if (heif_image_handle_get_raw_color_profile(m_handle, profile.data()).code == heif_error_Ok)
            result.setColorSpace(QColorSpace::fromIccProfile(profile));
    }

    *image = result;
    return true;
}

bool HeifHandler::supportsOption(ImageOption option) const
{
    return option == Size || option == ImageFormat || option == ScaledSize;
}

void HeifHandler::setOption(ImageOption option, const QVariant &value)
{
    if (option == ScaledSize)
        m_scaledSize = value.toSize();
}

// Averages whole blocks of pixels down to near the target, then lets Qt
// smooth the last step; a 100 MP photo never exists twice at full size.
QImage HeifHandler::shrink(const uint8_t *pixels, int width, int height, int stride, bool alpha,
                           const QSize &target)
{
    const int factor = qMax(1, qMin(width / target.width(), height / target.height()));
    const int w = width / factor;
    const int h = height / factor;
    const int channels = alpha ? 4 : 3;
    QImage blocks(w, h, alpha ? QImage::Format_ARGB32 : QImage::Format_RGB32);
    if (blocks.isNull())
        return {};
    const quint32 area = quint32(factor) * quint32(factor);
    std::vector<quint32> sums(size_t(w) * 4);
    for (int y = 0; y < h; ++y) {
        std::fill(sums.begin(), sums.end(), 0);
        for (int dy = 0; dy < factor; ++dy) {
            const uint8_t *row = pixels + size_t(y * factor + dy) * stride;
            for (int x = 0; x < w; ++x) {
                const uint8_t *p = row + size_t(x) * factor * channels;
                quint32 *sum = &sums[size_t(x) * 4];
                for (int dx = 0; dx < factor; ++dx, p += channels) {
                    sum[0] += p[0];
                    sum[1] += p[1];
                    sum[2] += p[2];
                    sum[3] += alpha ? p[3] : 255;
                }
            }
        }
        auto *out = reinterpret_cast<QRgb *>(blocks.scanLine(y));
        for (int x = 0; x < w; ++x) {
            const quint32 *sum = &sums[size_t(x) * 4];
            out[x] = qRgba(int(sum[0] / area), int(sum[1] / area), int(sum[2] / area), int(sum[3] / area));
        }
    }
    if (blocks.size() == target)
        return blocks;
    return blocks.scaled(target, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QVariant HeifHandler::option(ImageOption option) const
{
    if (!supportsOption(option) || !open())
        return {};
    if (option == Size)
        return QSize(heif_image_handle_get_width(m_handle), heif_image_handle_get_height(m_handle));
    return heif_image_handle_has_alpha_channel(m_handle) ? QImage::Format_RGBA8888 : QImage::Format_RGB32;
}
