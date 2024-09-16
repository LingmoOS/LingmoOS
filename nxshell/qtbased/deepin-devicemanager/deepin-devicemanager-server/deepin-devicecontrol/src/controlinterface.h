// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later



#ifndef CONTROLINTERFACE_H
#define CONTROLINTERFACE_H

#include "commonfunction.h"

#include <qdbusservice.h>
#include <QObject>
#include <QDBusContext>

class DriverManager;
class ModCore;
class ControlInterface : public QDBusService, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.DeviceControl")
public:
    explicit ControlInterface(QObject *parent = nullptr);

private:
    void initConnects();
    bool getUserAuthorPasswd(void);

signals:
    Q_SCRIPTABLE void sigUpdate();
#ifndef DISABLE_DRIVER
    Q_SCRIPTABLE void sigProgressDetail(int progress, const QString &strDeatils);
    Q_SCRIPTABLE void sigFinished(bool bsuccess, QString msg);
    Q_SCRIPTABLE void sigDownloadProgressChanged(QStringList msg);//驱动下载进度、速度、已下载大小
    Q_SCRIPTABLE void sigDownloadFinished();//下载完成
    Q_SCRIPTABLE void sigInstallProgressChanged(int progress);//安装进度
    Q_SCRIPTABLE void sigInstallProgressFinished(bool bsuccess, int err);
    Q_SCRIPTABLE void sigBackupProgressFinished(bool bsuccess);
#endif

public slots:
    /**
     * @brief getRemoveInfo
     * @return
     */
    Q_SCRIPTABLE QString getRemoveInfo();
    /**
     * @brief getAuthorizedInfo 获取被禁用的设备信息
     * @return
     */
    Q_SCRIPTABLE QString getAuthorizedInfo();
    /**
     * @brief enable 启用禁用设备
     * @param hclass 类型
     * @param name 名称
     * @param path 授权文件的路径
     * @param value 传过来的数值
     * @param enable_device 启用或者禁用
     * @param strDriver
     * @return 是否成功
     */
    Q_SCRIPTABLE bool enable(const QString &hclass, const QString &name, const QString &path, const QString &value, bool enable_device, const QString strDriver = "");
    /**
     * @brief enablePrinter 禁用打印机
     * @param hclass 设备类型
     * @param name 打印机名称
     * @param path -----
     * @param enable_device 是否被启用
     * @return
     */
    Q_SCRIPTABLE bool enablePrinter(const QString &hclass, const QString &name, const QString &path, bool enable_device);
    /**
     * @brief disableOutDevice 禁用设备
     * @param devInfo 设备信息
     * @return
     */
    Q_SCRIPTABLE void disableOutDevice(const QString &devInfo);
    /**
     * @brief disableOutDevice 禁用设备
     * @return
     */
    Q_SCRIPTABLE void disableInDevice();
    /**
     * @brief isDeviceEnabled 判断设备是否被禁用，通过查询数据库来判断
     * @param unique_id 设备的唯一 sid
     * @return
     */
    Q_SCRIPTABLE bool isDeviceEnabled(const QString &unique_id);
    /**
     * @brief setWakeupMachine
     * @param unique_id
     * @param path
     * @param wakeup
     * @return
     */
    Q_SCRIPTABLE bool setWakeupMachine(const QString &unique_id, const QString &path, bool wakeup);
    /**
     * @brief setNetworkWake 设置网卡唤醒
     * @param logicalName 网卡的逻辑名称
     * @param wakeup 是否允许唤醒
     * @return
     */
    Q_SCRIPTABLE bool setNetworkWake(const QString &logicalName, bool wakeup);
    /**
     * @brief updateWakeup 更新唤醒
     * @param devInfo 设备信息
     * @return
     */
    Q_SCRIPTABLE void updateWakeup(const QString &devInfo);
    /**
     * @brief isNetworkWakeup 判断网卡是否禁用
     * @param logicalName 网卡的逻辑名称
     * @return
     */
    Q_SCRIPTABLE int isNetworkWakeup(const QString &logicalName);
    /**
     * @brief seMonitorWorkingFlag 在数据库里设置设备是否监控
     * @param flag 是否监控标志
     * @return
     */
    Q_SCRIPTABLE void setMonitorWorkingDBFlag(bool flag);
    /**
     * @brief monitorWorkingBDFlag 设备是否监控
     * @return 是否监控标志
     */
    Q_SCRIPTABLE bool monitorWorkingDBFlag();

#ifndef DISABLE_DRIVER
    Q_SCRIPTABLE bool unInstallDriver(const QString &modulename);
    Q_SCRIPTABLE bool installDriver(const QString &filepath);
    Q_SCRIPTABLE void installDriver(const QString &modulename, const QString &version);
    Q_SCRIPTABLE void undoInstallDriver();
    Q_SCRIPTABLE QStringList checkModuleInUsed(const QString &modulename);
    Q_SCRIPTABLE bool isDriverPackage(const QString &filepath);
    Q_SCRIPTABLE bool isBlackListed(const QString &modName);
    Q_SCRIPTABLE bool isArchMatched(const QString &filePath);
    Q_SCRIPTABLE bool isDebValid(const QString &filePath);
    Q_SCRIPTABLE bool unInstallPrinter(const QString &vendor, const QString &model);
    Q_SCRIPTABLE bool backupDeb(const QString &debpath); //debpath格式须是： “/tmp/xx/debname
    Q_SCRIPTABLE bool delDeb(const QString &debname);
    Q_SCRIPTABLE bool aptUpdate();

#endif

private:
    /**
     * @brief authorizedEnable 通过authorized文件启用禁用设备
     * @param hclass 设备类型
     * @param name 设备名称
     * @param path 设备节点路径
     * @param unique_id 设备的唯一标识
     * @param enable_device 启用或者禁用
     * @param strDriver 驱动名称
     * @return
     */
    bool authorizedEnable(const QString &hclass, const QString &name, const QString &path, const QString &unique_id, bool enable_device, const QString strDriver = "");
    /**
     * @brief removeEnable 通过remove文件启用禁用设备
     * @param hclass 设备类型
     * @param name 设备名称
     * @param path 设备节点路径
     * @param unique_id 设备的唯一标识
     * @param enable 启用或者禁用
     * @param strDriver 驱动名称
     * @return
     */
    bool removeEnable(const QString &hclass, const QString &name, const QString &path, const QString &unique_id, bool enable, const QString strDriver = "");
    /**
     * @brief ioctlEnableNetwork 通过ioctl启用禁用网卡
     * @param hclass 设备类型
     * @param name 设备名称
     * @param logical_name 逻辑名称
     * @param unique_id 唯一标识
     * @param enable 启用或者禁用
     * @param strDriver 驱动名称
     * @return 返回是否成功
     */
    bool ioctlEnableNetwork(const QString &hclass, const QString &name, const QString &logical_name, const QString &unique_id, bool enable, const QString strDriver = "");
    /**
     * @brief construct_uri
     * @param buffer
     * @param buflen
     * @param base
     * @param value
     */
    void construct_uri(char *buffer, size_t buflen, const char *base, const char *value);
    /**
     * @brief modifyPath 修正路径 保证xxx.0
     *                比如将  /devices/pci0000:00/0000:00:14.0/usb1/1-8/1-8:1.1
     *                修正为  /devices/pci0000:00/0000:00:14.0/usb1/1-8/1-8:1.0
     * @param path 需要修正的路径 /devices/pci0000:00/0000:00:14.0/usb1/1-8/1-8:1.0
     */
    void modifyPath(QString &path);
    /**
     * @brief saveWakeupInfo 保存数据到数据库
     * @param unique_id 唯一标识
     * @param path 节点路径
     * @param wakeup 是否唤醒
     */
    void saveWakeupInfo(const QString &unique_id, const QString &path, bool wakeup);

private:
#ifndef DISABLE_DRIVER
    DriverManager *mp_drivermanager;
    ModCore *pcore = nullptr; //临时测试使用
#endif
};

#endif // CONTROLINTERFACE_H
