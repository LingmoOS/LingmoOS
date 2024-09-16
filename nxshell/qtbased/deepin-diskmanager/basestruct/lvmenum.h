// Copyright (C) 2022 ~ 2022 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef LVMENUM_H
#define LVMENUM_H

//new by liuwh 2022/3/11
/**
 * @enum PVFlag
 * @brief pv 标志
 */
enum LVMFlag {
    LVM_FLAG_NOT_PV = 0, //非pv设备
    LVM_FLAG_JOIN_VG = 1, //加入vg
    LVM_FLAG_NOT_JOIN_VG = 2, //未加入vg
};



//new by liuwh 2022/1/17
/**
 * @enum LVMPVAction
 * @brief lvm 动作
 */
enum LVMAction {
    LVM_ACT_UNkNOW = 0,             //未知动作

    LVM_ACT_PV = 100,
    LVM_ACT_ADDPV,                  //添加PV
    LVM_ACT_DELETEPV,               //删除PV
    LVM_ACT_PVMOVE,                 //移动PV 暂时未用

    LVM_ACT_LV = 200,
    LVM_ACT_LV_CREATE,              //lv 创建
    LVM_ACT_LV_DELETE,              //lv 删除
    LVM_ACT_LV_EXTEND,              //lv 扩展
    LVM_ACT_LV_REDUCE,              //lv 缩小
    LVM_ACT_LV_MOUNT,               //lv 挂载
    LVM_ACT_LV_UMOUNT,              //lv 卸载
    LVM_ACT_LV_FAST_CLEAR,          //lv 快速清除
    LVM_ACT_LV_SECURE_CLEAR,        //lv 安全清除


    LVM_ACT_VG = 300,
    LVM_ACT_VG_CREATE,              //vg 创建
    LVM_ACT_VG_DELETE,              //vg 删除
    LVM_ACT_VG_EXTEND,              //vg 扩展
    LVM_ACT_VG_REDUCE               //vg 缩小
};



//new by liuwh 2022/3/4
/**
 * @enum LVMError
 * @brief lvm 错误类型
 */
enum LVMError {
    LVM_ERR_NORMAL = 0,             //正常
    LVM_ERR_NO_CMD_SUPPORT,         //外部命令不支持
    LVM_ERR_IO_ERROR,               //输入输出错误

    LVM_ERR_PV = 100,
    LVM_ERR_PV_CREATE_FAILED,       //pv 创建失败
    LVM_ERR_PV_DELETE_FAILED,       //pv 删除失败
    LVM_ERR_PV_NO_EXISTS,           //pv 不存在
    LVM_ERR_PV_ARGUMENT,            //pv 参数错误
    LVM_ERR_PV_MOVE_FAILED,         //pv 移动错误

    LVM_ERR_LV = 200,               //lv 错误
    LVM_ERR_LV_CREATE_FAILED,       //lv 创建失败
    LVM_ERR_LV_ARGUMENT,            //lv 创建参数错误
    LVM_ERR_LV_ALREADY_EXISTS,      //lv 已经存在
    LVM_ERR_LV_NO_EXISTS,           //lv 不存在
    LVM_ERR_LV_DELETE_FAILED,       //lv 删除失败
    LVM_ERR_LV_CREATE_FS_FAILED,    //lv 文件系统创建失败
    LVM_ERR_LV_EXTEND_FS_FAILED,    //lv 文件系统不支持扩大
    LVM_ERR_LV_REDUCE_FS_FAILED,    //lv 文件系统不支持缩小
    LVM_ERR_LV_RESIZE_FS_NO_SUPPORT,//lv 文件系统不支持调整
    LVM_ERR_LV_EXTEND_FAILED,       //lv 扩展失败
    LVM_ERR_LV_REDUCE_FAILED,       //lv 缩小失败
    LVM_ERR_LV_IN_USED,             //lv 被使用
    //LVM_ERR_LV_MOUNT,             //lv 已经挂载 

    LVM_ERR_VG = 300,               //vg 错误
    LVM_ERR_VG_CREATE_FAILED,       //vg 创建失败
    LVM_ERR_VG_ARGUMENT,            //vg 创建参数错误
    LVM_ERR_VG_NO_EXISTS,           //vg 不存在
    LVM_ERR_VG_ALREADY_EXISTS,      //vg 重名
    LVM_ERR_VG_DELETE_FAILED,       //vg 删除失败
    LVM_ERR_VG_EXTEND_FAILED,       //vg 扩展失败
    LVM_ERR_VG_REDUCE_FAILED,       //vg 缩小失败
    LVM_ERR_VG_IN_USED              //vg 被使用
};



//new by liuwh 2022/1/20
/**
 * @enum LVM_CMD_Support
 * @brief lvm 命令支持
 */

struct LVM_CMD_Support {
    enum LVM_Support {
        NONE = 0, //not support
        EXTERNAL = 1 //using external func
    };

    LVM_Support LVM_CMD_lvchange = NONE;
    LVM_Support LVM_CMD_lvconvert = NONE;
    LVM_Support LVM_CMD_lvcreate = NONE;
    LVM_Support LVM_CMD_lvdisplay = NONE;
    LVM_Support LVM_CMD_lvextend = NONE;
    LVM_Support LVM_CMD_lvreduce = NONE;
    LVM_Support LVM_CMD_lvremove = NONE;
    LVM_Support LVM_CMD_lvrename = NONE;
    LVM_Support LVM_CMD_lvresize = NONE;
    LVM_Support LVM_CMD_lvs = NONE;
    LVM_Support LVM_CMD_lvscan = NONE;
    LVM_Support LVM_CMD_pvchange = NONE;
    LVM_Support LVM_CMD_pvck = NONE;
    LVM_Support LVM_CMD_pvcreate = NONE;
    LVM_Support LVM_CMD_pvdisplay = NONE;
    LVM_Support LVM_CMD_pvmove = NONE;
    LVM_Support LVM_CMD_pvremove = NONE;
    LVM_Support LVM_CMD_pvresize = NONE;
    LVM_Support LVM_CMD_pvs = NONE;
    LVM_Support LVM_CMD_pvscan = NONE;
    LVM_Support LVM_CMD_vgcfgbackup = NONE;
    LVM_Support LVM_CMD_vgcfgrestore = NONE;
    LVM_Support LVM_CMD_vgchange = NONE;
    LVM_Support LVM_CMD_vgck = NONE;
    LVM_Support LVM_CMD_vgconvert = NONE;
    LVM_Support LVM_CMD_vgcreate = NONE;
    LVM_Support LVM_CMD_vgdisplay = NONE;
    LVM_Support LVM_CMD_vgexport = NONE;
    LVM_Support LVM_CMD_vgextend = NONE;
    LVM_Support LVM_CMD_vgimport = NONE;
    LVM_Support LVM_CMD_vgimportclone = NONE;
    LVM_Support LVM_CMD_vgmerge = NONE;
    LVM_Support LVM_CMD_vgmknodes = NONE;
    LVM_Support LVM_CMD_vgreduce = NONE;
    LVM_Support LVM_CMD_vgremove = NONE;
    LVM_Support LVM_CMD_vgrename = NONE;
    LVM_Support LVM_CMD_vgs = NONE;
    LVM_Support LVM_CMD_vgscan = NONE;
    LVM_Support LVM_CMD_vgsplit = NONE;
};


#endif // LVMENUM_H
