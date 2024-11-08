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

#include "libkydiskinfo.h"
#include "diskmarcos.h"
#include <libkylog.h>
#include <kerr.h>
#include <lingmosdk/lingmosdk-base/sdkmarcos.h>
#include <blkid/blkid.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <libmount/libmount.h>
#include <unistd.h>
#include <libudev.h>
#include <lingmosdk/lingmosdk-base/cstring-extension.h>
#include "hd.h"
#include <dbus-1.0/dbus/dbus.h>

static int get_disk_fs(kdk_diskinfo *di)
{
    return 0;
}

static inline long test_valid_offset(int fd, off_t offset)
{
    char ch;

	if (lseek (fd, offset, 0) < 0)
		return 0;
	if (read (fd, &ch, 1) < 1)
		return 0;
	return 1;
}

static off_t find_disk_size(int fd)
{
    uintmax_t high, low = 0;

	for (high = 1024; test_valid_offset (fd, high); ) {
		if (high == UINTMAX_MAX)
			return -1;

		low = high;

		if (high >= UINTMAX_MAX/2)
			high = UINTMAX_MAX;
		else
			high *= 2;
	}

	while (low < high - 1)
	{
		uintmax_t mid = (low + high) / 2;

		if (test_valid_offset (fd, mid))
			low = mid;
		else
			high = mid;
	}
	test_valid_offset (fd, 0);
	return (low + 1);
}

static int get_disk_size(int fd, unsigned long long *bytes)
{
#ifdef BLKGETSIZE64
	if (ioctl(fd, BLKGETSIZE64, bytes) >= 0)
		return 0;
#elif defined BLKGETSIZE
	{
		unsigned long size;

		if (ioctl(fd, BLKGETSIZE, &size) >= 0) {
			*bytes = ((unsigned long long)size << 9);
			return 0;
		}
	}
#endif // BLKGETSIZE
    {
		struct stat st;

		if (fstat(fd, &st) == 0 && S_ISREG(st.st_mode)) {
			*bytes = st.st_size;
			return 0;
		}
		if (!S_ISBLK(st.st_mode))
			return -1;
	}
    *bytes   = find_disk_size(fd);
    return 0;
}

static int get_disk_space(int fd, kdk_diskinfo *di)
{
#ifdef  __linux__

    // 获取单个扇区大小
#ifdef  BLKSSZGET
    if (ioctl(fd, BLKSSZGET, &di->sector_size) < 0)
    {
        klog_err("ioctl 执行失败：%s\n", strerror(errno));
        perror("ioctl: BLKSSZGET ");
        return -1;
    }
#else
    di->sector_size = DEFAULT_SECTOR_SIZE;
#endif  // BLKSSZGET

    // 获取扇区数量
    unsigned long long bytes;
    if (get_disk_size(fd, &bytes))
    {
        klog_err("磁盘容量获取失败\n");
        klog_err("Unable to get disk size.\n");
        return -1;
    }
    di->sectors_num = (bytes >> 9);
    di->total_size_MiB = (float)(di->sector_size * di->sectors_num) / 1024.00 / 1024.00;

#endif  // __linux__


    return 0;
}

static int get_disk_identifier(int fd, kdk_diskinfo *di)
{
    __uint16_t hd[256];

    if (ioctl(fd, HDIO_GET_IDENTITY, hd))
    {
        if (errno == -ENOMSG)
        {
            klog_warning("该磁盘[%s]未写入标识信息\n", di->name);
            return -1;
        }
        klog_warning("ioctl 执行失败：%s\n", strerror(errno));
        return -1;
    }
    
    char *model = strndup((char*)&hd[27], 40);
    char *fwrev = strndup((char*)&hd[23], 8);   // 固件版本
    char *serno = strndup((char*)&hd[10], 20);

    di->serial  = malloc(strlen(serno) + 1);
    di->model   = malloc(strlen(model) + 1);
    di->fwrev = malloc(strlen(fwrev) + 1);
    if (! di->serial || ! di->model || !di->fwrev)
    {
        klog_emerg("内存分配失败：%s\n", strerror(errno));
        SAFE_FREE(model);
        SAFE_FREE(serno);
        SAFE_FREE(fwrev);
        return KDK_ENOMEM;
    }

    strcpy(di->serial, serno);
    strcpy(di->model, model);
    strcpy(di->fwrev, fwrev);

    strstripspace(di->serial);
    strstripspace(di->model);
    strstripspace(di->fwrev);

    SAFE_FREE(model);
    SAFE_FREE(serno);
    SAFE_FREE(fwrev);
    return 0;
}

static int get_disk_partitions(kdk_diskinfo *di)
{
    blkid_probe pr = blkid_new_probe_from_filename(di->name);
    if (! pr)
    {
        klog_err("无法获取%s分区表信息\n", di->name);
        di->partition_nums  = 0;
        di->format  = DISK_FMT_NONE;
        goto out;
    }

    blkid_partlist ls;

    ls = blkid_probe_get_partitions(pr);
    if (! ls)
    {
        di->partition_nums  = 0;
    }
    else
    {
        di->partition_nums = blkid_partlist_numof_partitions(ls);
    }

    if (di->partition_nums < 0)
        goto out;
    
    if (di->partition_nums == 0)    // 子分区，只有子分区才有type、uuid、label属性
    {
        const char *type, *uuid;
        blkid_do_probe(pr);
        blkid_probe_lookup_value(pr, "UUID", &uuid, NULL);
        blkid_probe_lookup_value(pr, "TYPE", &type, NULL);

        if (strcmp(type, "ext4") == 0)
            di->format   = DISK_FMT_EXT4;
        else if (strcmp(type, "fat32") == 0)
            di->format   = DISK_FMT_FAT32;
        else if (strcmp(type, "ntfs") == 0)
            di->format   = DISK_FMT_NTFS;
        else if (strcmp(type, "ext3") == 0)
            di->format   = DISK_FMT_EXT3;
        else if (strcmp(type, "vfat") == 0)
            di->format   = DISK_FMT_VFAT;
        else if (strcmp(type, "ext2") == 0)
            di->format   = DISK_FMT_EXT2;
        else if (strcmp(type, "fat16") == 0)
            di->format   = DISK_FMT_FAT16;
        else
            di->format   = DISK_FMT_UNKNOW;
        
        di->uuid    = malloc(strlen(uuid) + 1);
        if (! di->uuid)
        {
            klog_emerg("内存分配失败：%s\n", strerror(errno));
            blkid_free_probe(pr);
            return KDK_ENOMEM;
        }
        strcpy(di->uuid, uuid);
    }
    else
        di->format   = DISK_FMT_NONE;

out:
    blkid_free_probe(pr);
    return 0;
}

static int get_disk_type(kdk_diskinfo *di)
{
    // 获取接口类型

    // 获取磁盘类型
    // if (isdigit(di->name[strlen(di->name)]))    // 分区盘
    // {
    //     di->disk_type   = DISK_TYPE_NONE;
    //     goto out;
    // }
    // else
    // {
#ifdef  __linux__
        const char *p_name = di->name;
        p_name += strlastof(p_name, '/');
        char syspath[PATH_MAX];
        sprintf(syspath, "/sys/block/%s/queue/rotational", p_name);
        if(0 != access(syspath,0))
        {
            di->disk_type = DISK_TYPE_NONE;
            return 0;
        }
        int tfd = open(syspath, O_RDONLY);
        if (! tfd)
        {
            klog_err("rotational文件打开失败：%s\n", strerror(errno));
            perror("Unable to open rotational. ");
            return -1;
        }
        char buf[2] = {0};
        read(tfd, buf, 1);
        if (buf[0] == '0')
            di->disk_type   = DISK_TYPE_SSD;
        else
            di->disk_type   = DISK_TYPE_HDD;
#endif  // __linux__

        if (di->disk_type == DISK_TYPE_SSD)
            goto out;
    // }

    // 获取转速信息（固态除外）
    
    

out:
    close(tfd);
    return 0;
}

char** kdk_get_disklist()
{
    char **res = NULL;
    size_t counts = 0;

#ifdef  __linux__
#define DISKDIR "/dev/disk/by-path/"
    DIR *dir = opendir(DISKDIR);
    ASSERT_NOT_NULL(dir, NULL);
    struct dirent *dp;
    while ((dp = readdir(dir)))
    {
        if ((!strncmp(dp->d_name, ".", 1)) || (!strncmp(dp->d_name, "..", 2)))
            continue;
        
        char linkpath[PATH_MAX], targetpath[PATH_MAX];
        memset(targetpath, 0, PATH_MAX * sizeof(char));
        strcpy(linkpath, DISKDIR);
        strcat(linkpath, dp->d_name);
        if (readlink(linkpath, targetpath, PATH_MAX) < 0)
            continue;
        
        char **tmp = realloc(res, (counts + 1) * sizeof(char *));
        // FixMe: Memory Leak for res and dir
        // ASSERT_NOT_NULL(res, NULL);
        if (!tmp) {
            goto err_out;
        }
        res = tmp;
        res[counts] = malloc(PATH_MAX * sizeof(char) + 1 + PATH_MAX);
        res[counts][PATH_MAX + 1] = 0;

        // 调整出绝对路径
        char *pstr = targetpath;
        char prevpath[32];
        strcpy(prevpath, DISKDIR);
        char *ppp = &prevpath[strlen(prevpath)];
        ppp --;
        *ppp = 0;
        while (!strncmp(pstr, "..", 2))
        {
            while (*ppp != '/')
                ppp --;
            *ppp = 0;
            pstr += 3;
        }
        strcpy(res[counts], prevpath);
        strcat(res[counts], "/");
        strcat(res[counts], pstr);

        counts ++;
    }
    closedir(dir);
#elif __win32
#endif

    char **tmp = realloc(res, (counts + 1) * sizeof(char *));
    // FixMe: Memory Leak
    // ASSERT_NOT_NULL(tmp, NULL);
    if(!tmp)
        goto err_out;
    res = tmp;
    res[counts] = NULL;
    return res;
err_out:
    closedir(dir);
    while (counts)
    {
        free(res[counts - 1]);
        counts--;
    }
    free(res);
    return NULL;
}

void kdk_free_disklist(char** disklist)
{
    if (!disklist)
        return ;
    
    for (int i = 0; disklist[i]; i ++)
    {
        free(disklist[i]);
    }

    free(disklist);
}

kdk_diskinfo *_kdk_get_diskinfo(const char *diskname)
{
    if (!diskname)
        return NULL;
    
    int fd = open(diskname, O_RDONLY | O_NONBLOCK);
    if (fd <= 0)
    {
        klog_err("%s文件打开失败：%s\n", diskname, strerror(errno));
        return NULL;
    }

    kdk_diskinfo *res = (kdk_diskinfo *)calloc(1, sizeof(kdk_diskinfo));

    if (! res)
    {
        goto out;
    }

    res->name = malloc(strlen(diskname) + 1);
    strcpy(res->name, diskname);

    if (get_disk_identifier(fd, res))
    {
        struct udev *udev;
        struct udev_device *dev, *dev_block, *dev_nvme;

        udev = udev_new();
        // klog_debug("初始化udev\n");
        if (!udev)
        {
            goto err_out;
        }
        int pos = strlastof(diskname, '/');
        if (pos <= 0)
            pos = 0;
        char *dn = diskname + pos;
        // klog_debug("%s\n", dn);

        struct udev_device *dev_PP = NULL;
        struct udev_device *serial = NULL;
        struct udev_device *dev_P = NULL;
        const char *serial_val = NULL;

        dev_block = udev_device_new_from_subsystem_sysname(udev,"block",dn);
        dev_nvme = udev_device_new_from_subsystem_sysname(udev,"nvme",dn);
        udev_unref(udev);
        if (dev_block)
        {
            dev = dev_block;
        }
        else if(dev_nvme)
        {
            dev = dev_nvme;
        }
        else{
            klog_err("No disk information matched : %s\n",diskname);
        }

    // if (p_udev_device_new_from_subsystem_sysname_handle(udev,"block",dn))
        // {
        //     dev = p_udev_device_new_from_subsystem_sysname_handle(udev,"block",dn);
        // }
        // else if (p_udev_device_new_from_subsystem_sysname_handle(udev,"nvme",dn))
        // {
        //     dev = p_udev_device_new_from_subsystem_sysname_handle(udev,"nvme",dn);
        // }
        // else{
        //     klog_err("No disk information matched : %s\n",diskname);
        //     // goto err_out;
        // }

        dev_PP = udev_device_get_parent(udev_device_get_parent(dev));
        dev_P = udev_device_get_parent(dev);

        if(udev_device_get_sysattr_value(dev,"model")){
            // klog_debug("Dev DeviceModel: %s\n", udev_device_get_sysattr_value(dev_P,"model")); 
            if (udev_device_get_sysattr_value(dev,"serial"))
            {
                serial_val = udev_device_get_sysattr_value(dev,"serial");
                // klog_debug("Dev DeviceDevPath: %s\n", udev_device_get_syspath(dev)); 
                // klog_debug("Dev DeviceSerial: %s\n", serial_val); 
                res->serial = strdup(serial_val ? serial_val : "None");
            }
            else
            {
                klog_err("Input Special Device :%s\n", dn); 
            }
            res->model = strdup(udev_device_get_sysattr_value(dev,"model"));
        }
        else if (udev_device_get_sysattr_value(dev_P,"model"))
        {
            // klog_debug("Dev_P DeviceModel: %s\n", udev_device_get_sysattr_value(dev_P,"model")); 
            if (udev_device_get_sysattr_value(dev_P,"serial"))
            {
                serial_val = udev_device_get_sysattr_value(dev_P,"serial");
                // klog_debug("Dev_P DeviceDevPath: %s\n", udev_device_get_syspath(dev_PP)); 
            }
            else{
                serial = udev_device_get_parent(udev_device_get_parent(udev_device_get_parent(dev_PP)));
                serial_val = udev_device_get_sysattr_value(serial,"serial");
                // klog_debug("Dev_P DeviceDevPath: %s\n", udev_device_get_syspath(serial)); 
            }
            // klog_debug("Dev_P DeviceSerial: %s\n", serial_val); 
            res->serial = strdup(serial_val ? serial_val : "None");
            res->model = strdup(udev_device_get_sysattr_value(dev_P,"model"));
        }
        else if (udev_device_get_sysattr_value(dev_PP,"model"))
        {
            klog_debug("DeviceModel: %s\n", udev_device_get_sysattr_value(dev_PP,"model"));
            if (udev_device_get_sysattr_value(dev_PP,"serial")){
                serial_val = udev_device_get_sysattr_value(dev_PP,"serial");
                // klog_debug("DeviceDevPath: %s\n", udev_device_get_syspath(dev_PP)); 

            }
            else{
                serial = udev_device_get_parent(udev_device_get_parent(udev_device_get_parent(udev_device_get_parent(dev_PP))));
                serial_val = udev_device_get_sysattr_value(serial,"serial");
                // klog_debug("DeviceDevPath: %s\n", udev_device_get_syspath(serial)); 
            }
            // klog_debug("DeviceSerial: %s\n", serial_val); 
            res->serial = strdup(serial_val ? serial_val : "None");
            res->model = strdup(udev_device_get_sysattr_value(dev_PP,"model"));
        }
        else{
            klog_err("No disk MODEL info matched :%s\n",diskname); 
        }
        udev_device_unref(dev);
        
        const char *p_name = res->name;
        res->fwrev = (char *)malloc(FWREV_SIZE * sizeof(char));
        p_name += strlastof(p_name, '/');
        char syspath[PATH_MAX];
        sprintf(syspath, "/sys/block/%s/device/firmware_rev", p_name);
        int tfd = open(syspath, O_RDONLY);
        if (tfd <= 0)
        {
            strcpy(res->fwrev, "None");
        }else{
            char buf[FWREV_SIZE] = {0};
            read(tfd, buf, FWREV_SIZE);
            strcpy(res->fwrev, buf);
            close(tfd);
        }
    }

    if (get_disk_space(fd, res))
    {
        goto err_out;
    }

    if (get_disk_partitions(res))
    {
        goto err_out;
    }

    if (get_disk_type(res))
    {
        goto err_out;
    }

out:
    close(fd);
    return res;

err_out: 
    kdk_free_diskinfo(res);
    close(fd);
    return NULL;
}

static unsigned int getDiskSectorSize(char *diskname)
{
    unsigned int ret = 0;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = 0;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/disk", // object to call on
                                            "com.lingmo.lingmosdk.disk",  // interface to call on
                                            "getDiskSectorSize");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = 0;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &diskname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        ret = 0;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = 0;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        ret = 0;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        ret = 0;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = 0;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    if(ret == 0)
    {
        kdk_diskinfo *diskinfo = _kdk_get_diskinfo(diskname);
        if(diskinfo)
            return diskinfo->sector_size;
    }

    return ret;
}

static unsigned long long getDiskSectorNum(char *diskname)
{
    unsigned long long ret;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = 0;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/disk", // object to call on
                                            "com.lingmo.lingmosdk.disk",  // interface to call on
                                            "getDiskSectorNum");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = 0;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &diskname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        ret = 0;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = 0;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        ret = 0;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        ret = 0;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = 0;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    if(ret == 0)
    {
        kdk_diskinfo *diskinfo = _kdk_get_diskinfo(diskname);
        if(diskinfo)
            ret = diskinfo->sectors_num;
    }

    return ret;
}

static float getDiskTotalSizeMiB(char *diskname)
{
    double ret = 0.00;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = 0.00;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/disk", // object to call on
                                            "com.lingmo.lingmosdk.disk",  // interface to call on
                                            "getDiskTotalSizeMiB");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = 0.00;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &diskname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        ret = 0.00;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = 0.00;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        ret = 0.00;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        ret = 0.00;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = 0.00;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    if(ret == 0.00)
    {
        kdk_diskinfo *diskinfo = _kdk_get_diskinfo(diskname);
        if(diskinfo)
            return diskinfo->total_size_MiB;
    }

    return (double)ret;
}

static char *getDiskModel(char *diskname)
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = NULL;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/disk", // object to call on
                                            "com.lingmo.lingmosdk.disk",  // interface to call on
                                            "getDiskModel");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = NULL;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &diskname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        ret = NULL;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        ret = NULL;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = NULL;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    if(ret == NULL)
    {
        kdk_diskinfo *diskinfo = _kdk_get_diskinfo(diskname);
        if(diskinfo)
            return diskinfo->model;
    }

    return ret;
}

static char *getDiskSerial(char *diskname)
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = NULL;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/disk", // object to call on
                                            "com.lingmo.lingmosdk.disk",  // interface to call on
                                            "getDiskSerial");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = NULL;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &diskname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        ret = NULL;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        ret = NULL;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = NULL;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    if(ret == NULL)
    {
        kdk_diskinfo *diskinfo = _kdk_get_diskinfo(diskname);
        if(diskinfo)
            return diskinfo->serial;
    }

    return ret;
}

static unsigned int getDiskPartitionNums(char *diskname)
{
    unsigned int ret = 0;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = 0;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/disk", // object to call on
                                            "com.lingmo.lingmosdk.disk",  // interface to call on
                                            "getDiskPartitionNums");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = 0;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &diskname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        ret = 0;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = 0;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        ret = 0;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        ret = 0;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = 0;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    if(ret == 0)
    {
        kdk_diskinfo *diskinfo = _kdk_get_diskinfo(diskname);
        if(diskinfo)
            return diskinfo->partition_nums;
    }

    return ret;
}

static int getDiskType(char *diskname)
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = NULL;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/disk", // object to call on
                                            "com.lingmo.lingmosdk.disk",  // interface to call on
                                            "getDiskType");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = NULL;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &diskname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        ret = NULL;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        ret = NULL;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = NULL;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    if(strcmp(ret, "DISK_TYPE_HDD") == 0)
    {
        return 0;
    }
    else if(strcmp(ret, "DISK_TYPE_SSD") == 0)
    {
        return 1;
    }
    else if(strcmp(ret, "DISK_TYPE_NONE") == 0)
    {
        return 4;
    }

    if(ret == NULL)
    {
        kdk_diskinfo *diskinfo = _kdk_get_diskinfo(diskname);
        if(diskinfo)
            return diskinfo->disk_type;
    }

    return DISK_TYPE_NONE;
}

static char *getDiskVersion(char *diskname)
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = NULL;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/disk", // object to call on
                                            "com.lingmo.lingmosdk.disk",  // interface to call on
                                            "getDiskVersion");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = NULL;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &diskname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        ret = NULL;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        ret = NULL;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = NULL;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    if(ret == NULL)
    {
        kdk_diskinfo *diskinfo = _kdk_get_diskinfo(diskname);
        if(diskinfo)
            return diskinfo->fwrev;
    }

    return ret;
}

kdk_diskinfo *kdk_get_diskinfo(const char *diskname)
{
    kdk_diskinfo *diskinfo = (kdk_diskinfo *)calloc(1, sizeof(kdk_diskinfo));
    diskinfo->name = malloc(strlen(diskname) + 1);
    strcpy(diskinfo->name, diskname);

    unsigned int sector_size = getDiskSectorSize(diskname);
    diskinfo->sector_size = sector_size;

    unsigned long long sectors_num = getDiskSectorNum(diskname);
    diskinfo->sectors_num = sectors_num;

    float total_size_MiB = getDiskTotalSizeMiB(diskname);
    diskinfo->total_size_MiB = total_size_MiB;

    char *model = getDiskModel(diskname);
    if(model)
    {
        diskinfo->model = malloc(strlen(model) + 1);
        strcpy(diskinfo->model, model);
    }
    
    char *serial = getDiskSerial(diskname);
    if(serial)
    {
        diskinfo->serial = malloc(strlen(serial) + 1);
        strcpy(diskinfo->serial, serial);
    }

    diskinfo->partition_nums = getDiskPartitionNums(diskname);
    diskinfo->disk_type = getDiskType(diskname);

    char *fwrev = getDiskVersion(diskname);
    if(fwrev)
    {
        diskinfo->fwrev = malloc(strlen(fwrev) + 1);
        strcpy(diskinfo->fwrev, fwrev);
    }

    return diskinfo;
}

extern char** kdk_get_hard_disk()
{
    int count = 0;
    char **ret = NULL;

    hd_data_t *hd_data;
    hd_data = (hd_data_t *)calloc(1, sizeof *hd_data);
    if (!hd_data)
    {
        klog_err("%s 申请内存失败于%s", "kdk_get_hard_disk", "hd_data");
        ret = NULL;
        goto out;
    }

    hd_data->progress = NULL;
    hd_data->debug = ~(HD_DEB_DRIVER_INFO | HD_DEB_HDDB);

    hd_t *hd, *hd0;
    hd0 = hd_list(hd_data, hw_disk, 1, NULL);
    if(!hd0)
    {
        ret = NULL;
        goto out;
    }

    for (hd = hd0; hd; hd = hd->next)
    {  
        if(hd->sysfs_device_link != NULL)
        {
            // printf("hd->unix_dev_name = %s\n", hd->unix_dev_name);
            if(hd->unix_dev_name)
            {
                char **hard = (char **)realloc(ret, sizeof(char *) * (count + 1 + 1));
                if (!hard)
                {
                    goto err_out;
                }
                ret = hard;

                ret[count] = malloc(sizeof(char) * (strlen(hd->unix_dev_name) + 1));
                if(!ret[count])
                {
                    goto err_out;
                }  
                strcpy(ret[count], hd->unix_dev_name);
                count++;
            }
        }
    }
    ret[count] = NULL;
out:
    hd_free_hd_data(hd_data);
    free(hd_data);
    hd_free_hd_list(hd0);
    return ret;
err_out:
    hd_free_hd_data(hd_data);
    free(hd_data);
    hd_free_hd_list(hd0);

    while (count)
    {
        free(ret[count - 1]);
        count--;
    }
    free(ret);
    return NULL;
}

char* _kdk_get_hard_disk_size(const char *hardname)
{
    if(!hardname)
    {
        return NULL;
    }
    char *hard_size = (char *)malloc(128*sizeof(char));
    if (!hard_size)
    {
        return NULL;
    }
    memset(hard_size, 0, 128);
    kdk_diskinfo *sdainfo = _kdk_get_diskinfo(hardname);
    if (!sdainfo)
    {
        free(hard_size);
        return NULL;
    }
    sprintf(hard_size,"%f",sdainfo->total_size_MiB);
    kdk_free_diskinfo(sdainfo);
    if(strlen(hard_size) == 0)
    {
        free(hard_size);
        return NULL;
    }
    return hard_size;
}

char *kdk_get_hard_disk_size(const char *hardname)
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = NULL;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/disk", // object to call on
                                            "com.lingmo.lingmosdk.disk",  // interface to call on
                                            "getHardDiskSize");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = NULL;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &hardname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        ret = NULL;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        ret = NULL;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = NULL;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    if(ret == NULL)
    {
        ret = _kdk_get_hard_disk_size(hardname);
    }

    char *size = NULL;
    if(ret != NULL)
    {
        size = (char *)malloc(sizeof(char) * 128);
        if(!size)
        {
            size = NULL;
        }
        strcpy(size, ret);
    }

    return size;
}

char* _kdk_get_hard_fwrev(const char *hardname)
{
    if(!hardname)
    {
        return NULL;
    }
    char path[128] = "\0"; 
    char tmp[1024] = "\0";
    char fwrev[128] = "\0";
    char *hard_fwrev = (char *)malloc(128*sizeof(char));
    if (!hard_fwrev)
    {
        return NULL;
    }
    memset(hard_fwrev, 0, 128);

    kdk_diskinfo *sdainfo = _kdk_get_diskinfo(hardname);
    if (!sdainfo)
    {
        free(hard_fwrev);
        return NULL;
    }
    
    if(!strcmp(sdainfo->fwrev, "None"))
    {
        sprintf(path, "smartctl -i %s", hardname);
        FILE *fp = popen(path, "r");
        if(!fp)
        {
            free(hard_fwrev);
            kdk_free_diskinfo(sdainfo);
            return NULL;
        }
        while (fgets(tmp, 1024, fp))
        {
            if (0 == strncmp(tmp, "Firmware Version:", 17))
            {
                sscanf(tmp, "%*s %*s %s", fwrev);
                break;
            }
        }
        pclose(fp);
        if(fwrev[0] != '\0')
        {
            strcpy(hard_fwrev, fwrev);
        }
        else{
            free(hard_fwrev);
            kdk_free_diskinfo(sdainfo);
            return NULL;
        }
    }else{
        strcpy(hard_fwrev, sdainfo->fwrev);
        strstripspace(hard_fwrev);
    }
    kdk_free_diskinfo(sdainfo);
    return hard_fwrev;
}

char* kdk_get_hard_fwrev(const char *hardname)
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = NULL;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/disk", // object to call on
                                            "com.lingmo.lingmosdk.disk",  // interface to call on
                                            "getHardFwrev");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = NULL;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &hardname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        ret = NULL;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        ret = NULL;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = NULL;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    if(ret == NULL)
    {
        ret = _kdk_get_hard_fwrev(hardname);
    }

    char *fwrev = NULL;
    if(ret != NULL)
    {
        fwrev = (char *)malloc(sizeof(char) * 128);
        if(!fwrev)
        {
            fwrev = NULL;
        }
        strcpy(fwrev, ret);
    }

    return fwrev;
}

char* _kdk_get_hard_type(const char *hardname)
{
    if(!hardname)
    {
        return NULL;
    }
    char *hard_type = (char *)malloc(128*sizeof(char));
    if (!hard_type)
    {
        return NULL;
    }
    
    kdk_diskinfo *sdainfo = _kdk_get_diskinfo(hardname);
    if (!sdainfo)
    {
        free(hard_type);
        return NULL;
    }
    if(sdainfo->disk_type == 0)
    {
        strcpy(hard_type,"机械");
    }else{
        strcpy(hard_type,"固态");
    }
    kdk_free_diskinfo(sdainfo);
    return hard_type;
}

char* kdk_get_hard_type(const char *hardname)
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = NULL;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/disk", // object to call on
                                            "com.lingmo.lingmosdk.disk",  // interface to call on
                                            "getHardType");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = NULL;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &hardname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        ret = NULL;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        ret = NULL;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = NULL;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    if(ret == NULL)
    {
        ret = _kdk_get_hard_type(hardname);
    }

    char *type = NULL;
    if(ret != NULL)
    {
        type = (char *)malloc(sizeof(char) * 128);
        if(!type)
        {
            type = NULL;
        }
        strcpy(type, ret);
    }

    return type;
}

char* _kdk_get_hard_model(const char *hardname)
{
    if(!hardname)
    {
        return NULL;
    }
    
    char path[256] = "\0";
    char tmp[1024] = "\0";
    char *str = NULL;

    sprintf(path, "smartctl -i %s", hardname);
    FILE *fd = popen(path, "r");
    if(!fd)
    {
        return NULL;
    }
    while (fgets(tmp, 1024, fd))
    {
        if (0 == strncmp(tmp, "Model Number:", 13))
        {
            int tmp_pos = strlastof(tmp, ':');
            if (tmp_pos <= 0)
                tmp_pos = 0;
            str = tmp + tmp_pos;
            break;
        }
        else if(0 == strncmp(tmp, "Device Model:", 13))
        {
            int tmp_pos = strlastof(tmp, ':');
            if (tmp_pos <= 0)
                tmp_pos = 0;
            str = tmp + tmp_pos;
            break;
        }
    }
    pclose(fd);
    char *hard_model = (char *)malloc(128*sizeof(char));
    if (!hard_model)
    {
        return NULL;
    }
    if (str)
    {
        strcpy(hard_model, str);
        strstripspace(hard_model);
    }
    else{
        free(hard_model);
        return NULL;
    }
    return hard_model;
}

char* kdk_get_hard_model(const char *hardname)
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = NULL;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/disk", // object to call on
                                            "com.lingmo.lingmosdk.disk",  // interface to call on
                                            "getHardModel");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = NULL;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &hardname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        ret = NULL;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        ret = NULL;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = NULL;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    if(ret == NULL)
    {
        ret = _kdk_get_hard_model(hardname);
    }

    char *model = NULL;
    if(ret != NULL)
    {
        model = (char *)malloc(sizeof(char) * 128);
        if(!model)
        {
            model = NULL;
        }
        strcpy(model, ret);
    }

    return model;
}


char* _kdk_get_hard_serial(const char *hardname)
{
    if(!hardname)
    {
        return NULL;
    }
    char *hard_serial = (char *)malloc(256*sizeof(char));
    if (!hard_serial)
    {
        return NULL;
    }
    memset(hard_serial, 0, 256);
    char line[256] = "\0";
    char path[128] = "\0";
    char tmp[1024] = "\0";
    char serial[128] = "\0";
    char *str = NULL;
    char name[128] = "\0";
    char hard_name[128] = "\0";
    strcpy(hard_name, hardname);

    int pos = strlastof(hardname, '/');
    if (pos <= 0)
        pos = 0;
    char *dn = hardname + pos;

    sprintf(path, "/sys/class/block/%s/device/serial", dn);

    FILE *fp = fopen(path, "r");
    if(fp)
    {
        fgets(line, sizeof(line), fp);
        fclose(fp);
        if(line[0] != '\0')
        {
            strcpy(hard_serial, line);
            strstripspace(hard_serial);
        }
    }
    else{
        memset(path, 0, sizeof(path));
        sprintf(path, "smartctl -i %s", hardname);
        FILE *fp = popen(path, "r");
        if(!fp)
        {
            goto err_out;
        }
        while (fgets(tmp, 1024, fp))
        {
            if (0 == strncmp(tmp, "Serial Number:", 14))
            {
                int tmp_pos = strlastof(tmp, ':');
                if (tmp_pos <= 0)
                    tmp_pos = 0;
                str = tmp + tmp_pos;
                break;
            }
        }
        pclose(fp);
        if (str)
        {
            strcpy(hard_serial, str);
            strstripspace(hard_serial);
        }
        else{
            hd_data_t *hd_data;
            hd_data = (hd_data_t *)calloc(1, sizeof *hd_data);
            if (!hd_data)
            {
                goto err_out;
            }

            hd_data->progress = NULL;
            hd_data->debug = ~(HD_DEB_DRIVER_INFO | HD_DEB_HDDB);
            int i = 0;
            hd_t *hd, *hd0;

            hd0 = hd_list(hd_data, hw_disk, 1, NULL);
            if(!hd0)
            {
                free(hd_data);
                goto err_out;
            }
            
            for (hd = hd0; hd; hd = hd->next)
            {
                sprintf(name, "%s", hd->unix_dev_name);
                if(strcmp(name, hard_name) == 0)
                {
                    if(hd->serial)
                    {
                        strcpy(hard_serial, hd->serial);
                    }
                    else{
                        hd_free_hd_data(hd_data);
                        free(hd_data);
                        hd_free_hd_list(hd0);
                        goto err_out;
                    }
                }
            }
            hd_free_hd_data(hd_data);
            free(hd_data);
            hd_free_hd_list(hd0);
        }
    }
    if(strlen(hard_serial) == 0)
    {
        goto err_out;
    }
    return hard_serial;
err_out:
    free(hard_serial);
    return NULL;
}

char* kdk_get_hard_serial(const char *hardname)
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = NULL;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/disk", // object to call on
                                            "com.lingmo.lingmosdk.disk",  // interface to call on
                                            "getHardSerial");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = NULL;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &hardname, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        ret = NULL;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        ret = NULL;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = NULL;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    if(ret == NULL)
    {
        ret = _kdk_get_hard_serial(hardname);
    }

    char *serial = NULL;
    if(ret != NULL)
    {
        serial = (char *)malloc(sizeof(char) * 128);
        if(!serial)
        {
            serial = NULL;
        }
        strcpy(serial, ret);
    }

    return serial;
}

char* kdk_get_hard_vendor(const char *hardname)
{
    if(!hardname)
    {
        return NULL;
    }
    
    char name[64] = "\0";
    struct HWInfo *result = NULL;
    char vendor[256] = "\0";

    hd_data_t *hd_data;
    hd_data = (hd_data_t *)calloc(1, sizeof *hd_data);
    if (!hd_data)
    {
        return NULL;
    }

    hd_data->progress = NULL;
    hd_data->debug = ~(HD_DEB_DRIVER_INFO | HD_DEB_HDDB);

    hd_t *hd, *hd0;

    hd0 = hd_list(hd_data, hw_disk, 1, NULL);
    if(!hd0)
    {
        free(hd_data);
        return NULL;
    }

    for (hd = hd0; hd; hd = hd->next)
    {
        sprintf(name, "%s", hd->unix_dev_name);
        if(strcmp(name, hardname) == 0)
        {
            if(hd->vendor.name)
            {
                strcpy(vendor, hd->vendor.name);
            }
            else if(hd->sub_vendor.name)
            {
                strcpy(vendor, hd->sub_vendor.name);
            }
        }
    }
    hd_free_hd_data(hd_data);
    free(hd_data);
    hd_free_hd_list(hd0);
    
    if(strstr(vendor, "ST"))
    {
        strcpy(vendor, "Seagate");
    }
    if(strlen(vendor) == 0)
    {
        char buf[256] = "\0";
        char *buff[3];
        int i = 0;
        int tmp = 0;
        FILE *pipeLine = popen("lshw -C storage", "r");
        if (!pipeLine)
        {
            return NULL;
        }
        if (__glibc_likely(pipeLine != NULL))
        {
            while (fgets(buf, sizeof(buf), pipeLine))
            {
                if(strstr(buf, "storage"))
                {
                    tmp = 1;
                }
                if(tmp == 1)
                {
                    if(strstr(buf, "vendor"))
                    {
                        char *p = strtok(buf, ":");
                        while(p)
                        {
                            buff[i] = p;
                            i++;
                            p = strtok(NULL,"");
                        }
                        if(buff[1] != NULL)
                        {
                            strcpy(vendor, buff[1]);
                            strstripspace(vendor);
                        }
                    }
                }
                else{
                    continue;
                }
            }
        }
        pclose(pipeLine);
    }
    if(strlen(vendor) == 0)
    {
        return NULL;
    }
    char *hard_vendor = (char *)malloc(128*sizeof(char));
    if (!hard_vendor)
    {
        return NULL;
    }
    memset(hard_vendor, 0, 128);
    strcpy(hard_vendor, vendor);
    return hard_vendor;
}

void kdk_free_diskinfo(kdk_diskinfo *disk)
{
    SAFE_FREE(disk->name);
    SAFE_FREE(disk->serial);
    SAFE_FREE(disk->uuid);
    SAFE_FREE(disk->vendor);
    SAFE_FREE(disk->model);
    SAFE_FREE(disk->mount_path);
    SAFE_FREE(disk->fwrev);
    SAFE_FREE(disk);
}

int kdk_get_hard_running_time(const char *hardname)
{
    char cmd[128] = {"smartctl -A "};
    char tmp[1024] = {"\0"};
    strcat(cmd, hardname);
    FILE *fp = popen(cmd,"r");
    if(fp)
    {
        int temp = 0;
        while (fgets(tmp, 1024, fp))
        {
            //sata接口返回数据格式
            if(0 == strncmp(tmp,"  9",3))
            {
                sscanf(tmp, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %d", &temp);
                break;
            }
            //nvme接口返回数据格式
            if (0 == strncmp(tmp, "Power On Hours:", 15))
            {
                char hours[128] = "\0";
                sscanf(tmp, "%*s %*s %*s %s", hours);

                int len = strlen(hours);
                char tmp_h[len+1];
                int j = 0;
                for (int i = 0; i < len; i++) {
                    if (hours[i] == ',') {
                        continue;
                    } else if (hours[i] != '\0') {
                        tmp_h[j++] = hours[i];
                    }
                }
                tmp_h[j] = '\0';

                temp = atoi(tmp_h);
                break;
            }
        }
        return temp;
    }
    return -1;
}
