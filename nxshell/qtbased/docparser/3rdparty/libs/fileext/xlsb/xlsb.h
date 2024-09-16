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
#ifndef XLSB_H
#define XLSB_H

#include "fileext/fileext.hpp"

#include <cstdint>
#include <string>

namespace xlsb {

class Xlsb : public fileext::FileExtension
{
public:
    enum RecordType
    {
        BRT_ROW_HDR = 0x0,
        BRT_CELL_BLANK = 0x1,
        BRT_CELL_RK = 0x2,
        BRT_CELL_ERROR = 0x3,
        BRT_CELL_BOOL = 0x4,
        BRT_CELL_REAL = 0x5,
        BRT_CELL_ST = 0x6,
        BRT_CELL_ISST = 0x7,
        BRT_FMLA_STRING = 0x8,
        BRT_FMLA_NUM = 0x9,
        BRT_FMLA_BOOL = 0xA,
        BRT_FMLA_ERROR = 0xB,
        BRT_SS_ITEM = 0x13,
        BRT_WS_DIM = 0x94,
        BRT_BEGIN_SST = 0x9F
    };

    struct Record
    {
        uint32_t m_type;
        uint32_t m_size;

        Record()
        {
            m_type = 0;
            m_size = 0;
        }
    };

    Xlsb(const std::string& fileName);

    int convert(bool addStyle, bool extractImages, char mergingMode) override;
private:
    bool readNum(uint32_t &value, int bytes);
    bool readUint8(uint32_t &value);
    bool readUint16(uint32_t &value);
    bool readUint32(uint32_t &value);

    bool readXnum(double &value);
    bool readRkNum(double &value, bool &isInt);

    bool readXlWideStr(std::string &str);
    bool readRichStr(std::string &str);

    bool readRecord(Record &record);
    bool skipBytes(uint32_t bytesToSkip);

    bool parseColumn(std::string &text);
    bool parseRecordForWorksheets(Record &record, std::string &text);
    bool parseSharedStrings();
    bool parseWorkSheets(std::string &text);
private:
    std::vector<std::string> m_sharedStrings;
    ulong m_readed = 0;
    int m_pointer = 0;
    std::string m_buffer;

    uint32_t m_currentColumn = 0;
    uint32_t m_currentRow = 0;
    uint32_t m_rowStart = 0;
    uint32_t m_rowEnd = 0;
    uint32_t m_colStart = 0;
    uint32_t m_colEnd = 0;
};
}

#endif // XLSB_H
