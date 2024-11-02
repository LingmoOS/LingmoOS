## lingmo-nm介绍
### 简介
  灵墨网络工具（lingmo-network-manager，简称lingmo-nm）是操作系统的网络前端，其主要功能有：  
  1、托盘栏图标及右键菜单  
     查看当前网络状态  
     左键点击托盘网络图标显示lingmo-nm主界面  
     右键点击托盘网络图标显示右键菜单  
  2、对有线网络的管理  
     有线网络界面管理  
     连接/断开网络  
     网线拔插  
     多有线网卡支持  
     打开或关闭有线开关  
     查看或修改无线网络基本信息  
  3、对无线网络的管理  
     切换及刷新无线界面  
     查看无线网络列表  
     连接/断开无需密码的无线网  
     连接/断开有密码类型的无线网  
     无线网卡插拔  
     多无线网卡支持  
     打开或关闭无线开关  
     查看或修改无线网络基本信息  
     4、连接隐藏无线网络  
     连接无安全性的无线网络  
     连接WPA及WPA2个人安全性的无线网络  
     连接WPA及WPA2企业安全性的无线网络  
     连接WPA3个人安全性的无线网络  
     5、窗口样式调节  
     深色与浅色窗口样式切换  
     调节网络工具窗口透明度  
     普通窗口与具有毛玻璃效果的窗口的切换  
### 运行
  灵墨网络工具的进程为lingmo-nm，默认开机自启，依赖NetworkManger  
### 命令行和dbus接口

  lingmo-nm进程的命令行如下：  
  Usage: lingmo-nm  

#### dbus接口：  
  DBUS类型：SESSION BUS  
  DBUS名称：com.lingmo.network   
  OBJECT路径：/  
  接口名称：com.lingmo.network  

#### dbus方法  
  getWirelessList  
    参数：无  
    返回值：QMap<QString, QVector<QStringList>>  
    键:网卡名称   
    值:无线网络的集合(第一项为已连接网络信息，若无连接则为"--"，否则为"名称/信号强度/加密类型/Uuid/是否为本机开放热点";其余为未连接的网络信息，为"名称/信号强度/加密类型/是否为本机开放热点"  
    功能：获取无线列表及每个对应无线网络的信号强度、加密类型等信息。  

  getWiredList  
    参数：无  
    返回值：QMap<QString, QVector<QStringList>>  
    键:网卡名称   
    值:有线网络的集合("名称/UUID/对应DBUS路径"），若无已连接网络则第一项默认为"--"  
    功能：获取有线列表及每个对应的有线网络的uuid及dbus路径  

  setWiredSwitchEnable(bool enable)  
    参数：(bool）开启有线总开关（true）关闭有线总开关（false）  
    返回值：无  
    功能：打开或关闭有线总开关  

  setWirelessSwitchEnable(bool enable)  
    参数：(bool）开启无线总开关（true）关闭无线总开关（false）  
    返回值：无  
    功能：打开或关闭无线总开关  

  setDeviceEnable(QString devName, bool enable)  
    参数：（QString）devName 设备名称，（bool） enable 开启/关闭  
    返回值：无  
    功能：打开或关闭单个有线网卡开关  

  activateConnect（int type, QString devName, QString ssid）  
    参数：根据网卡类型 参数1 0:lan 1:wlan 参数2：网卡名称 参数3：uuid/ssid  
    返回值：无  
    功能：激活一个网络连接  

  deActivateConnect(int type, QString devName, QString ssid)  
    参数：根据网卡类型 参数1 0:lan 1:wlan 参数2：网卡名称 参数3：uuid/ssid  
    返回值：无  
    功能：断开一个网络连接  

  getDeviceListAndEnabled(int devType)  
    参数：int devType 0:lan 1:wlan  
    返回值：QMap<QString, bool>  
      键：设备名称  
      值：bool 开启（true）关闭 （false）  
    功能：获取设备列表和启用/禁用状态  

  getWirelessDeviceCap  
    返回值：QMap<QString, int>  
      键：设备名称  
      值：int 0：不支持热点 3：支持2.4GHz 7：支持5GHz&&2.4Ghz  
    功能：获取无线设备能力(2.4G/5G)  

  showPropertyWidget(QString devName, QString ssid)  
    参数：QString devName 设备名称 QString ssid 根据网卡类型 有线为uuid/无线为ssid  
    返回值：无  
    功能：唤起对应网络连接的属性页，可以对基础信息的显示或修改  

  showCreateWiredConnectWidget(QString devName)  
    参数：QString devName 有线网卡名称  
    返回值：无  
    功能：唤起新建有线连接界面  

  activeWirelessAp(const QString apName,
 		   const QString apPassword, 
                   const QString band, 
                   const QString apDevice)  
    参数：QString apName 热点名称  
          QString apPassword 热点密码  
          QString band 频带  
          QString apDevice  
    返回值：无  
    功能：开启移动热点  

  deactiveWirelessAp(const QString apName, const QString uuid)  
    参数：QStringList（名称/密码/设备名称/状态/UUID/频带）  
    返回值：无  
    功能： 断开移动热点  

  getStoredApInfo  
    参数：QStringList（名称/密码/设备名称/状态/UUID/频带）  
    返回值：无  
    功能：获取已保存的移动热点信息  
    
  getApInfoBySsid(QString devName, QString ssid)  
    参数：QString devName 设备名称 QString ssid  
    返回值：无  
    功能：通过名称获取已保存的移动热点信息  
   
  reScan()  
    参数：无  
    返回值：无  
    功能：申请重新进行无线扫描  
   
  keyRingInit()  
    参数：QString apName 热点名称 QString uuid  
    返回值：无  
    功能：断开移动热点  
   
  keyRingClear()  
    参数：无  
    返回值：无  
    功能： 断开移动热点  


### 原理与主要使用的技术
  lingmo-nm主要与NetworkManager进行交互，通过对应的dbus信号驱动UI界面的更新。并通过提供的dbus方法来进行网络相关功能的调用。  
                           
  灵墨网络工具按照功能划分成两层：UI层和逻辑层。UI层实现网络资源的显示，以及用户的交互，逻辑层通过NetworkManager提供的Dbus接口，完成网络资源的管理及其配置。  
### 配置文件
  lingmo-nm的配置文件保存在如下路径：  
    ~/.config/lingmo/lingmo-nm.conf				        有线无线开关状态 设备状态  
    org.lingmo.lingmo-nm.switch						对应的gsetting值 （有线无线开关状态）  
### 编译
  cd lingmo-nm  
  mkdir build  
  cd build  
  qmake ..  
  make  
  sudo make install   
### 运行命令
  ./lingmo-nm  
### 调试
  lingmo-nm目前并采用lingmo-log4qt模块的日志功能。日志默认保存在~/.log/lingmo-nm.log中  







    












