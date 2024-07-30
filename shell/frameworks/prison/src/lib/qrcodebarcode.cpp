/*
    SPDX-FileCopyrightText: 2010-2016 Sune Vuorela <sune@vuorela.dk>

    SPDX-License-Identifier: MIT
*/

#include "qrcodebarcode_p.h"
#include <qrencode.h>

#include <memory>

using namespace Prison;

using QRcode_ptr = std::unique_ptr<QRcode, decltype(&QRcode_free)>;
using QRinput_ptr = std::unique_ptr<QRinput, decltype(&QRinput_free)>;

QRCodeBarcode::QRCodeBarcode()
    : AbstractBarcodePrivate(Barcode::TwoDimensions)
{
}
QRCodeBarcode::~QRCodeBarcode() = default;

static void qrEncodeString(QRcode_ptr &code, const QByteArray &data)
{
    // try decreasing ECC levels, in case the higher levels result in overflowing the maximum content size
    for (auto ecc : {QR_ECLEVEL_Q, QR_ECLEVEL_M, QR_ECLEVEL_L}) {
        code.reset(QRcode_encodeString(data.constData(), 0, ecc, QR_MODE_8, true));
        if (code) {
            break;
        }
    }
}

QImage QRCodeBarcode::paintImage()
{
    QRcode_ptr code(nullptr, &QRcode_free);
    QRinput_ptr input(nullptr, &QRinput_free);
    if (m_data.typeId() == QMetaType::QString) {
        const QByteArray trimmedData(m_data.toString().trimmed().toUtf8());
        qrEncodeString(code, trimmedData);
    } else {
        const auto b = m_data.toByteArray();
        const auto isReallyBinary = std::any_of(b.begin(), b.end(), [](unsigned char c) {
            return std::iscntrl(c) && !std::isspace(c);
        });
        // prefer encodeString whenever possible, as that selects the more efficient encoding
        // automatically, otherwise we end up needlessly in the binary encoding unconditionally
        if (isReallyBinary) {
            input.reset(QRinput_new());
            QRinput_append(input.get(), QR_MODE_8, b.size(), reinterpret_cast<const uint8_t *>(b.constData()));
            code.reset(QRcode_encodeInput(input.get()));
        } else {
            qrEncodeString(code, b);
        }
    }

    if (!code) {
        return QImage();
    }
    const int margin = 4;
    /*32 bit colors, 8 bit pr byte*/
    uchar *img = new uchar[4 * sizeof(char *) * (2 * margin + code->width) * (2 * margin * +code->width)];
    uchar *p = img;
    QByteArray background;
    background.resize(4);
    background[3] = qAlpha(m_background.rgba());
    background[2] = qRed(m_background.rgba());
    background[1] = qGreen(m_background.rgba());
    background[0] = qBlue(m_background.rgba());
    QByteArray foreground;
    foreground.resize(4);
    foreground[3] = qAlpha(m_foreground.rgba());
    foreground[2] = qRed(m_foreground.rgba());
    foreground[1] = qGreen(m_foreground.rgba());
    foreground[0] = qBlue(m_foreground.rgba());
    for (int row = 0; row < code->width + 2 * margin; row++) {
        for (int col = 0; col < code->width + 2 * margin; col++) {
            if (row < margin || row >= (code->width + margin) || col < margin || col >= (code->width + margin)) {
                /*4 bytes for color*/
                for (int i = 0; i < 4; i++) {
                    *p = background[i];
                    p++;
                }
            } else {
                int c = (row - margin) * code->width + (col - margin);
                /*it is bit 1 that is the interesting bit for us from libqrencode*/
                if (code->data[c] & 1) {
                    /*4 bytes for color*/
                    for (int i = 0; i < 4; i++) {
                        *p = foreground[i];
                        p++;
                    }
                } else {
                    /*4 bytes for color*/
                    for (int i = 0; i < 4; i++) {
                        *p = background[i];
                        p++;
                    }
                }
            }
        }
    }

    const auto result =
        QImage(img, code->width + 2 * margin, code->width + 2 * margin, QImage::Format_ARGB32).copy(); // deep copy as we are going to delete img
    delete[] img;
    return result;
}
