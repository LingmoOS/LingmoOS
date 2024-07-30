/*
    SPDX-FileCopyrightText: 2011 Geoffry Song <goffrie@gmail.com>

    SPDX-License-Identifier: MIT
*/

#include "barcodeutil_p.h"
#include "code39barcode_p.h"
#include <QChar>

using namespace Prison;

static QList<bool> sequenceForChar(ushort c)
{
    switch (QChar::toUpper(c)) {
    case '0':
        return BarCodeUtil::barSequence("000110100");
    case '1':
        return BarCodeUtil::barSequence("100100001");
    case '2':
        return BarCodeUtil::barSequence("001100001");
    case '3':
        return BarCodeUtil::barSequence("101100000");
    case '4':
        return BarCodeUtil::barSequence("000110001");
    case '5':
        return BarCodeUtil::barSequence("100110000");
    case '6':
        return BarCodeUtil::barSequence("001110000");
    case '7':
        return BarCodeUtil::barSequence("000100101");
    case '8':
        return BarCodeUtil::barSequence("100100100");
    case '9':
        return BarCodeUtil::barSequence("001100100");
    case 'A':
        return BarCodeUtil::barSequence("100001001");
    case 'B':
        return BarCodeUtil::barSequence("001001001");
    case 'C':
        return BarCodeUtil::barSequence("101001000");
    case 'D':
        return BarCodeUtil::barSequence("000011001");
    case 'E':
        return BarCodeUtil::barSequence("100011000");
    case 'F':
        return BarCodeUtil::barSequence("001011000");
    case 'G':
        return BarCodeUtil::barSequence("000001101");
    case 'H':
        return BarCodeUtil::barSequence("100001100");
    case 'I':
        return BarCodeUtil::barSequence("001001100");
    case 'J':
        return BarCodeUtil::barSequence("000011100");
    case 'K':
        return BarCodeUtil::barSequence("100000011");
    case 'L':
        return BarCodeUtil::barSequence("001000011");
    case 'M':
        return BarCodeUtil::barSequence("101000010");
    case 'N':
        return BarCodeUtil::barSequence("000010011");
    case 'O':
        return BarCodeUtil::barSequence("100010010");
    case 'P':
        return BarCodeUtil::barSequence("001010010");
    case 'Q':
        return BarCodeUtil::barSequence("000000111");
    case 'R':
        return BarCodeUtil::barSequence("100000110");
    case 'S':
        return BarCodeUtil::barSequence("001000110");
    case 'T':
        return BarCodeUtil::barSequence("000010110");
    case 'U':
        return BarCodeUtil::barSequence("110000001");
    case 'V':
        return BarCodeUtil::barSequence("011000001");
    case 'W':
        return BarCodeUtil::barSequence("111000000");
    case 'X':
        return BarCodeUtil::barSequence("010010001");
    case 'Y':
        return BarCodeUtil::barSequence("110010000");
    case 'Z':
        return BarCodeUtil::barSequence("011010000");
    case '-':
        return BarCodeUtil::barSequence("010000101");
    case '.':
        return BarCodeUtil::barSequence("110000100");
    case ' ':
        return BarCodeUtil::barSequence("011000100");
    case '$':
        return BarCodeUtil::barSequence("010101000");
    case '/':
        return BarCodeUtil::barSequence("010100010");
    case '+':
        return BarCodeUtil::barSequence("010001010");
    case '%':
        return BarCodeUtil::barSequence("000101010");
    default:
        return QList<bool>(); // unknown character
    }
}

Code39Barcode::Code39Barcode()
    : AbstractBarcodePrivate(Barcode::OneDimension)
{
}
Code39Barcode::~Code39Barcode() = default;

QImage Code39Barcode::paintImage()
{
    QList<bool> barcode;
    // convert text into sequences of wide/narrow bars
    {
        // the guard sequence that goes on each end
        const QList<bool> endSequence = BarCodeUtil::barSequence("010010100");
        barcode += endSequence;
        barcode += false;
        // translate the string
        const auto str = BarCodeUtil::asLatin1ByteArray(m_data);
        for (int i = 0; i < str.size(); i++) {
            QList<bool> b = sequenceForChar(str.at(i));
            if (!b.empty()) {
                barcode += b;
                barcode += false; // add a narrow space between each character
            }
        }
        // ending guard
        barcode += endSequence;
    }

    /*
     calculate integer bar widths that fit inside `size'
     each character has 6 narrow bars and 3 wide bars and there is a narrow bar between characters
     restrictions:
       *) smallWidth * 2 <= largeWidth <= smallWidth * 3
          - in other words, the ratio largeWidth:smallWidth is between 3:1 and 2:1
       *) wide * largeWidth + narrow * smallWidth <= size.width()
          - the barcode has to fit within the given size
     */
    const int wide = barcode.count(true);
    const int narrow = barcode.count(false);
    // wide bar width
    const int largeWidth = 2;
    // narrow bar width
    const int smallWidth = 1;
    Q_ASSERT(largeWidth > smallWidth);

    const int quietZoneWidth = 10 * smallWidth;

    // one line of the result image
    QList<QRgb> line;
    line.reserve(wide * largeWidth + narrow * smallWidth + 2 * quietZoneWidth);
    line.insert(0, quietZoneWidth, m_background.rgba());
    for (int i = 0; i < barcode.size(); i++) {
        const QRgb color = (((i & 1) == 0) ? m_foreground : m_background).rgba(); // alternate between foreground and background color
        const int width = barcode.at(i) ? largeWidth : smallWidth;
        for (int j = 0; j < width; j++) {
            line.append(color);
        }
    }
    line.insert(line.size(), quietZoneWidth, m_background.rgba());

    // build the complete barcode
    QImage ret(line.size(), 1, QImage::Format_ARGB32);
    memcpy(ret.scanLine(0), line.data(), line.size() * sizeof(QRgb));
    return ret;
}
