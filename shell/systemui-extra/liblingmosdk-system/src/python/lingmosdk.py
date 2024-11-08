# 
#  liblingmosdk-system's Library
#  
#  Copyright (C) 2023, KylinSoft Co., Ltd.
# 
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 3 of the License, or (at your option) any later version.
# 
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this library.  If not, see <https://www.gnu.org/licenses/>.
# 
#  Authors: Yunhe Liu <liuyunhe@kylinos.cn>
# 
# 

import dbus


class SessionBase(object):
    def __init__(self, object_path, interface_name):
        self.bus = dbus.SessionBus()
        self.proxy = self.bus.get_object(
            "com.lingmo.lingmosdk.service", object_path)
        self.interface = dbus.Interface(
            self.proxy, dbus_interface=interface_name)

class Base(object):

    def __init__(self, object_path, interface_name):
        self.bus = dbus.SystemBus()
        self.proxy = self.bus.get_object(
            "com.lingmo.lingmosdk.service", object_path)
        self.interface = dbus.Interface(
            self.proxy, dbus_interface=interface_name)


##
# @brief 获取磁盘信息
# 主要用来获取磁盘的列表，磁盘的扇区数量，每个扇区的字节数，容量，型号、序列号、子分区数量，类型和固件版本信息
#  
class Disk(Base):

    def __init__(self):
        super().__init__("/com/lingmo/lingmosdk/disk",
                         "com.lingmo.lingmosdk.disk")

    ##
    # @brief 获取所有磁盘的列表
    # 
    # @return 磁盘的列表，每个字符串表示一个磁盘的绝对路径
    # 
    def getDiskList(self):
        return self.interface.getDiskList()

    ##
    # @brief 获取磁盘的每个扇区的字节数
    # 
    # @param dev_path 指定磁盘名称，应当是例如/dev/sda这种绝对路径
    # @return 每个扇区的字节数
    # 
    def getDiskSectorSize(self, dev_path):
        return self.interface.getDiskSectorSize(dev_path)

    ##
    # @brief 获取磁盘容量
    # 
    # @param dev_path 指定磁盘名称，应当是例如/dev/sda这种绝对路径
    # @return 磁盘容量，MiB为单位
    # 
    def getDiskTotalSizeMiB(self, dev_path):
        return self.interface.getDiskTotalSizeMiB(dev_path)

    ##
    # @brief 获取磁盘型号
    # 
    # @param dev_path 指定磁盘名称，应当是例如/dev/sda这种绝对路径
    # @return 磁盘型号
    # 
    def getDiskModel(self, dev_path):
        return self.interface.getDiskModel(dev_path)

    ##
    # @brief 获取磁盘序列号
    # 
    # @param dev_path 指定磁盘名称，应当是例如/dev/sda这种绝对路径
    # @return 磁盘序列号
    # 
    def getDiskSerial(self, dev_path):
        return self.interface.getDiskSerial(dev_path)

    ##
    # @brief 获取磁盘/分区下的子分区数量
    # 
    # @param dev_path 指定磁盘名称，应当是例如/dev/sda这种绝对路径
    # @return 该磁盘/分区下的子分区数量
    # 
    def getDiskPartitionNums(self, dev_path):
        return self.interface.getDiskPartitionNums(dev_path)

    ##
    # @brief 获取磁盘类型
    # 
    # @param dev_path 指定磁盘名称，应当是例如/dev/sda这种绝对路径
    # @return 磁盘类型，固态 or 机械 or 混合
    # 
    def getDiskType(self, dev_path):
        return self.interface.getDiskType(dev_path)

    ##
    # @brief 获取磁盘固件版本信息
    # 
    # @param dev_path 指定磁盘名称，应当是例如/dev/sda这种绝对路径
    # @return 固件版本信息
    # 
    def getDiskVersion(self, dev_path):
        return self.interface.getDiskVersion(dev_path)

    ##
    # @brief 获取磁盘扇区数量
    # 
    # @param dev_path 指定磁盘名称，应当是例如/dev/sda这种绝对路径
    # @return 扇区数量
    # 
    def getDiskSectorNum(self, dev_path):
        return self.interface.getDiskSectorNum(dev_path)


##
# @brief 获取cpu信息
# 主要用来获取cpu的架构、制造厂商、型号、额定主频、核心数量、虚拟化支持和线程数信息
# 
class Cpuinfo(Base):

    def __init__(self):
        super().__init__("/com/lingmo/lingmosdk/cpuinfo",
                         "com.lingmo.lingmosdk.cpuinfo")

    ##
    # @brief 获取cpu架构
    # 
    # @return 架构信息，如"x86_64"
    # 
    def getCpuArch(self):
        return self.interface.getCpuArch()

    ##
    # @brief 获取cpu制造厂商
    # 
    # @return 制造厂商，如“GenuineIntel”
    # 
    def getCpuVendor(self):
        return self.interface.getCpuVendor()

    ##
    # @brief 获取cpu型号
    # 
    # @return CPU型号名称，如“Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz”
    # 
    def getCpuModel(self):
        return self.interface.getCpuModel()

    ##
    # @brief 获取cpu额定主频
    # 
    # @return 额定主频赫兹数，如“1794.742”
    # 
    def getCpuFreqMHz(self):
        return self.interface.getCpuFreqMHz()

    ##
    # @brief 获取cpu核心数量
    # 
    # @return 所有可用的CPU核心数量
    # 
    def getCpuCorenums(self):
        return self.interface.getCpuCorenums()

    ##
    # @brief 获取cpu对虚拟化的支持
    # 
    # @return 若CPU支持虚拟化，则返回虚拟化技术，如“vmx”；若不支持，返回NULL
    # 
    def getCpuVirt(self):
        return self.interface.getCpuVirt()

    ##
    # @brief 获取cpu线程数
    # 
    # @return cpu支持的线程数
    # 
    def getCpuProcess(self):
        return self.interface.getCpuProcess()


##
# @brief 获取网卡信息
# 主要获取生成网卡列表、检测指定网卡是否处于UP状态、获取系统中当前处于 link up 状态的网卡列表、获取指定网卡的物理MAC地址、
# 获取指定网卡的第一个IPv4地址、获取指定网卡的所有IPv4地址、获取指定网卡的第一个IPv6地址、获取指定网卡的所有IPv6地址、
# 获取指定网卡的有线/无线类型、获取指定网卡的厂商名称和设备型号
# 
class NetCard(Base):

    def __init__(self):
        super().__init__("/com/lingmo/lingmosdk/netcard",
                         "com.lingmo.lingmosdk.netcard")

    ##
    # @brief 获取系统中所有的网卡
    # 
    # @return 网卡名称列表
    # 
    def getNetCardName(self):
        return self.interface.getNetCardName()

    ##
    # @brief 检测指定网卡是否处于UP状态
    # 
    # @param netcard_name 网卡名称，如eno1
    # @return Up返回1，Down返回0
    # 
    def getNetCardUp(self, netcard_name):
        return self.interface.getNetCardUp(netcard_name)

    ##
    # @brief 获取系统中当前处于 link up 状态的网卡列表
    # 
    # @return 处于link up状态的网卡列表
    # 
    def getNetCardUpcards(self):
        return self.interface.getNetCardUpcards()

    ##
    # @brief 获取指定网卡的物理MAC地址
    # 
    # @param netcard_name 网卡名称，如eno1
    # @return 物理MAC地址
    # 
    def getNetCardPhymac(self, netcard_name):
        return self.interface.getNetCardPhymac(netcard_name)

    ##
    # @brief 获取指定网卡的第一个IPv4地址
    # 
    # @param netcard_name 网卡名称，如eno1
    # @return IPv4地址
    # 
    def getNetCardPrivateIPv4(self, netcard_name):
        return self.interface.getNetCardPrivateIPv4(netcard_name)

    ##
    # @brief 获取指定网卡的所有IPv4地址
    # 
    # @param netcard_name 网卡名称，如eno1
    # @return IPv4地址列表
    # 
    def getNetCardIPv4(self, netcard_name):
        return self.interface.getNetCardIPv4(netcard_name)

    ##
    # @brief 获取指定网卡的第一个IPv6地址
    # 
    # @param netcard_name 网卡名称，如eno1
    # @return IPv6地址
    # 
    def getNetCardPrivateIPv6(self, netcard_name):
        return self.interface.getNetCardPrivateIPv6(netcard_name)

    ##
    # @brief 获取指定网卡的有线/无线类型
    # 
    # @param netcard_name 网卡名称，如eno1
    # @return 0 有线 1 无线
    # 
    def getNetCardType(self, netcard_name):
        return self.interface.getNetCardType(netcard_name)

    ##
    # @brief 获取指定网卡的厂商名称和设备型号
    #  
    # @param netcard_name 网卡名称，如eno1
    # @return 网卡的厂商名称和设备型号
    # 
    def getNetCardProduct(self, netcard_name):
        return self.interface.getNetCardProduct(netcard_name)

    ##
    # @brief 获取指定网卡的所有IPv6地址
    # 
    # @param netcard_name 网卡名称，如eno1
    # @return IPv6地址列表
    # 
    def getNetCardIPv6(self, netcard_name):
        return self.interface.getNetCardIPv6(netcard_name)


##
# @brief 获取bios信息
# 主要用来获取bios的厂商和版本信息
# 
class Bios(Base):

    def __init__(self):
        super().__init__("/com/lingmo/lingmosdk/bios",
                         "com.lingmo.lingmosdk.bios")

    ##
    # @brief 获取bios厂商
    # 
    # @return 厂商名
    # 
    def getBiosVendor(self):
        return self.interface.getBiosVendor()

    ##
    # @brief 获取bios版本
    # 
    # @return 版本号
    # 
    def getBiosVersion(self):
        return self.interface.getBiosVersion()


##
# @brief 获取主板信息
# 主要用来获取主板型号、生产日期、序列号、厂商信息
# 
class Mainboard(Base):

    def __init__(self):
        super().__init__("/com/lingmo/lingmosdk/mainboard",
                         "com.lingmo.lingmosdk.mainboard")

    ##
    # @brief 获取主板型号
    # 
    # @return 主板型号
    # 
    def getMainboardName(self):
        return self.interface.getMainboardName()

    ## 
    # @brief 获取主板生产日期
    # 
    # @return 主板生产日期
    # 
    def getMainboardDate(self):
        return self.interface.getMainboardDate()

    ##
    # @brief 获取主板序列号
    # 
    # @return 主板序列号
    # 
    def getMainboardSerial(self):
        return self.interface.getMainboardSerial()

    ##
    # @brief 获取主板厂商
    # 
    # @return 主板厂商名
    # 
    def getMainboardVendor(self):
        return self.interface.getMainboardVendor()


##
# @brief 获取usb设备信息dbus信号类
# 主要用来获取usb设备的名称、类型、pid、vid、序列号、路径信息
# 
class Peripheralsenum(Base):

    def __init__(self):
        super().__init__("/com/lingmo/lingmosdk/peripheralsenum",
                         "com.lingmo.lingmosdk.peripheralsenum")

    ##
    # @brief 获取外接usb设备信息
    #  
    # @return QStringList usb设备名称、类型、pid、vid、序列号、路径信息列表
    # 
    def getAllUsbInfo(self):
        return self.interface.getAllUsbInfo()


##
# @brief 获取包列表信息
# 主要用来获取系统中所有包列表、获取系统中指定包的版本号、检测指定包名的软件包是否正确安装
# 
class Packageinfo(Base):

    def __init__(self):
        super().__init__("/com/lingmo/lingmosdk/packageinfo",
                         "com.lingmo.lingmosdk.packageinfo")

    ##
    # @brief 获取系统中所有包列表
    #  
    # @return 系统中所有包列表
    # 
    def getPackageList(self):
        return self.interface.getPackageList()

    ##
    # @brief 获取系统中指定包的版本号
    # 
    # @param package_name 软件包名
    # @return  版本号
    # 
    def getPackageVersion(self, package_name):
        return self.interface.getPackageVersion(package_name)

    ##
    # @brief 检测指定包名的软件包是否正确安装
    #  
    # @param package_name 包名
    # @param version 版本号，大部分情况下为NULL，预留为snap做准备
    # @return 成功返回0，失败返回错误码
    # 
    def getPackageInstalled(self, package_name, version=None):
        return self.interface.getPackageInstalled(package_name, version)


##
# @brief 获取当前系统资源占用信息类
# 主要用来获取系统中物理内存总大小、物理内存使用率、物理内存使用大小、实际可用的物理内存大小、实际空闲的物理内存大小、
# 所有应用申请的虚拟内存总量、系统中Swap分区总大小、Swap分区使用率、Swap分区使用量、Swap分区空闲大小、CPU瞬时使用率、
# 操作系统开机时长
# 
class Resource(Base):

    def __init__(self):
        super().__init__("/com/lingmo/lingmosdk/resource",
                         "com.lingmo.lingmosdk.resource")

    ##
    # @brief 获取系统中物理内存总大小
    #  
    # @return 物理内存大小，KiB为单位
    # 
    def getMemTotalKiB(self):
        return self.interface.getMemTotalKiB()

    ##
    # @brief 获取物理内存使用率
    #  
    # @return 物理内存使用率
    # 
    def getMemUsagePercent(self):
        return self.interface.getMemUsagePercent()

    ##
    # @brief 获取物理内存使用大小，注意Buffer/Cache被计算为已使用内存
    #  
    # @return 物理内存使用大小，KiB为单位
    # 
    def getMemUsageKiB(self):
        return self.interface.getMemUsageKiB()

    ##
    # @brief 获取实际可用的物理内存大小，该数值约等于Free + Buffer + Cache
    #  
    # @return 可用物理内存大小，KiB为单位
    # 
    def getMemAvailableKiB(self):
        return self.interface.getMemAvailableKiB()

    ##
    # @brief 获取实际空闲的物理内存大小，注意Buffer/Cache被计算为已使用内存
    #  
    # @return 空闲的物理内存大小，KiB为单位
    # 
    def getMemFreeKiB(self):
        return self.interface.getMemFreeKiB()

    ##
    # @brief 获取所有应用申请的虚拟内存总量
    #  
    # @return 虚拟内存总申请量，KiB为单位
    # 
    def getMemVirtAllocKiB(self):
        return self.interface.getMemVirtAllocKiB()

    ##
    # @brief 获取系统中Swap分区总大小
    #  
    # @return Swap分区大小，KiB为单位
    # 
    def getMemSwapTotalKiB(self):
        return self.interface.getMemSwapTotalKiB()

    ##
    # @brief 获取Swap分区使用率
    #  
    # @return Swap分区使用率
    # 
    def getMemSwapUsagePercent(self):
        return self.interface.getMemSwapUsagePercent()

    ##
    # @brief 获取Swap分区使用量
    #  
    # @return Swap分区使用量，KiB为单位
    # 
    def getMemSwapUsageKiB(self):
        return self.interface.getMemSwapUsageKiB()

    ##
    # @brief 获取Swap分区空闲大小
    #  
    # @return Swap分区空闲大小，KiB为单位
    # 
    def getMemSwapFreeKiB(self):
        return self.interface.getMemSwapFreeKiB()

    ##
    # @brief 获取CPU瞬时使用率
    #  
    # @return CPU瞬时使用率，该值 < 1.00
    # 
    def getCpuCurrentUsage(self):
        return self.interface.getCpuCurrentUsage()

    ##
    # @brief 获取操作系统开机时长
    #  
    # @return 开机时长
    # 
    def getUpTime(self):
        return self.interface.getUpTime()


##
# @brief 获取某一进程的瞬时详细信息
# 主要用来获取某一进程的CPU使用率、获取某一进程的内存占用率、获取某一进程的进程状态、获取某一进程的端口号占用、
# 获取某一进程的启动时间、获取某一进程的运行时间、获取某一进程的cpu时间、获取某一进程的Command、获取某一进程的属主、
# 获取某一进程的信息、获取进程所有信息
# 
class Process(Base):

    def __init__(self):
        super().__init__("/com/lingmo/lingmosdk/process",
                         "com.lingmo.lingmosdk.process")

    ##
    # @brief 获取某一进程的CPU使用率
    #  
    # @param pid 进程号
    # @return 进程的CPU使用率
    # 
    def getProcInfoCpuUsage(self, pid):
        return self.interface.getProcInfoCpuUsage(pid)

    ##
    # @brief 获取某一进程的内存占用率
    #  
    # @param pid 进程号
    # @return 进程的内存占用率
    # 
    def getProcInfoMemUsage(self, pid):
        return self.interface.getProcInfoMemUsage(pid)

    ##
    # @brief 获取某一进程的进程状态
    #  
    # @param pid 进程号
    # @return 进程状态
    # 
    def getProcInfoStatus(self, pid):
        return self.interface.getProcInfoStatus(pid)

    ##
    # @brief 获取某一进程的端口号占用
    #  
    # @param pid 进程号
    # @return 进程使用的端口号
    # 
    def getProcInfoPort(self, pid):
        return self.interface.getProcInfoPort(pid)

    ##
    # @brief 获取某一进程的启动时间
    #  
    # @param pid 进程号
    # @return 进程的启动时间
    # 
    def getProcInfoStartTime(self, pid):
        return self.interface.getProcInfoStartTime(pid)

    ##
    # @brief 获取某一进程的运行时间
    #  
    # @param pid 进程号
    # @return 进程的运行时间
    # 
    def getProcInfoRunningTime(self, pid):
        return self.interface.getProcInfoRunningTime(pid)

    ##
    # @brief 获取某一进程的cpu时间
    #  
    # @param pid 进程号
    # @return 进程的cpu时间
    # 
    def getProcInfoCpuTime(self, pid):
        return self.interface.getProcInfoCpuTime(pid)

    ##
    # @brief 获取某一进程的Command
    #  
    # @param pid 进程号
    # @return 进程的Command
    # 
    def getProcInfoCmd(self, pid):
        return self.interface.getProcInfoCmd(pid)

    ##
    # @brief 获取某一进程的属主
    #  
    # @param pid 进程号
    # @return 进程的属主
    # 
    def getProcInfoUser(self, pid):
        return self.interface.getProcInfoUser(pid)

    ##
    # @brief 获取某一进程的信息
    #  
    # @param process_name 进程名
    # @return 某一进程CPU使用率、内存占用率、状态、端口号占用、启动时间、运行时间、cpu时间、Command、属主信息列表
    # 
    def getProcInfo(self, process_name):
        return self.interface.getProcInfo(process_name)

    ##
    # @brief 获取进程所有信息
    #  
    # @return 所有进程的CPU使用率、内存占用率、状态、端口号占用、启动时间、运行时间、cpu时间、Command、属主信息列表
    # 
    def getAllProcInfo(self):
        return self.interface.getAllProcInfo()

##
# @brief 获取屏幕分辨率信息
# 主要用来获取系统当前显示屏幕,分辨率,系统支持分辨率
# 
class GetSysLegalResolution(SessionBase):

    def __init__(self):
        super().__init__("/com/lingmo/lingmosdk/resolution",
                         "com.lingmo.lingmosdk.resolution")

    ##
    # @brief  获取系统当前显示屏幕,分辨率,系统支持分辨率
    #  
    # @return 当前显示屏幕,分辨率,系统支持分辨率信息列表
    # 
    def getSysLegalResolution(self):
        return self.interface.getSysLegalResolution()


##
# @brief 获取操作系统基础信息
# 主要用来获取操作系统架构信息、操作系统名称、操作系统版本号、操作系统激活状态、操作系统服务序列号、内核版本号、
# 当前登录用户的用户名（Effect User）、操作系统项目编号名、操作系统项目子编号名、操作系统产品标识码、操作系统宿主机的虚拟机类型、
# 操作系统宿主机的云平台类型、判断当前镜像系统是否为 专用机 系统、系统版本号/补丁版本号、系统当前显示屏幕,分辨率,系统支持分辨率
# 
class Sysinfo(Base):

    def __init__(self):
        super().__init__("/com/lingmo/lingmosdk/sysinfo",
                         "com.lingmo.lingmosdk.sysinfo")

    ##
    # @brief 获取操作系统架构信息
    #  
    # @return 操作系统架构信息
    # 
    def getSystemArchitecture(self):
        return self.interface.getSystemArchitecture()

    ##
    # @brief 获取操作系统名称
    #  
    # @return 操作系统名称
    # 
    def getSystemName(self):
        return self.interface.getSystemName()

    ##
    # @brief 获取操作系统版本号
    #  
    # @param defatul 0获取简略版本号，1获取详细版本号
    # @return 操作系统版本号
    # 
    def getSystemVersion(self, defatul=0):
        """
        defatul = 0 获取简略版本号
        defatul = 1 获取详细版本号
        """
        return self.interface.getSystemVersion(defatul)

    ##
    # @brief 获取操作系统激活状态
    #  
    # @return 2表示已过期；0表示未激活，处于试用期；1表示已激活；-1表示接口内部错误；
    # 
    def getSystemActivationStatus(self):
        return self.interface.getSystemActivationStatus()

    ##
    # @brief 获取操作系统服务序列号
    #  
    # @return 操作系统服务序列号
    # 
    def getSystemSerialNumber(self):
        return self.interface.getSystemSerialNumber()

    ##
    # @brief 获取内核版本号
    #  
    # @return 内核版本号
    # 
    def getSystemKernelVersion(self):
        return self.interface.getSystemKernelVersion()

    ##
    # @brief 获取当前登录用户的用户名（Effect User）
    #  
    # @return 当前登录用户的用户名（Effect User）
    # 
    def getSystemEffectUser(self):
        return self.interface.getSystemEffectUser()

    ##
    # @brief 获取操作系统项目编号名
    # 
    # @return 操作系统项目编号名
    # 
    def getSystemProjectName(self):
        return self.interface.getSystemProjectName()

    ##
    # @brief 获取操作系统项目子编号名
    #  
    # @return 操作系统项目子编号名
    # 
    def getSystemProjectSubName(self):
        return self.interface.getSystemProjectSubName()

    ##
    # @brief 获取操作系统产品标识码
    #  
    # @return 返回标志码
    #         0000 信息异常
    #         0001 仅PC特性
    #         0010 仅平板特性
    #         0011 支持平板与PC特性
    # 
    def getSystemProductFeatures(self):
        return self.interface.getSystemProductFeatures()

    ##
    # @brief 获取操作系统宿主机的虚拟机类型
    # 
    # @return 操作系统宿主机的虚拟机类型，类型如下：
    #         [none, qemu, kvm, zvm, vmware, hyper-v, orcale virtualbox, xen, bochs, \
    #         uml, parallels, bhyve, qnx, arcn, openvz, lxc, lxc-libvirt, systemd-nspawn,\
    #         docker, podman, rkt, wsl]
    #         其中 none 表示运行在物理机环境中；其他字符串代表具体的虚拟环境类型
    # 
    def getSystemHostVirtType(self):
        return self.interface.getSystemHostVirtType()

    ##
    # @brief 获取操作系统宿主机的云平台类型
    # 
    # @return 获取失败返回NULL，获取成功返回一个字符串，字符串内容如下：
    #         [none, huawei]
    #         其中 none 表示运行在物理机或未知的云平台环境中；其他字符串代表不同的云平台
    # 
    def getSystemHostCloudPlatform(self):
        return self.interface.getSystemHostCloudPlatform()

    ##
    # @brief  获取系统版本号
    #  
    # @return 系统版本号
    # 
    def getSystemOSVersion(self):
        return self.interface.getSystemOSVersion()

    ##
    # @brief  获取补丁版本号
    #  
    # @return 补丁版本号
    # 
    def getSystemUpdateVersion(self):
        return self.interface.getSystemUpdateVersion()

    ##
    # @brief  判断当前镜像系统是否为 专用机 系统
    #  
    # @return true代表是 false代表不是
    # 
    def getSystemIsZYJ(self):
        return self.interface.getSystemIsZYJ()

    # def getSysLegalResolution(self):
    #     return self.interface.getSysLegalResolution()


##
#  @brief 获取网络信息
#  主要用来获取指定网络端口的状态、区间端口状态、网关信息、防火墙信息
# 
class Net(Base):

    def __init__(self):
        super().__init__("/com/lingmo/lingmosdk/net",
                         "com.lingmo.lingmosdk.net")

    ##
    # @brief 获取指定网络端口的状态
    #  
    # @param port 端口号
    # @return 端口状态 TCP_ESTABLISHED:0x1   TCP_SYN_SENT:0x2    TCP_SYN_RECV:0x3    TCP_FIN_WAIT1:0x4    TCP_FIN_WAIT2:0x5
    #         TCP_TIME_WAIT:0x6    TCP_CLOSE:0x7    TCP_CLOSE_WAIT:0x8    TCP_LAST_ACL:0x9    TCP_LISTEN:0xa    TCP_CLOSING:0xb
    # 
    def getPortState(self, port):
        return self.interface.getPortState(port)

    ##
    # @brief 获取区间端口状态
    #  
    # @param start_port 开始端口
    # @param end_port 结束端口
    # @return 区间端口状态列表
    # 
    def getMultiplePortStat(self, start_port, end_port):
        return self.interface.getMultiplePortStat(start_port, end_port)

    ##
    # @brief 获取网关信息
    # 
    # @return 网关信息网卡名和地址列表
    # 
    def getGatewayInfo(self):
        return self.interface.getGatewayInfo()

    ##
    # @brief 获取防火墙信息
    #  
    # @return 防火墙信息列表
    # 
    def getFirewallState(self):
        return self.interface.getFirewallState()


##
# @brief 获取瞬时信息
# 主要用来获取瞬时网速，cpu温度、磁盘温度、磁盘转速
# 
class Runinfo(Base):

    def __init__(self):
        super().__init__("/com/lingmo/lingmosdk/runinfo",
                         "com.lingmo.lingmosdk.runinfo")

    ##
    # @brief 获取瞬时网速信息
    #  
    # @param netcard 网卡名称，如eno1
    # @return 瞬时网速
    # 
    def getNetSpeed(self, netcard):
        return self.interface.getNetSpeed(netcard)

    ##
    # @brief 获取磁盘转速
    #  
    # @param disk_path 磁盘名称，应当是例如/dev/sda这种绝对路径
    # @return 磁盘转速
    # 
    def getDiskRate(self, disk_path):
        return self.interface.getDiskRate(disk_path)

    ##
    # @brief 获取CPU温度
    #  
    # @return CPU温度
    # 
    def getCpuTemperature(self):
        return self.interface.getCpuTemperature()

    ##
    # @brief 获取磁盘温度
    #  
    # @param disk_path 磁盘名称，应当是例如/dev/sda这种绝对路径
    # @return 磁盘温度
    # 
    def getDiskTemperature(self, disk_path):
        return self.interface.getDiskTemperature(disk_path)


##
# @brief 获取地址信息
# 主要用来获取本地地址信息
# 
class Gps(Base):

    def __init__(self):
        super().__init__("/com/lingmo/lingmosdk/gps",
                         "com.lingmo.lingmosdk.gps")

    ##
    # @brief 获取本地地址信息
    # 
    # @return json格式的地址信息
    # 
    def getGPSInfo(self):
        return self.interface.getGPSInfo()


##
#  @brief 获取打印信息
#  主要用来生成本地的所有打印机(包含可用打印机和不可用打印机)、获取本地的所有可用打印机(打印机处于idle状态)、
#  设置打印参数，每次设置会清除之前设置打印参数,这个设置是全局设置，设置了之后所有打印机都会用这个参数进行打印、
#  下载网络文件、打印本地文件、清除某个打印机的所有打印队列、获取当前打印机状态(状态不是实时更新)、
#  获取当前打印任务状态（发送打印任务之后下需要等待一会再获取状态,状态不是实时更新）、获取url内的文件名
#  
class Print(Base):

    def __init__(self):
        super().__init__("/com/lingmo/lingmosdk/print",
                         "com.lingmo.lingmosdk.print")

    ##
    # @brief 获取本地的所有打印机(包含可用打印机和不可用打印机)
    #  
    # @return 地的所有打印机名称列表
    # 
    def getPrinterList(self):
        return self.interface.getPrinterList()

    ##
    #  @brief 获取本地的所有可用打印机(打印机处于idle状态)
    #  
    #  @return 本地的所有可用打印机名称列表
    # 
    def getPrinterAvailableList(self):
        return self.interface.getPrinterAvailableList()

    ##
    #  @brief 设置打印参数，每次设置会清除之前设置打印参数,这个设置是全局设置，设置了之后所有打印机都会用这个参数进行打印
    #  
    #  @param num cups属性，一张纸打印几页，如2 4等
    #  @param paper_type cups属性，纸张类型，如A4
    #  @param cups cups属性，如lrtb
    #  @param option cups属性，单面如one-sided，双面如two-sided-long-edge(长边翻转),two-sided-short-edge(短边翻转)
    # 
    def setPrinterOptions(self, num, paper_type, cups, option):
        return self.interface.setPrinterOptions(num, paper_type, cups, option)

    ##
    # @brief 打印本地文件
    #  
    # @param print_name 打印机名
    # @param file_path 文件绝对路径
    #  
    # @return 打印作业的id
    # 
    # @notice 虽然支持多种格式的打印，但是打印除pdf之外的格式打印的效果都不够好，建议打印pdf格式的文件
    #         打印机处于stop状态函数返回0
    #         格式不对的文件可以成功创建打印任务，但是打印不出来。什么叫格式不对，举个例子，比如当前要打印日志文件a.log,
    #         然后把a.log改名叫a.pdf,这个时候a.pdf就变成了格式不对的文件，打印不出来了
    # 
    # 
    def getPrinterPrintLocalFile(self, print_name, file_path):
        return self.interface.getPrinterPrintLocalFile(print_name, file_path)

    ##
    # @brief 清除某个打印机的所有打印队列
    #  
    # @param print_name 打印机名
    # @return 成功返回0,失败返回-1
    # 
    def getPrinterCancelAllJobs(self, print_name):
        return self.interface.getPrinterCancelAllJobs(print_name)

    ##
    # @brief 获取当前打印机状态(状态不是实时更新)
    #  
    # @param print_name 打印机名
    # @return 状态码
    # 
    def getPrinterStatus(self, print_name):
        return self.interface.getPrinterStatus(print_name)

    ##
    # @brief  获取url内的文件名
    #  
    # @param  url 路径
    # @return 解析出来的函数名
    # 
    def getPrinterFilename(self, url):
        return self.interface.getPrinterFilename(url)

    ##
    # @brief 获取当前打印任务状态（发送打印任务之后下需要等待一会再获取状态,状态不是实时更新）
    #  
    # @param print_name 打印机名
    # @param jobid 打印作业的id
    # @return 状态码
    # 
    def getPrinterJobStatus(self, print_name, jobid):
        return self.interface.getPrinterJobStatus(print_name, jobid)

    ##
    # @brief 下载网络文件
    #  
    # @param url 要打印的url
    # @param path 保存的文件路径
    #  
    # @return 打印作业的id
    # 
    def getPrinterDownloadRemoteFile(self, url, path):
        return self.interface.getPrinterDownloadRemoteFile(url, path)
