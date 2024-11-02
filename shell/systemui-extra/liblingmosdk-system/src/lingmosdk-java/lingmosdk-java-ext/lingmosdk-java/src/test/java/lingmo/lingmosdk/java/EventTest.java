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

package lingmo.lingmosdk.java;

import lingmo.lingmosdk.java.method.*;
import lombok.extern.slf4j.Slf4j;
import org.freedesktop.dbus.types.UInt64;
import org.junit.jupiter.api.Test;
import java.util.List;

@Slf4j
public class EventTest {
    @Test
    public void disk() throws InterruptedException {
        DiskMethod obj = new DiskMethod();
        List<String> list = obj.getDiskList();
        System.out.println(list);
        System.out.println("SecSize: " + obj.getDiskSectorSize(list.get(0)));
        System.out.println("TotaSize: " + obj.getDiskTotalSizeMiB(list.get(0)));
        System.out.println("Model: " + obj.getDiskModel(list.get(0)));
        System.out.println("Serial: " + obj.getDiskSerial(list.get(0)));
        System.out.println("PartNum: " + obj.getDiskPartitionNums(list.get(0)));
        System.out.println("Type: " + obj.getDiskType(list.get(0)));
        System.out.println("Version: " + obj.getDiskVersion(list.get(0)));
        System.out.println("SecNum: " + obj.getDiskSectorNum(list.get(0)));
    }
    @Test
    public void cpu() throws InterruptedException {
        CpuMethod obj = new CpuMethod();
        System.out.println("Arch: " + obj.getCpuArch());
        System.out.println("Vendor: " + obj.getCpuVendor());
        System.out.println("Model: " + obj.getCpuModel());
        System.out.println("Freq: " + obj.getCpuFreqMHz());
        System.out.println("Core: " + obj.getCpuCorenums());
        System.out.println("Virt: " + obj.getCpuVirt());
        System.out.println("Process: " + obj.getCpuProcess());
    }

    @Test
    public void netcard() throws InterruptedException {
        NetCardMethod obj = new NetCardMethod();
        List<String> list = obj.getNetCardName();
        System.out.println(list);
        System.out.println("CardUp: " + obj.getNetCardUp(list.get(0)));
        System.out.println("UpCards: " + obj.getNetCardUpcards());
        System.out.println("Mac: " + obj.getNetCardPhymac(list.get(0)));
        System.out.println("PrivateIPv4: " + obj.getNetCardPrivateIPv4(list.get(0)));
        System.out.println("Ipv4: " + obj.getNetCardIPv4(list.get(0)));
        System.out.println("PrivateIPv6: " + obj.getNetCardPrivateIPv6(list.get(0)));
        System.out.println("Type: " + obj.getNetCardType(list.get(0)));
        System.out.println("Product: " + obj.getNetCardProduct(list.get(0)));
        System.out.println("Ipv6: " + obj.getNetCardPrivateIPv6(list.get(0)));
    }

    @Test
    public void bios() throws InterruptedException {
        BiosMethod obj = new BiosMethod();
        System.out.println("Vendor" + obj.getBiosVendor());
        System.out.println("Version" + obj.getBiosVersion());
    }
    @Test
    public void mainboard() throws InterruptedException {
        MainBoardMethod obj = new MainBoardMethod();
        System.out.println("Name: " + obj.getMainboardName());
        System.out.println("date: " + obj.getMainboardDate());
        System.out.println("Serial: " + obj.getMainboardSerial());
        System.out.println("Vendor: " + obj.getMainboardVendor());
    }

    @Test
    public void usb() throws InterruptedException {
        PeripheralsenumMethod obj = new PeripheralsenumMethod();
        System.out.println("Usb: " + obj.getAllUsbInfo());
    }

    @Test
    public void packageinfo() throws InterruptedException {
        PackageInfoMethod obj = new PackageInfoMethod();
        List<String> list = obj.getPackageList();
        System.out.println(list);
        System.out.println("Version:" + obj.getPackageVersion(list.get(1)));
        System.out.println("state: " + obj.getPackageInstalled(list.get(1), obj.getPackageVersion(list.get(1))));
    }

    @Test
    public void resource() throws InterruptedException {
        ResourceMethod obj = new ResourceMethod();
        System.out.println("Total:" + obj.getMemTotalKiB());
        System.out.println("UsagePer:" + obj.getMemSwapUsagePercent());
        System.out.println("Usage:" + obj.getMemUsageKiB());
        System.out.println("Avail:" + obj.getMemAvailableKiB());
        System.out.println("Free:" + obj.getMemFreeKiB());
        System.out.println("Virt:" + obj.getMemVirtAllocKiB());
        System.out.println("SwapTotal:" + obj.getMemSwapTotalKiB());
        System.out.println("SwapUsagePer:" + obj.getMemSwapUsagePercent());
        System.out.println("SwapUsage:" + obj.getMemSwapUsageKiB());
        System.out.println("SwageFree:" + obj.getMemSwapFreeKiB());
        System.out.println("CpuUsage:" + obj.getCpuCurrentUsage());
        System.out.println("UpTime:" + obj.getUpTime());
    }

    @Test
    public void process() throws InterruptedException {

        ProcessMethod obj = new ProcessMethod();
        int proc = 2203;
        System.out.println("CpuUsage:" + obj.getProcInfoCpuUsage(proc));
        System.out.println("MemUsage:" + obj.getProcInfoMemUsage(proc));
        System.out.println("State:" + obj.getProcInfoStatus(proc));
        System.out.println("Port:" + obj.getProcInfoPort(proc));
        System.out.println("Start:" + obj.getProcInfoStartTime(proc));
        System.out.println("Running:" + obj.getProcInfoRunningTime(proc));
        System.out.println("CpuTime:" + obj.getProcInfoCpuTime(proc));
        System.out.println("Cmd:" + obj.getProcInfoCmd(proc));
        System.out.println("User:" + obj.getProcInfoUser(proc));
        System.out.println(obj.getProcInfo("code"));
        System.out.println(obj.getAllProcInfo());
    }

    @Test
    public void sysinfo() throws InterruptedException {
        SysInfoMethod obj = new SysInfoMethod();
        System.out.println("Arch:" + obj.getSystemArchitecture());
        System.out.println("Name:" + obj.getSystemName());
        System.out.println("Version:" + obj.getSystemVersion(false));
        System.out.println("Activation:" + obj.getSystemActivationStatus());
        System.out.println("Serial:" + obj.getSystemSerialNumber());
        System.out.println("KernelVer:" + obj.getSystemKernelVersion());
        System.out.println("Effect:" + obj.getSystemEffectUser());
        System.out.println("ProName:" + obj.getSystemProjectName());
        System.out.println("SubName:" + obj.getSystemProjectSubName());
        System.out.println("Product:" + obj.getSystemProductFeatures());
        System.out.println("Virt:" + obj.getSystemHostVirtType());
        System.out.println("Paltform:" + obj.getSystemHostCloudPlatform());
        System.out.println("OSVer:" + obj.getSystemOSVersion());
        System.out.println("UpdateVer:" + obj.getSystemUpdateVersion());
        System.out.println("ZYJ:" + obj.getSystemIsZYJ());
        System.out.println("Arch:" + obj.getSysLegalResolution());
    }

    @Test
    public void net() throws InterruptedException {
        NetMethod obj = new NetMethod();
        System.out.println("Port:" + obj.getPortState(5868));
        System.out.println("Multiple:" + obj.getMultiplePortStat(0,500));
        System.out.println("Gateway:" + obj.getGatewayInfo());
        System.out.println("Firewall:" + obj.getFirewallState());
    }

    @Test
    public void runInfo() throws InterruptedException {
        RunInfoMethod obj = new RunInfoMethod();
        System.out.println("NetSpeed:" + obj.getNetSpeed("enp3s0"));
        System.out.println("Rate:" + obj.getDiskRate("/dev/sda"));
        System.out.println("CpuTemp:" + obj.getCpuTemperature());
        System.out.println("DiskTemp:" + obj.getDiskTemperature("/dev/sda"));
    }

    @Test
    public static void gps() throws InterruptedException {
        GpsMethod obj = new GpsMethod();
        System.out.println(obj.getGPSInfo());
    }

    @Test
    public static void print() throws InterruptedException {
        PrintMethod obj = new PrintMethod();
        System.out.println("PrinterList:" + obj.getPrinterList());
        System.out.println("AvailableList:" + obj.getPrinterAvailableList());
        obj.setPrinterOptions(1, "A4", "", "one-sided");
        System.out.println("LocalFile:" + obj.getPrinterPrintLocalFile("", ""));
        System.out.println("CancleJob:" + obj.getPrinterCancelAllJobs(""));
        System.out.println("Statue:" + obj.getPrinterStatus(""));
        System.out.println("FileName:" + obj.getPrinterFilename(""));
        System.out.println("JobStatus:" + obj.getPrinterJobStatus("", 0));
        System.out.println("RemoteFile:" + obj.getPrinterDownloadRemoteFile("", ""));
    }
}
  
