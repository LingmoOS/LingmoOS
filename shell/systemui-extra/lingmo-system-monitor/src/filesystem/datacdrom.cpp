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

#include "datacdrom.h"

#include <unistd.h>
#include <fcntl.h>
#include <scsi/sg.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <mntent.h>

#include <QThread>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDebug>
#include <QProcess>

DataCDROM::DataCDROM(QString &blockName, QObject *parent) : QObject(parent)
{
    m_oBlockName = blockName;

    m_iHandle = -1;
    m_oMediumType.clear();

    m_u32MediumRSupport = 0;
    m_u32MediumWSupport = 0;

    m_u32TrackNumber = 0;
    m_u64UsedCapacity = 0;
    m_u64FreeCapacity = 0;
    m_u64Capacity = 0;
}

DataCDROM::~DataCDROM()
{

}

void DataCDROM::getCDROMInfo()
{
    int ret = 0;

    // 0. open device.
    if (!open()) {
        qWarning()<<"open cdrom device failed";
        return;
    }

    // 1. load support mediu types.
    ret = checkRWSupport();
    if (ret < 0) {
        qWarning()<<"check support type failed";
        close();
        return;
    }

    // 2. check have medium or not.
    ret = checkMediumType();
    if (ret < 0) {
        qWarning()<<"check medium type failed";
        close();
        return;
    }

    //3、get track num
    ret = cdRomGetTrackNum();
    if (ret < 0) {
        qWarning()<<"get cdrom track num failed";
        close();
        return;
    }

    //4、get capacity
    cdRomCapacity();

    //5、close device
    close();

    return;
}

bool DataCDROM::open()
{
    if (-1 != m_iHandle) {
        return true;
    }

    m_iHandle = ::open(m_oBlockName.toUtf8().constData(), O_NONBLOCK | O_RDONLY);
    if (m_iHandle < 0) {
        qWarning()<<QString("Open block device %1 faied.").arg(m_oBlockName);
        return false;
    }

    return true;
}

void DataCDROM::close()
{
    if (-1 != m_iHandle) {
        ::close(m_iHandle);
    }

    m_iHandle = -1;
}

bool DataCDROM::execSCSI(const unsigned char *cdb, const int cdbLength,
                         unsigned char *result, const int resultLength)
{
    sg_io_hdr_t sgio;

    if (!open()) {
        return false;
    }

    if (!cdb || !result || resultLength < 1) {
        return false;
    }

    memset(&sgio, 0, sizeof(sg_io_hdr_t));
    sgio.interface_id = 'S';
    sgio.dxfer_direction = SG_DXFER_FROM_DEV;
    sgio.flags = SG_FLAG_LUN_INHIBIT;
    sgio.cmdp = (unsigned char *)cdb;
    sgio.cmd_len = cdbLength;
    sgio.dxferp = result;
    sgio.dxfer_len = resultLength;

    if (0 != ioctl(m_iHandle, SG_IO, &sgio)) {
        qWarning("Do SCSI CDB cmd failed.");
        return false;
    }

    return true;
}

int DataCDROM::checkRWSupport()
{
    unsigned short len = 0;
    unsigned char  cdb[12] = { 0x46, // get configuraion
                               0x00,
                               0x00,
                               0x00, // fearture low
                               0x00, // reserved
                               0x00, // reserved
                               0x00, // reserved
                               0x00, // allocation length high
                               0x0C,
                               0 };
    unsigned char  result[65536] = {0};

    // get header length first.
    if (!execSCSI(cdb, 12, result, 12)) {
        qWarning()<<"scsi get header length failed.";
        return -1;
    }

    if (!(result[8] == cdb[2] && result[9] == cdb[3])) {
        qWarning()<<"scsi check header info failed";
        return -1;
    }

    len = (result[0] << 24) | (result[1] << 16) | (result[2] << 8) | result[3];
    cdb[7] = ((len & 0xFF00) >> 8);
    cdb[8] = (len & 0x00FF);
    // get real result.
    if (!execSCSI(cdb, 12, result, len)) {
        qWarning()<<"scsi get medium read type support";
        return -1;
    }

    len = result[11];
    // loop result, adjust support read medium type.
    for (int i = 0; i < len; ++i) {
        switch ((result[i + 12] << 8) | result[i + 13]) {
            case 0x0008: // CD-ROM
                m_u32MediumRSupport |= MEDIUM_CD_ROM;
                break;
            case 0x0009: // CD-R
                m_u32MediumRSupport |= MEDIUM_CD_R;
                break;
            case 0x000A: // CD-RW
                m_u32MediumRSupport |= MEDIUM_CD_RW;
                break;
            // 000B - 000F reserved.
            case 0x0010: // DVD-ROM
                m_u32MediumRSupport |= MEDIUM_DVD_ROM;
                break;
            case 0x0011: // DVD-R
                m_u32MediumRSupport |= MEDIUM_DVD_R;
                break;
            case 0x0012: // DVD-RAM
                m_u32MediumRSupport |= MEDIUM_DVD_RAM;
                break;
            // define in MMC-5
            case 0x0013: // DVD-RW
                m_u32MediumRSupport |= MEDIUM_DVD_RW_OVERWRITE;
                break;
            case 0x0014:
                m_u32MediumRSupport |= MEDIUM_DVD_RW_SEQ;
                break;
            case 0x0015:
                m_u32MediumRSupport |= MEDIUM_DVD_R_DL_SEQ;
                break;
            case 0x0016:
                m_u32MediumRSupport |= MEDIUM_DVD_R_DL_JUMP;
                break;
            case 0x001A: // DVD+RW
                m_u32MediumRSupport |= MEDIUM_DVD_PLUS_RW;
                break;
            case 0x001B: //DVD+R
                m_u32MediumRSupport |= MEDIUM_DVD_PLUS_R;
                break;
            case 0x002B:
                m_u32MediumRSupport |= MEDIUM_DVD_PLUS_R_DL;
                break;
            case 0x0002:
                // qDebug()<< "Removable CDROM.";
                break;
            default:
                // qDebug()<<"Unkown medium type" << ((result[i + 12] << 8) | result[i + 13]);
                break;
        }
    }

    // load write suppport
    // CD TAO
    cdb[2] = 0x00;
    cdb[3] = 0x2D;
    cdb[7] = 0x00;
    cdb[8] = 0x10;
    if (!execSCSI(cdb, 12, result, 16)) {
        qWarning()<<"scsi get cd tao write support failed.";
        return -1;
    }

    if (result[8] == cdb[2] && result[9] == cdb[3]) {
        m_u32MediumWSupport |= MEDIUM_CD_R;
        if (result[12] & 0x02) {
            m_u32MediumWSupport |= MEDIUM_CD_RW;
        }
    }

    //CD SAO
    cdb[2] = 0x00;
    cdb[3] = 0x2E;
    cdb[7] = 0x00;
    cdb[8] = 0x10;
    if (!execSCSI(cdb, 12, result, 16)) {
        qWarning()<<"scsi get cd sao write medium support failed";
        return -1;
    }

    if (result[8] == cdb[2] && result[9] == cdb[3]){
        m_u32MediumWSupport |= MEDIUM_CD_R;
        if (result[12] & 0x02) {
            m_u32MediumWSupport |= MEDIUM_CD_RW;
        }
    }

    //DVD
    cdb[2] = 0x00;
    cdb[3] = 0x1F;
    cdb[7] = 0x00;
    cdb[8] = 0x10;
    if (!execSCSI(cdb, 12, result, 16)) {
        qWarning()<<"scsi get DVD-ROM read medium support failed";
        return -1;
    }
    if (result[8] == cdb[2] && result[9] == cdb[3]) {
        m_u32MediumRSupport |= MEDIUM_DVD_ROM;
        //qDebug("%1 sure support read mode : DVD-ROM");
    }

    //DVD+RW
    cdb[2] = 0x00; cdb[3] = 0x2A; cdb[7] = 0x00; cdb[8] = 0x10;
    if (!execSCSI(cdb, 12, result, 16)) {
        qWarning()<<"scsi get DVD+RW write medium support failed";
        return -1;
    }
    if (result[8] == cdb[2] && result[9] == cdb[3]){
        if (result[12] & 0x01) {
            m_u32MediumWSupport |= MEDIUM_DVD_PLUS_RW;
        } else {
            m_u32MediumWSupport |= MEDIUM_DVD_ROM;
        }
    }

    //DVD+R
    cdb[2] = 0x00; cdb[3] = 0x2B; cdb[7] = 0x00; cdb[8] = 0x10;
    if (!execSCSI(cdb, 12, result, 16)) {
        qWarning()<<"scsi get DVD+R RW medium support failed";
        return -1;
    }

    if (result[8] == cdb[2] && result[9] == cdb[3]) {
        m_u32MediumRSupport |= MEDIUM_DVD_PLUS_R;
        if (result[12] & 0x01) {
            m_u32MediumWSupport |= MEDIUM_DVD_PLUS_R;
        }
    }

    //DVD-R / DVD-RW
    cdb[2] = 0x00;
    cdb[3] = 0x2F;
    cdb[7] = 0x00;
    cdb[8] = 0x10;
    if (!execSCSI(cdb, 12, result, 16)) {
        qWarning()<<"scsi get DVD-R/DVD-RW RW medium type support failed";
        return -1;
    }
    if (result[8] == cdb[2] && result[9] == cdb[3]) {
        m_u32MediumRSupport |= MEDIUM_DVD_R;
        //qDebug("it sure support read mode : DVD-R");
        m_u32MediumWSupport |= MEDIUM_DVD_R;
        //qDebug("it support write mode : DVD-R");
        m_u32MediumRSupport |= MEDIUM_DVD_RW;
        //qDebug("it sure support read mode : DVD-RW");
        if (result[12] & 0x02) {
            m_u32MediumWSupport |= MEDIUM_DVD_RW;
            //qDebug("%1 support write mode : DVD-RW");
        }
    }

    if (m_u32MediumRSupport & ((MEDIUM_DVD_RW_OVERWRITE | MEDIUM_DVD_RW_SEQ))) {
        m_u32MediumRSupport |= MEDIUM_DVD_RW;
    }

    qDebug()<<"m_u32MediumRSupport"<<m_u32MediumRSupport
           <<"m_u32MediumWSupport"<<m_u32MediumWSupport;
    return 0;
}

int DataCDROM::checkMediumType()
{
    unsigned short profile = 0;
    unsigned int len = 0;

    unsigned char  cdb[12] = {0x46, // operation code
                              0x01, // RT : 0x10b
                              0x00, // fearture high
                              0x00, // fearture low
                              0x00, // reserved
                              0x00, // reserved
                              0x00, // reserved
                              0x00, // allocation length high
                              0x0C, // allocation length low
                              0};
    unsigned char result[65536] = {0};

    m_oMediumType.clear();

    if (!execSCSI(cdb, 12, result, 12)) {
        qWarning()<<"scsi get medium type failed";
        return -1;
    }

    if (!(result[8] == cdb[2] && result[9] == cdb[3])) {
        qWarning()<<"scsi check medium type result failed";
        return -1;
    }

    profile = (result[6] << 8) | result[7];
    switch (profile) {
        case 0x0008: // CD-ROM
            if (m_u32MediumRSupport & MEDIUM_CD_ROM) {
                m_oMediumType = "CD-ROM";
                break;
            }
            qWarning("CDROM cannot support to read CD-ROM medium");
            break;
        case 0x0009: //CD-R
            if (m_u32MediumRSupport & MEDIUM_CD_R) {
                m_oMediumType = "CD-R";
                break;
            }
            qWarning("CDROM cannot support to read CD-R medium");
            break;
        case 0x000A: // CD-RW
            if (m_u32MediumRSupport & MEDIUM_CD_RW) {
                m_oMediumType = "CD-RW";
                break;
            }
            qWarning("CDROM cannot support to read CD-RW medium");
            break;
        case 0x0010: // DVD-ROM
            if (m_u32MediumRSupport & MEDIUM_DVD_ROM) {
                m_oMediumType = "DVD-ROM";
                break;
            }
            qWarning("CDROM cannot support to read DVD-ROM medium");
            break;
        case 0x0011: // DVD-R
            if (m_u32MediumRSupport & MEDIUM_DVD_R) {
                m_oMediumType = "DVD-R";
                break;
            }
            qWarning("CDROM cannot support to read DVD-R medium");
            break;
        case 0x0012: // DVD-RAM
            if (m_u32MediumRSupport & MEDIUM_DVD_RAM) {
                m_oMediumType = "DVD-RAM";
                break;
            }
            qWarning("CDROM cannot support to read DVD-RAM medium");
            break;
        case 0x0013: // DVD-RW
            if (m_u32MediumRSupport & MEDIUM_DVD_RW_OVERWRITE) {
                m_oMediumType = "DVD-RW";
                break;
            }
            qWarning("CDROM cannot support to read DVD-RW(OVERWRITE) medium");
            break;
        case 0x0014: // DVD-RW
            if (m_u32MediumRSupport & MEDIUM_DVD_RW_SEQ) {
                m_oMediumType = "DVD-RW";
                break;
            }
            qWarning("CDROM cannot support to read DVD-RW medium");
            break;
        case 0x001A: // DVD+RW
            if (m_u32MediumRSupport & MEDIUM_DVD_PLUS_RW) {
                m_oMediumType = "DVD+RW";
                break;
            }
            qWarning("CDROM cannot support to read DVD+RW medium");
            break;
        case 0x001B: // DVD+R
            if (m_u32MediumRSupport & MEDIUM_DVD_PLUS_R) {
                m_oMediumType = "DVD+R";
                break;
            }
            qWarning("CDROM cannot support to read DVD+R medium");
            break;
        default:
            qWarning()<<"the type" << profile << "undefined";
            return -1;
    }

    qDebug()<<"the medium type "<< m_oMediumType;
    return 0;
}

int DataCDROM::cdRomGetTrackNum()
{
    unsigned short len = 0;
    unsigned char  cdb[10] = { 0x51,
                               0x00,
                               0x00,
                               0x00,
                               0x00,
                               0x00,
                               0x00,
                               0x00,
                               0x20,
                               0 };
    unsigned char  result[32] = { 0 };

    if (!execSCSI(cdb, 10, result, 32)) {
        qWarning()<<"scsi get cd rom track num failed.";
        return -1;
    }

    len = (result[0] << 8) | result[1];
    if ((len - 32) % 8) {
        qWarning()<<"scsi get wrong reply from cdrom";
        return -1; // len is 32 + 8 * n
    }

#if 0
    switch (result[2] & 0x03)
    {
    case 0x00: // empty
        m_u8Status = MEDIUM_EMPTY;
        //qDebug()<<"empty mediun.";
        break;
    case 0x01: // incomplete
        m_u8Status = MEDIUM_INCOMPLETE;
        //qDebug("incomplete mediun.");
        break;
    case 0x02:
        m_u8Status = MEDIUM_FINALIZED;
        //qDebug("finalized mediun.");
        break;
    case 0x03:
        m_u8Status = MEDIUM_OTHER;
        break;
    default:
        break;
    }
#endif

    m_u32TrackNumber = (((result[11] << 8) & 0xFF00) | result[6]);

    return 0;
}

/*
*需要研究dvd+rw-mediainfo的实现代码，通过scsi获取DVD+RW/DVD-RW的
*容量
*/
void DataCDROM::DVDRWCapacity()
{
    int index = 0;
    QProcess process;
    QStringList deviceName;
    deviceName<<m_oBlockName;

    process.start("dvd+rw-mediainfo", deviceName);
    process.waitForFinished(10000);
    QString result = process.readAllStandardOutput();
    QStringList dvdInfo = result.split("\n");

    for (index = dvdInfo.size() - 1; index > 0; --index){
        if (dvdInfo.at(index).startsWith("READ FORMAT CAPACITIES:")) {
            QStringList formatCapacity = dvdInfo.takeAt(index + 1).split("=");
            qWarning()<<"format capacity is"<<formatCapacity.last();
            if (m_oMediumType.contains("DVD+RW")) {
                m_u64Capacity = formatCapacity.last().toULong();
            }
        }

        if (dvdInfo.at(index).startsWith("READ CAPACITY:")) {
            QStringList readCapacity = dvdInfo.takeAt(index).split("=");
            qWarning()<<"DVD RW read capacity is "<<readCapacity.last();
        }

        if (dvdInfo.at(index).startsWith("READ DVD STRUCTURE")) {
            QStringList dvdStruct = dvdInfo.takeAt(index + 2).split("=");
            qWarning()<<"DVD RW struct capacity is"<<dvdStruct.last();
            if (m_oMediumType.contains("DVD-RW")) {
                m_u64Capacity += dvdStruct.last().toULong();
            }
        }
    }

    return;
}


void DataCDROM::cdRomCapacity()
{
    unsigned char  cdb[10] = { 0x52,
                               0x01,
                               0x00,
                               0x00,
                               0x00,
                               0x00,
                               0x00,
                               0x00,
                               0x28,
                               0 };
    unsigned char  result[40] = { 0 }; // 256 + 12


    // qWarning()<<"medium type"<<m_oMediumType;
    if (!m_oMediumType.isEmpty()
        && (m_oMediumType.contains("DVD+RW") || m_oMediumType.contains("DVD-RW")))
    {
        DVDRWCapacity();
        qWarning()<<"DVD+RW/DVD-RW get capacity";
        return;
    }

    cdb[2] = (m_u32TrackNumber >> 24);
    cdb[3] = (m_u32TrackNumber >> 16);
    cdb[4] = (m_u32TrackNumber >> 8);
    cdb[5] = m_u32TrackNumber;

    if (!execSCSI(cdb, 10, result, 40)) {
        qWarning()<<"scsi get cd rom capacity failed.";
        return;
    }

    m_u64UsedCapacity = ((result[8] << 24) | (result[9] << 16) | (result[10] << 8) | result[11]);
    m_u64UsedCapacity *= 2048;
    m_u64FreeCapacity = ((result[16] << 24) | (result[17] << 16) | (result[18] << 8) | result[19]);
    m_u64FreeCapacity *= 2048;
    m_u64Capacity = m_u64UsedCapacity + m_u64FreeCapacity;

    qDebug()<<"total capacity:"<<m_u64Capacity
             << "used capacity:"<<m_u64UsedCapacity
             << "free capacity:"<<m_u64FreeCapacity;
    return;
}
