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

#include "libkyusb.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>
#include <libudev.h>
#include "libkylog.h"
#include <dbus-1.0/dbus/dbus.h>

/// @brief
/// @return usb device info
pDriverInfoList kdk_usb_get_list()
{
    struct udev *udev = NULL;
    struct udev_enumerate *enumerate = NULL;
    struct udev_list_entry *list_entry = NULL, *dev_list_entry = NULL;
    struct udev_device *dev = NULL, *device = NULL, *interface = NULL;

    pDriverInfoList list = NULL;
    pDriverInfoList prevnode = NULL;
    pDriverInfoList curnode = NULL;

    /* Create the udev object */
    udev = udev_new();
    if (NULL == udev)
    {
        klog_err("Can't create udev");
        return list;
    }

    /*enum devices*/
    enumerate = udev_enumerate_new(udev);
    if (NULL == enumerate)
    {
        klog_err("Can't create enumerate");
        udev_unref(udev);
        return list;
    }

    // 添加筛选条件 普通usb设备路径/sys/class/hidraw，U盘路径/sys/class/block
    //    udev_enumerate_add_match_subsystem(enumerate, "hidraw");
    //    udev_enumerate_add_match_subsystem(enumerate, "block");
    // 扫描设备
    udev_enumerate_scan_devices(enumerate);

    // 获取设备列表
    list_entry = udev_enumerate_get_list_entry(enumerate);

    // 遍历
    udev_list_entry_foreach(dev_list_entry, list_entry)
    {
        const char *path;
        // 创建dev_device设备对象
        path = udev_list_entry_get_name(dev_list_entry);
        device = udev_device_new_from_syspath(udev, path);
        if (!device)
        {
            kdk_usb_free(list);
            break;
        }

        // 获取interface父节点，此节点记录设备类型
        interface = udev_device_get_parent_with_subsystem_devtype(device, "usb", "usb_interface");
        if (!interface)
        {
            goto free;
        }

        // 非usb设备直接进入下一轮循环
        dev = udev_device_get_parent_with_subsystem_devtype(device, "usb", "usb_device");
        if (!dev)
        {
            goto free;
        }
        // 同一款外设会生成两个设备文件，原因不明。暂时先手动去重
        pDriverInfoList tmp = list;
        int isRepetive = 0;
        while (tmp)
        {
            if (0 == strcmp(tmp->data->devNode, udev_device_get_devnode(dev)))
            {
                isRepetive = 1;
            }
            tmp = tmp->next;
        }
        if (isRepetive)
        {
            goto free;
        }

        // 申请节点内存
        if (NULL == list)
        {
            curnode = (pDriverInfoList)calloc(1, sizeof(DriverInfoList));
            list = curnode;
        }
        else
            curnode = (pDriverInfoList)calloc(1, sizeof(DriverInfoList));

        if (NULL == curnode)
        {
            klog_err("Request Memory For List Failed");
            kdk_usb_free(list);
            return NULL;
        }

        curnode->data = (pDriverInfo)calloc(1, sizeof(DriverInfo));
        if (NULL == curnode->data)
        {
            klog_err("Request Memory For Data Failed");
            free(curnode);
            kdk_usb_free(list);
            return NULL;
        }
        sprintf(curnode->data->name, "%s", udev_device_get_sysattr_value(dev, "product") ? udev_device_get_sysattr_value(dev, "product") : "null");
        sprintf(curnode->data->type, "%s", udev_device_get_sysattr_value(interface, "bInterfaceClass") ? udev_device_get_sysattr_value(interface, "bInterfaceClass") : "null");
        sprintf(curnode->data->pid, "%s", udev_device_get_sysattr_value(dev, "idProduct") ? udev_device_get_sysattr_value(dev, "idProduct") : "null");
        sprintf(curnode->data->vid, "%s", udev_device_get_sysattr_value(dev, "idVendor") ? udev_device_get_sysattr_value(dev, "idVendor") : "null");
        sprintf(curnode->data->serialNo, "%s", udev_device_get_sysattr_value(dev, "serial") ? udev_device_get_sysattr_value(dev, "serial") : "null");
        sprintf(curnode->data->devNode, "%s", udev_device_get_devnode(dev));
        // sprintf(curnode->data->vendor, "%s", udev_device_get_sysattr_value(dev, "manufacturer"));

        if (NULL != prevnode)
            prevnode->next = curnode;
        prevnode = curnode;
        curnode = NULL;
    free:
        if (device)
            udev_device_unref(device);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return list;
}

void kdk_usb_free(pDriverInfoList list)
{
    pDriverInfoList tmp = NULL;
    while (list)
    {
        tmp = list;
        list = list->next;
        free(tmp->data);
        free(tmp);
    }
}

pUsbInfo kdk_usb_get_bus_info()
{
    int ret = libusb_init(NULL);
    if (ret != 0)
    {
        klog_err("fail to init: %d\n", ret);
        return NULL;
    }

    libusb_device **devs = NULL;
    ssize_t count = libusb_get_device_list(NULL, &devs);
    if (count < 0)
    {
        klog_err("fail to get device list: %d\n", count);
        libusb_exit(NULL);
        return NULL;
    }

    UsbInfo *result = calloc(count + 1, sizeof *result);
    if (NULL == result)
    {
        klog_err("fail to request memory: %d\n");
        libusb_exit(NULL);
        return NULL;
    }

    for (int i = 0; i < count; i++)
    {
        libusb_device *dev = devs[i];

        result[i].busNum = libusb_get_bus_number(dev);
        result[i].devNum = libusb_get_device_address(dev);
        result[i].next = result + i + 1;
    }

    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);

    return result;
}

// int kdk_usb_get_found(int busNum, int devAddress)
// {
//     return 0;
// }

char *_kdk_usb_get_productName(int busNum, int devAddress)
{
    char *result = NULL;
    int ret = libusb_init(NULL);
    if (ret != 0)
    {
        klog_err("fail to init: %d\n", ret);
        return NULL;
    }

    libusb_device **devs = NULL;
    ssize_t count = libusb_get_device_list(NULL, &devs);
    if (count < 0)
    {
        klog_err("fail to get device list: %d\n", count);
        libusb_exit(NULL);
        return NULL;
    }

    for (int i = 0; i < count; i++)
    {
        libusb_device *dev = devs[i];

        if (libusb_get_bus_number(dev) == busNum && libusb_get_device_address(dev) == devAddress)
        {
            libusb_device_handle *dev_handle;
            ret = libusb_open(dev, &dev_handle);
            if (ret < 0)
            {
                klog_err("fail to open device : %d\n", ret);
                return NULL;
            }

            unsigned char product[256];
            libusb_get_string_descriptor_ascii(dev_handle, 2, product, sizeof(product));
            libusb_close(dev_handle);

            result = strdup(product);
            break;
        }
    }
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
    return result;
}

char *kdk_usb_get_productName(int busNum, int devAddress)
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
                                            "/com/lingmo/lingmosdk/peripheralsenum", // object to call on
                                            "com.lingmo.lingmosdk.peripheralsenum",  // interface to call on
                                            "getUsbProductName");               // method name
    if (!info_msg)
    { // -1 is default timeout
        ret = NULL;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_INT32, &busNum, DBUS_TYPE_INT32, &devAddress, DBUS_TYPE_INVALID))
    {
        ret = NULL;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        ret = NULL;
    }

    if (sendMsgPending == NULL)
    {
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
        ret = _kdk_usb_get_productName(busNum, devAddress);
    }

    char *productName = NULL;
    if(ret != NULL)
    {
        productName = (char *)malloc(sizeof(char) * 256);
        if(!productName)
        {
            productName = NULL;
        }
        strcpy(productName, ret);
    }

    return productName;
}

char *_kdk_usb_get_manufacturerName(int busNum, int devAddress)
{
    char *result = NULL;
    int ret = libusb_init(NULL);
    if (ret != 0)
    {
        klog_err("fail to init: %d\n", ret);
        return NULL;
    }

    libusb_device **devs = NULL;
    ssize_t count = libusb_get_device_list(NULL, &devs);
    if (count < 0)
    {
        klog_err("fail to get device list: %d\n", count);
        libusb_exit(NULL);
        return NULL;
    }

    for (int i = 0; i < count; i++)
    {
        libusb_device *dev = devs[i];

        if (libusb_get_bus_number(dev) == busNum && libusb_get_device_address(dev) == devAddress)
        {
            libusb_device_handle *dev_handle;
            ret = libusb_open(dev, &dev_handle);
            if (ret < 0)
            {
                klog_err("fail to get device descriptor: %d\n", ret);
                return NULL;
            }

            unsigned char manufacturer[256];
            libusb_get_string_descriptor_ascii(dev_handle, 1, manufacturer, sizeof(manufacturer));
            libusb_close(dev_handle);

            result = strdup(manufacturer);
            break;
        }
    }
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);
    return result;
}

char *kdk_usb_get_manufacturerName(int busNum, int devAddress)
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
                                            "/com/lingmo/lingmosdk/peripheralsenum", // object to call on
                                            "com.lingmo.lingmosdk.peripheralsenum",  // interface to call on
                                            "getUsbManufacturerName");               // method name
    if (!info_msg)
    { // -1 is default timeout
        ret = NULL;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_INT32, &busNum, DBUS_TYPE_INT32, &devAddress, DBUS_TYPE_INVALID))
    {
        ret = NULL;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        ret = NULL;
    }

    if (sendMsgPending == NULL)
    {
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
        ret = _kdk_usb_get_manufacturerName(busNum, devAddress);
    }

    char *manufacturerName = NULL;
    if(ret != NULL)
    {
        manufacturerName = (char *)malloc(sizeof(char) * 256);
        if(!manufacturerName)
        {
            manufacturerName = NULL;
        }
        strcpy(manufacturerName, ret);
    }

    return manufacturerName;
}

int kdk_usb_get_mount(int busNum, int devAddress)
{
    int result = 0;

    char *product = kdk_usb_get_productName(busNum, devAddress);
    char *vendor = kdk_usb_get_manufacturerName(busNum, devAddress);
    if (!product || !vendor)
        return result;

    char tmp[1024];
    sprintf(tmp, "%s %s", vendor, product);

    FILE *fp = popen("udisksctl status", "r");
    if (NULL == fp)
        return result;
        
    char line[1024];
    while (fgets(line, 1024, fp))
    {
        if (NULL != strstr(line, tmp))
        {
            result = 1;
            break;
        }
    }

    return result;
}

char *kdk_usb_get_version(int busNum, int devAddress)
{
    char *result = NULL;
    int ret = libusb_init(NULL);
    if (ret != 0)
    {
        klog_err("fail to init: %d\n", ret);
        return NULL;
    }

    libusb_device **devs = NULL;
    ssize_t count = libusb_get_device_list(NULL, &devs);
    if (count < 0)
    {
        klog_err("fail to get device list: %d\n", count);
        libusb_exit(NULL);
        return NULL;
    }

    for (int i = 0; i < count; i++)
    {
        libusb_device *dev = devs[i];

        if (libusb_get_bus_number(dev) == busNum && libusb_get_device_address(dev) == devAddress)
        {
            struct libusb_device_descriptor desc;
            ret = libusb_get_device_descriptor(dev, &desc);
            if (ret < 0)
            {
                klog_err("Error getting device descriptor\n");
                break;
            }
            char version[256];
            sprintf(version, "%x.%x", desc.bcdUSB >> 8, desc.bcdUSB & 0xff);
            result = strdup(version);
            break;
        }
    }
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);

    return result;
}

int kdk_usb_get_deviceClass(int busNum, int devAddress)
{
    int result = LIBUSB_ERROR_OTHER;
    int ret = libusb_init(NULL);
    if (ret != 0)
    {
        klog_err("fail to init: %d\n", ret);
        return result;
    }

    libusb_device **devs = NULL;
    ssize_t count = libusb_get_device_list(NULL, &devs);
    if (count < 0)
    {
        klog_err("fail to get device list: %d\n", count);
        libusb_exit(NULL);
        return result;
    }

    for (int i = 0; i < count; i++)
    {
        libusb_device *dev = devs[i];

        if (libusb_get_bus_number(dev) == busNum && libusb_get_device_address(dev) == devAddress)
        {
            struct libusb_device_descriptor desc;
            ret = libusb_get_device_descriptor(dev, &desc);
            if (ret < 0)
            {
                klog_err("Error getting device descriptor\n");
                result = ret;
                break;
            }

            result = desc.bDeviceClass;
            break;
        }
    }
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);

    return result;
}

int kdk_usb_get_deviceSubClass(int busNum, int devAddress)
{
    int result = LIBUSB_ERROR_OTHER;
    int ret = libusb_init(NULL);
    if (ret != 0)
    {
        klog_err("fail to init: %d\n", ret);
        return result;
    }

    libusb_device **devs = NULL;
    ssize_t count = libusb_get_device_list(NULL, &devs);
    if (count < 0)
    {
        klog_err("fail to get device list: %d\n", count);
        libusb_exit(NULL);
        return result;
    }

    for (int i = 0; i < count; i++)
    {
        libusb_device *dev = devs[i];

        if (libusb_get_bus_number(dev) == busNum && libusb_get_device_address(dev) == devAddress)
        {
            struct libusb_device_descriptor desc;
            ret = libusb_get_device_descriptor(dev, &desc);
            if (ret < 0)
            {
                klog_err("Error getting device descriptor\n");
                result = ret;
                break;
            }

            result = desc.bDeviceSubClass;
            break;
        }
    }
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);

    return result;
}

int kdk_usb_get_deviceProtocol(int busNum, int devAddress)
{
    int result = LIBUSB_ERROR_OTHER;
    int ret = libusb_init(NULL);
    if (ret != 0)
    {
        klog_err("fail to init: %d\n", ret);
        return result;
    }

    libusb_device **devs = NULL;
    ssize_t count = libusb_get_device_list(NULL, &devs);
    if (count < 0)
    {
        klog_err("fail to get device list: %d\n", count);
        libusb_exit(NULL);
        return result;
    }

    for (int i = 0; i < count; i++)
    {
        libusb_device *dev = devs[i];

        if (libusb_get_bus_number(dev) == busNum && libusb_get_device_address(dev) == devAddress)
        {
            struct libusb_device_descriptor desc;
            ret = libusb_get_device_descriptor(dev, &desc);
            if (ret < 0)
            {
                klog_err("Error getting device descriptor\n");
                result = ret;
                break;
            }

            result = desc.bDeviceProtocol;
            break;
        }
    }
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);

    return result;
}

char *kdk_usb_get_speed(int busNum, int devAddress)
{
    char *result = NULL;
    int ret = libusb_init(NULL);
    if (ret != 0)
    {
        klog_err("fail to init: %d\n", ret);
        return NULL;
    }

    libusb_device **devs = NULL;
    ssize_t count = libusb_get_device_list(NULL, &devs);
    if (count < 0)
    {
        klog_err("fail to get device list: %d\n", count);
        libusb_exit(NULL);
        return NULL;
    }

    for (int i = 0; i < count; i++)
    {
        libusb_device *dev = devs[i];

        if (libusb_get_bus_number(dev) == busNum && libusb_get_device_address(dev) == devAddress)
        {
            struct libusb_device_descriptor desc;
            ret = libusb_get_device_descriptor(dev, &desc);
            if (ret < 0)
            {
                klog_err("Error getting device descriptor\n");
                break;
            }

            switch(libusb_get_device_speed(dev))
            {
            case 1:
                result = strdup("1.5MBit/s");
                break;
            case 2:
                result = strdup("12MBit/s");
                break;
            case 3:
                result = strdup("480MBit/s");
                break;
            case 4:
                result = strdup("5000MBit/s");
                break;
            case 5:
                result = strdup("10000MBit/s");
                break;
            default:
                result = strdup("unknown");
            }
            break;
        }
    }
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);

    return result;
}

pUsbDeviceDescriptor kdk_usb_get_device_descriptor(int busNum, int devAddress)
{
    pUsbDeviceDescriptor result = calloc(1, sizeof *result);
    if (NULL == result)
    {
        klog_err("fail to request memory");
        return NULL;
    }

    int ret = libusb_init(NULL);
    if (ret != 0)
    {
        klog_err("fail to init: %d", ret);
        return NULL;
    }

    libusb_device **devs = NULL;
    ssize_t count = libusb_get_device_list(NULL, &devs);
    if (count < 0)
    {
        klog_err("fail to get device list: %d", count);
        libusb_exit(NULL);
        return NULL;
    }

    for (int i = 0; i < count; i++)
    {
        libusb_device *dev = devs[i];

        if (libusb_get_bus_number(dev) == busNum && libusb_get_device_address(dev) == devAddress)
        {
            struct libusb_device_descriptor desc;
            ret = libusb_get_device_descriptor(dev, &desc);
            if (ret < 0)
            {
                klog_err("Error getting device descriptor\n");
                break;
            }
            result->lenth = desc.bLength;
            result->descriptorType = desc.bDescriptorType;
            result->maxPacketSize0 = desc.bMaxPacketSize0;
            result->numConfigurations = desc.bNumConfigurations;
            break;
        }
    }
    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);

    return result;
}

void kdk_usb_free_usb_info(pUsbInfo list)
{
    if(NULL == list)
        return;
    while (list)
    {
        pUsbInfo tmp = list;
        list = list->next;
        free(tmp);
    }
}

void kdk_usb_free_usb_device_descriptor(pUsbDeviceDescriptor *descriptor)
{
    if(NULL == descriptor)
        return;
    free(descriptor);
}
