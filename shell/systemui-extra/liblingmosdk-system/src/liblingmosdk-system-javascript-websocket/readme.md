# 系统能力SDK

创建/etc/websocket/lingmosdk-dbus-websocket.conf配置文件，在配置文件中配置端口号，例如
```
[WebSocketServer]
port=xxx
```
## 操作系统基础信息
**服务名称：** com.lingmo.lingmosdk.service

**路径名称：** /com/lingmo/lingmosdk/sysinfo

**接口名称：** com.lingmo.lingmosdk.sysinfo

    Methods：
        1. QString getSystemArchitecture(void)
            输出：操作系统架构信息
        2. QString getSystemName(void)
            输出：操作系统名称
        3. QString getSystemVersion(bool vsname)
            输入：获取版本号
            输出：操作系统版本号
        4. int getSystemActivationStatus(void)
            输出：操作系统激活状态
        5. QString getSystemSerialNumber(void)
            输出：操作系统服务序列号
        6. QString getSystemKernelVersion(void)
            输出：内核版本号
        7. QString getSystemEffectUser(void)
            输出：当前登录用户的用户名
        8. QString getSystemProjectName(void)
            输出：操作系统项目编号名
        9. QString getSystemProjectSubName(void)
            输出：操作系统项目子编号名
        10. int getSystemProductFeatures(void)
            输出：操作系统产品标识码
        11. QString getSystemHostVirtType(void)
            输出：操作系统宿主机的虚拟机类型
        12. QString getSystemHostCloudPlatform(void)
            输出：操作系统宿主机的云平台类型
        13. QString getSystemOSVersion(void)
            输出：操作系统OS版本号
        14. QString getSystemUpdateVersion(void)
            输出：操作系统Update版本号
        15. bool getSystemIsZYJ(void)
            输出：镜像系统是否为专用机系统
        16. QString getSysLegalResolution(void)
            输出：屏幕设置的分辨率
        

**JS接口：**  
    websocket:
        var websocket_url = 'ws://localhost:12345';
        var websocket = null;

        if (websocket === null) {
            websocket = new WebSocket(websocket_url);
            websocket.onopen = function () {
                console.log("connect websocketserver success");
            }
        } else {
            websocket.close();
            websocket = null;
        }
        function xxx() {
            new QWebChannel(websocket,function(channel){
                var sysinfo = channel.objects.sysinfo;
                //返回信息接收
                sysinfo.sendText.connect(function(message) {    
                    ...
                });
                //获取操作系统架构信息
                sysinfo.getSystemArchitecture();
                //获取操作系统名称
                sysinfo.getSystemName();
                //获取操作系统版本号
                sysinfo.getSystemVersion(pid);
                //获取操作系统激活状态
                sysinfo.getSystemActivationStatus();
                //获取操作系统服务序列号
                sysinfo.getSystemSerialNumber();
                //获取内核版本号
                sysinfo.getSystemKernelVersion();
                //获取当前登录用户的用户名
                sysinfo.getSystemEffectUser();
                //获取操作系统项目编号名
                sysinfo.getSystemProjectName();
                //获取操作系统项目子编号名
                sysinfo.getSystemProjectSubName();
                //获取操作系统产品标识码
                sysinfo.getSystemProductFeatures();
                //获取操作系统宿主机的虚拟机类型
                sysinfo.getSystemHostVirtType();
                //获取操作系统宿主机的云平台类型
                sysinfo.getSystemHostCloudPlatform();
                //获取操作系统OS版本号
                sysinfo.getSystemOSVersion();
                //获取操作系统Update版本号
                sysinfo.getSystemUpdateVersion();
                //获取镜像系统是否为专用机系统
                sysinfo.getSystemIsZYJ();
                //获取屏幕设置的分辨率
                sysinfo.getSysLegalResolution();
                }
            );
        }
---
## 网络连接
**服务名称：** com.lingmo.lingmosdk.service

**路径名称：** /com/lingmo/lingmosdk/netLink/

**接口名称：** com.lingmo.lingmosdk.netLink

    Methods：
        1. unsigned int getNetState(void)
            输出: 网络连接状态

**JS接口：**  
    websocket:
        var websocket_url = 'ws://localhost:12345';
        var websocket = null;

        if (websocket === null) {
            websocket = new WebSocket(websocket_url);
            websocket.onopen = function () {
                console.log("connect websocketserver success");
            }
        } else {
            websocket.close();
            websocket = null;
        }
        function xxx() {
            new QWebChannel(websocket,function(channel){
                 var netlink = channel.objects.netlink;
                 //返回信息接收
                 netlink.sendText.connect(function(message) {
                     ...
                 });
                 //获取网络连接状态
                 netlink.getNetState();
                }
            );
        }                
             
---
## 外设打印
**服务名称：** com.lingmo.lingmosdk.service

**路径名称：** /com/lingmo/lingmosdk/print

**接口名称：** com.lingmo.lingmosdk.print

    Methods：
        1. QStringList getPrinterList(void)
            输出：获取打印机列表
        2. QStringList getPrinterAvailableList(void)
            输出：获取可用打印机列表
        3. void setPrinterOptions(int number_up, QString media, QString number_up_layout, QString sides)
            输出：设置打印参数
        4. int getPrinterPrintLocalFile(QString printname,QString printpath)
            输出：打印本地文件
        5. int getPrinterCancelAllJobs(QString printname)
            输出：取消所有作业
        6. int getPrinterStatus(QString printname)
            输出：获取打印机状态
        7. QString getPrinterFilename(QString printpath)
            输出：打印文件名
        8. int getPrinterJobStatus(QString printname, int printid)
            输出：获取打印机作业状态
        9. int getPrinterDownloadRemoteFile(QString printfile, QString printpath)
            输出：打印远程文件
        
**JS接口：**  
    websocket:
        var websocket_url = 'ws://localhost:12345';
        var websocket = null;

        if (websocket === null) {
            websocket = new WebSocket(websocket_url);
            websocket.onopen = function () {
                console.log("connect websocketserver success");
            }
        } else {
            websocket.close();
            websocket = null;
        }
        function xxx() {
            new QWebChannel(websocket,function(channel){
                 var printer = channel.objects.print;
                 //返回消息接收
                 printer.sendText.connect(function(message) {
                    ...
                 });
                 //获取打印机列表
                 printer.getPrinterList();
                 //获取可用打印机列表
                 printer.getPrinterAvailableList();
                 //设置打印参数
                 printer.setPrinterOptions(command);
                 //打印本地文件
                 printer.getPrinterPrintLocalFile(command);
                 //取消所有作业
                 printer.getPrinterCancelAllJobs(printer_type);
                 //获取打印机状态
                 printer.getPrinterStatus(printer_type);
                 //打印文件名
                 printer.getPrinterFilename(download_link);
                 //获取打印机作业状态
                 printer.getPrinterJobStatus(command);
                 //打印远程文件
                 printer.getPrinterDownloadRemoteFile(command);
                }
            );
        }     
---
## 外设设备枚举
**服务名称：** com.lingmo.lingmosdk.service

**路径名称：** /com/lingmo/lingmosdk/peripheralsenum/

**接口名称：** com.lingmo.lingmosdk.peripheralsenum

    Methods：
        1. QStringList getAllUsbInfo(void)
            输出：所有usb设备的名称、类型、PID、VID、序列号、设备节点（若没有对应信息，输出null）

**JS接口：**  
    websocket:
        var peripheralsenum = channel.objects.peripheralsenum;
        //返回信息接收
        peripheralsenum.sendText.connect(function(message) {
            ...
        });
        //获取外设设备信息
        peripheralsenum.getAllUsbInfo();

---
## 网卡信息
**服务名称：** com.lingmo.lingmosdk.service

**路径名称：** /com/lingmo/lingmosdk/netcard/

**接口名称：** com.lingmo.lingmosdk.netcard

    Methods：
        1. QStringList getNetCardName(void)
            输出：所有网卡的名称
        2. int getNetCardUp(QString netCardName)
            输入：网卡名
            输出：网卡的UP状态
        3. QStringList getNetCardUpcards(void)
            输出：处于 link up 状态的网卡列表
        4. QString getNetCardPhymac(QString netCardName)
            输入：网卡名
            输出：网卡的物理MAC地址
        5. QString getNetCardPrivateIPv4(QString netCardName)
            输入：网卡名
            输出：网卡的第一个IPv4地址
        6. QStringList getNetCardIPv4(QString netCardName)
            输入：网卡名
            输出：网卡的所有IPv4地址
        7. QString getNetCardPrivateIPv6(QString netCardName)
            输入：网卡名
            输出：网卡的第一个IPv6地址
        8. int getNetCardType(QString netCardName)
            输入：网卡名
            输出：网卡类型
        9. QStringList getNetCardProduct(QString netCardName)
            输入：网卡名
            输出：网卡厂商型号
        10. QStringList getNetCardIPv6(QString netCardName)
            输入：网卡名
            输出：网卡的所有IPv6地址
**JS接口：**  
    websocket:
        var websocket_url = 'ws://localhost:12345';
        var websocket = null;

        if (websocket === null) {
            websocket = new WebSocket(websocket_url);
            websocket.onopen = function () {
                console.log("connect websocketserver success");
            }
        } else {
            websocket.close();
            websocket = null;
        }
        function xxx() {
            new QWebChannel(websocket,function(channel){
                var netcard = channel.objects.netcard;
                //返回信息接收
                netcard.sendText.connect(function(message) {
                    ...
                });
                //获取网卡名称
                netcard.getNetCardName();
                //获取网卡的UP状态
                netcard.getNetCardUp(pid);
                //获取处于link up状态的网卡列表
                netcard.getNetCardUpcards();
                //获取网卡的物理MAC地址
                netcard.getNetCardPhymac(pid);
                //获取网卡的第一个IPv4地址
                netcard.getNetCardPrivateIPv4(pid);
                //获取网卡的所有IPv4地址
                netcard.getNetCardIPv4(pid);
                //获取网卡的第一个IPv6地址
                netcard.getNetCardPrivateIPv6(pid);
                //获取网卡类型
                netcard.getNetCardType(pid);
                //获取网卡厂商型号
                netcard.getNetCardProduct(pid);
                //获取网卡的所有IPv6地址
                netcard.getNetCardIPv6(pid);
                }
            );
        }     
---
## BIOS信息
**服务名称：** com.lingmo.lingmosdk.service

**路径名称：** /com/lingmo/lingmosdk/bios/

**接口名称：** com.lingmo.lingmosdk.bios

    Methods：
        1. QStringList getBiosVendor(void)
            输出：BIOS厂商
        2. QString getBiosVersion(void)
            输出：BIOS版本号

**JS接口：**  
    websocket:
        var websocket_url = 'ws://localhost:12345';
        var websocket = null;

        if (websocket === null) {
            websocket = new WebSocket(websocket_url);
            websocket.onopen = function () {
                console.log("connect websocketserver success");
            }
        } else {
            websocket.close();
            websocket = null;
        }
        function xxx() {
            new QWebChannel(websocket,function(channel){
                 var bios = channel.objects.bios;
                 //返回信息接收
                 bios.sendText.connect(function(message) {
                     ...
                 });
                 //获取BIOS厂商
                 bios.getBiosVendor();
                 //获取BIOS版本号
                 bios.getBiosVersion();
                }
            );
        }   
---
## 主板信息
**服务名称：** com.lingmo.lingmosdk.service

**路径名称：** /com/lingmo/lingmosdk/mainboard/

**接口名称：** com.lingmo.lingmosdk.mainboard

    Methods：
        1. QString getMainboardName(void)
            输出：主板型号
        2. QString getMainboardDate(void)
            输出：发布日期
        3. QString getMainboardSerial(void)
            输出：主板序列号
        4. QString getMainboardVendor(void)
            输出：主板厂商

**JS接口：**  
    websocket:
        var websocket_url = 'ws://localhost:12345';
        var websocket = null;

        if (websocket === null) {
            websocket = new WebSocket(websocket_url);
            websocket.onopen = function () {
                console.log("connect websocketserver success");
            }
        } else {
            websocket.close();
            websocket = null;
        }
        function xxx() {
            new QWebChannel(websocket,function(channel){
                 var mainboard = channel.objects.mainboard;
                 //返回信息接收
                 mainboard.sendText.connect(function(message) {
                     ...
                 });
                 //获取主板型号
                 mainboard.getMainboardName();
                 //获取发布日期
                 mainboard.getMainboardDate();
                 //获取主板序列号
                 mainboard.getMainboardSerial();
                 //获取主板厂商
                 mainboard.getMainboardVendor();
                }
            );
        }  
---
## 磁盘
**服务名称：** com.lingmo.lingmosdk.service

**路径名称：** /com/lingmo/lingmosdk/disk/

**接口名称：** com.lingmo.lingmosdk.disk


    Methods：
        1. QStringList getDiskList();
            输出：磁盘列表
        2. int getDiskSectorSize(QString DiskName);
            输入：磁盘名称
            输出：扇区字节数
        3. double getDiskTotalSizeMiB(QString DiskName);
            输入：磁盘名称
            输出：磁盘容量
        4. QString getDiskModel(QString DiskName);
            输入：磁盘名称
            输出：磁盘型号
        5. QString getDiskSerial(QString DiskName);
            输入:磁盘名称
            输出：磁盘序列号
        6. int getDiskPartitionNums(QString DiskName);
            输入：磁盘名称
            输出：子分区数量
        7. QString getDiskType(QString DiskName);
            输入：磁盘名称
            输出：磁盘类型
        8. QString getDiskVersion(QString DiskName);
            输入：磁盘名称
            输出：固件版本信息
        9. unsigned long long getDiskSectorNum(QString DiskName);
            输入：磁盘名称
            输出：磁盘扇区数量

**JS接口：**  
    websocket:
        var websocket_url = 'ws://localhost:12345';
        var websocket = null;

        if (websocket === null) {
            websocket = new WebSocket(websocket_url);
            websocket.onopen = function () {
                console.log("connect websocketserver success");
            }
        } else {
            websocket.close();
            websocket = null;
        }
        function xxx() {
            new QWebChannel(websocket,function(channel){
                 var disk = channel.objects.disk;
                 //返回信息接收
                 disk.sendText.connect(function(message) {
                     ...
                 });
                 //获取磁盘列表
                 disk.getDiskList();
                 //获取扇区字节数
                 disk.getDiskSectorSize(pid);
                 //获取磁盘容量
                 disk.getDiskTotalSizeMiB(pid);
                 //获取磁盘型号
                 disk.getDiskModel(pid);
                 //获取磁盘序列号
                 disk.getDiskSerial(pid);
                 //获取子分区数量
                 disk.getDiskPartitionNums(pid);
                 //获取磁盘类型
                 disk.getDiskType(pid);
                 //获取固件版本信息
                 disk.getDiskVersion(pid);
                 //获取磁盘扇区数量
                 disk.getDiskSectorNum(pid);
                }
            );
        }  
---
## 网络
**服务名称：** com.lingmo.lingmosdk.service

**路径名称：** /com/lingmo/lingmosdk/net/

**接口名称：** com.lingmo.lingmosdk.net

    Methods：获取端口状态，网关信息，防火墙状态
        1. int getPortState(int port)
            输入：端口号
            输出：端口状态
            （0-FREE， 1-TCP_ESTABLISHED, 2-TCP_SYN_SENT, 3-TCP_SYN_RECV,
            4-TCP_FIN_WAIT1， 5-TCP_FIN_WAIT2， 6-TCP_TIME_WAIT, 7-TCP_CLOSE, 8-TCP_CLOSE_WAIT, 9-TCP_LAST_ACL， 10-TCP_LISTEN, 11-TCP_CLOSING）
        2. QStringList getMultiplePortStat(int start, int end)
            输入：开始端口号， 结束端口号
            输出：接受端口状态
        3. QStringList getGatewayInfo(void)
            输出：网关信息-名称，地址
        4. QStringList getFirewallState(void)
            输出：防火墙信息

**JS接口：** 
    websocket:
        var websocket_url = 'ws://localhost:12345';
        var websocket = null;

        if (websocket === null) {
            websocket = new WebSocket(websocket_url);
            websocket.onopen = function () {
                console.log("connect websocketserver success");
            }
        } else {
            websocket.close();
            websocket = null;
        }
        function xxx() {
            new QWebChannel(websocket,function(channel){
                 var net = channel.objects.net;
                 //返回信息接收
                 net.sendText.connect(function(message) {
                     ...
                 });
                 //获取端口状态
                 net.getPortState(pid);
                 //获取接受端口状态
                 net.getMultiplePortStat(beginpid, endpid);
                 //获取网关信息
                 net.getGatewayInfo();
                 //获取防火墙信息
                 net.getFirewallState();
                }
            );
        }  
---
## 运行时信息
**服务名称：** com.lingmo.lingmosdk.service

**路径名称：** /com/lingmo/lingmosdk/runinfo/

**接口名称：** com.lingmo.lingmosdk.runinfo

    Methods：
        1. double getNetSpeed(QString nc)
            输入：硬盘绝对路径
            输出：实时网速
        2. int getDiskRate(QString diskpath)
            输入：硬盘绝对路径
            输出：磁盘转速
        3. double getCpuTemperature(void)
            输出：实时CPU温度
        4. double getDiskTemperature(QString diskpath)
            输入：硬盘绝对路径
            输出：实时硬盘温度

**JS接口：**  
    websocket:
        var websocket_url = 'ws://localhost:12345';
        var websocket = null;

        if (websocket === null) {
            websocket = new WebSocket(websocket_url);
            websocket.onopen = function () {
                console.log("connect websocketserver success");
            }
        } else {
            websocket.close();
            websocket = null;
        }
        function xxx() {
            new QWebChannel(websocket,function(channel){
                 var runinfo = channel.objects.runinfo;
                 //返回信息接收
                 runinfo.sendText.connect(function(message) {
                     ...
                 });
                 //获取实时网速
                 runinfo.getNetSpeed(pid);
                 //获取磁盘转速
                 runinfo.getDiskRate(pid);
                 //获取实时CPU温度
                 runinfo.getCpuTemperature();
                 //获取实时硬盘温度
                 runinfo.getDiskTemperature(pid);
                }
            );
        }  

---
## 地理位置
**服务名称：** com.lingmo.lingmosdk.service

**路径名称：** /com/lingmo/lingmosdk/gps/

**接口名称：** com.lingmo.lingmosdk.gps

    Methods：
        1. QString getGPSInfo(void)
            输出：国家城市经纬度

**JS接口：**  
    websocket:
        var websocket_url = 'ws://localhost:12345';
        var websocket = null;

        if (websocket === null) {
            websocket = new WebSocket(websocket_url);
            websocket.onopen = function () {
                console.log("connect websocketserver success");
            }
        } else {
            websocket.close();
            websocket = null;
        }
        function xxx() {
            new QWebChannel(websocket,function(channel){
                 var gps = channel.objects.gps;
                 //返回信息接收
                 gps.sendText.connect(function(message) {
                     ...
                 });
                 //接收国家城市经纬度
                 gps.getGPSInfo();
                }
            );
        }                

---
## 进程信息
**服务名称：** com.lingmo.lingmosdk.service

**路径名称：** /com/lingmo/lingmosdk/process/

**接口名称：** com.lingmo.lingmosdk.process

    Methods：
        1. double getProcInfoCpuUsage(int pid)
            输入：进程号
            输出：CPU瞬时使用率
        2. double getProcInfoIoUsage(int pid)
            输入：进程号
            输出：IO使用率
        3. double getProcInfoMemUsage(int pid)
            输入：进程号
            输出：内存占用率
        4. QString getProcInfoStatus(int pid)
            输入：进程号
            输出：进程状态
        5. int getProcInfoPort(int pid)
            输入：进程号
            输出：进程使用的端口号
        6. QString getProcInfoStartTime(int pid)
            输入：进程号
            输出：进程的启动时间
        7. QString getProcInfoRunningTime(int pid)
            输入：进程号
            输出：进程的运行时间
        8. QString getProcInfoCpuTime(int pid)
            输入：进程号
            输出：CPU时间
        9. QString getProcInfoCmd(int pid)
            输入：进程号
            输出：cmd
        10. QString getProcInfoUser(int pid)
            输入：进程号
            输出：属主
        11. QStringList getProcInfo(QString  processname)
            输入：进程名
            输出：某进程所有信息
        12. QStringList getAllProcInfo(void)
            输出：系统中所有进程信息

**JS接口：**  
    websocket:
        var websocket_url = 'ws://localhost:12345';
        var websocket = null;

        if (websocket === null) {
            websocket = new WebSocket(websocket_url);
            websocket.onopen = function () {
                console.log("connect websocketserver success");
            }
        } else {
            websocket.close();
            websocket = null;
        }
        function xxx() {
            new QWebChannel(websocket,function(channel){
                var process = channel.objects.process;
                //返回信息接收
                process.sendText.connect(function(message) {
                    ...
                });
                //获取CPU瞬时使用率
                process.getProcInfoCpuUsage(pid);
                //获取实时网速
                process.getProcInfoIoUsage(pid);
                //获取内存占用率
                process.getProcInfoMemUsage(pid);
                //获取进程状态
                process.getProcInfoStatus(pid);
                //获取进程使用的端口号
                process.getProcInfoPort(pid);
                //获取进程的启动时间
                process.getProcInfoStartTime(pid);
                //获取进程的运行时间
                process.getProcInfoRunningTime(pid);
                //获取CPU时间
                process.getProcInfoCpuTime(pid);
                //获取cmd
                process.getProcInfoCmd(pid);
                //获取属主

## cpu信息
**服务名称：** com.lingmo.lingmosdk.service

**路径名称：** /com/lingmo/lingmosdk/cpuinfo/

**接口名称：** com.lingmo.lingmosdk.cpuinfo

    Methods：
        1.  QString getCpuArch();
            输出：CPU架构
        2.  QString getCpuVendor();
            输出：CPU制造厂商
        3.  QString getCpuModel();
            输出：CPU型号
        4.  QString getCpuFreqMHz();
            输出：CPU额定主频
        5.  int getCpuCorenums();
            输出：CPU核心数量
        6.  QString getCpuVirt();
            输出：对虚拟化的支持
        7.  int getCpuProcess();
            输出：CPU线程数
            
          

**JS接口：**  
    websocket:
        var websocket_url = 'ws://localhost:12345';
        var websocket = null;

        if (websocket === null) {
            websocket = new WebSocket(websocket_url);
            websocket.onopen = function () {
                console.log("connect websocketserver success");
            }
        } else {
            websocket.close();
            websocket = null;
        }
        function xxx() {
            new QWebChannel(websocket,function(channel){
                 var cpuinfo = channel.objects.cpuinfo;
                 //返回信息接收
                 cpuinfo.sendText.connect(function(message) {
                     ...
                 });
                 //获得CPU架构
                 cpuinfo.getCpuArch();
                 //获得CPU制造厂商
                 cpuinfo.getCpuVendor();
                 //获得CPU型号
                 cpuinfo.getCpuModel();
                 //获得CPU额定主频
                 cpuinfo.getCpuFreqMHz();
                 //获得CPU核心数量
                 cpuinfo.getCpuCorenums();
                 //对虚拟化的支持
                 cpuinfo.getCpuVirt();
                 //获得CPU线程数
                 cpuinfo.getCpuProcess();
                }
            );
        }  

## 包列表信息
**服务名称：** com.lingmo.lingmosdk.service

**路径名称：** /com/lingmo/lingmosdk/packageinfo/

**接口名称：** com.lingmo.lingmosdk.packageinfo

    Methods：
        1.  QStringList getPackageList();
            输出：系统中所有包列表
        2.  QString getPackageVersion(QString);
            输入：软件包名
            输出：系统中指定包的版本号
        3.  int getPackageInstalled(QString,QString);
            输入：软件包名 版本号
            输出：软件包是否正确安装
            
          

**JS接口：**  
    websocket:
        var websocket_url = 'ws://localhost:12345';
        var websocket = null;

        if (websocket === null) {
            websocket = new WebSocket(websocket_url);
            websocket.onopen = function () {
                console.log("connect websocketserver success");
            }
        } else {
            websocket.close();
            websocket = null;
        }
        function xxx() {
            pkgname = $("#pkgname").val()
            vsname = $("#vsname").val()
            new QWebChannel(websocket,function(channel){
                 var packageinfo = channel.objects.packageinfo;
                 //返回信息接收
                 packageinfo.sendText.connect(function(message) {
                     ...
                 });
                 //获得系统中所有包列表
                 packageinfo.getPackageList();
                 //获得系统中指定包的版本号
                 packageinfo.getPackageVersion(pkgname);
                 //软件包是否正确安装
                 packageinfo.getPackageInstalled(pkgname, vsname);
                }
            );
        }  

## 资源信息
**服务名称：** com.lingmo.lingmosdk.service

**路径名称：** /com/lingmo/lingmosdk/resource

**接口名称：** com.lingmo.lingmosdk.resource

    Methods：
        1.  double getMemTotalKiB(void);
            输出：物理内存总大小
        2.  double getMemUsagePercent(void);
            输出：物理内存使用率
        3.  double getMemUsageKiB(void);
            输出：物理内存使用大小
        4.  double getMemAvailableKiB(void);
            输出：可用物理内存大小
        5.  double getMemFreeKiB(void);
            输出：空闲物理内存大小
        6.  double getMemVirtAllocKiB(void);
            输出：虚拟内存总量
        7.  double getMemSwapTotalKiB(void);
            输出：Swap分区总大小
        8.  double getMemSwapUsagePercent(void);
            输出：Swap分区使用率
        9.  double getMemSwapUsageKiB(void);
            输出：Swap分区使用量
        10. double getMemSwapFreeKiB(void);
            输出：Swap分区空闲大小
        11. double getCpuCurrentUsage(void);
            输出：CPU瞬时使用率
        12. QString getUpTime(void);
            输出：开机天数
            
          

**JS接口：**  
    websocket:
        var websocket_url = 'ws://localhost:12345';
        var websocket = null;

        if (websocket === null) {
            websocket = new WebSocket(websocket_url);
            websocket.onopen = function () {
                console.log("connect websocketserver success");
            }
        } else {
            websocket.close();
            websocket = null;
        }
        function xxx() {
            new QWebChannel(websocket,function(channel){
                 var resource = channel.objects.resource;
                 //返回信息接收
                 resource.sendText.connect(function(message) {
                     ...
                 });
                 //获得物理内存总大小
                 resource.getMemTotalKiB();
                 //获得物理内存使用率
                 resource.getMemUsagePercent();
                 //获得物理内存使用大小
                 resource.getMemUsageKiB();
                 //获得可用物理内存大小
                 resource.getMemAvailableKiB();
                 //获得空闲物理内存大小
                 resource.getMemFreeKiB();
                 //获得虚拟内存总量
                 resource.getMemVirtAllocKiB();
                 //获得Swap分区总大小 
                 resource.getMemSwapTotalKiB();
                 //获得Swap分区使用率
                 resource.getMemSwapUsagePercent();
                 //获得Swap分区使用量
                 resource.getMemSwapUsageKiB();
                 //获得Swap分区空闲大小
                 resource.getMemSwapFreeKiB();
                 //获得CPU瞬时使用率
                 resource.getCpuCurrentUsage();
                 //获得开机天数
                 resource.getUpTime();  
                }
            );
        }  