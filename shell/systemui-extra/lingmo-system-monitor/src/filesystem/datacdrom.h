/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: renpeijia <renpeijia@kylinos.cn>
 *
 */

#ifndef DATACDROM_H
#define DATACDROM_H

#include <QObject>

enum MEDIUM_TYPE
{
    MEDIUM_UNKOWN = 0x0000,
    MEDIUM_CD_ROM = 0x0001,
    MEDIUM_CD_R = 0x0002,
    MEDIUM_CD_RW = 0x0004,
    MEDIUM_DVD_ROM = 0x0008,
    MEDIUM_DVD_R = 0x0010,
    MEDIUM_DVD_RAM = 0x0020,
    MEDIUM_DVD_R_DL_SEQ = 0x0040,
    MEDIUM_DVD_R_DL_JUMP = 0x0080,
    MEDIUM_DVD_PLUS_RW = 0x0100,
    MEDIUM_DVD_PLUS_R = 0x0101,
    MEDIUM_DVD_PLUS_RW_DL = 0x0102,
    MEDIUM_DVD_PLUS_R_DL = 0x0104,
    MEDIUM_DVD_RW_OVERWRITE = 0x0108,
    MEDIUM_DVD_RW_SEQ = 0x0110,
    MEDIUM_CD_RW_S0 = 0x0120, // in MMC-5 CD-RW has 8 subtypes.
    MEDIUM_CD_RW_S1 = 0x0140,
    MEDIUM_CD_RW_S2 = 0x0180,
    MEDIUM_CD_RW_S3 = 0x0200,
    MEDIUM_CD_RW_S4 = 0x0201,
    MEDIUM_CD_RW_S5 = 0x0202,
    MEDIUM_CD_RW_S6 = 0x0204,
    MEDIUM_CD_RW_S7 = 0x0208,
    MEDIUM_DVD_RW = 0x0210
};

enum MEDIUM_STATUS
{
    MEDIUM_EMPTY = 0,
    MEDIUM_INCOMPLETE,
    MEDIUM_FINALIZED,
    MEDIUM_OTHER
};

enum BURN_MODE
{
    BURN_CD_TAO = 0x01,
    BURN_CD_SAO = 0x02,
    BURN_CD_DAO = 0x04 // close session/track when TAO over.
};

class DataCDROM : public QObject
{
    Q_OBJECT

public:
    explicit DataCDROM(QString &blockName, QObject *parent = nullptr);
    ~DataCDROM();
public:
    void getCDROMInfo();
    unsigned long getCDROMCapacity() {
        return m_u64Capacity;
    }

    unsigned long getCDROMUsedCapacity() {
        return m_u64UsedCapacity;
    }

private:
    bool open();
    void close();
    bool execSCSI(const unsigned char *, const int, unsigned char *, const int);
    int  checkRWSupport();
    int  checkMediumType();
    int  cdRomGetTrackNum();
    void DVDRWCapacity();
    void cdRomCapacity();

private:
    int            m_iHandle;
    unsigned int   m_u32MediumRSupport;
    unsigned int   m_u32MediumWSupport;

    QString        m_oBlockName;
    QString        m_oMediumType;

    unsigned int     m_u32TrackNumber;
    unsigned long    m_u64UsedCapacity;
    unsigned long    m_u64FreeCapacity;
    unsigned long    m_u64Capacity;
};

#endif // DATACDROM_H
