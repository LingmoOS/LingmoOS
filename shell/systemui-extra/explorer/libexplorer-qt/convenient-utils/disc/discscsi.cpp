/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "discscsi.h"
#include <QDebug>
//#include <lingmo-log4qt.h>

DiscScsi::DiscScsi(){

}

DiscScsi::DiscScsi(QString device){
    mDevice = device;
    initMembers();
}

DiscScsi::~DiscScsi(){
    if(-1 != mDeviceHandle)
        ::close(mDeviceHandle);
}

/* 设置cmd，用于从scsi驱动中读取或写入信息
 * @return 返回引用
 */
uchar& DiscScsi::operator[](ulong i){
    if(mSgIo.cmd_len < i+1)
        mSgIo.cmd_len = i+1;
    return mCmd.cmd[i];
}

/*初始化成员变量，全部置0*/
void DiscScsi::initMembers(){
    ::memset(&mSgIo, 0 ,sizeof(struct sg_io_hdr));
    ::memset(&mSense, 0, sizeof(struct request_sense));
    ::memset(&mCmd, 0, sizeof(struct cdrom_generic_command));

    mCmd.quiet = 1;
    mCmd.sense = &mSense;

    mDeviceHandle = -1;
    if(!mDevice.isEmpty())
        mDeviceHandle = ::open(mDevice.toUtf8().constData(), O_NONBLOCK|O_RDONLY);
}

/* 数据传输接口，用于从/dev/srx设备中读取信息，或者向/dev/srx中写入信息
 * @return 成功true,失败false
 */
bool DiscScsi::transport(DiscScsiTransferDirection dir,
                                void *buffer, ulong length){
    int ret;
    if(-1 == mDeviceHandle){
        qDebug()<<"open "<<mDevice<<" failed...";
        return false;
    }

    mSgIo.interface_id = 'S';
    mSgIo.mx_sb_len    = sizeof(struct request_sense);
    mSgIo.cmdp         = mCmd.cmd;
    mSgIo.sbp          = (uchar*)&mSense;
    mSgIo.flags        = SG_FLAG_LUN_INHIBIT | SG_FLAG_DIRECT_IO;
    mSgIo.dxferp       = buffer;
    mSgIo.dxfer_len    = length;
    mSgIo.timeout      = 5000;
    switch(dir){
    case SCSI_READ:
        mSgIo.dxfer_direction = SG_DXFER_FROM_DEV;
        break;
    case SCSI_WRITE:
        mSgIo.dxfer_direction = SG_DXFER_TO_DEV;
        break;
    default:
        mSgIo.dxfer_direction = SG_DXFER_NONE;
        break;
    }

    ret = ::ioctl(mDeviceHandle, SG_IO, &mSgIo);
    if((mSgIo.info & SG_INFO_OK_MASK) != SG_INFO_OK)
        ret = -1;

    if(-1 == ret){
        int errCode =
            (mSense.error_code<<24 & 0xF000) |
            (mSense.sense_key <<16 & 0x0F00) |
            (mSense.asc       <<8  & 0x00F0) |
            (mSense.ascq           & 0x000F);
        return errCode!=0? false:true;
    }

    return true;
}
