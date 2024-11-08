/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
 * Authors: Yang Ling <yangling@kylinos.cn>
 *
 */

#ifndef VOLUMEMANAGER_H
#define VOLUMEMANAGER_H

#include <QList>
#include <memory>
#include <QObject>
#include <QString>
#include <QHash>
#include <QMutex>

#undef slots
#undef signals
#undef emit

#include <gio/gio.h>
#include <gio/gunixmounts.h>


class GetOccupiedAppsInfoThread;
namespace Experimental_Peony{

class Volume;
class Mount;
class Drive;

class Q_DECL_EXPORT VolumeManager : public QObject
{
    Q_OBJECT
public:
    static VolumeManager* getInstance();
    ~VolumeManager();
    /*静态的获取当前所有有效的设备分区，函数名后期再做更换*/
    QList<Volume>*   allVaildVolumes();
    void printVolumeList();

    QString getTargetUriFromUnixDevice(const QString &unixDevice);/* 根据device获取volume的uri */

    GetOccupiedAppsInfoThread* getOccupiedInfoThread(){
        return m_occupiedAppsInfoThread;
    }

    Volume* getOccupiedVolume(){
        return m_occupiedVolume;
    }
    QMutex* getMutex(){
        return &m_mutex;
    };
    bool isEmptyDrive(const Volume &volume);/* 判断是否为空光驱 */


private:
    explicit VolumeManager(QObject *parent = nullptr);
    bool gpartedIsOpening();


    void initManagerInfo();
    //void initMonitorInfo();         //监控卷分区
    QList<Mount*>   allMounts();
    QList<GMount*>  allGMounts();
    QList<Volume*>  allVolumes();
    QList<GVolume*> allGVolumes();   
    QList<GDrive*>  allGDrives();    /* 获取连接的GDrive集合 */
    QList<Drive*>   allDrives();     /* 将GDrive转为Drive */

    //QString guessContentType(GMount*);
    static void volumeAddCallback(GVolumeMonitor*,GVolume*,VolumeManager*);
    static void volumeRemoveCallback(GVolumeMonitor*,GVolume*,VolumeManager*);
    static void mountAddCallback(GVolumeMonitor*,GMount*,VolumeManager*);
    static void mountRemoveCallback(GVolumeMonitor*,GMount*,VolumeManager*);
    static void driveConnectCallback(GVolumeMonitor*,GDrive*,VolumeManager*);
    static void driveDisconnectCallback(GVolumeMonitor*,GDrive*,VolumeManager*);
    static void driveChangedCallback(GVolumeMonitor*,GDrive*,VolumeManager*);
    static void volumeChangeCallback(GVolumeMonitor*,GVolume*,VolumeManager*);
    static void mountChangedCallback(GMount *mount, VolumeManager *pThis);
    static void mountPreUnmountCallback(GVolumeMonitor*, GMount*,VolumeManager*);

private:
    GVolumeMonitor* m_volumeMonitor = nullptr;
    quint64 m_volumeAddHandle;
    quint64 m_volumeRemoveHandle;
    quint64 m_volumeChangeHandle;
    quint64 m_mountAddHandle;
    quint64 m_mountRemoveHandle;
    quint64 m_driveConnectHandle;
    quint64 m_driveDisconnectHandle;
    quint64 m_driveChangedHandle;
    quint64 m_mountOpreationHandle;
    quint64 m_mountPreUnmountHandle;
    bool m_gpartedIsOpening = false;
    QHash<QString,Volume*>* m_volumeList = nullptr;
    GetOccupiedAppsInfoThread* m_occupiedAppsInfoThread = nullptr;
    Volume *m_occupiedVolume = nullptr; /* 被占用的volume，当前用于只有ffmpeg占用时强制弹出,link to bug#117263 */
    QMutex m_mutex;

    //我应该在检测到信号时更新卷设备列表？还是在用到时重新全部get一次？感觉前者好点?
Q_SIGNALS:
    void volumeUpdate(const Volume&,QString);        //gparted打开与关闭时可能会伴随设备信息的更新
    void volumeAdd(const Volume&);
    //volumeRemove()对应的槽内需要匹配device或者mountpoint
    void volumeRemove(const QString&);//1、设备被真正移除 2、gparetd打开时卸载设备
    //void volumeUnmount(const QString& device);
    void mountAdd(const Volume&);     //重设挂载点信息
    void mountRemove(const QString& device);
    void signal_unmountFinished(const QString &uri);/* 卸载完成信号 */
    void signal_mountFinished();/* 挂载完成信号，目前用于侧边栏设备挂载后路径跳转 */
    void signal_encryptedVolumeMountFinished(const QString &uri);
};

class Q_DECL_EXPORT Drive{
public:
    //property
    QString name() const;
    QString icon() const;
    QString device() const;
    bool canEject() const;
    bool canStop() const;
    //bool unmountAble();
    GDrive* getGDrive() const;
    Drive(GDrive* gdrive);
    ~Drive();
    //method
    void eject(GMountUnmountFlags ejectFlag);
    void setMountPath(const QString& mountPath);

private:
    GDrive* m_drive = nullptr;
    bool m_canEject = false ;
    bool m_canStop = false;
    QString m_name;
    QString m_icon;
    QString m_device;
    QString m_mountPath;

private:
    void initDriveInfo();
};

class Q_DECL_EXPORT Mount{
public:
    QString name() const;
    QString uuid() const;
    QString icon() const;
    bool canEject() const;
    bool canStop() const;
    bool canUnmount() const;
    QString device() const;
    QString mountPoint() const;
    GMount* getGMount() const;
    Mount(GMount* gmount);
    ~Mount();
    void unmount();
private:
    bool m_canEject = false ;
    bool m_canStop = false ;
    bool m_canUnmount = false ;
    GMount* m_mount = nullptr;
    QString m_name;
    QString m_uuid;
    QString m_icon;
    QString m_device;
    QString m_mountPoint;    
    GUnixMountEntry * m_entry = nullptr;
private:
    void initMountInfo();
    void queryDeviceByMountpoint();
};

class Q_DECL_EXPORT Volume{
public:
    //property-to-get
    QString name() const;
    QString icon() const;
    QString uuid() const;
    QString device() const;
    QString originalDevice() const;
    QString mountPoint() const;
    GVolume* getGVolume() const;
    GDrive* getGDrive() const;
    GMount* getGMount() const;
    //property-to-set
    void setLabel(const QString& label);
    void setDevice(const QString &device);
    void setIconName(const QString &iconName);
    void setFromMount(const Mount& mount);//通过Mount求Volume
    void setFromDrive(const Drive& drive);//通过Drive获取Volume
    void setMountPoint(QString point);
    QString getMountPoint();
    Volume* initRootVolume();   //根分区
    Volume(GVolume* gvolume);
    Volume(const Volume& other);//深拷贝
    ~Volume();
    bool operator==(const Volume& other) const;
    bool operator==(const Volume* other) const;
    //bool operator==(const QString& deivce) const;
    //method
    bool canEject() const;
    bool canStop() const;
    bool canUnmount() const;
    bool canMount() const;
    void eject(GMountUnmountFlags ejectFlag);
    void unmount();
    void mount();
    void format() const;
    bool getHidden() const;
    void setHidden(bool hidden);

private:
    bool m_canEject = false ;
    bool m_canStop = false;
    bool m_canUnmount = false ;
    bool m_canMount = false;
    GVolume* m_volume = nullptr;
    GMount* m_gMount =nullptr;
    GDrive* m_gdrive = nullptr;
    QString  m_name;
    QString  m_uuid;
    QString  m_icon;
    QString  m_originalDevice;
    QString  m_device;
    QString  m_mountPoint;

    bool m_hidden = false;
private:
    void initVolumeInfo();
};
}


#include <QIcon>
#include <QDialog>
#include <QWidget>
class MessageDialog : public QDialog{
    Q_OBJECT
public:
    explicit MessageDialog(QWidget *parent = nullptr);
    ~MessageDialog(){}

    void init(std::map<QString,QIcon>& occupiedAppMap, const QString& message);

};

#include <QThread>
class GetOccupiedAppsInfoThread:public QThread
{
    Q_OBJECT
public:
    explicit GetOccupiedAppsInfoThread(QObject *parent = nullptr);
    void run() override;

    static void show_processes_cb(GMountOperation *mountOp, char *message, GArray *processes, char **choices, gpointer user_data);
    GMountOperation *getMountOp() const;

Q_SIGNALS:
    void signal_occupiedAppInfo(std::map<QString,QIcon>& occupiedAppMap, const QString& message);

private:
    GMainLoop *loop = nullptr;
    GMountOperation *m_mountOp = nullptr;
};


#ifdef LINGMO_UDF_BURN
#include "explorer-core_global.h"
namespace UdfBurn {
class UdfFormatDialog;
class DiscControl;
class PEONYCORESHARED_EXPORT UdfFormatDialogWrapper : public QObject{
    Q_OBJECT
public:
    explicit UdfFormatDialogWrapper(const QString &uri, DiscControl *discControl, QWidget *parent = nullptr);
    ~UdfFormatDialogWrapper();

    void raise();
    void show();

private:
    UdfFormatDialog * m_dialog = nullptr;
};
}

#endif

#endif // VOLUMEMANAGER_H
