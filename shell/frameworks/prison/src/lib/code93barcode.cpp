/*
    SPDX-FileCopyrightText: 2011 Geoffry Song <goffrie@gmail.com>

    SPDX-License-Identifier: MIT
*/

#include "barcodeutil_p.h"
#include "code93barcode_p.h"
#include <QChar>

using namespace Prison;

// returns a list of 9 bar colors, where `true' means foreground and `false' means background color
static QList<bool> sequenceForID(int id)
{
    switch (id) {
    case 0:
        return BarCodeUtil::barSequence("100010100"); // 0-9
    case 1:
        return BarCodeUtil::barSequence("101001000");
    case 2:
        return BarCodeUtil::barSequence("101000100");
    case 3:
        return BarCodeUtil::barSequence("101000010");
    case 4:
        return BarCodeUtil::barSequence("100101000");
    case 5:
        return BarCodeUtil::barSequence("100100100");
    case 6:
        return BarCodeUtil::barSequence("100100010");
    case 7:
        return BarCodeUtil::barSequence("101010000");
    case 8:
        return BarCodeUtil::barSequence("100010010");
    case 9:
        return BarCodeUtil::barSequence("100001010");
    case 10:
        return BarCodeUtil::barSequence("110101000"); // A-Z
    case 11:
        return BarCodeUtil::barSequence("110100100");
    case 12:
        return BarCodeUtil::barSequence("110100010");
    case 13:
        return BarCodeUtil::barSequence("110010100");
    case 14:
        return BarCodeUtil::barSequence("110010010");
    case 15:
        return BarCodeUtil::barSequence("110001010");
    case 16:
        return BarCodeUtil::barSequence("101101000");
    case 17:
        return BarCodeUtil::barSequence("101100100");
    case 18:
        return BarCodeUtil::barSequence("101100010");
    case 19:
        return BarCodeUtil::barSequence("100110100");
    case 20:
        return BarCodeUtil::barSequence("100011010");
    case 21:
        return BarCodeUtil::barSequence("101011000");
    case 22:
        return BarCodeUtil::barSequence("101001100");
    case 23:
        return BarCodeUtil::barSequence("101000110");
    case 24:
        return BarCodeUtil::barSequence("100101100");
    case 25:
        return BarCodeUtil::barSequence("100010110");
    case 26:
        return BarCodeUtil::barSequence("110110100");
    case 27:
        return BarCodeUtil::barSequence("110110010");
    case 28:
        return BarCodeUtil::barSequence("110101100");
    case 29:
        return BarCodeUtil::barSequence("110100110");
    case 30:
        return BarCodeUtil::barSequence("110010110");
    case 31:
        return BarCodeUtil::barSequence("110011010");
    case 32:
        return BarCodeUtil::barSequence("101101100");
    case 33:
        return BarCodeUtil::barSequence("101100110");
    case 34:
        return BarCodeUtil::barSequence("100110110");
    case 35:
        return BarCodeUtil::barSequence("100111010");
    case 36:
        return BarCodeUtil::barSequence("100101110"); // -
    case 37:
        return BarCodeUtil::barSequence("111010100"); // .
    case 38:
        return BarCodeUtil::barSequence("111010010"); // space
    case 39:
        return BarCodeUtil::barSequence("111001010"); // $
    case 40:
        return BarCodeUtil::barSequence("101101110"); // /
    case 41:
        return BarCodeUtil::barSequence("101110110"); // +
    case 42:
        return BarCodeUtil::barSequence("110101110"); // $
    case 43:
        return BarCodeUtil::barSequence("100100110"); // ($)
    case 44:
        return BarCodeUtil::barSequence("111011010"); // (%)
    case 45:
        return BarCodeUtil::barSequence("111010110"); // (/)
    case 46:
        return BarCodeUtil::barSequence("100110010"); // (+)
    case 47:
        return BarCodeUtil::barSequence("101011110"); // stop sequence
    default:
        // unknown ID... shouldn't happen
        qWarning("Code93Barcode::sequenceForID called with unknown ID");
        return QList<bool>();
    }
}

// returns the list of IDs that represent a character
static QList<int> codesForChar(uint c)
{
    QList<int> ret;
    switch (c) {
    case 0:
        ret += 44;
        ret += 30;
        break;
    case 1:
        ret += 43;
        ret += 10;
        break;
    case 2:
        ret += 43;
        ret += 11;
        break;
    case 3:
        ret += 43;
        ret += 12;
        break;
    case 4:
        ret += 43;
        ret += 13;
        break;
    case 5:
        ret += 43;
        ret += 14;
        break;
    case 6:
        ret += 43;
        ret += 15;
        break;
    case 7:
        ret += 43;
        ret += 16;
        break;
    case 8:
        ret += 43;
        ret += 17;
        break;
    case 9:
        ret += 43;
        ret += 18;
        break;
    case 10:
        ret += 43;
        ret += 19;
        break;
    case 11:
        ret += 43;
        ret += 20;
        break;
    case 12:
        ret += 43;
        ret += 21;
        break;
    case 13:
        ret += 43;
        ret += 22;
        break;
    case 14:
        ret += 43;
        ret += 23;
        break;
    case 15:
        ret += 43;
        ret += 24;
        break;
    case 16:
        ret += 43;
        ret += 25;
        break;
    case 17:
        ret += 43;
        ret += 26;
        break;
    case 18:
        ret += 43;
        ret += 27;
        break;
    case 19:
        ret += 43;
        ret += 28;
        break;
    case 20:
        ret += 43;
        ret += 29;
        break;
    case 21:
        ret += 43;
        ret += 30;
        break;
    case 22:
        ret += 43;
        ret += 31;
        break;
    case 23:
        ret += 43;
        ret += 32;
        break;
    case 24:
        ret += 43;
        ret += 33;
        break;
    case 25:
        ret += 43;
        ret += 34;
        break;
    case 26:
        ret += 43;
        ret += 35;
        break;
    case 27:
        ret += 44;
        ret += 10;
        break;
    case 28:
        ret += 44;
        ret += 11;
        break;
    case 29:
        ret += 44;
        ret += 12;
        break;
    case 30:
        ret += 44;
        ret += 13;
        break;
    case 31:
        ret += 44;
        ret += 14;
        break;
    case 32:
        ret += 38;
        break;
    case 33:
        ret += 45;
        ret += 10;
        break;
    case 34:
        ret += 45;
        ret += 11;
        break;
    case 35:
        ret += 45;
        ret += 12;
        break;
    case 36:
        ret += 39;
        break;
    case 37:
        ret += 42;
        break;
    case 38:
        ret += 45;
        ret += 15;
        break;
    case 39:
        ret += 45;
        ret += 16;
        break;
    case 40:
        ret += 45;
        ret += 17;
        break;
    case 41:
        ret += 45;
        ret += 18;
        break;
    case 42:
        ret += 45;
        ret += 19;
        break;
    case 43:
        ret += 41;
        break;
    case 44:
        ret += 45;
        ret += 21;
        break;
    case 45:
        ret += 36;
        break;
    case 46:
        ret += 37;
        break;
    case 47:
        ret += 40;
        break;
    case 48:
        ret += 0;
        break;
    case 49:
        ret += 1;
        break;
    case 50:
        ret += 2;
        break;
    case 51:
        ret += 3;
        break;
    case 52:
        ret += 4;
        break;
    case 53:
        ret += 5;
        break;
    case 54:
        ret += 6;
        break;
    case 55:
        ret += 7;
        break;
    case 56:
        ret += 8;
        break;
    case 57:
        ret += 9;
        break;
    case 58:
        ret += 45;
        ret += 35;
        break;
    case 59:
        ret += 44;
        ret += 15;
        break;
    case 60:
        ret += 44;
        ret += 16;
        break;
    case 61:
        ret += 44;
        ret += 17;
        break;
    case 62:
        ret += 44;
        ret += 18;
        break;
    case 63:
        ret += 44;
        ret += 19;
        break;
    case 64:
        ret += 44;
        ret += 31;
        break;
    case 65:
        ret += 10;
        break;
    case 66:
        ret += 11;
        break;
    case 67:
        ret += 12;
        break;
    case 68:
        ret += 13;
        break;
    case 69:
        ret += 14;
        break;
    case 70:
        ret += 15;
        break;
    case 71:
        ret += 16;
        break;
    case 72:
        ret += 17;
        break;
    case 73:
        ret += 18;
        break;
    case 74:
        ret += 19;
        break;
    case 75:
        ret += 20;
        break;
    case 76:
        ret += 21;
        break;
    case 77:
        ret += 22;
        break;
    case 78:
        ret += 23;
        break;
    case 79:
        ret += 24;
        break;
    case 80:
        ret += 25;
        break;
    case 81:
        ret += 26;
        break;
    case 82:
        ret += 27;
        break;
    case 83:
        ret += 28;
        break;
    case 84:
        ret += 29;
        break;
    case 85:
        ret += 30;
        break;
    case 86:
        ret += 31;
        break;
    case 87:
        ret += 32;
        break;
    case 88:
        ret += 33;
        break;
    case 89:
        ret += 34;
        break;
    case 90:
        ret += 35;
        break;
    case 91:
        ret += 44;
        ret += 20;
        break;
    case 92:
        ret += 44;
        ret += 21;
        break;
    case 93:
        ret += 44;
        ret += 22;
        break;
    case 94:
        ret += 44;
        ret += 23;
        break;
    case 95:
        ret += 44;
        ret += 24;
        break;
    case 96:
        ret += 44;
        ret += 32;
        break;
    case 97:
        ret += 46;
        ret += 10;
        break;
    case 98:
        ret += 46;
        ret += 11;
        break;
    case 99:
        ret += 46;
        ret += 12;
        break;
    case 100:
        ret += 46;
        ret += 13;
        break;
    case 101:
        ret += 46;
        ret += 14;
        break;
    case 102:
        ret += 46;
        ret += 15;
        break;
    case 103:
        ret += 46;
        ret += 16;
        break;
    case 104:
        ret += 46;
        ret += 17;
        break;
    case 105:
        ret += 46;
        ret += 18;
        break;
    case 106:
        ret += 46;
        ret += 19;
        break;
    case 107:
        ret += 46;
        ret += 20;
        break;
    case 108:
        ret += 46;
        ret += 21;
        break;
    case 109:
        ret += 46;
        ret += 22;
        break;
    case 110:
        ret += 46;
        ret += 23;
        break;
    case 111:
        ret += 46;
        ret += 24;
        break;
    case 112:
        ret += 46;
        ret += 25;
        break;
    case 113:
        ret += 46;
        ret += 26;
        break;
    case 114:
        ret += 46;
        ret += 27;
        break;
    case 115:
        ret += 46;
        ret += 28;
        break;
    case 116:
        ret += 46;
        ret += 29;
        break;
    case 117:
        ret += 46;
        ret += 30;
        break;
    case 118:
        ret += 46;
        ret += 31;
        break;
    case 119:
        ret += 46;
        ret += 32;
        break;
    case 120:
        ret += 46;
        ret += 33;
        break;
    case 121:
        ret += 46;
        ret += 34;
        break;
    case 122:
        ret += 46;
        ret += 35;
        break;
    case 123:
        ret += 44;
        ret += 25;
        break;
    case 124:
        ret += 44;
        ret += 26;
        break;
    case 125:
        ret += 44;
        ret += 27;
        break;
    case 126:
        ret += 44;
        ret += 28;
        break;
    case 127:
        ret += 44;
        ret += 29;
        break;
    }
    return ret; // return an empty list for a non-ascii character code
}

// calculate a checksum
static int checksum(const QList<int> &codes, int wrap)
{
    int check = 0;
    for (int i = 0; i < codes.size(); i++) {
        // weight goes from 1 to wrap, right-to-left, then repeats
        const int weight = (codes.size() - i - 1) % wrap + 1;
        check += codes.at(i) * weight;
    }
    return check % 47;
}

Code93Barcode::Code93Barcode()
    : AbstractBarcodePrivate(Barcode::OneDimension)
{
}
Code93Barcode::~Code93Barcode() = default;

QImage Code93Barcode::paintImage()
{
    QList<bool> barcode;
    // convert text into sequences of fg/bg bars
    {
        // translate the string into a code sequence
        QList<int> codes;
        const auto str = BarCodeUtil::asLatin1ByteArray(m_data);
        for (int i = 0; i < str.size(); i++) {
            codes += codesForChar(str.at(i));
        }

        // calculate checksums
        codes.append(checksum(codes, 20)); // "C" checksum
        codes.append(checksum(codes, 15)); // "K" checksum: includes previous checksum

        // now generate the barcode
        // the guard sequence that goes on each end
        const QList<bool> endSequence = sequenceForID(47);
        barcode += endSequence;
        // translate codes into bars
        for (int i = 0; i < codes.size(); i++) {
            barcode += sequenceForID(codes.at(i));
        }
        // ending guard
        barcode += endSequence;
        // termination bar
        barcode += true;
    }

    const int barWidth = 1;
    const int quietZoneWidth = 10 * barWidth;

    // build one line of the result image
    QList<QRgb> line;
    line.reserve(barWidth * barcode.size() + 2 * quietZoneWidth);
    line.insert(0, quietZoneWidth, m_background.rgba());
    for (int i = 0; i < barcode.size(); i++) {
        const QRgb color = (barcode.at(i) ? m_foreground : m_background).rgba();
        for (int j = 0; j < barWidth; j++) {
            line.append(color);
        }
    }
    line.insert(line.size(), quietZoneWidth, m_background.rgba());

    // build the complete barcode
    QImage ret(line.size(), 1, QImage::Format_ARGB32);
    memcpy(ret.scanLine(0), line.data(), line.size() * sizeof(QRgb));
    return ret;
}
