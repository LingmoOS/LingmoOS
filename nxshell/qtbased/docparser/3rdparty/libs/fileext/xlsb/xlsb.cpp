/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "xlsb.h"
#include "../ooxml/ooxml.hpp"

namespace xlsb {

std::string unichar2Utf8(unsigned int unichar)
{
    //unichars which have only two bytes are represented by 1, 2 or 3 bytes in utf8.
    char out[4];
    if (unichar < 0x80)
    {
        out[0] = (char)unichar;
        return std::string(out, 1);
    }
    else if (unichar < 0x800)
    {
        out[0] = 0xC0 | (unichar >> 6);
        out[1] = 0x80 | (unichar & 0x0000003F);
        return std::string(out, 2);
    }
    else if (unichar < 0x10000)
    {
        out[0] = 0xE0 | (unichar >> 12);
        out[1] = 0x80 | ((unichar & 0x00000FFF) >> 6);
        out[2] = 0x80 | (unichar & 0x0000003F);
        return std::string(out, 3);
    }
    else
    {
        //4-bytes length unichars (code points from 0x10000 to 0x10FFFF) have 4-bytes length characters equivalent in utf8
        unsigned short second_byte = unichar & 0x000003FF;
        unichar = (unichar & 0x03FF0000) >> 6;
        unichar = (unichar | second_byte) + 0x10000;
        out[0] = 0xF0 | (unichar >> 18);
        out[1] = 0x80 | ((unichar & 0x0003FFFF) >> 12);
        out[2] = 0x80 | ((unichar & 0x00000FFF) >> 6);
        out[3] = 0x80 | (unichar & 0x0000003F);
        return std::string(out, 4);
    }
    return "";
}

bool utf16UnicharHas4Bytes(unsigned int ch)
{
    return (ch & 0xFC00) == 0xD800;
}

Xlsb::Xlsb(const std::string &fileName)
    :fileext::FileExtension (fileName)
{
}

int Xlsb::convert(bool addStyle, bool extractImages, char mergingMode)
{
    if (!parseSharedStrings())
        return -1;
    if (!parseWorkSheets(m_text))
        return -1;

    return 0;
}

bool Xlsb::readNum(uint32_t &value, int bytes)
{
    value = 0;
    if (m_buffer.size() - m_pointer < bytes)
        return false;

    for (int i = 0; i < bytes; ++i) {
        u_char uc = m_buffer[m_pointer++];
        value += (uc << (i * 8));
    }
    return true;
}

bool Xlsb::readUint8(uint32_t &value)
{
    return readNum(value, 1);
}

bool Xlsb::readUint16(uint32_t &value)
{
    return readNum(value, 2);
}

bool Xlsb::readUint32(uint32_t &value)
{
    return readNum(value, 4);
}

bool Xlsb::readXnum(double &value)
{
    uint8_t* valPtr = (uint8_t*)&value;
    if (m_buffer.size() - m_pointer < 8)
        return false;

    for (int i = 0; i < 8; ++i)
        valPtr[8 - i] = m_buffer[m_pointer++];

    return true;
}

bool Xlsb::readRkNum(double &value, bool &isInt)
{
    value = 0;
    uint32_t uvalue = 0;
    if (!readNum(uvalue, 4))
        return false;

    bool fx100 = (uvalue & 0x00000001) > 0;
    bool fint = (uvalue & 0x00000002) > 0;
    if (fint) {
        isInt = true;
        int svalue = (int) uvalue;
        svalue /= 4;	//remove 2 last bits
        if (fx100)
            svalue /= 100;
        value = svalue;
    } else {
        isInt = false;
        uvalue = uvalue & 0xFFFFFFFC;
        uint64_t* val_ptr = (uint64_t*)&value;
        uint32_t* uval_ptr = (uint32_t*)&uvalue;
        *val_ptr = (uint64_t)*uval_ptr << 32;
        if (fx100)
            value /= 100.0;
    }

    return true;
}

bool Xlsb::readXlWideStr(std::string &str)
{
    uint32_t strSize = 0;
    if (!readNum(strSize, 4))
        return false;

    if (strSize * 2 > m_buffer.size() - m_pointer)
        return false;

    str.reserve(2 * strSize);
    for (int i = 0; i < strSize; ++i) {
        unsigned int uc = *((unsigned short*)&m_buffer[m_pointer]);
        m_pointer += 2;
        if (uc != 0) {
            if (utf16UnicharHas4Bytes(uc)) {
                if (++i >= strSize) {
                    return false;
                }
                uc = (uc << 16) | *((unsigned short*)&m_buffer[m_pointer]);
                m_pointer += 2;
            }
            str += unichar2Utf8(uc);
        }
    }

    return true;
}

bool Xlsb::readRichStr(std::string &str)
{
    if (m_buffer.size() == m_pointer)
        return false;

    ++m_pointer;
    if (!readXlWideStr(str))
        return false;
    return true;
}

bool Xlsb::readRecord(Xlsb::Record &record)
{
    record.m_type = 0;
    record.m_size = 0;

    if (m_buffer.empty())
        return false;

    for (int i = 0; i < 2 && m_readed <= m_buffer.size(); ++i) {
        uint32_t byte = m_buffer[m_readed++];
        record.m_type += ((byte & 0x7F) << (i * 7));
        if (byte < 128)
            break;
    }

    int a = m_buffer.length();
    for (int i = 0; i < 4 && m_readed <= m_buffer.size(); ++i)	//read record size
    {
        uint32_t byte = m_buffer[m_readed++];
        record.m_size += ((byte & 0x7F) << (i * 7));
        if (byte < 128)
            break;
    }

    return true;
}

bool Xlsb::skipBytes(uint32_t bytesToSkip)
{
    if (bytesToSkip <= m_buffer.size() - m_pointer)
        m_pointer += bytesToSkip;

    return true;
}

bool Xlsb::parseColumn(std::string &text)
{
    uint32_t column;
    if (!readUint32(column))
        return false;
    if (m_currentColumn > 0)
        text += "   ";

    while (column > m_currentColumn) {
        text += "   ";
        ++m_currentColumn;
    }

    if (!skipBytes(4))
        return false;

    m_currentColumn = column + 1;
    return true;
}

bool Xlsb::parseRecordForWorksheets(Xlsb::Record &record, std::string &text)
{
    m_pointer = m_readed;
    switch (record.m_type) {
    case BRT_CELL_BLANK: {
        if (!parseColumn(text))
            return false;
        return true;
    }
    case BRT_FMLA_BOOL:
    case BRT_CELL_BOOL: {
        if (!parseColumn(text))
            return false;

        uint32_t value;
        if (!readUint8(value))
            return false;

        if (value)
            text += "1";
        else
            text += "0";
        return true;
    }
    case BRT_FMLA_NUM:
    case BRT_CELL_REAL: {
        if (!parseColumn(text))
            return false;
        double value;
        if (!readXnum(value))
            return false;

        text += std::to_string(value);
        return true;
    }
    case BRT_FMLA_STRING:
    case BRT_CELL_ST: {
        if (!parseColumn(text))
            return false;

        if (!readXlWideStr(text))
            return false;

        return true;
    }
    case BRT_WS_DIM: {
        bool b = true;
        b = b && readUint32(m_rowStart);
        b = b && readUint32(m_rowEnd);
        b = b && readUint32(m_colStart);
        b = b && readUint32(m_colEnd);
        return b;
    }
    case BRT_ROW_HDR: {
        uint32_t row;
        if (!readUint32(row))
            return false;
        for (int i = m_currentRow; i < row; ++i)
            text += "\n";
        m_currentRow = row;
        m_currentColumn = 0;
        return true;
    }
    case BRT_CELL_RK: {
        if (!parseColumn(text))
            return false;

        double value;
        bool isInt;
        if (!readRkNum(value, isInt))
            return false;

        if (!isInt)
            text += std::to_string(value);
        else
            text += std::to_string((int)value);

        return true;
    }
    case BRT_CELL_ISST: {
        if (!parseColumn(text))
            return false;

        uint32_t index;
        if (!readUint32(index))
            return false;

        if (index < m_sharedStrings.size())
            text += m_sharedStrings[index];
        return true;
    }
    }

    return true;
}

bool Xlsb::parseSharedStrings()
{
    m_readed = 0;
    ooxml::Ooxml::extractFile(m_fileName, "xl/sharedStrings.bin", m_buffer);

    while (m_readed <= m_buffer.size()) {
        Record record;
        if (!readRecord(record))
            return false;

        m_pointer = m_readed;
        if (BRT_SS_ITEM == record.m_type) {
            std::string str;
            if (!readRichStr(str))
                return false;

            m_sharedStrings.emplace_back(str);
        }

        m_readed += record.m_size;
    }

    return true;
}

bool Xlsb::parseWorkSheets(std::string &text)
{
    std::string sheetFileName = "xl/worksheets/sheet1.bin";
    int sheetIndex = 1;
    while (ooxml::Ooxml::exists(m_fileName, sheetFileName)) {
        m_readed = 0;
        m_buffer.clear();
        ooxml::Ooxml::extractFile(m_fileName, sheetFileName, m_buffer);

        while (m_readed < m_buffer.size()) {
            Record record;
            if (!readRecord(record))
                return false;

            if (!parseRecordForWorksheets(record, text))
                return false;

            m_readed += record.m_size;
        }

        ++sheetIndex;
        sheetFileName = "xl/worksheets/sheet" + std::to_string(sheetIndex) + ".bin";
    }

    return true;
}

}
