// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef __SIGNAL_TYPE__H__
#define __SIGNAL_TYPE__H__


#define DISK_SIGNAL_TYPE_UMNT 1             //卸载信号
#define DISK_SIGNAL_TYPE_DEL  2             //删除信号
#define DISK_SIGNAL_TYPE_SHOW 3             //显示信号
#define DISK_SIGNAL_TYPE_CREATE_TABLE 4     //清楚分区表信号
#define DISK_SIGNAL_USBUPDATE     5         //usb刷新信号
#define DISK_SIGNAL_TYPE_CLEAR 6            //磁盘或分区清除动作信号
#define DISK_SIGNAL_TYPE_VGCREATE 7         //vg创建信号
#define DISK_SIGNAL_TYPE_PVDELETE 8         //pv删除信号
#define DISK_SIGNAL_TYPE_VGDELETE 9         //vg删除信号
#define DISK_SIGNAL_TYPE_LVDELETE 10        //lv删除信号
#define DISK_SIGNAL_TYPE_DECRYPT 11         //解密
#define DISK_SIGNAL_TYPE_CREATE_FAILED 12   //新建失败

#define DISK_SIGNAL_TYPE_AUTOMNT 100        //自动挂载



#endif
