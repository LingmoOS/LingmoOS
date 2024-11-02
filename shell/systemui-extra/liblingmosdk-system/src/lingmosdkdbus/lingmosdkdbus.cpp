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

#include "lingmosdkdbus.h"
#include <QDBusReply>
#include <dlfcn.h>

#define LINGMOSDK_SYSINFO_LIB                               "libkysysinfo.so"
#define KDK_GET_HOST_SERIAL_HANDLE                      "_kdk_get_host_serial"

typedef char *(*handle_kdk_get_host_serial)();

#define LINGMOSDK_DISK_LIB                                  "libkydiskinfo.so"
#define KDK_GET_DISKINFO_HANDLE                         "_kdk_get_diskinfo"
#define KDK_GET_HARD_DISK_SIZE_HANDLE                   "_kdk_get_hard_disk_size"
#define KDK_GET_HARD_FWREV_HANDLE                       "_kdk_get_hard_fwrev"
#define KDK_GET_HARD_TYPE_HANDLE                        "_kdk_get_hard_type"
#define KDK_GET_HARD_MODEL_HANDLE                       "_kdk_get_hard_model"
#define KDK_GET_HARD_SERIAL_HANDLE                      "_kdk_get_hard_serial"

typedef kdk_diskinfo *(*handle_kdk_get_diskinfo)(const char *diskname);
typedef char *(*handle_kdk_get_hard_disk_size)(const char *diskname);
typedef char *(*handle_kdk_get_hard_fwrev)(const char *diskname);
typedef char *(*handle_kdk_get_hard_type)(const char *diskname);
typedef char *(*handle_kdk_get_hard_model)(const char *diskname);
typedef char *(*handle_kdk_get_hard_serial)(const char *diskname);

#define LINGMOSDK_HW_LIB                                    "libkyhw.so"
#define KDK_DISPLAY_GET_VENDOR_HANDLE                   "_kdk_display_get_vendor"
#define KDK_DISPLAY_GET_PRODUCT_HANDLE                  "_kdk_display_get_product"
#define KDK_DISPLAY_GET_DESCRIPTION_HANDLE              "_kdk_display_get_description"
#define KDK_DISPLAY_GET_PHYSICAL_ID_HANDLE              "_kdk_display_get_physical_id"
#define KDK_DISPLAY_GET_BUS_INFO_HANDLE                 "_kdk_display_get_bus_info"
#define KDK_DISPLAY_GET_VERSION_HANDLE                  "_kdk_display_get_version"
#define KDK_DISPLAY_GET_WIDTH_HANDLE                    "_kdk_display_get_width"
#define KDK_DISPLAY_GET_CLOCK_HANDLE                    "_kdk_display_get_clock"
#define KDK_DISPLAY_GET_CAPABILITIES_HANDLE             "_kdk_display_get_capabilities"
#define KDK_DISPLAY_GET_CONFIGURATION_HANDLE            "_kdk_display_get_configuration"
#define KDK_DISPLAY_GET_RESOURCES_HANDLE                "_kdk_display_get_resources"

typedef char *(*handle_kdk_display_get_vendor)();
typedef char *(*handle_kdk_display_get_product)();
typedef char *(*handle_kdk_display_get_description)();
typedef char *(*handle_kdk_display_get_physical_id)();
typedef char *(*handle_kdk_display_get_bus_info)();
typedef char *(*handle_kdk_display_get_version)();
typedef char *(*handle_kdk_display_get_width)();
typedef char *(*handle_kdk_display_get_clock)();
typedef char *(*handle_kdk_display_get_capabilities)();
typedef char *(*handle_kdk_display_get_configuration)();
typedef char *(*handle_kdk_display_get_resources)();

#define KDK_BOARD_GET_SERIAL_HANDLE                     "_kdk_board_get_serial"

typedef char *(*handle_kdk_board_get_serial)();

#define KDK_USB_GET_PRODUCTNAME_HANDLE                  "_kdk_usb_get_productName"
#define KDK_USB_GET_MANUFACTURERNAME_HANDLE             "_kdk_usb_get_manufacturerName"

typedef char *(*handle_kdk_usb_get_productName)(int busNum, int devAddress);
typedef char *(*handle_kdk_usb_get_manufacturerName)(int busNum, int devAddress);

#define LINGMOSDK_NETINFO_LIB                               "libkynetinfo.so"
#define KDK_NET_GET_IPTABLE_RULES_HANDLE                "_kdk_net_get_iptable_rules"
#define KDK_NET_GET_PROC_PORT_HANDLE                    "_kdk_net_get_proc_port"

typedef pChain (*handle_kdk_net_get_iptable_rules)();
typedef char **(*handle_kdk_net_get_proc_port)();

#define LINGMOSDK_REALTIME_LIB                              "libkyrealtime.so"
#define KDK_REAL_GET_DISK_TEMPERATURE_HANDLE            "_kdk_real_get_disk_temperature"
#define KDK_REAL_GET_DISK_RATE_HANDLE                   "_kdk_real_get_disk_rate"

typedef int (*handle_kdk_real_get_disk_temperature)(const char *diskname);
typedef int (*handle_kdk_real_get_disk_rate)(const char *diskname);

#define LINGMOSDK_ACCOUNTS_LIB                              "liblingmosdk-accounts.so"
#define KDK_SYSTEM_CHANGE_PASSWORD_HANDLE            "_kdk_system_change_password"

typedef bool (*handle_kdk_system_change_password)(const char *username, const char *password, int *err_num);

KySdkDisk::KySdkDisk(){}

KySdkDisk::~KySdkDisk(){}

QStringList KySdkDisk::getDiskList() const
{
    QStringList res = {};
    char** disk = kdk_get_disklist();
    for (int i = 0; disk[i]; i++)
    {
        res << QString::fromLocal8Bit(disk[i]);
    }

    return res;
}

unsigned int KySdkDisk::getDiskSectorSize(const QString diskname) const
{
    std::string dn = diskname.toStdString();
    // kdk_diskinfo* p = kdk_get_diskinfo(dn.c_str());
    // if(p != NULL)
    // {
    //     return p->sector_size;
    // }else{
    //     return NULL;
    // }

    void* libHandle = dlopen(LINGMOSDK_DISK_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return 0;
    }
    
    handle_kdk_get_diskinfo p_kdk_get_diskinfo_handle = NULL;
    *(void **) (&p_kdk_get_diskinfo_handle) = dlsym(libHandle, KDK_GET_DISKINFO_HANDLE);

    if(!p_kdk_get_diskinfo_handle)
    {
        dlclose(libHandle);
        return 0;
    }

    kdk_diskinfo *diskinfo = p_kdk_get_diskinfo_handle(dn.c_str());
    
    if(!diskinfo)
    {
        dlclose(libHandle);
        return 0;
    }
    dlclose(libHandle);
    
    return diskinfo->sector_size;
}

double KySdkDisk::getDiskTotalSizeMiB(const QString diskname) const
{
    std::string dn = diskname.toStdString();
    // kdk_diskinfo* p = kdk_get_diskinfo(dn.c_str());
    // if(p != NULL)
    // {
    //     return static_cast<double>(p->total_size_MiB);
    // }else{
    //     return NULL;
    // }

    void* libHandle = dlopen(LINGMOSDK_DISK_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return 0.00;
    }
    
    handle_kdk_get_diskinfo p_kdk_get_diskinfo_handle = NULL;
    *(void **) (&p_kdk_get_diskinfo_handle) = dlsym(libHandle, KDK_GET_DISKINFO_HANDLE);

    if(!p_kdk_get_diskinfo_handle)
    {
        dlclose(libHandle);
        return 0.00;
    }

    kdk_diskinfo *diskinfo = p_kdk_get_diskinfo_handle(dn.c_str());
    
    if(!diskinfo)
    {
        dlclose(libHandle);
        return 0.00;
    }
    dlclose(libHandle);
    
    return static_cast<double>(diskinfo->total_size_MiB);
}

QString KySdkDisk::getDiskModel(const QString diskname) const
{
    std::string dn = diskname.toStdString();
    // kdk_diskinfo* p = kdk_get_diskinfo(dn.c_str());
    // if(p != NULL)
    // {
    //     return p->model;
    // }else{
    //     return NULL;
    // }

    void* libHandle = dlopen(LINGMOSDK_DISK_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    
    handle_kdk_get_diskinfo p_kdk_get_diskinfo_handle = NULL;
    *(void **) (&p_kdk_get_diskinfo_handle) = dlsym(libHandle, KDK_GET_DISKINFO_HANDLE);

    if(!p_kdk_get_diskinfo_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    kdk_diskinfo *diskinfo = p_kdk_get_diskinfo_handle(dn.c_str());
    
    if(!diskinfo)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return diskinfo->model;
}

QString KySdkDisk::getDiskSerial(const QString diskname) const
{
    std::string dn = diskname.toStdString();
//     kdk_diskinfo* p = kdk_get_diskinfo(dn.c_str());
//     if(p != NULL)
//     {
//         return p->serial;
//     }else{
//         return NULL;
//     }

    void* libHandle = dlopen(LINGMOSDK_DISK_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    
    handle_kdk_get_diskinfo p_kdk_get_diskinfo_handle = NULL;
    *(void **) (&p_kdk_get_diskinfo_handle) = dlsym(libHandle, KDK_GET_DISKINFO_HANDLE);

    if(!p_kdk_get_diskinfo_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    kdk_diskinfo *diskinfo = p_kdk_get_diskinfo_handle(dn.c_str());
    
    if(!diskinfo)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return diskinfo->serial;
}

unsigned int KySdkDisk::getDiskPartitionNums(const QString diskname) const
{
    std::string dn = diskname.toStdString();
    // kdk_diskinfo* p = kdk_get_diskinfo(dn.c_str());
    // if(p != NULL)
    // {
    //     return p->partition_nums;
    // }else{
    //     return NULL;
    // }
    void* libHandle = dlopen(LINGMOSDK_DISK_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return 0;
    }
    
    handle_kdk_get_diskinfo p_kdk_get_diskinfo_handle = NULL;
    *(void **) (&p_kdk_get_diskinfo_handle) = dlsym(libHandle, KDK_GET_DISKINFO_HANDLE);

    if(!p_kdk_get_diskinfo_handle)
    {
        dlclose(libHandle);
        return 0;
    }

    kdk_diskinfo *diskinfo = p_kdk_get_diskinfo_handle(dn.c_str());
    
    if(!diskinfo)
    {
        dlclose(libHandle);
        return 0;
    }
    dlclose(libHandle);
    
    return diskinfo->partition_nums;
}

QString KySdkDisk::getDiskType(const QString diskname) const
{
    std::string dn = diskname.toStdString();
    // kdk_diskinfo* p = kdk_get_diskinfo(dn.c_str());
    void* libHandle = dlopen(LINGMOSDK_DISK_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    
    handle_kdk_get_diskinfo p_kdk_get_diskinfo_handle = NULL;
    *(void **) (&p_kdk_get_diskinfo_handle) = dlsym(libHandle, KDK_GET_DISKINFO_HANDLE);

    if(!p_kdk_get_diskinfo_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    kdk_diskinfo *diskinfo = p_kdk_get_diskinfo_handle(dn.c_str());
    
    if(!diskinfo)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    if(diskinfo != NULL)
    {
        if(diskinfo->disk_type == DISK_TYPE_HDD) {
            return "DISK_TYPE_HDD";
        }
        else if(diskinfo->disk_type == DISK_TYPE_SSD) {
            return "DISK_TYPE_SSD";
        }
        else if (diskinfo->disk_type == DISK_TYPE_NONE)
        {
            return "DISK_TYPE_NONE";
        }
        else return "DISK_TYPE_OTHER";
    }else{
        return NULL;
    }
}

QString KySdkDisk::getDiskVersion(const QString diskname) const
{
    std::string dn = diskname.toStdString();
    // kdk_diskinfo* p = kdk_get_diskinfo(dn.c_str());
    // if(p != NULL)
    // {
    //     return QString::fromLocal8Bit((p->fwrev)).remove("\n");
    // }else{
    //     return NULL;
    // }
    void* libHandle = dlopen(LINGMOSDK_DISK_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    
    handle_kdk_get_diskinfo p_kdk_get_diskinfo_handle = NULL;
    *(void **) (&p_kdk_get_diskinfo_handle) = dlsym(libHandle, KDK_GET_DISKINFO_HANDLE);

    if(!p_kdk_get_diskinfo_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    kdk_diskinfo *diskinfo = p_kdk_get_diskinfo_handle(dn.c_str());
    
    if(!diskinfo)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    return QString::fromLocal8Bit((diskinfo->fwrev)).remove("\n");
}

unsigned long long KySdkDisk::getDiskSectorNum(const QString diskname) const
{
    std::string dn = diskname.toStdString();
    // kdk_diskinfo* p = kdk_get_diskinfo(dn.c_str());
    // if(p != NULL)
    // {
    //     return p->sectors_num;
    // }else{
    //     return NULL;
    // }

    void* libHandle = dlopen(LINGMOSDK_DISK_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return 0.00;
    }
    
    handle_kdk_get_diskinfo p_kdk_get_diskinfo_handle = NULL;
    *(void **) (&p_kdk_get_diskinfo_handle) = dlsym(libHandle, KDK_GET_DISKINFO_HANDLE);

    if(!p_kdk_get_diskinfo_handle)
    {
        dlclose(libHandle);
        return 0.00;
    }

    kdk_diskinfo *diskinfo = p_kdk_get_diskinfo_handle(dn.c_str());
    
    if(!diskinfo)
    {
        dlclose(libHandle);
        return 0.00;
    }
    dlclose(libHandle);
    return diskinfo->sectors_num;
}

QString KySdkDisk::getHardDiskSize(const QString diskname) const
{
    std::string dn = diskname.toStdString();
    // return QString::fromLocal8Bit(kdk_get_hard_disk_size(dn.c_str()));
    void* libHandle = dlopen(LINGMOSDK_DISK_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    
    handle_kdk_get_hard_disk_size p_kdk_get_hard_disk_size_handle = NULL;
    *(void **) (&p_kdk_get_hard_disk_size_handle) = dlsym(libHandle, KDK_GET_HARD_DISK_SIZE_HANDLE);

    if(!p_kdk_get_hard_disk_size_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char *size = p_kdk_get_hard_disk_size_handle(dn.c_str());
    
    if(!size)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(size);
}

QString KySdkDisk::getHardFwrev(const QString diskname) const
{
    std::string dn = diskname.toStdString();
    // return QString::fromLocal8Bit(kdk_get_hard_fwrev(dn.c_str()));
    void* libHandle = dlopen(LINGMOSDK_DISK_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    
    handle_kdk_get_hard_fwrev p_kdk_get_hard_fwrev_handle = NULL;
    *(void **) (&p_kdk_get_hard_fwrev_handle) = dlsym(libHandle, KDK_GET_HARD_FWREV_HANDLE);

    if(!p_kdk_get_hard_fwrev_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char *fwrev = p_kdk_get_hard_fwrev_handle(dn.c_str());
    
    if(!fwrev)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(fwrev);
}

QString KySdkDisk::getHardType(const QString diskname) const
{
    std::string dn = diskname.toStdString();
    // return QString::fromLocal8Bit(kdk_get_hard_type(dn.c_str()));
    void* libHandle = dlopen(LINGMOSDK_DISK_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    
    handle_kdk_get_hard_type p_kdk_get_hard_type_handle = NULL;
    *(void **) (&p_kdk_get_hard_type_handle) = dlsym(libHandle, KDK_GET_HARD_TYPE_HANDLE);

    if(!p_kdk_get_hard_type_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char *type = p_kdk_get_hard_type_handle(dn.c_str());
    
    if(!type)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(type);
}

QString KySdkDisk::getHardModel(const QString diskname) const
{
    std::string dn = diskname.toStdString();
    // return QString::fromLocal8Bit(kdk_get_hard_model(dn.c_str()));
    void* libHandle = dlopen(LINGMOSDK_DISK_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    
    handle_kdk_get_hard_model p_kdk_get_hard_model_handle = NULL;
    *(void **) (&p_kdk_get_hard_model_handle) = dlsym(libHandle, KDK_GET_HARD_MODEL_HANDLE);

    if(!p_kdk_get_hard_model_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char *model = p_kdk_get_hard_model_handle(dn.c_str());
    
    if(!model)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(model);
}

QString KySdkDisk::getHardSerial(const QString diskname) const
{
    std::string dn = diskname.toStdString();
    // return QString::fromLocal8Bit(kdk_get_hard_serial(dn.c_str()));
    void* libHandle = dlopen(LINGMOSDK_DISK_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    
    handle_kdk_get_hard_serial p_kdk_get_hard_serial_handle = NULL;
    *(void **) (&p_kdk_get_hard_serial_handle) = dlsym(libHandle, KDK_GET_HARD_SERIAL_HANDLE);

    if(!p_kdk_get_hard_serial_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char *serial = p_kdk_get_hard_serial_handle(dn.c_str());
    
    if(!serial)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(serial);
}

KySdkCpuInfo::KySdkCpuInfo(){}

KySdkCpuInfo::~KySdkCpuInfo(){}

QString KySdkCpuInfo::getCpuArch() const
{
    return QString::fromLocal8Bit(kdk_cpu_get_arch());
}

QString KySdkCpuInfo::getCpuVendor() const
{
    return QString::fromLocal8Bit(kdk_cpu_get_vendor());
}

QString KySdkCpuInfo::getCpuModel() const
{
    return QString::fromLocal8Bit(kdk_cpu_get_model());
}

QString KySdkCpuInfo::getCpuFreqMHz() const
{
    return QString::fromLocal8Bit(kdk_cpu_get_freq_MHz());
}

unsigned int KySdkCpuInfo::getCpuCorenums() const
{
    return kdk_cpu_get_corenums();
}

QString KySdkCpuInfo::getCpuVirt() const
{
    return QString::fromLocal8Bit(kdk_cpu_get_virt());
}

unsigned int KySdkCpuInfo::getCpuProcess() const
{
    return kdk_cpu_get_process();
}

KySdkNetCard::KySdkNetCard(){}

KySdkNetCard::~KySdkNetCard(){}

QStringList KySdkNetCard::getNetCardName() const
{
    QStringList res = {};
    char** net = kdk_nc_get_cardlist();
    for (int i = 0; net[i]; i++)
    {
        res << QString::fromLocal8Bit(net[i]);
    }
    return res;
}

int KySdkNetCard::getNetCardUp(const QString netCardName) const
{
    std::string stdStr = netCardName.toStdString();
    return kdk_nc_is_up(stdStr.c_str());
}

QStringList KySdkNetCard::getNetCardUpcards() const
{
    QStringList res = {};
    char** upcards = kdk_nc_get_upcards();
    for (int i = 0; upcards[i]; i++)
    {
        res << QString::fromLocal8Bit(upcards[i]);
    }
    return res;
}

QString KySdkNetCard::getNetCardPhymac(const QString netCardName) const
{
    std::string stdStr = netCardName.toStdString();
    return QString::fromLocal8Bit(kdk_nc_get_phymac(stdStr.c_str()));
}

QString KySdkNetCard::getNetCardPrivateIPv4(const QString netCardName) const
{
    std::string stdStr = netCardName.toStdString();
    return QString::fromLocal8Bit(kdk_nc_get_private_ipv4(stdStr.c_str()));
}

QStringList KySdkNetCard::getNetCardIPv4(const QString netCardName) const
{
    QStringList res;
    char ** list = kdk_nc_get_ipv4(netCardName.toStdString().c_str());
    int i = 0;
    while(list && list[i])
    {
        res << list[i++];
    }
    return res;
}

QString KySdkNetCard::getNetCardPrivateIPv6(const QString netCardName) const
{
    std::string stdStr = netCardName.toStdString();
    return QString::fromLocal8Bit(kdk_nc_get_private_ipv6(stdStr.c_str()));
}

int KySdkNetCard::getNetCardType(const QString netCardName) const
{
    std::string stdStr = netCardName.toStdString();
    return kdk_nc_is_wireless(stdStr.c_str());
}

QStringList KySdkNetCard::getNetCardProduct(const QString netCardName) const
{
    std::string ncn = netCardName.toStdString();
    QStringList res = {};
    char vend[256] = "", prod[256] = "";
    if(kdk_nc_get_vendor_and_product(ncn.c_str(), vend, prod) == 0) {
        res << QString::fromLocal8Bit(vend).remove("\n");
        res << QString::fromLocal8Bit(prod).remove("\n");
    }
//    qFatal("kdk_nc_get_vendor_and_product 调用失败");
    return res;
}

QStringList KySdkNetCard::getNetCardIPv6(const QString netCardName) const
{
    QStringList res;
    char **list = kdk_nc_get_ipv6(netCardName.toStdString().c_str());
    int i = 0;
    while (list && list[i])
    {
        res << list[i++];
    }
    return res;
}

KySdkBios::KySdkBios(){}

KySdkBios::~KySdkBios(){}

QString KySdkBios::getBiosVendor() const
{
    return QString::fromLocal8Bit(kdk_bios_get_vendor()).remove("\n");
}

QString KySdkBios::getBiosVersion() const
{
    return QString::fromLocal8Bit(kdk_bios_get_version()).remove("\n");
}

KySdkMainBoard::KySdkMainBoard(){}

KySdkMainBoard::~KySdkMainBoard(){}

QString KySdkMainBoard::getMainboardName() const
{
   return QString::fromLocal8Bit(kdk_board_get_name()).remove("\n");
}

QString KySdkMainBoard::getMainboardDate() const
{
   return QString::fromLocal8Bit(kdk_board_get_date()).remove("\n");
}

QString KySdkMainBoard::getMainboardSerial() const
{
//    return QString::fromLocal8Bit(kdk_board_get_serial()).remove("\n");

    void* libHandle = dlopen(LINGMOSDK_HW_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    handle_kdk_board_get_serial p_kdk_board_get_serial_handle = NULL;
    *(void **) (&p_kdk_board_get_serial_handle) = dlsym(libHandle, KDK_BOARD_GET_SERIAL_HANDLE);

    if(!p_kdk_board_get_serial_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char* serial = p_kdk_board_get_serial_handle();
    
    if(!serial)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(serial).remove("\n");
}

QString KySdkMainBoard::getMainboardVendor() const
{
   return QString::fromLocal8Bit(kdk_board_get_vendor()).remove("\n");
}

KySdkPeripheralsEnum::KySdkPeripheralsEnum(){}

QStringList KySdkPeripheralsEnum::getAllUsbInfo() const
{
    QStringList res = {};
    pDriverInfoList usb = kdk_usb_get_list();
    if(usb)
    {
        pDriverInfoList usb_list = usb;
        while(usb_list)
        {
            res << "name:" + QString::fromLocal8Bit(usb_list->data->name) + "," +
                   "type:" + QString::fromLocal8Bit(usb_list->data->type) + "," +
                   "pid:" + QString::fromLocal8Bit(usb_list->data->pid) + "," +
                   "vid:" + QString::fromLocal8Bit(usb_list->data->vid) + "," +
                   "serialNo:" + QString::fromLocal8Bit(usb_list->data->serialNo) + "," +
                   "devNode:" + QString::fromLocal8Bit(usb_list->data->devNode);
            usb_list = usb_list->next;
        }
    }
    return res;
}

QString KySdkPeripheralsEnum::getUsbProductName(int busNum, int devAddress) const
{
    void* libHandle = dlopen(LINGMOSDK_HW_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return 0;
    }
    
    handle_kdk_usb_get_productName p_kdk_usb_get_productName_handle = NULL;
    *(void **) (&p_kdk_usb_get_productName_handle) = dlsym(libHandle, KDK_USB_GET_PRODUCTNAME_HANDLE);

    if(!p_kdk_usb_get_productName_handle)
    {
        dlclose(libHandle);
        return 0;
    }

    char* productName = p_kdk_usb_get_productName_handle(busNum, devAddress);     
    if(!productName)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(productName);
}

QString KySdkPeripheralsEnum::getUsbManufacturerName(int busNum, int devAddress) const
{
    void* libHandle = dlopen(LINGMOSDK_HW_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return 0;
    }
    
    handle_kdk_usb_get_manufacturerName p_kdk_usb_get_manufacturerName_handle = NULL;
    *(void **) (&p_kdk_usb_get_manufacturerName_handle) = dlsym(libHandle, KDK_USB_GET_MANUFACTURERNAME_HANDLE);

    if(!p_kdk_usb_get_manufacturerName_handle)
    {
        dlclose(libHandle);
        return 0;
    }

    char* manufacturerName = p_kdk_usb_get_manufacturerName_handle(busNum, devAddress);     
    if(!manufacturerName)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(manufacturerName);
}

KySdkDisplay::KySdkDisplay(){}

KySdkDisplay::~KySdkDisplay(){}

QString KySdkDisplay::getDisplayVendor() const
{
    // return QString::fromLocal8Bit(kdk_display_get_vendor());
    void* libHandle = dlopen(LINGMOSDK_HW_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    handle_kdk_display_get_vendor p_kdk_display_get_vendor_handle = NULL;
    *(void **) (&p_kdk_display_get_vendor_handle) = dlsym(libHandle, KDK_DISPLAY_GET_VENDOR_HANDLE);

    if(!p_kdk_display_get_vendor_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char* vendor = p_kdk_display_get_vendor_handle();
    
    if(!vendor)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(vendor);
}

QString KySdkDisplay::getDisplayProduct() const
{
    // return QString::fromLocal8Bit(kdk_display_get_product());
    void* libHandle = dlopen(LINGMOSDK_HW_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    handle_kdk_display_get_product p_kdk_display_get_product_handle = NULL;
    *(void **) (&p_kdk_display_get_product_handle) = dlsym(libHandle, KDK_DISPLAY_GET_PRODUCT_HANDLE);

    if(!p_kdk_display_get_product_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char* product = p_kdk_display_get_product_handle();
    
    if(!product)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(product);
}

QString KySdkDisplay::getDisplayDescription() const
{
    // return QString::fromLocal8Bit(kdk_display_get_description());
    void* libHandle = dlopen(LINGMOSDK_HW_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    handle_kdk_display_get_description p_kdk_display_get_description_handle = NULL;
    *(void **) (&p_kdk_display_get_description_handle) = dlsym(libHandle, KDK_DISPLAY_GET_DESCRIPTION_HANDLE);

    if(!p_kdk_display_get_description_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char* description = p_kdk_display_get_description_handle();
    
    if(!description)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(description);
}

QString KySdkDisplay::getDisplayPhysicalId() const
{
    // return QString::fromLocal8Bit(kdk_display_get_physical_id());
    void* libHandle = dlopen(LINGMOSDK_HW_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    handle_kdk_display_get_physical_id p_kdk_display_get_physical_id_handle = NULL;
    *(void **) (&p_kdk_display_get_physical_id_handle) = dlsym(libHandle, KDK_DISPLAY_GET_PHYSICAL_ID_HANDLE);

    if(!p_kdk_display_get_physical_id_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char* id = p_kdk_display_get_physical_id_handle();
    
    if(!id)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(id);
}

QString KySdkDisplay::getDisplayBusInfo() const
{
    // return QString::fromLocal8Bit(kdk_display_get_bus_info());
    void* libHandle = dlopen(LINGMOSDK_HW_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    handle_kdk_display_get_bus_info p_kdk_display_get_bus_info_handle = NULL;
    *(void **) (&p_kdk_display_get_bus_info_handle) = dlsym(libHandle, KDK_DISPLAY_GET_BUS_INFO_HANDLE);

    if(!p_kdk_display_get_bus_info_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char* info = p_kdk_display_get_bus_info_handle();
    
    if(!info)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(info);
}

QString KySdkDisplay::getDisplayVersion() const
{
    // return QString::fromLocal8Bit(kdk_display_get_version());
    void* libHandle = dlopen(LINGMOSDK_HW_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    handle_kdk_display_get_version p_kdk_display_get_version_handle = NULL;
    *(void **) (&p_kdk_display_get_version_handle) = dlsym(libHandle, KDK_DISPLAY_GET_VERSION_HANDLE);

    if(!p_kdk_display_get_version_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char* version = p_kdk_display_get_version_handle();
    
    if(!version)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(version);
}

QString KySdkDisplay::getDisplayWidth() const
{
    // return QString::fromLocal8Bit(kdk_display_get_width());
    void* libHandle = dlopen(LINGMOSDK_HW_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    handle_kdk_display_get_width p_kdk_display_get_width_handle = NULL;
    *(void **) (&p_kdk_display_get_width_handle) = dlsym(libHandle, KDK_DISPLAY_GET_WIDTH_HANDLE);

    if(!p_kdk_display_get_width_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char* width = p_kdk_display_get_width_handle();
    
    if(!width)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(width);
}

QString KySdkDisplay::getDisplayClock() const
{
    // return QString::fromLocal8Bit(kdk_display_get_clock());
    void* libHandle = dlopen(LINGMOSDK_HW_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    handle_kdk_display_get_clock p_kdk_display_get_clock_handle = NULL;
    *(void **) (&p_kdk_display_get_clock_handle) = dlsym(libHandle, KDK_DISPLAY_GET_CLOCK_HANDLE);

    if(!p_kdk_display_get_clock_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char* clock = p_kdk_display_get_clock_handle();
    
    if(!clock)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(clock);
}

QString KySdkDisplay::getDisplayCapabilities() const
{
    // return QString::fromLocal8Bit(kdk_display_get_capabilities());
    void* libHandle = dlopen(LINGMOSDK_HW_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    handle_kdk_display_get_capabilities p_kdk_display_get_capabilities_handle = NULL;
    *(void **) (&p_kdk_display_get_capabilities_handle) = dlsym(libHandle, KDK_DISPLAY_GET_CAPABILITIES_HANDLE);

    if(!p_kdk_display_get_capabilities_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char* capabilities = p_kdk_display_get_capabilities_handle();
    
    if(!capabilities)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(capabilities);
}

QString KySdkDisplay::getDisplayConfiguration() const
{
    // return QString::fromLocal8Bit(kdk_display_get_configuration());
    void* libHandle = dlopen(LINGMOSDK_HW_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    handle_kdk_display_get_configuration p_kdk_display_get_configuration_handle = NULL;
    *(void **) (&p_kdk_display_get_configuration_handle) = dlsym(libHandle, KDK_DISPLAY_GET_CONFIGURATION_HANDLE);

    if(!p_kdk_display_get_configuration_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char* configuration = p_kdk_display_get_configuration_handle();
    
    if(!configuration)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(configuration);
}

QString KySdkDisplay::getDisplayResources() const
{
    // return QString::fromLocal8Bit(kdk_display_get_resources());
    void* libHandle = dlopen(LINGMOSDK_HW_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    handle_kdk_display_get_resources p_kdk_display_get_resources_handle = NULL;
    *(void **) (&p_kdk_display_get_resources_handle) = dlsym(libHandle, KDK_DISPLAY_GET_RESOURCES_HANDLE);

    if(!p_kdk_display_get_resources_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char* resources = p_kdk_display_get_resources_handle();
    
    if(!resources)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(resources);
}


KySdkPackageInfo::KySdkPackageInfo(){}

QStringList KySdkPackageInfo::getPackageList() const
{
    QStringList res = {};
    kdk_package_list *pack = kdk_package_get_packagelist();
    qDebug() << pack->nums;
    for (size_t i = 0; i < pack->nums; i ++)
    {
        res << "包名：" + QString::fromLocal8Bit(pack->list[i]->name) + "," +
               "版本号：" + QString::fromLocal8Bit(pack->list[i]->version) + "," +
               "包类型：" + QString::fromLocal8Bit(pack->list[i]->section) + "," +
               "包状态：" + QString::fromLocal8Bit(pack->list[i]->status) + "," +
               "包大小：" + QString::number(pack->list[i]->size_kb);
    }
    return res;
}

QString KySdkPackageInfo::getPackageVersion(const QString packageName) const
{
    std::string package = packageName.toStdString();
    return QString::fromLocal8Bit(kdk_package_get_version(package.c_str()));
}

int KySdkPackageInfo::getPackageInstalled(const QString packageName, const QString version) const
{
    std::string package = packageName.toStdString();
    std::string ver = version.toStdString();
    return kdk_package_is_installed(package.c_str(), ver.c_str());
}

KySdkResource::KySdkResource(){}

double KySdkResource::getMemTotalKiB() const
{
    return static_cast<double>(kdk_rti_get_mem_res_total_KiB());
}

double KySdkResource::getMemUsagePercent() const
{
    return static_cast<double>(kdk_rti_get_mem_res_usage_percent());
}

double KySdkResource::getMemUsageKiB() const
{
    return static_cast<double>(kdk_rti_get_mem_res_usage_KiB());
}

double KySdkResource::getMemAvailableKiB() const
{
    return static_cast<double>(kdk_rti_get_mem_res_available_KiB());
}

double KySdkResource::getMemFreeKiB() const
{
    return static_cast<double>(kdk_rti_get_mem_res_free_KiB());
}

double KySdkResource::getMemVirtAllocKiB() const
{
    return kdk_rti_get_mem_virt_alloc_KiB();
}

double KySdkResource::getMemSwapTotalKiB() const
{
    return static_cast<double>(kdk_rti_get_mem_swap_total_KiB());
}

double KySdkResource::getMemSwapUsagePercent() const
{
    return static_cast<double>(kdk_rti_get_mem_swap_usage_percent());
}

double KySdkResource::getMemSwapUsageKiB() const
{
    return static_cast<double>(kdk_rti_get_mem_swap_usage_KiB());
}

double KySdkResource::getMemSwapFreeKiB() const
{
    return static_cast<double>(kdk_rti_get_mem_swap_free_KiB());
}

double KySdkResource::getCpuCurrentUsage() const
{
    return static_cast<double>(kdk_rti_get_cpu_current_usage());
}

QString KySdkResource::getUpTime() const
{
    unsigned int day, hour, min, sec;
    if(kdk_rti_get_uptime(&day, &hour, &min, &sec) == 0)
    {
         return QString::number(day) + "天" + QString::number(hour) + "小时" + QString::number(min) + "分" + QString::number(sec) + "秒";
    }

}

KySdkProcess::KySdkProcess(){}

double KySdkProcess::getProcInfoCpuUsage(int pid) const
{
    return static_cast<double>(kdk_get_process_cpu_usage_percent(pid));
}

double KySdkProcess::getProcInfoMemUsage(int pid) const
{
    return static_cast<double>(kdk_get_process_mem_usage_percent(pid));
}

QString KySdkProcess::getProcInfoStatus(int pid) const
{
    return QString::fromLocal8Bit(kdk_get_process_status(pid));
}

int KySdkProcess::getProcInfoPort(int pid) const
{
    return kdk_get_process_port(pid);
}

QString KySdkProcess::getProcInfoStartTime(int pid) const
{
    return QString::fromLocal8Bit(kdk_get_process_start_time(pid)).remove("\n");
}

QString KySdkProcess::getProcInfoRunningTime(int pid) const
{
    return QString::fromLocal8Bit(kdk_get_process_running_time(pid));
}

QString KySdkProcess::getProcInfoCpuTime(int pid) const
{
    return QString::fromLocal8Bit(kdk_get_process_cpu_time(pid));
}

QString KySdkProcess::getProcInfoCmd(int pid) const
{
    return QString::fromLocal8Bit(kdk_get_process_command(pid));
}

QString KySdkProcess::getProcInfoUser(int pid) const
{
    return QString::fromLocal8Bit(kdk_get_process_user(pid));
}

QStringList KySdkProcess::getProcInfo(const QString procName) const
{
    std::string pN = procName.toStdString();
    QStringList res = {};
    char **proc = kdk_procname_get_process_infomation(const_cast<char*>(pN.c_str()));
    size_t count = 0;
    while (proc[count])
    {
        res << QString::fromLocal8Bit(proc[count]).remove("\n");
        count++;
    }
    kdk_proc_freeall(proc);
    return res;
}

QStringList KySdkProcess::getAllProcInfo() const
{
    QStringList res = {};
    char** proc_info = kdk_get_process_all_information();
    size_t count = 0;
    while (proc_info[count])
    {
        res << QString::fromLocal8Bit(proc_info[count]).remove("\n");
        count++;
    }
    kdk_proc_freeall(proc_info);
    return res;
}

KySdkResolution::KySdkResolution(){}

QString KySdkResolution::getSystemArchitecture() const
{
    return QString::fromLocal8Bit(kdk_system_get_architecture());
}

QString KySdkResolution::getSystemName() const
{
    return QString::fromLocal8Bit(kdk_system_get_systemName());
}

QString KySdkResolution::getSystemVersion(bool verbose) const
{
    return QString::fromLocal8Bit(kdk_system_get_version(verbose));
}

int KySdkResolution::getSystemActivationStatus() const
{
    int p,d;
    return kdk_system_get_activationStatus(&p, &d);
}

QString KySdkResolution::getSystemSerialNumber() const
{
    return QString::fromLocal8Bit(kdk_system_get_serialNumber()).remove("\n");;
}

QString KySdkResolution::getSystemKernelVersion() const
{
    return QString::fromLocal8Bit(kdk_system_get_kernelVersion()).remove("\n");
}

QString KySdkResolution::getSystemEffectUser() const
{
    return QString::fromLocal8Bit(kdk_system_get_eUser());
}

QString KySdkResolution::getSystemEffectUserLoginTime() const
{
    return QString::fromLocal8Bit(kdk_system_get_eUser_login_time());
}

QString KySdkResolution::getSystemProjectName() const
{
    return QString::fromLocal8Bit(kdk_system_get_projectName());
}

QString KySdkResolution::getSystemProjectSubName() const
{
    return QString::fromLocal8Bit(kdk_system_get_projectSubName());
}

unsigned int KySdkResolution::getSystemProductFeatures() const
{
    return kdk_system_get_productFeatures();
}

QString KySdkResolution::getSystemHostVirtType() const
{
    return QString::fromLocal8Bit(kdk_system_get_hostVirtType());
}

QString KySdkResolution::getSystemHostCloudPlatform() const
{
    return QString::fromLocal8Bit(kdk_system_get_hostCloudPlatform());
}

QString KySdkResolution::getSystemOSVersion() const
{
    version_t ver = kdk_system_get_version_detaile();
    return QString::fromLocal8Bit(ver.os_version);
}

QString KySdkResolution::getSystemUpdateVersion() const
{
    version_t ver = kdk_system_get_version_detaile();
    return QString::fromLocal8Bit(ver.update_version);
}

bool KySdkResolution::getSystemIsZYJ() const
{
    return kdk_system_is_zyj();
}

QString KySdkResolution::getHostSerial() const
{
    // return QString::fromLocal8Bit(_kdk_get_host_serial());
    void* libHandle = dlopen(LINGMOSDK_SYSINFO_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return NULL;
    }
    handle_kdk_get_host_serial p_kdk_get_host_serial_handle = NULL;
    *(void **) (&p_kdk_get_host_serial_handle) = dlsym(libHandle, KDK_GET_HOST_SERIAL_HANDLE);

    if(!p_kdk_get_host_serial_handle)
    {
        dlclose(libHandle);
        return NULL;
    }

    char* serial = p_kdk_get_host_serial_handle();
    
    if(!serial)
    {
        dlclose(libHandle);
        return NULL;
    }
    dlclose(libHandle);
    
    return QString::fromLocal8Bit(serial);
}

KySdkNet::KySdkNet()
{
}

int KySdkNet::getPortState(int port) const
{
    return kdk_net_get_port_stat(port);
}

QStringList KySdkNet::getMultiplePortStat(int start, int end) const
{
    QStringList res = {};
    if(end < start)
        return res;
    int result[end - start + 1];
    if(0 == kdk_net_get_multiple_port_stat(start, end, result))
    {
        size_t count = 0;
        size_t num = end - start + 1;
        for(;count < num; count++)
        {
            res << QString::number(result[count]);
        }
    }
    return res;
}

QStringList KySdkNet::getGatewayInfo() const
{
    prouteMapList p = kdk_net_get_route();
    QStringList res = {};
    if(p == NULL)
    {
         return res;
    }
    while(p) {
        res << QString(QLatin1String(p->name)) << QString(QLatin1String(p->addr));
        p = p->next;
    }
    return res;
}

QStringList KySdkNet::getFirewallState() const
{
    QStringList res = {};
    // pChain chain = kdk_net_get_iptable_rules();
    // pChain tmpchain = chain;
    // while(tmpchain)
    // {
    //     pChainData tmpData = tmpchain->data;
    //     res << "total:" + QString::fromLocal8Bit(tmpchain->total) + "," +
    //            "policy:" + QString::fromLocal8Bit(tmpchain->policy);
    //     if(tmpData)
    //     {
    //         res << "target:" + QString::fromLocal8Bit(tmpData->target) + "," +
    //                "prot:" + QString::fromLocal8Bit(tmpData->prot) + "," +
    //                "opt:" + QString::fromLocal8Bit(tmpData->opt) + "," +
    //                "source:" + QString::fromLocal8Bit(tmpData->source)+ "," +
    //                "destination:" + QString::fromLocal8Bit(tmpData->destination) + "," +
    //                "option:" + QString::fromLocal8Bit(tmpData->option);
    //     }
    //     tmpchain = tmpchain->next;
    // }
    // return res;

    void* libHandle = dlopen(LINGMOSDK_NETINFO_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return res;
    }
    handle_kdk_net_get_iptable_rules p_kdk_net_get_iptable_rules_handle = NULL;
    *(void **) (&p_kdk_net_get_iptable_rules_handle) = dlsym(libHandle, KDK_NET_GET_IPTABLE_RULES_HANDLE);

    if(!p_kdk_net_get_iptable_rules_handle)
    {
        dlclose(libHandle);
        return res;
    }

    pChain chain = p_kdk_net_get_iptable_rules_handle();
    
    if(!chain)
    {
        dlclose(libHandle);
        return res;
    }
    dlclose(libHandle);

    pChain tmpchain = chain;
    while(tmpchain)
    {
        pChainData tmpData = tmpchain->data;
        res << "total:" + QString::fromLocal8Bit(tmpchain->total) + "," +
               "policy:" + QString::fromLocal8Bit(tmpchain->policy);
        while (tmpData)
        {
            res << "target:" + QString::fromLocal8Bit(tmpData->target) + "," +
                   "prot:" + QString::fromLocal8Bit(tmpData->prot) + "," +
                   "opt:" + QString::fromLocal8Bit(tmpData->opt) + "," +
                   "source:" + QString::fromLocal8Bit(tmpData->source)+ "," +
                   "destination:" + QString::fromLocal8Bit(tmpData->destination) + "," +
                   "option:" + QString::fromLocal8Bit(tmpData->option);
            tmpData = tmpData->next;
        }
        tmpchain = tmpchain->next;
    }
    return res;
}

QStringList KySdkNet::getProcPort() const
{
    QStringList res = {};
    // char ** list = kdk_net_get_proc_port();
    // int i = 0;
    // while(list && list[i])
    // {
    //     res << list[i++];
    // }
    // return res;

    void* libHandle = dlopen(LINGMOSDK_NETINFO_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return res;
    }
    handle_kdk_net_get_proc_port p_kdk_net_get_proc_port_handle = NULL;
    *(void **) (&p_kdk_net_get_proc_port_handle) = dlsym(libHandle, KDK_NET_GET_PROC_PORT_HANDLE);

    if(!p_kdk_net_get_proc_port_handle)
    {
        dlclose(libHandle);
        return res;
    }

    char **list = p_kdk_net_get_proc_port_handle();
    
    if(!list)
    {
        dlclose(libHandle);
        return res;
    }
    dlclose(libHandle);

    int i = 0;
    while(list && list[i])
    {
        res << list[i++];
    }
    return res;
}

KySdkRunInfo::KySdkRunInfo(){}

double KySdkRunInfo::getNetSpeed(const QString nc) const
{
    std::string dp = nc.toStdString();
    return static_cast<double>(kdk_real_get_net_speed(dp.c_str()));
}

int KySdkRunInfo::getDiskRate(const QString diskpath) const
{
    std::string dp = diskpath.toStdString();
    // return kdk_real_get_disk_rate(dp.c_str());
    void* libHandle = dlopen(LINGMOSDK_REALTIME_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return 0;
    }
    handle_kdk_real_get_disk_rate p_kdk_real_get_disk_rate_handle = NULL;
    *(void **) (&p_kdk_real_get_disk_rate_handle) = dlsym(libHandle, KDK_REAL_GET_DISK_RATE_HANDLE);

    if(!p_kdk_real_get_disk_rate_handle)
    {
        dlclose(libHandle);
        return 0;
    }

    int rate = p_kdk_real_get_disk_rate_handle(dp.c_str());
    
    dlclose(libHandle);
    return static_cast<double>(rate);
}

double KySdkRunInfo::getCpuTemperature() const
{
    return static_cast<double>(kdk_real_get_cpu_temperature());
}
double KySdkRunInfo::getDiskTemperature(const QString diskpath) const
{
    std::string dp = diskpath.toStdString();
    // return static_cast<double>(kdk_real_get_disk_temperature(dp.c_str()));
    void* libHandle = dlopen(LINGMOSDK_REALTIME_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return 0;
    }
    handle_kdk_real_get_disk_temperature p_kdk_real_get_disk_temperature_handle = NULL;
    *(void **) (&p_kdk_real_get_disk_temperature_handle) = dlsym(libHandle, KDK_REAL_GET_DISK_TEMPERATURE_HANDLE);

    if(!p_kdk_real_get_disk_temperature_handle)
    {
        dlclose(libHandle);
        return 0;
    }

    int temperature = p_kdk_real_get_disk_temperature_handle(dp.c_str());
    
    dlclose(libHandle);
    return static_cast<double>(temperature);
}

KySdkGps::KySdkGps(){}
QString KySdkGps::getGPSInfo() const
{
    return QString::fromLocal8Bit(kdk_loaction_get());
}

KySdkPrint::KySdkPrint(){}

QStringList KySdkPrint::getPrinterList() const
{
    QStringList res = {};
    char **printers = kdk_printer_get_list();
    size_t index = 0;
    while (printers[index])
    {
        res << QString::fromLocal8Bit(printers[index]);
        index ++;
    }
    return res;
}

QStringList KySdkPrint::getPrinterAvailableList() const
{
    QStringList res = {};
    char **printers = kdk_printer_get_available_list();
    size_t index = 0;
    while (printers[index])
    {
        res << QString::fromLocal8Bit(printers[index]);
        index ++;
    }
    return res;
}

void KySdkPrint::setPrinterOptions(int number_up, const QString media, const QString number_up_layout, const QString sides) const
{
    std::string med = media.toStdString();
    std::string nul = number_up_layout.toStdString();
    std::string sid = sides.toStdString();
    kdk_printer_set_options(number_up, med.c_str(), nul.c_str(), sid.c_str());
}

int KySdkPrint::getPrinterPrintLocalFile (const QString printername, const QString filepath) const
{
    std::string printers = printername.toStdString();
    std::string fp = filepath.toStdString();
    return kdk_printer_print_local_file(printers.c_str(), fp.c_str());
}

int KySdkPrint::getPrinterCancelAllJobs(const QString printername) const
{
    std::string printers = printername.toStdString();
    return kdk_printer_cancel_all_jobs(printers.c_str());
}

int KySdkPrint::getPrinterStatus(const QString printername) const
{
    std::string printers = printername.toStdString();
    return kdk_printer_get_status(printers.c_str());
}

QString KySdkPrint::getPrinterFilename(const QString url) const
{
    std::string urlpath = url.toStdString();
    
    return QString::fromLocal8Bit(kdk_printer_get_filename(urlpath.c_str()));
}
int KySdkPrint::getPrinterJobStatus(const QString printername, int jobid) const
{
    std::string printers = printername.toStdString();
    return kdk_printer_get_job_status(printers.c_str(), jobid);
}

int KySdkPrint::getPrinterDownloadRemoteFile(const QString url, const QString filepath) const
{
    std::string urlpath = url.toStdString();
    std::string fp = filepath.toStdString();
    return kdk_printer_print_download_remote_file(urlpath.c_str(), fp.c_str());
}

// KySdkRest::KySdkRest(){}

// KySdkRest::~KySdkRest(){}

// QStringList KySdkRest::getSysLegalResolution() const
// {
//     QStringList res = {};
//     char **power = kdk_system_get_resolving_power();
//     size_t count = 0;
//     while(power[count])
//     {
//         res << QString::fromLocal8Bit(power[count]);
//         count ++;
//     }
//     kdk_resolving_freeall(power);
//     return res;
// }

KySdkAccounts::KySdkAccounts(){}

bool KySdkAccounts::changePassword(const QString userName, const QString passWord) const
{
    std::string usename = userName.toStdString();
    std::string passwd = passWord.toStdString();

    int *err_num = 0;

    void* libHandle = dlopen(LINGMOSDK_ACCOUNTS_LIB, RTLD_LAZY);
    if(!libHandle)
    {
        return 0;
    }
    handle_kdk_system_change_password p_kdk_system_change_password_handle = NULL;
    *(void **) (&p_kdk_system_change_password_handle) = dlsym(libHandle, KDK_SYSTEM_CHANGE_PASSWORD_HANDLE);

    if(!p_kdk_system_change_password_handle)
    {
        dlclose(libHandle);
        return 0;
    }

    bool ret = p_kdk_system_change_password_handle(usename.c_str(), passwd.c_str(), err_num);
    
    dlclose(libHandle);
    return ret;
}

