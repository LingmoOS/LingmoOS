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


#ifndef COMMON_H
#define COMMON_H

/** 1. 需要考虑光盘卸载的情况吗？
*/

/*
 * struct disc_info taken from cdrwtool.h
 *
 * disc status (status):
 *     00b - empty disc
 *     01b - incomplete disc (appendable)
 *     10b - Complete disc
 *     11b - Others
 *
 * State of last session (border)
 *     00b - Empty session
 *     01b - Incomplete session
 *     10b - Reseverd
 *     11b - Complete session (only possible when disc status is complete)
 */
typedef struct disc_info {
    unsigned short length;
#if  __BYTE_ORDER == __BIG_ENDIAN   //大端字节序
    unsigned char reserved1 : 3;
    unsigned char erasable  : 1;
    unsigned char border    : 2;
    unsigned char status    : 2;
#else                               //小端字节序
    unsigned char status    : 2;
    unsigned char border    : 2;
    unsigned char erasable  : 1;
    unsigned char reserved1 : 3;
#endif
     unsigned char n_first_track;
     unsigned char n_sessions_l;
     unsigned char first_track_l;
     unsigned char last_track_l;
#if  __BYTE_ORDER == __BIG_ENDIAN
    unsigned char did_v     : 1;
    unsigned char dbc_v     : 1;
    unsigned char uru       : 1;
    unsigned char reserved2 : 2;
    unsigned char dbit      : 1;
    unsigned char bg_f_status : 1;
#else
    unsigned char bg_f_status : 1;
    unsigned char dbit      : 1;
    unsigned char reserved2 : 2;
    unsigned char uru       : 1;
    unsigned char dbc_v     : 1;
    unsigned char did_v     : 1;
#endif

    /*
     * disc type
     *    00h - CD-DA of CDROM
     *    10h - CD-I
     *    20h - CD-ROM XA
     *    FFh - Undefined
     *    All other values are reserved
     */
    unsigned char disc_type;
    unsigned char n_sessions_m;
    unsigned char first_track_m;
    unsigned char last_track_m;
    unsigned int  disc_id;
    /*
     * Last session lead-in start time
     * if the disc is complete this shall be FF/FF/FF
     */
    unsigned char lead_in_r;
    unsigned char lead_in_m;
    unsigned char lead_in_s;
    unsigned char lead_in_f;

    /*
     * Last possible start time for start of lead-in
     * if the disc is complete this shall be FF/FF/FF
     */
    unsigned char lead_out_r;
    unsigned char lead_out_m;
    unsigned char lead_out_s;
    unsigned char lead_out_f;

    unsigned char disc_bar_code[8];

    //
    // We need to make sure the structure has a proper size
    // I think it needs to be a power of 2.
    // With ide-scsi there is no problem. But without the
    // GPCMD_READ_DISC_INFO command failes if the size is 34
    //

/*     unsigned char reserved3; */
/*     unsigned char opc_entries; */
}disc_info_t;

/*注释为开发中自测的结果，维护者可更新*/
enum MediaType{
    MEDIA_UNKNOWN       = 0x1,
    MEDIA_NONE          = 0x2,

    /*DVD*/
    MEDIA_DVD_ROM       = 0x4,      //ROM表示只读
    MEDIA_DVD_R         = 0x8,      //DVD-R
    MEDIA_DVD_R_SEQ     = 0x10,     //可追加DVD-R
    MEDIA_DVD_R_DL      = 0x20,     //双层DVD-R
    MEDIA_DVD_R_DL_SEQ  = 0x40,
    MEDIA_DVD_R_DL_JUMP = 0x80,
    MEDIA_DVD_RAM       = 0x100,
    MEDIA_DVD_RW        = 0x200,    //DVD-RW
    MEDIA_DVD_RW_OVWR   = 0x400,
    MEDIA_DVD_RW_SEQ    = 0x800,	//不可追加的DVD-RW
    MEDIA_DVD_PLUS_RW   = 0x1000,   //DVD+RW
    MEDIA_DVD_PLUS_R    = 0x2000,	//DVD+R
    MEDIA_DVD_PLUS_R_DL = 0x4000,	//双层DVD+R
    MEDIA_DVD_PLUS_RW_DL= 0x8000,	//双层DVD+RW
    MEDIA_DVD_R_ALL     = MEDIA_DVD_R | MEDIA_DVD_R_SEQ | MEDIA_DVD_R_DL |
                          MEDIA_DVD_R_DL_SEQ | MEDIA_DVD_R_DL_JUMP,     //DVD-R所有 0xE8
    MEDIA_DVD_PLUS_R_ALL= MEDIA_DVD_PLUS_R | MEDIA_DVD_PLUS_R_DL,             //DVD+R所有
    MEDIA_DVD_RW_ALL    = MEDIA_DVD_RW | MEDIA_DVD_RW_OVWR | MEDIA_DVD_RW_SEQ,//DVD-RW所有
    MEDIA_DVD_PLUS_RW_ALL=MEDIA_DVD_PLUS_RW | MEDIA_DVD_PLUS_RW_DL,           //DVD+RW所有
    MEDIA_DVD_ALL       = MEDIA_DVD_R_ALL | MEDIA_DVD_PLUS_R_ALL | MEDIA_DVD_RW_ALL |
                          MEDIA_DVD_PLUS_RW_ALL | MEDIA_DVD_ROM | MEDIA_DVD_RAM, //DVD所有

    /*CD*/
    MEDIA_CD_ROM        = 0x10000,  //只读CD
    MEDIA_CD_R          = 0x20000,
    MEDIA_CD_RW         = 0x40000,
    MEDIA_WRITEABLE_CD  = MEDIA_CD_R | MEDIA_CD_RW,
    MEDIA_CD_ALL        = MEDIA_WRITEABLE_CD | MEDIA_CD_ROM,

    //MEDIA_WRITEABLE_DVD_DL

    /*HD*/
    MEDIA_HD_DVD_ROM    = 0x80000,  //只读HD-DVD
    MEDIA_HD_DVD_R      = 0x100000,
    MEDIA_HD_DVD_RAM    = 0x200000,
    /*BD*/
};

#endif // COMMON_H
