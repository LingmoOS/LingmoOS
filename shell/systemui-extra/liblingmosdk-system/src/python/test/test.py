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

from lingmosdk import *
import sys


def disk_test(args):
    disk = Disk()
    print("getDiskList接口:", disk.getDiskList())
    if len(args) == 0:
        print("其他接口需要参数磁盘路径")
        sys.exit()
    print("getDiskSectorSize接口", disk.getDiskSectorSize(args[0]))
    print("getDiskTotalSizeMiB接口", disk.getDiskTotalSizeMiB(args[0]))
    print("getDiskModel接口", disk.getDiskModel(args[0]))
    print("getDiskSerial接口", disk.getDiskSerial(args[0]))
    print("getDiskPartitionNums接口", disk.getDiskPartitionNums(args[0]))
    print("getDiskType接口", disk.getDiskType(args[0]))
    print("getDiskVersion接口", disk.getDiskVersion(args[0]))
    print("getDiskSectorNum接口", disk.getDiskSectorNum(args[0]))


def cpu_test(args):
    cpu = Cpuinfo()
    print("getCpuArch接口:", cpu.getCpuArch())
    print("getCpuVendor接口:", cpu.getCpuVendor())
    print("getCpuModel接口:", cpu.getCpuModel())
    print("getCpuFreqMHz接口:", cpu.getCpuFreqMHz())
    print("getCpuCorenums接口:", cpu.getCpuCorenums())
    print("getCpuVirt接口:", cpu.getCpuVirt())
    print("getCpuProcess接口:", cpu.getCpuProcess())


def netcard_test(args):
    netcard = NetCard()
    print("getNetCardName接口:", netcard.getNetCardName())
    print("getNetCardUpcards接口:", netcard.getNetCardUpcards())
    if len(args) == 0:
        print("其他接口需要参数网卡名")
        sys.exit()
    print("getNetCardUp接口:", netcard.getNetCardUp(args[0]))
    print("getNetCardPhymac接口:", netcard.getNetCardPhymac(args[0]))
    print("getNetCardPrivateIPv4接口:", netcard.getNetCardPrivateIPv4(args[0]))
    print("getNetCardIPv4接口:", netcard.getNetCardIPv4(args[0]))
    print("getNetCardPrivateIPv6接口:", netcard.getNetCardPrivateIPv6(args[0]))
    print("getNetCardType接口:", netcard.getNetCardType(args[0]))
    print("getNetCardProduct接口:", netcard.getNetCardProduct(args[0]))
    print("getNetCardIPv6接口:", netcard.getNetCardIPv6(args[0]))


def bios_test(args):
    bios = Bios()
    print("getBiosVendor接口:", bios.getBiosVendor())
    print("getBiosVersion接口:", bios.getBiosVersion())


def mainboard_test(args):
    mainboard = Mainboard()
    print("getMainboardName接口:", mainboard.getMainboardName())
    print("getMainboardDate接口:", mainboard.getMainboardDate())
    print("getMainboardSerial接口:", mainboard.getMainboardSerial())
    print("getMainboardVendor接口:", mainboard.getMainboardVendor())


def peripheralsenum_test(args):
    peri = Peripheralsenum()
    print("getMainboardName接口:", peri.getAllUsbInfo())


def package_test(args):
    package = Packageinfo()
    print("getPackageList接口:", package.getPackageList())

    if len(args) == 0:
        print("其他接口需要参数包名")
        sys.exit()
    print("getPackageVersion接口:", package.getPackageVersion(args[0]))
    try:
        print("getPackageInstalled接口:",
              package.getPackageInstalled(args[0], args[1]))
    except IndexError:
        print("getPackageInstalled接口需要指定包版本")


def resource_test(args):
    resource = Resource()
    print("getMemTotalKiB接口:", resource.getMemTotalKiB())
    print("getMemUsagePercent接口:", resource.getMemUsagePercent())
    print("getMemUsageKiB接口:", resource.getMemUsageKiB())
    print("getMemAvailableKiB接口:", resource.getMemAvailableKiB())
    print("getMemFreeKiB接口:", resource.getMemFreeKiB())
    print("getMemVirtAllocKiB接口:", resource.getMemVirtAllocKiB())
    print("getMemSwapTotalKiB接口:", resource.getMemSwapTotalKiB())
    print("getMemSwapUsagePercent接口:", resource.getMemSwapUsagePercent())
    print("getMemSwapUsageKiB接口:", resource.getMemSwapUsageKiB())
    print("getMemSwapFreeKiB接口:", resource.getMemSwapFreeKiB())
    print("getCpuCurrentUsage接口:", resource.getCpuCurrentUsage())
    print("getUpTime接口:", resource.getUpTime())


def process_test(args):
    process = Process()
    print("getAllProcInfo接口", process.getAllProcInfo())
    if len(args) == 0:
        print("其他接口需要进程id或进程名")
        sys.exit()
    # 通过进程名获取
    print("getProcInfo接口", process.getProcInfo(args[0]))

    int_arg = int(args[0])
    print("getProcInfoCpuUsage接口", process.getProcInfoCpuUsage(int_arg))
    print("getProcInfoMemUsage接口", process.getProcInfoMemUsage(int_arg))
    print("getProcInfoStatus接口", process.getProcInfoStatus(int_arg))
    print("getProcInfoPort接口", process.getProcInfoPort(int_arg))
    print("getProcInfoStartTime接口", process.getProcInfoStartTime(int_arg))
    print("getProcInfoRunningTime接口", process.getProcInfoRunningTime(int_arg))
    print("getProcInfoCpuTime接口", process.getProcInfoCpuTime(int_arg))
    print("getProcInfoCmd接口", process.getProcInfoCmd(int_arg))
    print("getProcInfoUser接口", process.getProcInfoUser(int_arg))


def sysinfo_test(args):
    sysinfo = Sysinfo()
    sysinfo_ = GetSysLegalResolution()
    print("getSystemArchitecture接口:", sysinfo.getSystemArchitecture())
    print("getSystemName接口:", sysinfo.getSystemName())
    print("getSystemActivationStatus接口:", sysinfo.getSystemActivationStatus())
    print("getSystemSerialNumber接口:", sysinfo.getSystemSerialNumber())
    print("getSystemKernelVersion接口:", sysinfo.getSystemKernelVersion())
    print("getSystemEffectUser接口:", sysinfo.getSystemEffectUser())
    print("getSystemProjectName接口:", sysinfo.getSystemProjectName())
    print("getSystemProjectSubName接口:", sysinfo.getSystemProjectSubName())
    print("getSystemProductFeatures接口:", sysinfo.getSystemProductFeatures())
    print("getSystemHostVirtType接口:", sysinfo.getSystemHostVirtType())
    print("getSystemHostCloudPlatform接口:",
          sysinfo.getSystemHostCloudPlatform())
    print("getSystemOSVersion接口:", sysinfo.getSystemOSVersion())
    print("getSystemUpdateVersion接口:", sysinfo.getSystemUpdateVersion())
    print("getSystemIsZYJ接口:", sysinfo.getSystemIsZYJ())
    # print("getSysLegalResolution接口:", sysinfo.getSysLegalResolution())
    print("getSysLegalResolution接口:", sysinfo_.getSysLegalResolution())
    if len(args) == 0:
        print("其他接口需要参数0 或 1  \n 0 获取简略版本号 \n  1 获取详细版本号")
        sys.exit()
    print("getSystemVersion接口:", sysinfo.getSystemVersion(args[0]))


def net_test(args):
    net = Net()
    print("getGatewayInfo接口：", net.getGatewayInfo())
    print("getFirewallState接口：", net.getFirewallState())
    if len(args) == 0:
        print("getPortState接口需要输入指定端口")
        sys.exit()
    print("getPortState接口：", net.getPortState(int(args[0])))
    if len(args) == 1:
        print("getMultiplePortStat接口需要输入起始端口与终止端口")
        sys.exit()
    print("getMultiplePortStat接口：", net.getMultiplePortStat(
        int(args[0]), int(args[1])))


def runinfo_test(args):
    runinfo = Runinfo()
    print("getCpuTemperature接口", runinfo.getCpuTemperature())
    if len(args) == 0:
        print("其它接口需要参数 磁盘路径 或网卡名")
        sys.exit()
    print("getNetSpeed接口", runinfo.getNetSpeed(args[0]))
    print("getDiskRate接口", runinfo.getDiskRate(args[0]))
    print("getDiskTemperature接口", runinfo.getDiskTemperature(args[0]))


def gps_test(args):
    gps = Gps()
    print("getCpuTemperature接口", gps.getGPSInfo())


def print_test(args):
    printer = Print()
    print("getPrinterList接口:", printer.getPrinterList())
    print("getPrinterAvailableList接口:", printer.getPrinterAvailableList())
    if len(args) == 0:
        print("其它接口需要参数较多 需要看调用文档")
        sys.exit()

    if len(args) == 1:
        print("getPrinterCancelAllJobs接口:", printer.getPrinterCancelAllJobs(args[0]))
        print("getPrinterStatus接口:", printer.getPrinterStatus(args[0]))
        print("getPrinterFilename接口:", printer.getPrinterFilename(args[0]))
    if len(args) == 2:
        print("getPrinterPrintLocalFile接口:", printer.getPrinterPrintLocalFile(args[0], args[1]))

        print("getPrinterJobStatus接口:", printer.getPrinterJobStatus(args[0], int(args[1])))
        print("getPrinterDownloadRemoteFile接口:", printer.getPrinterDownloadRemoteFile(args[0], args[1]))
    if len(args) == 4:
        print("setPrinterOptions:", printer.setPrinterOptions(int(args[0]), args[1], args[2], args[3]))



def default_print():
    print("请添加-h 或 --help参数查看使用文档")


def help_print(a):
    print("""
    参数列表如下 
    -h 或 --help        参数查看使用文档

    -d 或 --disk        磁盘信息模块相关接口
    -c 或 --cpuinfo     CPU信息模块相关接口
    -n 或 --netcard     网卡信息模块相关接口
    -b 或 --bios        BIOS信息模块相关接口
    -m 或 --mainboard   主板信息模块相关接口
    --peripheralsenum   USB设备信息模块相关接口
    -p 或 --package     包列表信息模块相关接口
    -r 或 --resource    资源信息模块相关接口
    -P 或 --process     进程信息模块相关接口
    -s 或 --sysinfo     系统信息模块相关接口
    -N 或 --net         网络信息模块相关接口
    -R 或 --runinfo     系统运行时信息模块相关接口
    -g 或 --gps         地理位置信息模块相关接口
    --print             打印管理模块相关接口
    """)


enu = {
    "-d": disk_test,
    "--disk": disk_test,
    "-c": cpu_test,
    "--cpuinfo": cpu_test,
    "-n": netcard_test,
    "--netcard": netcard_test,
    "-b": bios_test,
    "--bios": bios_test,
    "-m": mainboard_test,
    "--mainboard": mainboard_test,
    "--peripheralsenum": peripheralsenum_test,
    "-p": package_test,
    "-r": resource_test,
    "-P": process_test,
    "-s": sysinfo_test,
    "-N": net_test,
    "-R": runinfo_test,
    "-g": gps_test,
    "--package": package_test,
    "--resource": resource_test,
    "--process": process_test,
    "--sysinfo": sysinfo_test,
    "--net": net_test,
    "--runinfo": runinfo_test,
    "--gps": gps_test,
    "--print": print_test,
    "-h": help_print,
    "--help": help_print
}

args = sys.argv[1:]
if len(args) == 0:
    default_print()
    sys.exit()

if args[0] in enu.keys():
    enu.get(args[0])(args[1:])
else:
    default_print()
    sys.exit()
