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

package org.example;
import lingmo.lingmosdk.java.method.*;

import java.util.List;

public class Main {
    public static void main(String[] args) {
        switch (args[0])
        {
            case "disk": {
                disk(args);
            }
            break;
            case "cpu": {
                cpu();
            }
            break;
            case "netcard": {
                netcard(args);
            }
            break;
            case "bios": {
                bios();
            }
            break;
            case "mainboard": {
                mainboard();
            }
            case "usb": {
                usb();
            }
            break;
            case "resource": {
                resource();
            }
            break;
            case "process": {
                process(args);
            }
            break;
            case "sysinfo": {
                sysinfo();
            }
            break;
            case "net": {
                net(args);
            }
            break;
            case "runinfo": {
                runinfo(args);
            }
            break;
            case "gps": {
                gps();
            }
            break;
            case "print": {
                print(args);
            }
            break;
            default:{
                System.out.println("使用方式  java -jar <模块> [模块参数]\n 具体模块名及参考使用说明");
            }
        }
        System.exit(0);
    }

    public static void disk(String[] args){
        DiskMethod obj = new DiskMethod();
        List<String> list = obj.getDiskList();
        System.out.println(list);
        if(args.length < 2){
            System.out.println("获取磁盘信息 : java -jar test.jar disk <磁盘路径> ");
            System.exit(0);
        }
        System.out.println("SecSize: " + obj.getDiskSectorSize(args[1]));
        System.out.println("TotaSize: " + obj.getDiskTotalSizeMiB(args[1]));
        System.out.println("Model: " + obj.getDiskModel(args[1]));
        System.out.println("Serial: " + obj.getDiskSerial(args[1]));
        System.out.println("PartNum: " + obj.getDiskPartitionNums(args[1]));
        System.out.println("Type: " + obj.getDiskType(args[1]));
        System.out.println("Version: " + obj.getDiskVersion(args[1]));
        System.out.println("SecNum: " + obj.getDiskSectorNum(args[1]));
    }

    public static void cpu(){
        CpuMethod obj = new CpuMethod();
        System.out.println("Arch: " + obj.getCpuArch());
        System.out.println("Vendor: " + obj.getCpuVendor());
        System.out.println("Model: " + obj.getCpuModel());
        System.out.println("Freq: " + obj.getCpuFreqMHz());
        System.out.println("Core: " + obj.getCpuCorenums());
        System.out.println("Virt: " + obj.getCpuVirt());
        System.out.println("Process: " + obj.getCpuProcess());
    }
    public static void netcard(String[] args){
        NetCardMethod obj = new NetCardMethod();
        List<String> list = obj.getNetCardName();
        System.out.println(list);
        System.out.println("UpCards: " + obj.getNetCardUpcards());
        if(args.length < 2){
            System.out.println("获取网卡信息 : java -jar test.jar netcard <网卡名> ");
            System.exit(0);
        }
        System.out.println("CardUp: " + obj.getNetCardUp(args[1]));
        System.out.println("Mac: " + obj.getNetCardPhymac(args[1]));
        System.out.println("PrivateIPv4: " + obj.getNetCardPrivateIPv4(args[1]));
        System.out.println("Ipv4: " + obj.getNetCardIPv4(args[1]));
        System.out.println("PrivateIPv6: " + obj.getNetCardPrivateIPv6(args[1]));
        System.out.println("Type: " + obj.getNetCardType(args[1]));
//        System.out.println("Product: " + obj.getNetCardProduct(list.get(0)));
        System.out.println("Ipv6: " + obj.getNetCardPrivateIPv6(args[1]));
    }
    public static void bios(){
        BiosMethod obj = new BiosMethod();
        System.out.println("Vendor" + obj.getBiosVendor());
        System.out.println("Version" + obj.getBiosVersion());
    }
    public static void mainboard(){
        MainBoardMethod obj = new MainBoardMethod();
        System.out.println("Name: " + obj.getMainboardName());
        System.out.println("date: " + obj.getMainboardDate());
        System.out.println("Serial: " + obj.getMainboardSerial());
        System.out.println("Vendor: " + obj.getMainboardVendor());
    }
    public static void usb(){
        PeripheralsenumMethod obj = new PeripheralsenumMethod();
        System.out.println("Usb: " + obj.getAllUsbInfo());
    }
    public static void packageinfo(String[] args){
        PackageInfoMethod obj = new PackageInfoMethod();
        List<String> list = obj.getPackageList();
        System.out.println(list);
        if(args.length < 2){
            System.out.println("获取软件包信息 : java -jar test.jar packageinfo <包名> ");
            System.exit(0);
        }
        System.out.println("Version:" + obj.getPackageVersion(args[1]));
        System.out.println("state: " + obj.getPackageInstalled(args[1], obj.getPackageVersion(args[1])));
    }
    public static void resource(){
        ResourceMethod obj = new ResourceMethod();
        System.out.println("Total:" + obj.getMemTotalKiB());
        System.out.println("UsagePer:" + obj.getMemUsagePercent());
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
    public static void process(String[] args){
        if(args.length < 3){
            System.out.println("获取进程信息 : java -jar test.jar process <进程号> <进程名> ");
            System.exit(0);
        }
        ProcessMethod obj = new ProcessMethod();
        int proc = Integer.parseInt(args[1]);
        System.out.println("CpuUsage:" + obj.getProcInfoCpuUsage(proc));
        System.out.println("MemUsage:" + obj.getProcInfoMemUsage(proc));
        System.out.println("State:" + obj.getProcInfoStatus(proc));
        System.out.println("Port:" + obj.getProcInfoPort(proc));
        System.out.println("Start:" + obj.getProcInfoStartTime(proc));
        System.out.println("Running:" + obj.getProcInfoRunningTime(proc));
        System.out.println("CpuTime:" + obj.getProcInfoCpuTime(proc));
        System.out.println("Cmd:" + obj.getProcInfoCmd(proc));
        System.out.println("User:" + obj.getProcInfoUser(proc));
        System.out.println(obj.getProcInfo(args[2]));
//        System.out.println(obj.getAllProcInfo());
    }
    public static void sysinfo(){
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
//        System.out.println("Arch:" + obj.getSysLegalResolution());
    }
    public static void net(String[] args){
        if(args.length < 3){
            System.out.println("获取网络信息 : java -jar test.jar net <单端口号> <区间端口号开始> <区间端口号结束> ");
            System.exit(0);
        }
        NetMethod obj = new NetMethod();
        System.out.println("Port:" + obj.getPortState(5868));
        System.out.println("Multiple:" + obj.getMultiplePortStat(0,10));
        System.out.println("Gateway:" + obj.getGatewayInfo());
        System.out.println("Firewall:" + obj.getFirewallState());
    }
    public static void runinfo(String[] args){
        if(args.length < 3){
            System.out.println("获取运行时信息 : java -jar test.jar runinfo <网卡名> <磁盘路径> ");
            System.exit(0);
        }
        RunInfoMethod obj = new RunInfoMethod();
        System.out.println("NetSpeed:" + obj.getNetSpeed("enp3s0"));
        System.out.println("Rate:" + obj.getDiskRate("/dev/sda"));
        System.out.println("CpuTemp:" + obj.getCpuTemperature());
        System.out.println("DiskTemp:" + obj.getDiskTemperature("/dev/sda"));
    }
    public static void gps(){
        GpsMethod obj = new GpsMethod();
        System.out.println(obj.getGPSInfo());
    }
    public static void print(String[] args){

        PrintMethod obj = new PrintMethod();
        System.out.println("PrinterList:" + obj.getPrinterList());
        System.out.println("AvailableList:" + obj.getPrinterAvailableList());
        if(args.length == 1){
            System.out.println("其它接口需要参数较多 需要看调用文档");
            System.exit(0);
        }
        if(args.length == 3){
            if(Integer.parseInt(args[2]) == 0){
                System.out.println("CancleJob:" + obj.getPrinterCancelAllJobs(args[1]));
                System.out.println("Statue:" + obj.getPrinterStatus(args[1]));
            }
            else
                System.out.println("FileName:" + obj.getPrinterFilename(args[1]));
        }
        if(args.length == 4){
            switch (Integer.parseInt(args[3])){
                case 0:{
                    System.out.println("LocalFile:" + obj.getPrinterPrintLocalFile(args[1],args[2]));
                }
                break;
                case 1:{
                    System.out.println("JobStatus:" + obj.getPrinterJobStatus(args[1],Integer.parseInt(args[2])));
                }
                break;
                case 2:{
                    System.out.println("RemoteFile:" + obj.getPrinterDownloadRemoteFile(args[1],args[2]));
                }
                break;
                default:
            }
        }
        if(args.length == 5)
            obj.setPrinterOptions(Integer.parseInt(args[1]),args[2],args[3],args[4]);
    }
}
