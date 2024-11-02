/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#ifndef LINGMOSDK_SYSTEM_DISK_INFO_H__
#define LINGMOSDK_SYSTEM_DISK_INFO_H__

/**
 * @file libkydiskinfo.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 获取磁盘信息
 * @version 2.2
 * @date 2021-11-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-disk
 * @{
 * 
 */

#ifdef __cplusplus
extern "C" {
#endif

#define FWREV_SIZE    20

typedef enum _kdk_disk_type{
    DISK_TYPE_HDD,      // 机械
    DISK_TYPE_SSD,      // 固态
    DISK_TYPE_HHD,      // 混合
    DISK_TYPE_FLASH,    // 闪存，比如U盘啥的
    DISK_TYPE_NONE      // 分区盘
}kdk_disk_type;

typedef enum _kdk_disk_interface_type{
    DISK_INTERFACE_SATA,
    DISK_INTERFACE_SATA2,
    DISK_INTERFACE_SATA3,
    DISK_INTERFACE_mSATA,
    DISK_INTERFACE_SAS,
    DISK_INTERFACE_M2,
    DISK_INTERFACE_NVMe,
    DISK_INTERFACE_PCIE,
    DISK_INTERFACE_NONE
}kdk_disk_interface_type;

typedef enum _kdk_disk_format{
    DISK_FMT_UNKNOW,
    DISK_FMT_FAT16,
    DISK_FMT_FAT32,
    DISK_FMT_VFAT,
    DISK_FMT_NTFS,
    DISK_FMT_EXT2,
    DISK_FMT_EXT3,
    DISK_FMT_EXT4,
    DISK_FMT_NONE
}kdk_disk_format;

typedef enum _kdk_disk_partition_type{
    DISK_PART_DISK,     // 基础硬盘
    DISK_PART_MASTER,   // 主分区
    DISK_PART_EXT,      // 扩展分区
    DISK_PART_LOGIC,    // 逻辑分区
    DISK_PART_RAID,     // RAID分区
    DISK_PART_UNKNOW
}kdk_disk_partition_type;

/**
 * @struct kdk_diskinfo
 * @brief 磁盘信息结构体
 */
typedef struct _kdk_diskinfo{
    char *name;     // 绝对路径
    char *vendor;   // 制造商
    char *model;    // 型号
    char *serial;   // 序列号

    kdk_disk_type disk_type;  // 磁盘类型，固态 or 机械 or 混合
    kdk_disk_interface_type inter_type;   // 接口类型
    unsigned int rpm;  // 转速，注意固态是没有转速概念的

    unsigned long long sectors_num;   // 扇区数量
    unsigned int sector_size;   // 每个扇区的字节数
    float total_size_MiB;  // 磁盘容量，MiB为单位
    
    unsigned int partition_nums; // 该磁盘/分区下的子分区数量
    char *uuid;     // UUID
    short mounted;   // 是否已挂载
    char *mount_path;   // 挂载路径
    kdk_disk_format format; // 格式化类型
    kdk_disk_partition_type part_type;  // 分区类型
    char *fwrev;    //固件版本信息

}kdk_diskinfo;

/**
 * @brief 获取所有磁盘的列表
 * 
 * @return char**， 每个字符串表示一个磁盘的绝对路径， 结尾以NULL字符表示结束
 */
extern char** kdk_get_disklist();

/**
 * @brief 释放由kdk_get_disklist返回的磁盘列表
 * 
 * @param disklist 由kdk_get_disk_list返回的字符串指针
 */
extern void kdk_free_disklist(char** disklist);


/**
 * @brief 获取指定磁盘的磁盘信息
 * 
 * @param diskname 指定磁盘名称，应当是例如/dev/sda这种绝对路径，或者是disklist中的某个元素
 * @return kdk_diskinfo* 该磁盘的详细信息，具体信息自取
 */
extern kdk_diskinfo *kdk_get_diskinfo(const char *diskname);

/**
 * @brief 获取系统接入所有硬盘
 * 
 * @return char** 硬盘名称
 */
extern char** kdk_get_hard_disk();

/**
 * @brief 获取硬盘大小
 * 
 * @param hardname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return char* 硬盘大小，失败返回NULL。返回的字符串需要被 free 释放
 */
extern char* kdk_get_hard_disk_size(const char *hardname);

/**
 * @brief 获取硬盘固件版本
 * 
 * @param hardname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return char* 硬盘固件版本，失败返回NULL。返回的字符串需要被 free 释放
 */
extern char* kdk_get_hard_fwrev(const char *hardname);

/**
 * @brief 获取硬盘类型
 * 
 * @param hardname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return char* 硬盘类型，失败返回NULL。返回的字符串需要被 free 释放
 */
extern char* kdk_get_hard_type(const char *hardname);

/**
 * @brief 获取硬盘型号
 * 
 * @param hardname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return char* 硬盘型号，失败返回NULL。返回的字符串需要被 free 释放
 */
extern char* kdk_get_hard_model(const char *hardname);

/**
 * @brief 获取硬盘序列号
 * 
 * @param hardname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return char* 硬盘序列号，失败返回NULL。返回的字符串需要被 free 释放
 */
extern char* kdk_get_hard_serial(const char *hardname);

/**
 * @brief 获取硬盘厂商
 * 
 * @param hardname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return char* 硬盘厂商，失败返回NULL。返回的字符串需要被 free 释放
 */
extern char* kdk_get_hard_vendor(const char *hardname);

/**
 * @brief 释放由kdk_get_diskinfo返回的磁盘信息结构体
 * 
 * @param disk 由kdk_get_diskinfo返回的结构体指针
 */
extern void kdk_free_diskinfo(kdk_diskinfo *disk);

/**
 * @brief 获取硬盘运行时长
 * 
 * @param hardname 指定磁盘名称，应当是例如/dev/sda这种绝对路径
 * @return int 硬盘从出厂到现在已运行时长总和
 */
extern int kdk_get_hard_running_time(const char *hardname);

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example lingmosdk-system/src/disk/test/getdiskinfo.c
 * \example lingmosdk-system/src/disk/test/getdisklist.c
 * 
 */
