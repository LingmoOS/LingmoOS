/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "aztecbarcode_p.h"
#include "barcodeutil_p.h"
#include "bitvector_p.h"
#include "prison_debug.h"
#include "reedsolomon_p.h"

#include <QImage>
#include <QPainter>

#include <algorithm>
#include <vector>

// see https://en.wikipedia.org/wiki/Aztec_Code for encoding tables, magic numbers, etc

using namespace Prison;

enum {
    FullMaxSize = 151,
    FullRadius = 74,
    FullGridInterval = 16,
    FullModeMessageSize = 40,
    FullLayerCount = 32,

    CompactMaxSize = 27,
    CompactRadius = 13,
    CompactModeMessageSize = 28,
    CompactLayerCount = 4,
};

AztecBarcode::AztecBarcode()
    : AbstractBarcodePrivate(Barcode::TwoDimensions)
{
}
AztecBarcode::~AztecBarcode() = default;

// encoding properties depending on layer count
struct aztec_layer_property_t {
    uint8_t layer;
    uint8_t codeWordSize;
    uint16_t gf;
};

static const aztec_layer_property_t aztec_layer_properties[] = {{2, 6, ReedSolomon::GF64},
                                                                {8, 8, ReedSolomon::GF256},
                                                                {22, 10, ReedSolomon::GF1024},
                                                                {32, 12, ReedSolomon::GF4096}};

// amounts of bits in an Aztec code depending on layer count
static int aztecCompactDataBits(int layer)
{
    return (88 + 16 * layer) * layer;
}

static int aztecFullDataBits(int layer)
{
    return (112 + 16 * layer) * layer;
}

QImage AztecBarcode::paintImage()
{
    const auto inputData = aztecEncode(BarCodeUtil::asLatin1ByteArray(m_data));

    int layerCount = 0;
    int codewordCount = 0;
    int availableBits = 0;
    int stuffSize = 0; // extra bits added during bit stuffing, which might make us overrun the available size
    bool compactMode = false;
    BitVector encodedData;

    do {
        layerCount = 0;
        // find the smallest layout we can put the data in, while leaving 23% for error correction
        for (auto i = 1; i <= FullLayerCount; ++i) {
            if (aztecFullDataBits(i) * 0.77 > (inputData.size() + stuffSize)) {
                layerCount = i;
                break;
            }
        }
        for (auto i = 1; i <= CompactLayerCount; ++i) {
            if (aztecCompactDataBits(i) * 0.77 > (inputData.size() + stuffSize)) {
                layerCount = i;
                compactMode = true;
                break;
            }
        }
        if (layerCount == 0) {
            qCWarning(Log) << "data too large for Aztec code" << inputData.size();
            return {};
        }

        // determine code word size
        const auto propIt = std::lower_bound(aztec_layer_properties, aztec_layer_properties + 4, layerCount, [](const aztec_layer_property_t &lhs, int rhs) {
            return lhs.layer < rhs;
        });

        // bit stuffing
        auto stuffedData = bitStuffAndPad(inputData, (*propIt).codeWordSize);
        stuffSize = stuffedData.size() - inputData.size();

        availableBits = compactMode ? aztecCompactDataBits(layerCount) : aztecFullDataBits(layerCount);
        codewordCount = stuffedData.size() / (*propIt).codeWordSize;
        const auto rsWordCount = availableBits / (*propIt).codeWordSize - codewordCount;

        // compute error correction
        ReedSolomon rs((*propIt).gf, rsWordCount);
        const auto rsData = rs.encode(stuffedData);

        // pad with leading 0 bits to align to code word boundaries
        encodedData.reserve(availableBits);
        if (int diff = availableBits - stuffedData.size() - rsData.size()) {
            encodedData.appendMSB(0, diff);
        }
        encodedData.append(stuffedData);
        encodedData.append(rsData);

        // try again in the rare case that we overrun the available bits due to bit stuffing and padding
    } while (encodedData.size() > availableBits);

    // determine mode message
    BitVector modeMsg;
    if (compactMode) {
        modeMsg.appendMSB(layerCount - 1, 2);
        modeMsg.appendMSB(codewordCount - 1, 6);
        ReedSolomon rs(ReedSolomon::GF16, 5);
        modeMsg.append(rs.encode(modeMsg));
    } else {
        modeMsg.appendMSB(layerCount - 1, 5);
        modeMsg.appendMSB(codewordCount - 1, 11);
        ReedSolomon rs(ReedSolomon::GF16, 6);
        modeMsg.append(rs.encode(modeMsg));
    }

    // render the result
    if (compactMode) {
        QImage img(CompactMaxSize, CompactMaxSize, QImage::Format_RGB32);
        img.fill(m_background);
        paintCompactGrid(&img);
        paintCompactData(&img, encodedData, layerCount);
        paintCompactModeMessage(&img, modeMsg);
        return cropAndScaleCompact(&img, layerCount);
    } else {
        QImage img(FullMaxSize, FullMaxSize, QImage::Format_RGB32);
        img.fill(m_background);
        paintFullGrid(&img);
        paintFullData(&img, encodedData, layerCount);
        paintFullModeMessage(&img, modeMsg);
        return cropAndScaleFull(&img, layerCount);
    }
}

// code points and encoding modes for each of the first 127 ASCII characters, the rest is encoded in Binary mode
enum Mode {
    NoMode,
    Upper,
    Lower,
    Mixed,
    Punct,
    Digit,
    Binary,
    MODE_COUNT,
    Special,
};

enum SpecialChar {
    Space,
    CarriageReturn,
    Comma,
    Dot,
    SPECIAL_CHAR_COUNT,
};

struct aztec_code_t {
    uint8_t code;
    uint8_t mode;
};

static const aztec_code_t aztec_code_table[] = {
    {0, Binary}, // 0
    {2, Mixed},       {3, Mixed},       {4, Mixed},
    {5, Mixed},       {6, Mixed},       {7, Mixed},
    {8, Mixed}, // 7 BEL \a
    {9, Mixed},       {10, Mixed},      {11, Mixed}, // 10 LF / ^J
    {12, Mixed},      {13, Mixed},      {CarriageReturn, Special}, // 13 CR / ^M - but also 1 Punct
    {14, Binary},     {15, Binary},     {16, Binary},
    {17, Binary},     {18, Binary},     {19, Binary},
    {20, Binary}, // 20 ^T
    {21, Binary},     {22, Binary},     {23, Binary},
    {24, Binary},     {25, Binary},     {26, Binary},
    {15, Mixed}, // 27 ^[
    {16, Mixed},      {17, Mixed},      {18, Mixed}, // 30 ^^
    {19, Mixed},      {Space, Special}, // 32 SP
    {6, Punct},       {7, Punct},       {8, Punct}, // 35 #
    {9, Punct},       {10, Punct},      {11, Punct},
    {12, Punct},      {13, Punct}, // 40 (
    {14, Punct},      {15, Punct},      {16, Punct}, // 43 +
    {Comma, Special}, // 44 ,
    {18, Punct}, // 45 -
    {Dot, Special}, // 46 .
    {20, Punct}, // 47 /
    {2, Digit}, // 48 0
    {3, Digit},       {4, Digit},       {5, Digit},
    {6, Digit},       {7, Digit},       {8, Digit},
    {9, Digit},       {10, Digit},      {11, Digit}, // 57 9
    {21, Punct}, // 58 :
    {22, Punct}, // 59 ;
    {23, Punct}, // 60 <
    {24, Punct},      {25, Punct}, // 62 >
    {26, Punct}, // 63 ?
    {20, Mixed}, // 64 @
    {2, Upper}, // 65 A
    {3, Upper},       {4, Upper},       {5, Upper},
    {6, Upper},       {7, Upper},       {8, Upper},
    {9, Upper},       {10, Upper},      {11, Upper},
    {12, Upper},      {13, Upper},      {14, Upper},
    {15, Upper},      {16, Upper},      {17, Upper},
    {18, Upper},      {19, Upper},      {20, Upper},
    {21, Upper},      {22, Upper},      {23, Upper},
    {24, Upper},      {25, Upper},      {26, Upper},
    {27, Upper}, // 90 Z
    {27, Punct}, // 91 [
    {21, Mixed}, // 92 backslash
    {28, Punct}, // 93 ]
    {22, Mixed}, // 94 ^
    {23, Mixed}, // 95 _
    {24, Mixed}, // 96 `
    {2, Lower}, // 97 a
    {3, Lower},       {4, Lower},       {5, Lower},
    {6, Lower},       {7, Lower},       {8, Lower},
    {9, Lower},       {10, Lower},      {11, Lower},
    {12, Lower},      {13, Lower},      {14, Lower},
    {15, Lower},      {16, Lower},      {17, Lower},
    {18, Lower},      {19, Lower},      {20, Lower},
    {21, Lower},      {22, Lower},      {23, Lower},
    {24, Lower},      {25, Lower},      {26, Lower},
    {27, Lower}, // 122 z
    {29, Punct}, // 123 {
    {25, Mixed}, // 124 |
    {30, Punct}, // 125 }
    {26, Mixed}, // 126 ~
    {27, Mixed} // 127 DEL ^?
};
Q_STATIC_ASSERT(sizeof(aztec_code_table) == 256);

static const struct {
    uint8_t c1;
    uint8_t c2;
    aztec_code_t sym;
} aztec_code_double_symbols[] = {
    {'\r', '\n', {2, Punct}}, // CR LF
    {'.', ' ', {3, Punct}}, // . SP
    {',', ' ', {4, Punct}}, // , SP
    {':', ' ', {5, Punct}} // : SP
};

static const int aztec_code_size[] = {0, 5, 5, 5, 5, 4, 8};
Q_STATIC_ASSERT(sizeof(aztec_code_size) / sizeof(int) == MODE_COUNT);

// codes for ambiguous characters, ie. those that can be encoded in multiple modes
static const aztec_code_t aztec_special_chars[SPECIAL_CHAR_COUNT][MODE_COUNT] = {
    /*   NoMode      Upper      Lower        Mixed        Punct      Digit     Binary  */
    {{0, NoMode}, {1, Upper}, {1, Lower}, {1, Mixed}, {1, Upper}, {1, Digit}, {0, NoMode}}, /* SP */
    {{0, NoMode}, {1, Punct}, {1, Punct}, {14, Mixed}, {1, Punct}, {1, Punct}, {0, NoMode}}, /* CR */
    {{0, NoMode}, {17, Punct}, {17, Punct}, {17, Punct}, {17, Punct}, {12, Digit}, {0, NoMode}}, /* Comma */
    {{0, NoMode}, {19, Punct}, {19, Punct}, {19, Punct}, {19, Punct}, {13, Digit}, {0, NoMode}}, /* Dot */
};

// shift code table, source mode -> target mode
// NoMode indicates shift is not available, use latch instead
static const aztec_code_t aztec_shift_codes[MODE_COUNT - 1][MODE_COUNT - 1] = {
    /*     NoMode         Upper           Lower         Mixed          Punct          Digit   */
    {{0, NoMode}, {0, NoMode}, {0, NoMode}, {0, NoMode}, {0, NoMode}, {0, NoMode}},
    {{0, NoMode}, {0, NoMode}, {0, NoMode}, {0, NoMode}, {0, Punct}, {0, NoMode}},
    {{0, NoMode}, {28, Upper}, {0, NoMode}, {0, NoMode}, {0, Punct}, {0, NoMode}},
    {{0, NoMode}, {0, NoMode}, {0, NoMode}, {0, NoMode}, {0, Punct}, {0, NoMode}},
    {{0, NoMode}, {0, NoMode}, {0, NoMode}, {0, NoMode}, {0, NoMode}, {0, NoMode}},
    {{0, NoMode}, {15, Upper}, {0, NoMode}, {0, NoMode}, {0, Punct}, {0, NoMode}}};

// latch code table, source mode -> target mode
static const aztec_code_t aztec_latch_codes[MODE_COUNT - 1][MODE_COUNT] = {
    /*     NoMode         Upper           Lower         Mixed          Punct          Digit          Binary */
    {{0, NoMode}, {0, NoMode}, {0, NoMode}, {0, NoMode}, {0, NoMode}, {0, NoMode}, {0, NoMode}},
    {{0, NoMode}, {0, NoMode}, {28, Lower}, {29, Mixed}, {29, Mixed}, {30, Digit}, {31, Binary}},
    {{0, NoMode}, {30, Digit}, {0, NoMode}, {29, Mixed}, {29, Mixed}, {30, Digit}, {31, Binary}},
    {{0, NoMode}, {29, Upper}, {28, Lower}, {0, NoMode}, {30, Punct}, {28, Lower}, {31, Binary}},
    {{0, NoMode}, {31, Upper}, {31, Upper}, {31, Upper}, {0, NoMode}, {31, Upper}, {31, Upper}},
    {{0, NoMode}, {14, Upper}, {14, Upper}, {14, Upper}, {14, Upper}, {0, NoMode}, {14, Upper}}};

static Mode aztecCodeLatchTo(Mode currentMode, Mode targetMode, BitVector *v)
{
    if (currentMode == targetMode) {
        return targetMode;
    }
    const auto latchCode = aztec_latch_codes[currentMode][targetMode];
    qCDebug(Log) << "latch" << latchCode.code << aztec_code_size[currentMode];
    v->appendMSB(latchCode.code, aztec_code_size[currentMode]);
    return static_cast<Mode>(latchCode.mode);
}

static void aztecEncodeBinary(std::vector<aztec_code_t>::iterator &it, const std::vector<aztec_code_t>::iterator &end, BitVector *v)
{
    // determine length of the binary sequence
    const auto binEndIt = std::find_if(it, end, [](aztec_code_t sym) {
        return sym.mode != Binary;
    });
    const auto length = std::distance(it, binEndIt);

    // write length field
    qCDebug(Log) << "binary length" << length;
    if (length < 32) {
        v->appendMSB(length, 5);
    } else {
        v->appendMSB(0, 5);
        v->appendMSB(length - 31, 11);
    }

    // write data
    for (; it != binEndIt; ++it) {
        qCDebug(Log) << "binary data" << (*it).code;
        v->appendMSB((*it).code, 8);
    }
}

static void aztecEncodeResolveAmbigious(Mode currentMode, const std::vector<aztec_code_t>::iterator &begin, const std::vector<aztec_code_t>::iterator &end)
{
    Q_ASSERT(begin != end);
    Q_ASSERT(currentMode != (*begin).mode);
    Q_ASSERT((*begin).mode == Special);

    // forward search
    auto it = begin;
    for (; it != end && (*it).mode == Special; ++it) {
        if (aztec_special_chars[(*it).code][currentMode].mode == currentMode) {
            qCDebug(Log) << "special resolved to current mode by forward search";
            (*it).mode = aztec_special_chars[(*it).code][currentMode].mode;
            (*it).code = aztec_special_chars[(*it).code][currentMode].code;
        }
    }

    // backward search
    auto backIt = it;
    while (std::distance(begin, backIt) >= 1 && it != end) {
        --backIt;
        if ((*backIt).mode == Special && aztec_special_chars[(*backIt).code][(*it).mode].mode == (*it).mode) {
            qCDebug(Log) << "special resolved by backward search";
            (*backIt).mode = aztec_special_chars[(*backIt).code][(*it).mode].mode;
            (*backIt).code = aztec_special_chars[(*backIt).code][(*it).mode].code;
        } else {
            break;
        }
    }

    // pick one if we still have an ambiguous symbol
    if ((*begin).mode != Special) {
        return;
    }
    (*begin).mode = aztec_special_chars[(*begin).code][currentMode].mode;
    (*begin).code = aztec_special_chars[(*begin).code][currentMode].code;
    it = begin + 1;
    if (it != end && (*it).mode == Special) {
        aztecEncodeResolveAmbigious(static_cast<Mode>((*begin).mode), it, end);
    }
}

static Mode aztecNextMode(Mode currentMode, const std::vector<aztec_code_t>::iterator &nextSym, const std::vector<aztec_code_t>::iterator &end, bool &tryShift)
{
    Q_ASSERT(currentMode != (*nextSym).mode);
    Q_ASSERT(nextSym != end);
    Q_ASSERT((*nextSym).mode != Special);
    auto it = nextSym;
    ++it;
    if (it != end && (*it).mode == Special) {
        aztecEncodeResolveAmbigious(static_cast<Mode>((*nextSym).mode), it, end);
    }

    if ((it == end || (*it).mode == currentMode) && std::distance(nextSym, it) == 1) {
        tryShift = true;
    }

    qCDebug(Log) << currentMode << (*nextSym).mode << tryShift << std::distance(nextSym, it);
    return static_cast<Mode>((*nextSym).mode);
}

BitVector AztecBarcode::aztecEncode(const QByteArray &data) const
{
    // phase one: translate single and double chars to code points
    std::vector<aztec_code_t> codes;
    codes.reserve(data.size());
    for (int i = 0; i < data.size(); ++i) {
        const uint8_t c1 = data.at(i);
        // double char codes
        if (i < data.size() - 1) {
            const uint8_t c2 = data.at(i + 1);
            bool found = false;
            for (const auto &dblCode : aztec_code_double_symbols) {
                if (dblCode.c1 != c1 || dblCode.c2 != c2) {
                    continue;
                }
                codes.push_back(dblCode.sym);
                ++i;
                found = true;
            }
            if (found) {
                continue;
            }
        }

        // > 127 binary-only range
        if (c1 > 127) {
            codes.push_back({c1, Binary});
            // encodable single ASCII character
        } else {
            codes.push_back(aztec_code_table[c1]);
        }
    }

    // phase two: insert shift and latch codes, translate to bit stream
    Mode currentMode = Upper;
    BitVector result;
    for (auto it = codes.begin(); it != codes.end();) {
        if ((*it).mode == Binary) {
            auto newMode = aztecCodeLatchTo(currentMode, Binary, &result);
            while (newMode != Binary) {
                currentMode = newMode;
                newMode = aztecCodeLatchTo(currentMode, Binary, &result);
            }
            aztecEncodeBinary(it, codes.end(), &result);
            continue;
        }
        // resolve special codes
        if ((*it).mode == Special) {
            aztecEncodeResolveAmbigious(currentMode, it, codes.end());
        }

        // deal with mode changes
        Mode nextMode = currentMode;
        if ((*it).mode != currentMode) {
            bool tryShift = false;
            const auto newMode = aztecNextMode(currentMode, it, codes.end(), tryShift);

            // shift to new mode if desired and possible
            if (tryShift && aztec_shift_codes[currentMode][newMode].mode != NoMode) {
                qCDebug(Log) << "shift" << aztec_shift_codes[currentMode][newMode].code << aztec_code_size[currentMode];
                result.appendMSB(aztec_shift_codes[currentMode][newMode].code, aztec_code_size[currentMode]);
                currentMode = newMode;
            }

            // latch to new mode
            while (currentMode != newMode && newMode != NoMode && currentMode != NoMode) {
                currentMode = aztecCodeLatchTo(currentMode, newMode, &result);
                nextMode = currentMode;
            }
        }

        qCDebug(Log) << (*it).code << aztec_code_size[currentMode];
        result.appendMSB((*it).code, aztec_code_size[currentMode]);
        ++it;

        currentMode = nextMode;
    }

    return result;
}

BitVector AztecBarcode::bitStuffAndPad(const BitVector &input, int codeWordSize) const
{
    BitVector res;
    res.reserve(input.size());

    // bit stuff codewords with leading codeWordSize 0/1 bits
    int i = 0;
    while (i < input.size() - (codeWordSize - 1)) {
        int v = input.valueAtMSB(i, codeWordSize - 1);
        res.appendMSB(v, codeWordSize - 1);
        i += codeWordSize - 1;
        if (v == 0) {
            res.appendBit(true);
        } else if (v == (1 << (codeWordSize - 1)) - 1) {
            res.appendBit(false);
        } else {
            res.appendBit(input.at(i++));
        }
    }
    while (i < input.size()) {
        res.appendBit(input.at(i++));
    }

    // check if we are code word aligned already
    const auto trailingBits = res.size() % codeWordSize;
    if (!trailingBits) { // nothing to pad
        return res;
    }

    // pad with ones to nearest code word boundary
    // last bit has to be zero if we'd otherwise would have all ones though
    bool allOnes = true;
    for (int i = res.size() - trailingBits; i < res.size(); ++i) {
        allOnes &= res.at(i);
    }
    while (res.size() % codeWordSize) {
        if ((res.size() % codeWordSize) == (codeWordSize - 1)) {
            res.appendBit(allOnes ? false : true);
        } else {
            res.appendBit(true);
        }
    }

    return res;
}

void AztecBarcode::paintFullGrid(QImage *img) const
{
    QPainter p(img);
    p.translate(img->width() / 2, img->height() / 2);

    // alignment grids
    QPen pen(m_foreground);
    pen.setDashPattern({1, 1});
    p.setPen(pen);
    for (int i = 0; i < img->width() / 2; i += FullGridInterval) {
        p.drawLine(-i, -FullRadius, -i, FullRadius);
        p.drawLine(i, -FullRadius, i, FullRadius);
        p.drawLine(-FullRadius, -i, FullRadius, -i);
        p.drawLine(-FullRadius, i, FullRadius, i);
    }

    // bullseye background
    p.setBrush(m_background);
    p.setPen(Qt::NoPen);
    p.drawRect(-7, -7, 14, 14);

    // bullseye
    p.setBrush(Qt::NoBrush);
    p.setPen(m_foreground);
    p.drawPoint(0, 0);
    p.drawRect(-2, -2, 4, 4);
    p.drawRect(-4, -4, 8, 8);
    p.drawRect(-6, -6, 12, 12);

    // bullseye orientation marker
    p.drawRect(-7, -7, 1, 1);
    p.drawRect(7, -7, 0, 1);
    p.drawPoint(7, 6);
}

static const int aztecFullLayerOffset[] = {
    //   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26 27 28 29 30 31
    66, 64, 62, 60, 57, 55, 53, 51, 49, 47, 45, 42, 40, 38, 36, 34, 32, 30, 28, 25, 23, 21, 19, 17, 15, 13, 10, 8, 6, 4, 2, 0};

void AztecBarcode::paintFullData(QImage *img, const BitVector &data, int layerCount) const
{
    QPainter p(img);
    p.setPen(m_foreground);

    auto it = data.begin();
    for (int layer = layerCount - 1; layer >= 0; --layer) {
        const auto x1 = aztecFullLayerOffset[layer];
        const auto y1 = x1;
        const auto gridInMiddle = (x1 - FullRadius) % FullGridInterval == 0;
        const auto x2 = gridInMiddle ? x1 + 2 : x1 + 1;
        const auto segmentLength = FullMaxSize - 2 * y1 - 2 - (gridInMiddle ? 1 : 0);

        for (int rotation = 0; rotation < 4; ++rotation) {
            p.resetTransform();
            p.translate(img->width() / 2, img->height() / 2);
            p.rotate(-90 * rotation);
            p.translate(-img->width() / 2, -img->height() / 2);

            for (int i = 0; it != data.end(); ++i, ++it) {
                const auto x = (i % 2 == 0) ? x1 : x2;
                auto y = i / 2 + y1;
                if (((y - FullRadius - 1) % FullGridInterval) == 0) { // skip grid lines
                    ++y;
                    i += 2;
                }
                if (y >= y1 + segmentLength) {
                    break;
                }
                if (*it) {
                    p.drawPoint(x, y);
                }
            }
        }
    }
}

void AztecBarcode::paintFullModeMessage(QImage *img, const BitVector &modeData) const
{
    Q_ASSERT(modeData.size() == FullModeMessageSize);

    QPainter p(img);
    p.setPen(m_foreground);

    auto it = modeData.begin();
    for (int rotation = 0; rotation < 4; ++rotation) {
        p.resetTransform();
        p.translate(img->width() / 2, img->height() / 2);
        p.rotate(90 * rotation);

        for (int i = -5; i <= 5; ++i) {
            if (i == 0) { // skip grid line
                continue;
            }
            if (*it) {
                p.drawPoint(i, -7);
            }
            ++it;
        }
    }
}

QImage AztecBarcode::cropAndScaleFull(QImage *img, int layerCount)
{
    const auto offset = aztecFullLayerOffset[layerCount - 1];
    const auto minSize = FullMaxSize - 2 * offset;

    QImage out(minSize, minSize, img->format());
    QPainter p(&out);
    p.setRenderHint(QPainter::SmoothPixmapTransform, false);
    const auto srcRect = img->rect().adjusted(offset, offset, -offset, -offset);
    p.drawImage(out.rect(), *img, srcRect);
    return out;
}

void AztecBarcode::paintCompactGrid(QImage *img) const
{
    QPainter p(img);
    p.translate(img->width() / 2, img->height() / 2);

    // bullseye
    p.setPen(m_foreground);
    p.drawPoint(0, 0);
    p.drawRect(-2, -2, 4, 4);
    p.drawRect(-4, -4, 8, 8);

    // bullseye orientation marker
    p.drawRect(-5, -5, 1, 1);
    p.drawRect(5, -5, 0, 1);
    p.drawPoint(5, 4);
}

static const int aztecCompactLayerOffset[] = {6, 4, 2, 0};

void AztecBarcode::paintCompactData(QImage *img, const BitVector &data, int layerCount) const
{
    QPainter p(img);
    p.setPen(m_foreground);

    auto it = data.begin();
    for (int layer = layerCount - 1; layer >= 0; --layer) {
        const auto x1 = aztecCompactLayerOffset[layer];
        const auto y1 = x1;
        const auto x2 = x1 + 1;
        const auto segmentLength = CompactMaxSize - 2 * y1 - 2;

        for (int rotation = 0; rotation < 4; ++rotation) {
            p.resetTransform();
            p.translate(img->width() / 2, img->height() / 2);
            p.rotate(-90 * rotation);
            p.translate(-img->width() / 2, -img->height() / 2);

            for (int i = 0; it != data.end(); ++i, ++it) {
                const auto x = (i % 2 == 0) ? x1 : x2;
                auto y = i / 2 + y1;
                if (y >= y1 + segmentLength) {
                    break;
                }
                if (*it) {
                    p.drawPoint(x, y);
                }
            }
        }
    }
}

void AztecBarcode::paintCompactModeMessage(QImage *img, const BitVector &modeData) const
{
    Q_ASSERT(modeData.size() == CompactModeMessageSize);

    QPainter p(img);
    p.setPen(m_foreground);

    auto it = modeData.begin();
    for (int rotation = 0; rotation < 4; ++rotation) {
        p.resetTransform();
        p.translate(img->width() / 2, img->height() / 2);
        p.rotate(90 * rotation);

        for (int i = -3; i <= 3; ++i) {
            if (*it) {
                p.drawPoint(i, -5);
            }
            ++it;
        }
    }
}

QImage AztecBarcode::cropAndScaleCompact(QImage *img, int layerCount)
{
    const auto offset = aztecCompactLayerOffset[layerCount - 1];
    const auto minSize = CompactMaxSize - 2 * offset;

    QImage out(minSize, minSize, img->format());
    QPainter p(&out);
    p.setRenderHint(QPainter::SmoothPixmapTransform, false);
    const auto srcRect = img->rect().adjusted(offset, offset, -offset, -offset);
    p.drawImage(out.rect(), *img, srcRect);
    return out;
}
