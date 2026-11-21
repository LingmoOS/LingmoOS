// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImageIOHandler>
#include <QImage>
#include <QSize>

struct heif_context;
struct heif_image_handle;

// The primary image, turned and mirrored as the file asks.
class HeifHandler final : public QImageIOHandler
{
public:
    ~HeifHandler() override;

    static bool canRead(QIODevice *device);

    bool canRead() const override;
    bool read(QImage *image) override;

    QVariant option(ImageOption option) const override;
    bool supportsOption(ImageOption option) const override;
    void setOption(ImageOption option, const QVariant &value) override;

private:
    bool open() const;
    static QImage shrink(const uint8_t *pixels, int width, int height, int stride, bool alpha,
                         const QSize &target);

    mutable QByteArray m_data;
    mutable heif_context *m_context = nullptr;
    mutable heif_image_handle *m_handle = nullptr;
    mutable bool m_opened = false;
    QSize m_scaledSize;
    bool m_read = false;
};
