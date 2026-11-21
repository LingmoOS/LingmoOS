// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "jxlhandler.h"

#include <QColorSpace>
#include <QIODevice>
#include <QImage>
#include <QImageReader>
#include <QVariant>

#include <jxl/decode.h>
#include <jxl/resizable_parallel_runner.h>

#include <memory>

using DecoderPointer = std::unique_ptr<JxlDecoder, decltype(&JxlDecoderDestroy)>;

bool JxlHandler::canRead(QIODevice *device)
{
    const QByteArray head = device->peek(32);
    return JxlSignatureCheck(reinterpret_cast<const uint8_t *>(head.constData()), size_t(head.size()))
            >= JXL_SIG_CODESTREAM;
}

bool JxlHandler::canRead() const
{
    if (m_read || !device())
        return false;
    // Once the header is read, the device has been read to its end.
    if (m_infoRead)
        return m_size.isValid();
    return canRead(device());
}

// The size and alpha from the header alone.
bool JxlHandler::readInfo() const
{
    if (m_infoRead)
        return m_size.isValid();
    m_infoRead = true;
    if (!device())
        return false;
    m_data = device()->readAll();

    DecoderPointer decoder(JxlDecoderCreate(nullptr), JxlDecoderDestroy);
    if (!decoder || JxlDecoderSubscribeEvents(decoder.get(), JXL_DEC_BASIC_INFO) != JXL_DEC_SUCCESS)
        return false;
    JxlDecoderSetInput(decoder.get(), reinterpret_cast<const uint8_t *>(m_data.constData()), size_t(m_data.size()));
    JxlDecoderCloseInput(decoder.get());
    if (JxlDecoderProcessInput(decoder.get()) != JXL_DEC_BASIC_INFO)
        return false;
    JxlBasicInfo info;
    if (JxlDecoderGetBasicInfo(decoder.get(), &info) != JXL_DEC_SUCCESS)
        return false;
    // Orientations 5-8 swap the sides; the decoder turns the pixels to match.
    m_size = info.orientation >= JXL_ORIENT_TRANSPOSE ? QSize(int(info.ysize), int(info.xsize))
                                                      : QSize(int(info.xsize), int(info.ysize));
    m_alpha = info.alpha_bits > 0;
    return m_size.isValid();
}

bool JxlHandler::read(QImage *image)
{
    if (m_read || !readInfo())
        return false;
    m_read = true;
    // The decoder allocates its own buffers; a forged size is refused first.
    const qint64 limit = qint64(QImageReader::allocationLimit()) * 1024 * 1024;
    if (limit > 0 && qint64(m_size.width()) * m_size.height() * 4 > limit)
        return false;

    DecoderPointer decoder(JxlDecoderCreate(nullptr), JxlDecoderDestroy);
    std::unique_ptr<void, decltype(&JxlResizableParallelRunnerDestroy)>
            runner(JxlResizableParallelRunnerCreate(nullptr), JxlResizableParallelRunnerDestroy);
    if (!decoder || !runner
            || JxlDecoderSubscribeEvents(decoder.get(), JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING
                                                        | JXL_DEC_FULL_IMAGE) != JXL_DEC_SUCCESS
            || JxlDecoderSetParallelRunner(decoder.get(), JxlResizableParallelRunner, runner.get()) != JXL_DEC_SUCCESS
            // Premultiplied files come out straight too, as Format_RGBA8888 says.
            || JxlDecoderSetUnpremultiplyAlpha(decoder.get(), JXL_TRUE) != JXL_DEC_SUCCESS)
        return false;
    JxlDecoderSetInput(decoder.get(), reinterpret_cast<const uint8_t *>(m_data.constData()), size_t(m_data.size()));
    JxlDecoderCloseInput(decoder.get());

    const JxlPixelFormat format { 4, JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, 0 };
    QImage result;
    QColorSpace colorSpace;
    while (true) {
        const JxlDecoderStatus status = JxlDecoderProcessInput(decoder.get());
        if (status == JXL_DEC_BASIC_INFO) {
            JxlBasicInfo info;
            if (JxlDecoderGetBasicInfo(decoder.get(), &info) != JXL_DEC_SUCCESS)
                return false;
            JxlResizableParallelRunnerSetThreads(runner.get(),
                    JxlResizableParallelRunnerSuggestThreads(info.xsize, info.ysize));
        } else if (status == JXL_DEC_COLOR_ENCODING) {
            size_t size = 0;
            if (JxlDecoderGetICCProfileSize(decoder.get(), JXL_COLOR_PROFILE_TARGET_DATA, &size) == JXL_DEC_SUCCESS
                    && size > 0) {
                QByteArray profile(qsizetype(size), Qt::Uninitialized);
                if (JxlDecoderGetColorAsICCProfile(decoder.get(), JXL_COLOR_PROFILE_TARGET_DATA,
                                                   reinterpret_cast<uint8_t *>(profile.data()), size)
                        == JXL_DEC_SUCCESS)
                    colorSpace = QColorSpace::fromIccProfile(profile);
            }
        } else if (status == JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
            // Through Qt's allocation limit, so a forged size cannot exhaust memory.
            if (!allocateImage(m_size, QImage::Format_RGBA8888, &result))
                return false;
            size_t needed = 0;
            if (JxlDecoderImageOutBufferSize(decoder.get(), &format, &needed) != JXL_DEC_SUCCESS
                    || needed != size_t(result.sizeInBytes())
                    || JxlDecoderSetImageOutBuffer(decoder.get(), &format, result.bits(), needed) != JXL_DEC_SUCCESS)
                return false;
        } else if (status == JXL_DEC_FULL_IMAGE) {
            // Only the first frame of an animation.
            break;
        } else if (status == JXL_DEC_SUCCESS) {
            break;
        } else {
            return false;
        }
    }
    if (result.isNull())
        return false;
    if (colorSpace.isValid())
        result.setColorSpace(colorSpace);
    *image = m_alpha ? result : result.convertToFormat(QImage::Format_RGB32);
    return true;
}

bool JxlHandler::supportsOption(ImageOption option) const
{
    return option == Size || option == ImageFormat;
}

QVariant JxlHandler::option(ImageOption option) const
{
    if (!supportsOption(option) || !readInfo())
        return {};
    if (option == Size)
        return m_size;
    return m_alpha ? QImage::Format_RGBA8888 : QImage::Format_RGB32;
}
