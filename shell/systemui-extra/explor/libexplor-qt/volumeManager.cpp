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

#include "volumeManager.h"
#include <QDebug>
#include <QTimer>
#include <QThread>
#include<QMessageBox>
#include<QProcess>
#include <QInputDialog>
#include <QTimer>
#include"sync-thread.h"
#include "file-utils.h"

#include <udisks/udisks.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <glib.h>

using namespace Experimental_Peony;
static VolumeManager* m_globalManager = nullptr;

static void askPasswdCallback(GMountOperation  *op,
                              gchar            *message,
                              gchar            *default_user,
                              gchar            *default_domain,
                              GAskPasswordFlags flags,
                              gpointer          user_data) {
    //Q_UNUSED (message)
    Q_UNUSED (default_user)
    Q_UNUSED (default_domain)
    if (flags & G_ASK_PASSWORD_NEED_PASSWORD) {
        if (g_mount_operation_get_password(op)) {
            g_mount_operation_reply(op, G_MOUNT_OPERATION_HANDLED);
            return;
        } else {
            auto object = G_OBJECT (user_data);
            g_object_set_data_full(object, "message", gpointer(g_strdup(message)), g_free);
            g_object_set_data(object, "need-password", gpointer(true));
        }
    }
    g_mount_operation_reply(op, G_MOUNT_OPERATION_UNHANDLED);
}

QString getDeviceUUID(const char *device) {
    struct stat statbuf;
    if (stat (device, &statbuf) != 0)
    {
        return nullptr;
    }

    g_autoptr(UDisksClient) client = udisks_client_new_sync (NULL,NULL);
    g_autoptr(UDisksBlock) block = udisks_client_get_block_for_dev(client, statbuf.st_rdev);
    if (!block)
        return nullptr;

    const gchar *uuid = udisks_block_get_id_uuid(block);
    return uuid;
}

static bool driveHasMedia(GDrive *gdrive){
    /* 此方式替代g_drive_has_media只对evice.startsWith("/dev/sd")生效！Linkto Bug#205118插入拓展坞后文件管理器侧边栏多出了两个移动设备 */
    g_autofree gchar* unix_device = g_drive_get_identifier(gdrive, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
    QString device = unix_device;
    int fd = open(device.toStdString().c_str(), O_RDONLY | O_NONBLOCK);
    if(-1==fd){/* open失败 */
        return false;
    }
    close(fd);
    return true;
}


void VolumeManager::printVolumeList(){
    qDebug()<<endl<<endl<<endl;
    //static int count=0;
    QHash<QString,Volume*>::const_iterator item = m_volumeList->begin();
    QHash<QString,Volume*>::const_iterator end = m_volumeList->end();
    for(; item!=end; ++item){
        Volume* volume = item.value();
        QString name = volume->name();
        QString uuid = volume->uuid();
        bool canEject = volume->canEject();
        QString device = volume->device();
        QString mountPoint = volume->mountPoint();
        qDebug()<<device<<" "<<name<<" "<<canEject<<" "<<uuid<<" "<<mountPoint;
    }
    //++count;

    //沿着VolumeManager的析构
//    if(10==count){
//        qDebug()<<"10 times................."<<endl;
//        delete m_globalManager;
//    }

}
#include "file-enumerator.h"
#include "file-info.h"
QString VolumeManager::getTargetUriFromUnixDevice(const QString &unixDevice){
    /* volume item,遍历方式获取uri */
    Peony::FileEnumerator e;
    e.setEnumerateDirectory("computer:///");
    e.enumerateSync();
    QString uri;
    for (auto fileInfo : e.getChildren()) {
        QString uriStr = fileInfo.get()->uri();
        g_autoptr(GFile) gFile = g_file_new_for_uri(uriStr.toUtf8().constData());
        GError *err = nullptr;
        QString device;
        QString targetUri;
        g_autoptr(GFileInfo) gFileInfo = g_file_query_info(gFile,
                                                           "standard::*," "time::*," "access::*," "mountable::*," "metadata::*," "trash::*," G_FILE_ATTRIBUTE_ID_FILE,
                                                           G_FILE_QUERY_INFO_NONE,
                                                           g_cancellable_new(),
                                                           &err);

        if (err) {
            qDebug()<<err->code<<err->message;
            g_error_free(err);
            continue;
        }else{
            if (g_file_info_has_attribute(gFileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI)) {
               targetUri = g_file_info_get_attribute_string(gFileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
            }
            if(g_file_info_has_attribute(gFileInfo,G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE_FILE)){
                device = g_file_info_get_attribute_string(gFileInfo,G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE_FILE);
            }
        }

        /* 由volume的unixDevice获取target uri */
        if(device==unixDevice){
            uri = targetUri;
            break;
        }
    }
    return Peony::FileUtils::urlDecode(uri);
}

bool VolumeManager::isEmptyDrive(const Volume &volume)
{
    g_autoptr (GDrive) gdrive = volume.getGDrive();
    if (gdrive && !g_drive_has_media(gdrive)) {/* 空光驱 */
        return  true;
    }
    return false;
}

static void forceUnmountOfOccupiedDeviceCb(GMount* gmount, GAsyncResult* result, VolumeManager *pThis) {

    GError *err = nullptr;
    g_mount_unmount_with_operation_finish(gmount, result, &err);
    if (err) {
        QMessageBox::warning(nullptr, QObject::tr("Force unmount failed"), QObject::tr("Error: %1\n").arg(err->message));
        g_error_free(err);
    } else {        
        QMutexLocker lk(pThis->getMutex());
        if(pThis->getOccupiedVolume() && (pThis->getOccupiedVolume()->canEject() || pThis->getOccupiedVolume()->canStop())){
            qDebug()<<"Force eject Operation:"<<pThis->getOccupiedVolume()->device()<<pThis->getOccupiedVolume();
            pThis->getOccupiedVolume()->eject(G_MOUNT_UNMOUNT_FORCE);
        }

    }
}

VolumeManager::VolumeManager(QObject *parent) : QObject(parent)
{
    initManagerInfo();
//    QTimer* timer = new QTimer();
//    connect(timer,&QTimer::timeout,this,&VolumeManager::printVolumeList);
//    timer->start(10*1000);

    /* 显示占用app信息对话框 */
    qRegisterMetaType<std::map<QString,QIcon> >("std::map<QString,QIcon>&");
    m_occupiedAppsInfoThread = new GetOccupiedAppsInfoThread();
    connect(m_occupiedAppsInfoThread, &GetOccupiedAppsInfoThread::signal_occupiedAppInfo, this, [=](std::map<QString,QIcon>& occupiedAppMap, const QString& message){
        if(1 == occupiedAppMap.size() && occupiedAppMap.count("ffmpeg")){/* 过滤应用，只有ffmpeg占用时可以强制弹出 */
            {
                QMutexLocker lk(&m_mutex);
                if(!m_occupiedVolume)
                    return;

                qDebug()<<"Force unmount Operation: "<<m_occupiedVolume->device();
                g_mount_unmount_with_operation(m_occupiedVolume->getGMount(), G_MOUNT_UNMOUNT_FORCE, nullptr, nullptr,
                                               GAsyncReadyCallback(forceUnmountOfOccupiedDeviceCb), this);

                return;
            }
        }

        if(0 == occupiedAppMap.size()){
            QTimer::singleShot(500,[=](){
                QMutexLocker lk(&m_mutex);
                m_occupiedVolume->eject(G_MOUNT_UNMOUNT_NONE);
            });
            //QMessageBox::critical(nullptr, QObject::tr("Eject failed"), message);
        }else{
            MessageDialog* dlg = new MessageDialog();
            dlg->init(occupiedAppMap, message);
            dlg->setAttribute(Qt::WA_DeleteOnClose);
            dlg->exec();
        }
    }, Qt::QueuedConnection);
    m_occupiedAppsInfoThread->start();
}

VolumeManager::~VolumeManager(){
    if(m_volumeMonitor){
        g_signal_handler_disconnect(m_volumeMonitor, m_mountAddHandle);
        g_signal_handler_disconnect(m_volumeMonitor, m_volumeAddHandle);
        g_signal_handler_disconnect(m_volumeMonitor, m_mountRemoveHandle);
        g_signal_handler_disconnect(m_volumeMonitor, m_volumeChangeHandle);
        g_signal_handler_disconnect(m_volumeMonitor, m_volumeRemoveHandle);
        g_signal_handler_disconnect(m_volumeMonitor, m_driveConnectHandle);
        g_signal_handler_disconnect(m_volumeMonitor, m_driveDisconnectHandle);
        g_signal_handler_disconnect(m_volumeMonitor, m_driveChangedHandle);
        g_signal_handler_disconnect(m_volumeMonitor, m_mountPreUnmountHandle);
        g_object_unref(m_volumeMonitor);
        m_volumeMonitor = nullptr;
    }

    if(!m_volumeList->isEmpty()){
        QHash<QString,Volume*>::iterator item = m_volumeList->begin();
        QHash<QString,Volume*>::iterator end = m_volumeList->end();
        for(; item!=end; ++item){
            Volume* volumeItem = item.value();
            delete volumeItem;
        }
        m_volumeList->clear();
        delete m_volumeList;
    }
    {
        QMutexLocker lk(&m_mutex);
        if(m_occupiedVolume){
            delete m_occupiedVolume;
            m_occupiedVolume = nullptr;
        }
    }
}

void VolumeManager::initManagerInfo(){
    m_volumeMonitor = g_volume_monitor_get();
    if(!m_volumeMonitor)
        return;

    m_mountAddHandle = g_signal_connect(m_volumeMonitor,"mount-added",G_CALLBACK(mountAddCallback),this);
    m_volumeAddHandle = g_signal_connect(m_volumeMonitor,"volume-added",G_CALLBACK(volumeAddCallback),this);
    m_mountRemoveHandle = g_signal_connect(m_volumeMonitor,"mount-removed",G_CALLBACK(mountRemoveCallback),this);
    m_volumeRemoveHandle = g_signal_connect(m_volumeMonitor,"volume-removed",G_CALLBACK(volumeRemoveCallback),this);
    m_volumeChangeHandle = g_signal_connect(m_volumeMonitor,"volume-changed",G_CALLBACK(volumeChangeCallback),this);
    m_driveConnectHandle = g_signal_connect(m_volumeMonitor,"drive-connected",G_CALLBACK(driveConnectCallback),this);
    m_driveDisconnectHandle = g_signal_connect(m_volumeMonitor,"drive-disconnected",G_CALLBACK(driveDisconnectCallback),this);
    m_driveChangedHandle = g_signal_connect(m_volumeMonitor,"drive-changed",G_CALLBACK(driveChangedCallback),this);
    m_mountPreUnmountHandle = g_signal_connect(m_volumeMonitor,"mount-pre-unmount",G_CALLBACK(mountPreUnmountCallback),this);
}

/*gparted应用是否打开*/
bool VolumeManager::gpartedIsOpening(){
    GList* volumes = nullptr;
    Q_UNUSED(volumes)
    GList* drives = nullptr;
    GList* l;
    GDrive* drive;
    if(m_volumeMonitor){
        //volumes = g_volume_monitor_get_volumes(m_volumeMonitor);
        drives = g_volume_monitor_get_connected_drives(m_volumeMonitor);
    }

   // m_gpartedIsOpening = true;  /* hotfix bug#164497 【文件管理器】左侧目录，手动弹出空光盘时数据盘、光驱消失 */

    for(l = drives; l!=nullptr; l=l->next){
        drive = (GDrive*) l->data;
        if(g_drive_has_volumes(drive)){
            m_gpartedIsOpening = false;
            break;
        }
    }

//    if(nullptr != volumes){
//        m_gpartedIsOpening = false;
//        g_list_free_full(volumes,g_object_unref);
//    }
    if(nullptr != drives){
        g_list_free_full(drives,g_object_unref);
    }

    return m_gpartedIsOpening;
}
/*使用volume-changed信号处理设备的name属性更新*/
void VolumeManager::volumeChangeCallback(GVolumeMonitor *monitor,
        GVolume *gvolume,VolumeManager *pThis){
    Q_UNUSED(monitor)

    if(!pThis->m_volumeList)
        return;
    QString device,name;
    char *gdevice,*gname;

    QHash<QString,Volume*>::iterator findItem,end;
    Q_UNUSED(findItem)
    Q_UNUSED(end)
    //情景：使用其他工具修改卷标后，卷标需要更新
    gdevice = g_volume_get_identifier(gvolume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
    gname = g_volume_get_name(gvolume);
    device = gdevice;
    name =gname;
    g_free(gdevice);
    g_free(gname);

//    findItem = pThis->m_volumeList->find(device);
//    end = pThis->m_volumeList->end();
//    if(findItem != end && name != findItem.value()->name()){
//        findItem.value()->setLabel(name);
//        Q_EMIT pThis->volumeUpdate(Volume(*findItem.value()),"name");//更新name属性
//    }
    // note: volume device file might be changed while mounting/unmounting an encrypted volume.
    // use gvolume for quering volume item is more reliable for now.
    for (auto volumeItem : pThis->m_volumeList->values()) {
        if (volumeItem->getGVolume() == gvolume) {
            qDebug()<<__func__<<__LINE__<<volumeItem->device()<<name<<device;
            volumeItem->setLabel(name);
            volumeItem->setDevice(device);
            Q_EMIT pThis->volumeUpdate(Volume(*volumeItem),"name");
        }
    }
}

void VolumeManager::volumeAddCallback(GVolumeMonitor *monitor,
        GVolume *gvolume,VolumeManager *pThis){
    Q_UNUSED(monitor)
    if(!pThis->m_volumeList)
        return;
    bool itemIsExisted = false;
    int volumeCount = pThis->m_volumeList->count();
    Q_UNUSED(volumeCount)
    Volume *addItem = new Volume(gvolume);

    itemIsExisted = pThis->m_volumeList->contains(addItem->device());
    qDebug()<<__func__<<__LINE__<<addItem->device()<<itemIsExisted<<endl;
    if(itemIsExisted){
        /* 先删除后添加，更新volume,例如异常U盘格式化 */
        QString device = addItem->device();
        pThis->m_volumeList->remove(device);
        Q_EMIT pThis->volumeRemove(device);
        pThis->m_volumeList->remove(device);
        pThis->m_volumeList->insert(device, addItem);
        qDebug()<<__func__<<__LINE__<<device;
        Q_EMIT pThis->volumeAdd(Volume(*addItem));
        //情景1、关闭gparted时，所有具有卸载属性的设备均会触发volume-added信号
        //      该情景似乎不需要更新属性信息，确认一下name属性？
    } else {
        // 判断volume的drive是否存在与列表中，如果是就要将其drive隐藏
        GDrive *gdrive = g_volume_get_drive(gvolume);
        if (gdrive) {
            g_autofree char* gdevice = g_drive_get_identifier(gdrive, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
            if (pThis->m_volumeList->contains(gdevice)) {
                auto driveItem = pThis->m_volumeList->value(gdevice);
                driveItem->setHidden(true);
                /* hotfix bug#161064 【HWE】【文件管理器】插入多个U盘，偶现文管侧边栏1个U盘显示两个名称现象，其中一个名称点击后提示U盘异常 */
                Q_EMIT pThis->volumeRemove(gdevice);
                qDebug()<<__func__<<__LINE__<<driveItem->device()<<driveItem->getHidden();
                Q_EMIT pThis->volumeAdd(Volume(*driveItem));
            }
            g_object_unref(gdrive);
        }

        //情景1、未打开gparted时插入新设备
        //情景2、已打开gparted->插入新设备不拔出->关闭gparted后触发volume-added信号
        //情景3、默认用数据线连接的手机("仅充电")
        pThis->m_volumeList->remove(addItem->device());
        pThis->m_volumeList->insert(addItem->device(),addItem);
        qDebug()<<__func__<<__LINE__<<addItem->device();
        Q_EMIT pThis->volumeAdd(Volume(*addItem));
    }
}

void VolumeManager::volumeRemoveCallback(GVolumeMonitor *monitor,
        GVolume *gvolume,VolumeManager *pThis){
    Q_UNUSED(monitor)
    if(!pThis->m_volumeList)
        return;

    // note: volume device file might be changed while mounting/unmounting an encrypted volume.
    // use gvolume for quering volume item is more reliable for now.
    for (auto volumeItem : pThis->m_volumeList->values()) {
        if (volumeItem->getGVolume() == gvolume) {
            pThis->m_volumeList->remove(volumeItem->originalDevice());
            Q_EMIT pThis->volumeRemove(volumeItem->originalDevice());
            delete volumeItem;
        }
    }

    GDrive *gdrive = g_volume_get_drive(gvolume);
    if (gdrive) {
        // 可能是光驱，弹出之后drive还在
        g_autofree char* gdevice = g_drive_get_identifier(gdrive, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        if (pThis->m_volumeList->contains(gdevice)) {
            // 如果没有volume，这个drive应该显示出来，参考volumeAddedCallback流程
            GList *volumes = g_drive_get_volumes(gdrive);
            if (volumes) {
                // noop
            } else {
                // 由于旧的volume信息不好更新，所以移除旧的volume，重新添加
                QString device = gdevice;
                auto addItem = new Volume(nullptr);
                auto drive = new Drive(gdrive);
                addItem->setFromDrive(*drive);
                if (device.startsWith("/dev/sr")) {
                    addItem->setHidden(false);
                }
                /* hotfix bug#154563 【文件管理器】【PTOF】格式化U盘后，文件管理器侧边栏出现两个U盘  */
                if (device.startsWith("/dev/sd")) {
                    QString uuid = getDeviceUUID(device.toUtf8().constData());
                    auto size = Peony::FileUtils::getDeviceSize(device.toUtf8().constData());
                    qDebug()<<__func__<<__LINE__<<uuid<<size;
                    if (uuid.isEmpty() && size == 0) {
                        addItem->setHidden(true);
                        // if drive has media, it is not represent a docking station.
                        // so it should not be hidden.
                        if (driveHasMedia(gdrive)) {
                            addItem->setHidden(false);
                        }
                    }
                }//end
                pThis->m_volumeList->remove(device);
                Q_EMIT pThis->volumeRemove(device);
                qDebug()<<__func__<<__LINE__<<device<<addItem->device();
                pThis->m_volumeList->insert(device, addItem);
                Q_EMIT pThis->volumeAdd(Volume(*addItem));
            }
        }
        g_object_unref(gdrive);
    } else {
        // drive已经disconnect，从volume进行索引
        g_autofree char *gdevice = g_volume_get_identifier(gvolume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        if (pThis->m_volumeList->contains(gdevice)) {
            auto volumeItem = pThis->m_volumeList->value(gdevice);
            Q_UNUSED(volumeItem)
            pThis->m_volumeList->remove(gdevice);
            Q_EMIT pThis->volumeRemove(gdevice);
        }
    }

    //情景1、未打开gparted时正常弹出设备
    //情景2、打开gparted瞬间所有可卸载的分区均会触发volume-removed信号
    //      这种情景下设备需要继续支持访问,不能够移除
    bool phoneFlag;    //手机(mtp、gphoto2)
    bool blankCDFlag;   //手机(仅充电)、非空光盘
    bool phoneOrCD;
    char* gdevice = g_volume_get_identifier(gvolume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
    QString device = gdevice;
    g_free(gdevice);

    //qDebug()<<__func__<<__LINE__<<device<<endl;
    if(!pThis->m_volumeList->contains(device)){
        //情景3、已经发生了暴力拔出的情况，设备已经不存在了，这里不用做处理了
        //情景4、数据线连接的手机状态改变："仅充电"->"传输文件(mtp)"或"传输图片(gphoto)"
        return;
    }

    g_autoptr (GMount) gmount = g_volume_get_mount(gvolume);//情景3时该处的值不为nullptr
    phoneFlag = device.contains("/dev/bus");
    blankCDFlag = device.contains("/dev/sr") && pThis->m_volumeList->value(device)->mountPoint().isEmpty();
    phoneOrCD = device.contains("/dev/bus") || device.contains("/dev/sr");
    Q_UNUSED(phoneOrCD)
    qDebug()<<__func__<<__LINE__<<device<<(gmount!=nullptr)<<endl;
    if(gmount){
        if(!phoneFlag && !blankCDFlag){
            //情景5、手机的mtp与gphoto2状态相互转换时只能保留一个
            return;
        }
        //情景2、处于挂载状态的设备在打开gparted瞬间需要保留该设备 更新设备的name属性?
        //情景8、空光盘弹出后空光驱应该被移除
        //Q_EMIT pThis->volumeUpdate();
    }
    qDebug()<<__func__<<__LINE__<<device<<(gmount!=nullptr)<<endl;

    if (blankCDFlag)
        return;

    //情景1、确定可以删除设备
    //情景5、数据线连接的手机状态改变： "传输文件(mtp)" 与 "传输图片(gphoto)" 相互转换(即/dev/bus/xxx设备只能保留一个)
    //情景6、数据线连接的手机状态改变："mtp"或"gphoto" -> "仅充电"
    //情景9、手机(mtp或gphoto2状态)暴力拔出
    pThis->m_volumeList->remove(device);

    Q_EMIT pThis->volumeRemove(device);
}

//收到mount-removed时使用挂载点属性更新设备的状态
void VolumeManager::mountRemoveCallback(GVolumeMonitor *monitor,
        GMount *gmount,VolumeManager *pThis){
    Q_UNUSED(monitor)
    if(!pThis->m_volumeList)
        return;
    //情景1、卸载或弹出操作（不区分有无gparted进程）
    //情景2、直接暴力拔出操作 (不区分有无gparted进程)
    //上述2种情境下GMount*只能获取到设备的挂载点，无法获取设备路径，因此尝试采用挂载点区分设备?
    Volume* volumeItem;
    Mount* mountItem = new Mount(gmount);
    QString mountPoint = mountItem->mountPoint();

    if(mountItem->mountPoint().startsWith("smb://")){/* 远程服务器特殊处理 */
        Q_EMIT pThis->mountRemove(mountPoint);
        delete mountItem;
        mountItem = nullptr;
        return;
    }

    QHash<QString,Volume*>::iterator item = pThis->m_volumeList->begin();
    QHash<QString,Volume*>::iterator end = pThis->m_volumeList->end();
    //qDebug()<<__func__<<__LINE__<<mountPoint<<endl;
    g_signal_connect(gmount, "changed", G_CALLBACK(mountChangedCallback),pThis);/* 监听mount的changed信号，获取mountPoint */
    //查看gparted进程是否存在,以便确定是否要移除设备
    pThis->gpartedIsOpening();

    for(;item!=end; ++item){
        if(item.value()->mountPoint() == mountPoint)
            break;
    }
    //qDebug()<<__func__<<__LINE__<<endl;
    if(item == end)
        return;

    //qDebug()<<__func__<<__LINE__<<pThis->m_gpartedIsOpening<<endl;
    volumeItem = item.value();
    if(pThis->m_gpartedIsOpening){
        //gparted打开 + 卸载设备/拔出 => 设备不应该再显示
        pThis->m_volumeList->erase(item);
        Q_EMIT pThis->volumeRemove(mountPoint);
    }else{
        //gparted关闭 + 卸载设备 => 置空设备的挂载点
        volumeItem->setMountPoint("");
        Q_EMIT pThis->mountRemove(mountPoint);
    }


    delete mountItem;
}

void VolumeManager::mountAddCallback(GVolumeMonitor *monitor,
        GMount *gmount,VolumeManager *pThis){
    Q_UNUSED(monitor)
    if(!pThis->m_volumeList)
        return;
    //情景1、未打开gparted时插入新设备的自动挂载操作
    //情景2、未打开gparted时用户手动从卸载状态转为挂载状态
    //情景3、打开gparted后->插入新设备不拔出->关闭gparted 此时新设备会自动挂载
    //情景4、打开gparted后->卸载设备a(此时前端不应该显示a)并且不拔出->关闭gparted->此时设备a需要重新显示
    Mount* mountItem = new Mount(gmount);
    QString device = mountItem->device();
    g_autoptr (GVolume) gvolume = g_mount_get_volume(gmount);
    if (gvolume) {
        for (auto volume : pThis->m_volumeList->values()) {
            if (volume->getGVolume() == gvolume) {
                // 加密U盘的device name可能改变，列表需要按之前的调整
                device = volume->originalDevice();
                break;
            }
        }
    }
    g_signal_connect(gmount, "changed", G_CALLBACK(mountChangedCallback),pThis);/* 监听mount的changed信号，获取mountPoint */
    //qDebug()<<__func__<<__LINE__<<device<<mountItem->name()<<endl;
    if(device.isEmpty()){
        //情景5、数据线连接的手机状态改变："仅充电"->"mtp"或"gphoto" 时会挂载一个没有dev设备的GMount*
        //情景6、数据线连接的手机状态改变：mtp"和"gphoto" 相互转换时会挂载一个没有dev设备的GMount*
        //上述情景5、6 ,该设备不应该保存
        if(mountItem->mountPoint().startsWith("smb://")){/* 远程服务器的device is empty */
            Volume* volume = new Volume(nullptr);
            volume->setFromMount(*mountItem);
            Q_EMIT pThis->mountAdd(Volume(*volume));
        }

        delete mountItem;
        return;
    }

    Volume* volume = new Volume(nullptr);
    volume->setFromMount(*mountItem);
    volume->setDevice(device);

    if(pThis->m_volumeList->contains(device)){
        //情景1、2、3在volumeAddCallback()情景2中已添加至链表，更新挂载点信息即可
        pThis->m_volumeList->value(device)->setMountPoint(mountItem->mountPoint());
        Q_EMIT pThis->mountAdd(Volume(*volume));
    }/*else{
        //情景4、重新显示设备
        pThis->m_volumeList->insert(volume->device(),volume);
        Q_EMIT pThis->volumeAdd(Volume(*volume));
    }*/

    delete mountItem;
}

void VolumeManager::mountChangedCallback(GMount *mount, VolumeManager *pThis)
{
    if(!pThis->m_volumeList)
        return;
    /* 获取mountPoint 挂载点 */
    g_autoptr (GFile) rootFile = g_mount_get_root(mount);
    if(!rootFile)
        return;

    QString mountPoint = g_file_get_uri(rootFile);
    Mount* mountItem = new Mount(mount);

    QString device = mountItem->device();
    g_autoptr (GVolume) gvolume = g_mount_get_volume(mount);
    if (gvolume) {
        for (auto volume : pThis->m_volumeList->values()) {
            if (volume->getGVolume() == gvolume) {
                // 加密U盘的device name可能改变，列表需要按之前的调整
                device = volume->originalDevice();
                break;
            }
        }
    }
    if( pThis->m_volumeList->contains(device) && pThis->m_volumeList->value(device)->getMountPoint().isEmpty()){
        pThis->m_volumeList->value(device)->setMountPoint(mountPoint);/* 更新m_volumeList中volume的mounpoint */
        Q_EMIT pThis->mountAdd(*(pThis->m_volumeList->value(device)));/* 发出更新item的moun属性的信号，手机挂载 */
    }
    else{
        Q_EMIT pThis->mountRemove(mountPoint);/* 发出更新item的moun属性的信号,手机卸载时 */
        Q_EMIT pThis->signal_unmountFinished(mountPoint);
    }

    delete mountItem;
}

void VolumeManager::mountPreUnmountCallback(GVolumeMonitor *monitor, GMount *gmount,VolumeManager *pThis)
{
    Q_UNUSED(monitor)
    if(!pThis->m_volumeList)
        return;

    g_autoptr (GVolume) gVolume = g_mount_get_volume(gmount);
    Volume* volume = new Volume(gVolume);
    if(pThis->m_volumeList->contains(volume->device())){
        {
            QMutexLocker lk(pThis->getMutex());
            pThis->m_occupiedVolume = volume;
            qDebug()<<"mount pre-unmount: "<<volume->device()<<pThis->m_occupiedVolume;
        }
    }
}

void VolumeManager::driveConnectCallback(GVolumeMonitor *monitor,
                                         GDrive *gdrive,VolumeManager *pThis)
{
    Q_UNUSED(monitor)
    if(!pThis->m_volumeList)
        return;
    /* 添加光驱设备，例如空光驱插入 */
    Drive* dirve = new Drive(gdrive);
    QString device = dirve->device();

    if(!pThis->m_volumeList->contains(device) && !device.isEmpty() &&
            (device.contains("/dev/sr")||device.startsWith("/dev/sd")))/* 异常U盘也需要显示 */
    {
        Volume* volume = new Volume(nullptr);
        volume->setFromDrive(*dirve);

        /* hotfix bug#158557 【文件管理器】【安全密钥】文件管理器将ukey设备识别为光驱，显示在了文管侧边栏 */
        if(device.contains("/dev/sr")){
            QString uuid = getDeviceUUID(device.toUtf8().constData());
            auto size = Peony::FileUtils::getDeviceSize(device.toUtf8().constData());
            if (uuid.isEmpty() && size == 0 && "UltraSec USK220 KEY" == volume->name()) {
                volume->setHidden(true);
            }
        }//end

        // try fix #90641, a docking station should be hidden.
        if (device.startsWith("/dev/sd")) {
            auto size = Peony::FileUtils::getDeviceSize(device.toUtf8().constData());
            QString uuid = getDeviceUUID(device.toUtf8().constData());
            if (uuid.isEmpty() && size == 0) {
                volume->setHidden(true);
                // if drive has media, it is not represent a docking station.
                // so it should not be hidden.
                if (driveHasMedia(gdrive)) {
                    volume->setHidden(false);
                }
            }
        }
        // 如果有volume，应该被隐藏
        GList *volumes = g_drive_get_volumes(gdrive);
        if (volumes) {
            volume->setHidden(true);
            g_list_free_full(volumes, g_object_unref);
        }

        if(volume->canEject()){
            pThis->m_volumeList->remove(device);
            pThis->m_volumeList->insert(device, volume);
            qDebug()<<__func__<<__LINE__<<device<<volume->getHidden();
            Q_EMIT pThis->volumeAdd(Volume(*volume));
        }
    }
}

/* 使用drive-connected信号处理暴力拔出的情况
 * 为了处理暴力拔出的情况，实时维护@m_volumeList是有必要的
*/
void VolumeManager::driveDisconnectCallback(GVolumeMonitor *monitor,
                                            GDrive *gdrive,VolumeManager *pThis){
    Q_UNUSED(monitor)
    if(!pThis->m_volumeList)
        return;
    char* gdevice = g_drive_get_identifier(gdrive,G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
    //qDebug()<<__func__<<__LINE__<<gdevice<<endl;
    QString device = gdevice;
    g_free(gdevice);

    //drive已经断开了连接，设备却还在内部存储着
    if(pThis->m_volumeList->contains(device)){
        qDebug()<<__func__<<__LINE__<<device<<endl;
        //情景1：暴力拔出了U盘、数据光盘、空光盘、手机(仅充电)等外置设备
        //情景2：默认数据线连接的手机从"仅充电"->"传输文件(mtp)"或"传输图片(gphoto)"
        //此时的先后顺序: 1、drive-connected 2、volume-removed 3、mount-removed
        pThis->m_volumeList->remove(device);
        Q_EMIT pThis->volumeRemove(device);
    }/*else{
        //非暴力拔出的情况,如正常弹出(在其他回调内部处理)
        //此时的先后顺序：1、mount-removed 2、volume-removed 3、drive-connnected(拔出才会触发)
    }*/
    {
        QMutexLocker lk(pThis->getMutex());
        if(pThis->m_occupiedVolume){
            delete pThis->m_occupiedVolume;
            pThis->m_occupiedVolume = nullptr;
        }
    }
}

void VolumeManager::driveChangedCallback(GVolumeMonitor *monitor, GDrive *gdrive, VolumeManager *pThis)
{
    Q_UNUSED(monitor)
    if(!pThis->m_volumeList)
        return;

    Drive* dirve = new Drive(gdrive);
    Volume* volume = new Volume(nullptr);
    volume->setFromDrive(*dirve);
    QString device = dirve->device();

    if(device.startsWith("/dev/sr") && volume->canEject()){
        pThis->m_volumeList->remove(device);
        pThis->m_volumeList->insert(device, volume);
        qDebug()<<__func__<<__LINE__<<device<<volume->getHidden();
    }

}

VolumeManager* VolumeManager::getInstance(){
    if(nullptr == m_globalManager)
        m_globalManager = new VolumeManager();
    return m_globalManager;
}

QList<GVolume*> VolumeManager::allGVolumes(){
    QList<GVolume*> volumeList;
    GList *l,*volumes = nullptr;
    GVolume *gvolume;

    if(m_volumeMonitor)
        volumes = g_volume_monitor_get_volumes(m_volumeMonitor);

    //m_gpartedIsOpening = (volumes == nullptr);    //gparted打开时volumes为nullptr /* hotfix bug#164497 【文件管理器】左侧目录，手动弹出空光盘时数据盘、光驱消失 */
    for(l = volumes; l != nullptr; l = l->next){
        gvolume = (GVolume*)l->data;
        volumeList.push_back(gvolume);
    }

    // 需要在此方法调用处释放GVolume实例
    if(volumes)
        g_list_free (volumes);

    return volumeList;
}

QList<Volume*> VolumeManager::allVolumes()
{
    int volumeCount;
    //QList<Volume> volume;
    QList<Volume*> volumes;
    QList<GVolume*> gVolumes;//GVolume*由~Volume()释放

    gVolumes = allGVolumes();
    if(gVolumes.isEmpty())
        return volumes;

    volumeCount = gVolumes.count();
    for(int i=0; i<volumeCount; ++i){
        //构造Volume以及内部数据
        Volume* volumeItem = new Volume(gVolumes.at(i));
        //插入list
        volumes.append(volumeItem);
        g_object_unref(gVolumes.at(i));
    }
    //qDebug()<<"--------------------------->>>>"<<endl;

    return volumes;
}

//返回值是否替换成指针更好，利用堆内存
//是否需要深拷贝？还是model与VolumeManager共用一套数据? 这里选用深拷贝
//排序是放在model做还是？
QList<Volume>* VolumeManager::allVaildVolumes(){
    QList<Volume>* validVolumeList;
    QList<Mount*>  mounts;
    QList<Volume*> volumes;
    int mountCount,volumeCount;

    mounts  = allMounts();
    volumes = allVolumes();//从这里得到是否gparted已打开

    mountCount = mounts.count();
    volumeCount = volumes.count();

    if (m_volumeList) {
        for (auto volume : *m_volumeList) {
            delete volume;
        }
        delete m_volumeList;
    }

    m_volumeList = new QHash<QString,Volume*>();
    //根文件系统
    Volume* rootVolume = new Volume(nullptr);
    rootVolume->initRootVolume();
    m_volumeList->remove(rootVolume->device());
    m_volumeList->insert(rootVolume->device(),rootVolume);

    //vaildVolumeList = std::make_shared<QList<Volume*>>();
    for(int i=0; i<mountCount; ++i){
        Volume* volumeItem = new Volume(nullptr);
        volumeItem->setFromMount(*mounts.at(i));//从Mount对象构造Volume对象数据
        //qDebug()<<__func__<<__LINE__<<volumeItem->device()<<volumeItem->name();
        m_volumeList->remove(volumeItem->device());
        m_volumeList->insert(volumeItem->device(),volumeItem);
        delete mounts.at(i);
    }

    //qDebug()<<__func__<<__LINE__<<m_gpartedIsOpening<<mounts.count()<<" "<<volumes.count()<<m_volumeList->count()<<endl;
    if(!m_gpartedIsOpening){ //gparted未打开时，才考虑卷设备未挂载的情况
        for(int i=0; i<volumeCount; ++i){
            Volume* volumeItem = volumes.at(i);
            //qDebug()<<__func__<<__LINE__<<volumeItem->device()<<endl;
            if(m_volumeList->contains(volumeItem->device())) {
                delete volumeItem;
                continue;
            }
            //qDebug()<<__func__<<__LINE__<<volumeItem->device()<<volumeItem->name();
            auto oldVolume = m_volumeList->take(volumeItem->device());
            delete oldVolume;
            m_volumeList->insert(volumeItem->device(),volumeItem);
        }
    }
    //qDebug()<<__func__<<__LINE__<<m_gpartedIsOpening<<mounts.count()<<" "<<volumes.count()<<m_volumeList->count()<<endl;

    /* 添加光驱设备 */
    QList<Drive*> driveList = allDrives();
    for(auto entry : driveList)
    {
        Volume* volumeItem = new Volume(nullptr);
        volumeItem->setFromDrive(*entry);

        // 如果有volume，应该被隐藏
        bool bHasVolume = false;
        GList *volumes = g_drive_get_volumes(entry->getGDrive());
        if (volumes) {
            volumeItem->setHidden(true);
            bHasVolume = true;
            g_list_free_full(volumes, g_object_unref);
        }

        QString device = volumeItem->device();
        if(m_volumeList->contains(device)) {
            delete volumeItem;
            //delete entry;
            continue;
        }

        bool shouldDeleteItem = true;
        if(device.contains("/dev/sr")){/* 判断是否为光驱设备 */
            shouldDeleteItem = false;
            auto oldVolume = m_volumeList->take(volumeItem->device());
            delete oldVolume;
            m_volumeList->insert(volumeItem->device(), volumeItem);
            /* hotfix bug#158557 【文件管理器】【安全密钥】文件管理器将ukey设备识别为光驱，显示在了文管侧边栏 */
            if(device.contains("/dev/sr")){
                QString uuid = getDeviceUUID(device.toUtf8().constData());
                auto size = Peony::FileUtils::getDeviceSize(device.toUtf8().constData());
                if (uuid.isEmpty() && size == 0 && "UltraSec USK220 KEY" == volumeItem->name()) {
                    volumeItem->setHidden(true);
                }
            }//end
        }
        if(volumeItem->canEject() && device.contains("/dev/sd")){/* 异常U盘设备 */
            shouldDeleteItem = false;
            auto oldVolume = m_volumeList->take(volumeItem->device());
            delete oldVolume;
            m_volumeList->insert(volumeItem->device(), volumeItem);

            // try fix #90641, a docking station should be hidden.
            if (device.startsWith("/dev/sd")) {
                QString uuid = getDeviceUUID(device.toUtf8().constData());
                auto size = Peony::FileUtils::getDeviceSize(device.toUtf8().constData());
                if (uuid.isEmpty() && size == 0) {
                    volumeItem->setHidden(true);
                    // if drive has media, it is not represent a docking station.
                    // so it should not be hidden.
                    if (entry->getGDrive()) {
                        if (driveHasMedia(entry->getGDrive())) {
                            volumeItem->setHidden(false);
                        }
                    }
                }
                else if(uuid.isEmpty() && size != 0 && entry->getGDrive()){
                    qDebug()<<"the icon of volume"<<volumeItem->device()<<volumeItem->icon();
                    if("drive-removable-media" == volumeItem->icon()){/* 由此判断区分本地固态硬盘(SATA、SSD等)和异常U盘 */
                        //fix show SATA, SSD unparted device /dev/sda issue, link to bug#135269,125009,206525
                        volumeItem->setHidden(true);
                    }
                }
            }
            if(bHasVolume){/* 解决:U盘多个分区时，侧边栏会显示drive */
                volumeItem->setHidden(true);
            }
        }
        if (shouldDeleteItem)
            delete volumeItem;
        //delete entry;
    }



    QHash<QString,Volume*>::const_iterator item = m_volumeList->begin();
    QHash<QString,Volume*>::const_iterator end = m_volumeList->end();

    validVolumeList = new QList<Volume>();
    for(;item!=end; item++){
        Volume volume = *(item.value());
        validVolumeList->append(volume);
    }

    return validVolumeList;
}

QList<GMount*> VolumeManager::allGMounts(){
    GMount* gmount = nullptr;
    GList *mounts = nullptr, *l = nullptr;
    QList<GMount*> mountList;
    if(m_volumeMonitor)
        mounts = g_volume_monitor_get_mounts(m_volumeMonitor);

    for(l = mounts; l != nullptr;l = l->next){
        gmount = (GMount*)l->data;
        mountList.push_back(gmount);
    }

    // 需要在此方法调用处释放GMount实例
    if(mounts)
        g_list_free (mounts);

    return mountList;
}

QList<Mount*> VolumeManager::allMounts(){
    int mountCount;
    QList<Mount*> mounts;
    QList<GMount*> gMounts;//QList内的GMount*最终会传给Mount类，~Mount()去释放

    gMounts = allGMounts();
    if(gMounts.isEmpty())
        return mounts;

    mountCount = gMounts.count();
    for(int i=0; i<mountCount; ++i){
        Mount* mountItem =  new Mount(gMounts.at(i));
        if(mountItem->device().isEmpty()){
            //情景：查询设备时数据线连接的手机处于 "mtp"或"gphoto"状态
            //     此时有一个没有dev设备的GMount*不应该保存
            delete mountItem;
            g_object_unref(gMounts.at(i));
            continue;
        }
        mounts.append(mountItem);
        g_object_unref(gMounts.at(i));
    }
    //qDebug()<<"=================================>>>>>"<<endl;

    return mounts;
}

QList<GDrive *> VolumeManager::allGDrives()
{
    GDrive* gdrive = nullptr;
    GList *gdrives = nullptr, *l = nullptr;
    QList<GDrive*> gdriveList;
    if(m_volumeMonitor)
        gdrives = g_volume_monitor_get_connected_drives(m_volumeMonitor);

    for(l = gdrives; l != nullptr;l = l->next){
        gdrive = (GDrive*)l->data;
        gdriveList.push_back(gdrive);
    }

    // 需要在此方法调用处释放GDrive实例
    if(gdrives)
        g_list_free (gdrives);

    return gdriveList;
}

QList<Drive *> VolumeManager::allDrives()
{
    QList<Drive*> drives;
    QList<GDrive*> gdrives = allGDrives();
    if(gdrives.size()<=0)
        return drives;

    for(auto entry: gdrives)
    {
        Drive* drive = new Drive(entry);
        drives.append(drive);
        g_object_unref(entry);
    }

    return drives;
}
//QString VolumeManager::guessContentType(GMount* gmount){
//    char** guessType;
//}

Volume::Volume(GVolume* gvolume):m_volume(gvolume){
    initVolumeInfo();
}

Volume::Volume(const Volume& other){
    m_name = other.m_name;
    m_originalDevice = other.m_originalDevice;
    m_device = other.m_device;
    m_uuid = other.m_uuid;
    m_icon = other.m_icon;
    m_mountPoint = other.m_mountPoint;
    m_canEject = other.m_canEject;
    m_canStop = other.m_canStop;
    m_canUnmount = other.m_canUnmount;
    m_canMount = other.m_canMount;
    m_gMount = other.m_gMount == nullptr? nullptr : (GMount*)g_object_ref(other.m_gMount);
    m_gdrive = other.m_gdrive == nullptr? nullptr : (GDrive*)g_object_ref(other.m_gdrive);
    m_volume = nullptr;
    m_hidden = other.m_hidden;
    if(other.m_volume)
        m_volume = (GVolume*)g_object_ref(other.m_volume);
}

Volume::~Volume(){
    if(m_volume)
        g_object_unref(m_volume);

    if (m_gMount) {
        g_object_unref(m_gMount);
    }
    if (m_gdrive) {
        g_object_unref(m_gdrive);
    }
}

void Volume::initVolumeInfo()
{
    if(!m_volume)   //如果m_volume为nullptr，可能会是一种用Mount来填充Volume数据的方式
        return;

    m_volume = static_cast<GVolume *>(g_object_ref(m_volume));

    m_gMount = nullptr;
    m_canUnmount = false;
    char* gname = g_volume_get_name(m_volume);
    char* guuid = g_volume_get_uuid(m_volume);
    char* gdevice = g_volume_get_identifier(m_volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);

    //挂载点
    GMount* gmount = nullptr;
    GFile* rootFile = nullptr;
    QString tmpDevice;
    gmount = g_volume_get_mount(m_volume);
    if(gmount){
        m_gMount = gmount;
        rootFile = g_mount_get_root(gmount);
        m_canUnmount = g_mount_can_unmount(gmount);//是否可卸载
    }
    if(rootFile){
        char* gmountPoint = g_file_get_uri(rootFile);
        m_mountPoint = gmountPoint;
        g_free(gmountPoint);
        gmountPoint = g_filename_from_uri(m_mountPoint.toUtf8().constData(), nullptr, nullptr);
        m_mountPoint = gmountPoint;
        g_free(gmountPoint);
        g_object_unref(rootFile);
    }
    //是否可弹出
    GDrive* gdrive = g_volume_get_drive(m_volume);
    //qDebug()<<__func__<<__LINE__<<(gdrive==nullptr)<<endl;
    if(gdrive){
        m_canEject = g_drive_can_eject(gdrive);
        m_canStop = g_drive_can_stop(gdrive);
        g_autofree char* gdevice = g_drive_get_identifier(gdrive, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        tmpDevice = gdevice;
        g_object_unref(gdrive);
    }
    //TODO... icon

    m_name = gname;
    m_uuid = guuid;
    m_originalDevice = gdevice;
    m_device = gdevice;
    GIcon* gicon = g_volume_get_icon(m_volume);
    m_icon = Peony::FileUtils::getIconStringFromGIcon(gicon, tmpDevice);
    // fix #81852, refer to #57660, #70014, #96652, task #25343
    if (QString(m_icon) == "drive-harddisk-usb") {
        double size = 0.0;
        if(!tmpDevice.isEmpty()){
            size = Peony::FileUtils::getDeviceSize(tmpDevice.toUtf8().constData());
        }else{
            size = Peony::FileUtils::getDeviceSize(m_device.toUtf8().constData());
        }

        if (size < 128) {
            m_icon = "drive-removable-media-usb";
        }
    }

    if(m_volume)
        m_canMount = g_volume_can_mount(m_volume);

    g_object_unref(gicon);
    g_free(gname);
    g_free(guuid);
    g_free(gdevice);
    /*if(gmount && G_IS_OBJECT(gmount))//放在析构中释放
        g_object_unref(gmount);*/
}

//利用设备路径(也可用uuid)判断设备是否相同
bool Volume::operator==(const Volume& other) const{
    return m_originalDevice == other.m_originalDevice;
}

bool Volume::operator==(const Volume* other) const{
    return m_originalDevice == other->m_originalDevice;
}

//bool Volume::operator==(const QString& device) const{
//    return m_device == m_device;
//}

void Volume::eject(GMountUnmountFlags ejectFlag)
{
    GDrive* gdrive =nullptr;
    if(m_volume)
        gdrive = g_volume_get_drive(m_volume);
    else if(m_gMount)
        gdrive = g_mount_get_drive(m_gMount);
    else if(m_gdrive)
        gdrive = m_gdrive;
    else
        return;

    if(!gdrive)
        return;

    Drive drive(gdrive);
    if(m_gMount){
        GFile* rootFile = g_mount_get_root(m_gMount);
        if(rootFile){
            char* mountPath = g_file_get_uri(rootFile);
            drive.setMountPath(mountPath);
            g_object_unref(rootFile);
            g_free(mountPath);
        }
    }else{
        drive.setMountPath("");
    }
    drive.eject(ejectFlag);

}

void Volume::unmount()
{
    if(m_volume)
        m_gMount = (GMount*) g_object_ref(g_volume_get_mount(m_volume));

    if(!m_gMount)
        return;
    Mount mount(m_gMount);
    mount.unmount();
}

static void mount_async_callback(GVolume *volume, GAsyncResult *res, Volume *p_this)
{
    GError *err = nullptr;
    bool successed = g_volume_mount_finish(volume, res, &err);
    if (err) {
        if (g_error_matches(err, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED)) {
            bool need_password = bool (g_object_get_data(G_OBJECT (volume), "need-password"));
            if (need_password) {
                QInputDialog d;
                d.setTextEchoMode(QLineEdit::Password);
                d.setLabelText(QString(static_cast<char *>(g_object_get_data(G_OBJECT (volume), "message"))));
                if (d.exec()) {
                    auto password = d.textValue();
                    auto mount_op = g_mount_operation_new();
                    gulong signal = g_signal_connect(mount_op, "ask-password", G_CALLBACK (askPasswdCallback), volume);
                    g_object_set_data(G_OBJECT (volume), "signal", gpointer(signal));
                    g_mount_operation_set_password(mount_op, password.toUtf8().constData());
                    g_volume_mount(volume, G_MOUNT_MOUNT_NONE, mount_op, nullptr, GAsyncReadyCallback (mount_async_callback), p_this);
                    g_object_unref(mount_op);
                }
            }
        } else {
            bool need_password = bool (g_object_get_data(G_OBJECT (volume), "need-password"));
            if (need_password) {
                QString errMsg = err->message;
                if (errMsg.contains("Incorrect passphrase")) {
                    QMessageBox::critical(0, 0, QObject::tr("Failed to activate device: Incorrect passphrase"));
                }
            }
        }

        //QMessageBox::critical(0, 0, QString("%1 %2 %3").arg(g_quark_to_string(err->domain)).arg(err->code).arg(err->message));
        g_error_free(err);
    }

    if (successed) {
        QString unmountNotify = QObject::tr("The device has been mount successfully!");
        Peony::SyncThread::notifyUser(unmountNotify);
        Q_EMIT VolumeManager::getInstance()->signal_mountFinished();
    }
}
void Volume::mount()
{
    if(m_volume) {
        auto mount_op = g_mount_operation_new();
        gulong signal = g_signal_connect(mount_op, "ask-password", G_CALLBACK (askPasswdCallback), m_volume);
        g_object_set_data(G_OBJECT (m_volume), "signal", gpointer(signal));

        g_volume_mount(m_volume,
                       G_MOUNT_MOUNT_NONE,
                       mount_op,
                       nullptr,
                       GAsyncReadyCallback(mount_async_callback),
                       this);

        g_object_unref(mount_op);
    }
}

bool Volume::getHidden() const
{
    return m_hidden;
}

void Volume::setHidden(bool hidden)
{
    m_hidden = hidden;
}


void Volume::setFromMount(const Mount& mount){
    m_name = mount.name();
    m_uuid = mount.uuid();
    m_icon = mount.icon();
    m_originalDevice = mount.device();
    m_device = mount.device();
    m_canEject = mount.canEject();
    m_canStop = mount.canStop();
    m_canUnmount = mount.canUnmount();
    m_mountPoint = mount.mountPoint();
    m_gMount =(GMount*) g_object_ref(mount.getGMount());
    m_volume = g_mount_get_volume(m_gMount);
}

void Volume::setFromDrive(const Drive &drive)
{
    m_name = drive.name();
    m_canEject = drive.canEject();
    m_canStop = drive.canStop();
    m_icon = drive.icon();
    m_originalDevice = drive.device();
    m_device = drive.device();
    m_gdrive = (GDrive*)g_object_ref(drive.getGDrive());
}

void Volume::setMountPoint(QString point){
    m_mountPoint.clear();
    m_mountPoint = point;
}

QString Volume::getMountPoint()
{
    return m_mountPoint;
}

void Volume::setLabel(const QString &label){
    m_name = label;
}

void Volume::setDevice(const QString &device)
{
    m_device = device;
}

void Volume::setIconName(const QString &iconName)
{
    m_icon = iconName;
}

//根分区信息
Volume* Volume::initRootVolume(){
    m_uuid = "";
    m_mountPoint = "/";
    m_canEject = false;
    m_canStop = false;
    m_volume = nullptr;
    m_name = "File System";

    GUnixMountEntry* entry = g_unix_mount_at("/",nullptr);
    if(!entry)
        entry = g_unix_mount_for("/",nullptr);
    if(!entry)
        return this;

    const char* device = g_unix_mount_get_device_path(entry);
    m_device = device;
    m_originalDevice = device;
    g_unix_mount_free(entry);

    return this;
}

Drive::Drive(GDrive* gdrive):m_drive(gdrive){
    initDriveInfo();
}

Drive::~Drive(){
    if(m_drive)
        g_object_unref(m_drive);
}

void Drive::initDriveInfo(){
    m_canEject = false;

    if(!m_drive)
        return;

    m_drive = static_cast<GDrive *>(g_object_ref(m_drive));

    g_autofree gchar* unix_device = g_drive_get_identifier(m_drive, G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE);
    m_device = unix_device;
    m_canEject = g_drive_can_eject(m_drive);
    m_canStop = g_drive_can_stop(m_drive);
    g_autofree gchar* drive_name = g_drive_get_name(m_drive);
    m_name = drive_name;
    GIcon* gicon = g_drive_get_icon(m_drive);
    m_icon = Peony::FileUtils::getIconStringFromGIcon(gicon, m_device);
    // fix #81852, refer to #57660, #70014, #96652, task #25343
    if (QString(m_icon) == "drive-harddisk-usb") {
        double size = Peony::FileUtils::getDeviceSize(m_device.toUtf8().constData());
        if (size < 128) {
            m_icon = "drive-removable-media-usb";
        }
    }

    g_object_unref(gicon);
}

QString Drive::name() const
{
    return m_name;
}

QString Drive::icon() const
{
    return m_icon;
}

QString Drive::device() const
{
    return m_device;
}

bool Drive::canEject()const{
    return m_canEject;
}

bool Drive::canStop() const
{
    return m_canStop;
}

GDrive *Drive::getGDrive() const
{
    return m_drive;
}

static GAsyncReadyCallback eject_cb(GDrive *gDrive, GAsyncResult *result, QString* targetUri)
{
    g_autoptr (GError) error = nullptr;
    bool successed = g_drive_eject_with_operation_finish(gDrive, result, &error);
    qDebug()<<"The result that drive eject with operation finish:"<<successed;
    if (error) {
        qDebug()<<"error code of drive stop:"<<error->code<<", error message:"<<error->message;
        if(targetUri){
            delete targetUri;
            targetUri = nullptr;
        }

        if(G_IO_ERROR_BUSY == error->code){/* 卷被占用时，防止二次弹出信息提示框 */            
            return nullptr;
        }
        if(! strcmp(error->message,"Not authorized to perform operation")){//umount /data need permissions.
            /* gmountOperation会弹出授权框，防止二次弹框 */
            //QMessageBox::warning(nullptr,QObject::tr("Eject failed"),QObject::tr("Not authorized to perform operation."), QMessageBox::Ok);
            return nullptr;
        }

        //fix bug#104482, pull device immediately when eject it, error message not translated issue
        QString errMessage = error->message;
        if (error->code == G_IO_ERROR_FAILED){
            errMessage = QObject::tr("Eject device failed, the reason may be that the device has been removed, etc.");
        }
        QMessageBox warningBox(QMessageBox::Warning, QObject::tr("Eject failed"), errMessage, QMessageBox::Ok);
        warningBox.exec();
    } else {
        /* 弹出完成信息提示 */
        QString ejectNotify = QObject::tr("Data synchronization is complete and the device can be safely unplugged!");
        Peony::SyncThread::notifyUser(ejectNotify);
        QString str = *targetUri;
        Q_EMIT VolumeManager::getInstance()->signal_unmountFinished(*targetUri);
    }
    if(targetUri){
        delete targetUri;
        targetUri = nullptr;
    }
    return nullptr;
}

/* Eject some device by stop it's drive. Such as: mobile harddisk. */
static void ejectDevicebyDrive(GObject* object,GAsyncResult* result, QString* targetUri)
{
    g_autoptr (GError) error = nullptr;

    if(!g_drive_stop_finish (G_DRIVE(object), result, &error)){
        if((NULL != error) && (G_IO_ERROR_FAILED_HANDLED != error->code)){
            // @note 这里不要拼接字符串，多次弹出会崩溃
//            QString errorMsg = QObject::tr("Unable to eject").arg(pThis->name());
            qDebug()<<"error code of drive stop:"<<error->code<<", error message:"<<error->message;
            if(G_IO_ERROR_BUSY == error->code){/* 卷被占用时，防止二次弹出信息提示框 */
                return;
            }
            if(! strcmp(error->message,"Not authorized to perform operation")){/* gmountOperation会弹出授权框，防止二次弹框 */
                return;
            }

            //fix bug#104482, pull device immediately when eject it, error message not translated issue
            QString errMessage = error->message;
            if (error->code == G_IO_ERROR_FAILED){
                errMessage = QObject::tr("Eject device failed, the reason may be that the device has been removed, etc.");
            }
            QMessageBox warningBox(QMessageBox::Warning, QObject::tr("Eject failed"), errMessage, QMessageBox::Ok);
            warningBox.exec();
        }
    }else {
        /* 弹出完成信息提示 */
        QString ejectNotify = QObject::tr("Data synchronization is complete and the device can be safely unplugged!");
        Peony::SyncThread::notifyUser(ejectNotify);
        Q_EMIT VolumeManager::getInstance()->signal_unmountFinished(*targetUri);
    }
    if(targetUri){
        delete targetUri;
        targetUri = nullptr;
    }
}

void Drive::eject(GMountUnmountFlags ejectFlag)
{
    // fix #92731, note that if we didn't pass a mount-operation instance,
    // drive will do operation without user interaction.
    auto mount_op = VolumeManager::getInstance()->getOccupiedInfoThread()->getMountOp();
    QString *targetUri = new QString(VolumeManager::getInstance()->getTargetUriFromUnixDevice(m_device));
    qDebug()<<"eject flag: "<<ejectFlag<<m_device<<m_canEject<<g_drive_can_stop(m_drive)<<g_drive_is_removable(m_drive);
    if(m_canEject && !m_device.startsWith("/dev/sd")){ /* U盘使用安全移除 */
        g_drive_eject_with_operation(m_drive, ejectFlag, mount_op, nullptr, GAsyncReadyCallback(eject_cb), targetUri);
    }
    else if(g_drive_can_stop(m_drive) || (g_drive_is_removable(m_drive) && !m_device.startsWith("/dev/mmc"))){// for mobile harddisk.
        /* 加"(g_drive_is_removable(m_drive) && !m_device.startsWith("/dev/mmc"))"这个判断是为了解决bug#184111和bug#149182；有些U盘的can-stop为false,其中为一款sd卡的devicename */
        g_drive_stop(m_drive, ejectFlag, mount_op, NULL, GAsyncReadyCallback(ejectDevicebyDrive), targetUri);
    }else if(g_drive_is_removable(m_drive)){
        //fix bug#149182, SD card eject can not recgonize issue
        g_drive_eject_with_operation(m_drive, ejectFlag, mount_op, nullptr, GAsyncReadyCallback(eject_cb), targetUri);
    }
}

void Drive::setMountPath(const QString &mountPath)
{
    m_mountPath = mountPath;
}

Mount::Mount(GMount* gmount) {
    m_mount = gmount;
    initMountInfo();
}

Mount::~Mount(){
    if(m_entry)
        g_unix_mount_free(m_entry);
    if(m_mount)
        g_object_unref(m_mount);
}

void Mount::initMountInfo(){
    GFile* rootFile;
    GVolume* gvolume;
    char* gmountPoint,*uuid;
    QString tmpDevice;

    if(!m_mount)
        return;

    m_mount = static_cast<GMount *>(g_object_ref(m_mount));
    //1、mountPoint 挂载点
    rootFile = g_mount_get_root(m_mount);
    if(rootFile){
        gmountPoint = g_file_get_uri(rootFile);
        m_mountPoint = gmountPoint;
        g_free(gmountPoint);
        if(!(m_mountPoint.startsWith("smb://"))){
            gmountPoint = g_filename_from_uri(m_mountPoint.toUtf8().constData(),nullptr,nullptr);
            m_mountPoint = gmountPoint;
            g_free(gmountPoint);
        }
        g_object_unref(rootFile);
    }

    //2、unix-device dev设备路径 、uuid
    gvolume = g_mount_get_volume(m_mount);
    if(!gvolume){//this means gparted is opening. 意味着开启了分区编辑器
        queryDeviceByMountpoint();
    }else{
        char* device = g_volume_get_identifier(gvolume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        uuid = g_volume_get_uuid(gvolume);//g_mount_get_uuid()在设备处于挂载状态时返回值为nullptr
        GDrive* gdrive = g_volume_get_drive(gvolume);
        if(gdrive){
            g_autofree char* gdevice = g_drive_get_identifier(gdrive, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
            tmpDevice = gdevice;
        }
        g_object_unref(gdrive);
        m_device = device;
        m_uuid = uuid;

        g_free(uuid);
        g_free(device);
        g_object_unref(gvolume);
    }

    //3、name 设备名、挂载点名
    g_autofree gchar* mount_name = g_mount_get_name(m_mount);
    m_name = mount_name;

    //4、can eject? 是否可弹出
    GDrive* gdrive = g_mount_get_drive(m_mount);/* gdrive为nullptr表示gparted打开状态 */
    //qDebug()<<__func__<<__LINE__<<(gdrive==nullptr)<<endl;
    if(gdrive){
        m_canEject = g_drive_can_eject(gdrive);
        m_canStop = g_drive_can_stop(gdrive);
        //g_object_unref(gdrive);
    }else{
        m_canEject = g_mount_can_eject(m_mount);/* gpartedd打开时gio获取到设备均不可弹出 */
        m_canStop = false;
    }

    //5、can unmount? 是否可卸载
    m_canUnmount = g_mount_can_unmount(m_mount);
    /* 获取图标 */
    GIcon* gicon = g_mount_get_icon(m_mount);
    m_icon = Peony::FileUtils::getIconStringFromGIcon(gicon, tmpDevice);
    // fix #81852, refer to #57660, #70014, #96652, task #25343
    if (m_device.startsWith("/dev/sd") && m_icon.endsWith(".ico") || QString(m_icon) == "drive-harddisk-usb") {/* 镜像U盘或移动硬盘的图标处理，linkto bug#174770 */
        double size = 0.0;
        if(!tmpDevice.isEmpty()){
            size = Peony::FileUtils::getDeviceSize(tmpDevice.toUtf8().constData());
        }else{
            size = Peony::FileUtils::getDeviceSize(m_device.toUtf8().constData());
        }
        if (size < 128) {
            m_icon = "drive-removable-media-usb";
        }
    }
    if(m_device.startsWith("/dev/sr") && m_icon.endsWith(".ico")){/* 镜像光盘的图标处理,linkto bug#174770 */
        m_icon = "media-optical";
    }

    g_object_unref (gicon);
}

void Mount::queryDeviceByMountpoint(){
    const char* device;
    char* mountPoint;
    if(m_mountPoint.isEmpty())
        return;

    //处理uri转码
    if(m_mountPoint.startsWith("file:///")){
        mountPoint = g_filename_from_uri(m_mountPoint.toUtf8().constData(),nullptr,nullptr);
        m_mountPoint = mountPoint;
        g_free(mountPoint);
    }
    //mountPoint = m_mountPoint.toUtf8().constData();
    //qDebug()<<__func__<<__LINE__<<m_mountPoint<<endl;
    m_entry = g_unix_mount_at(m_mountPoint.toUtf8().constData(),nullptr);
    if(!m_entry)
        m_entry = g_unix_mount_for(m_mountPoint.toUtf8().constData(),nullptr);
    if(!m_entry)
        return;
    //qDebug()<<__func__<<__LINE__<<m_mountPoint<<endl;
    device = g_unix_mount_get_device_path(m_entry);
    m_device = device;
}

/*==================Volume property==============*/
QString Volume::name() const{
    return m_name;
}

QString Volume::uuid() const{
    return m_uuid;
}

QString Volume::icon() const{
    return m_icon;
}

QString Volume::device() const{
    return m_device;
}

QString Volume::originalDevice() const
{
    return m_originalDevice;
}

QString Volume::mountPoint() const{
    return m_mountPoint;
}

GVolume* Volume::getGVolume() const{
    return m_volume;
}

GDrive *Volume::getGDrive() const
{
    return m_gdrive;
}

GMount *Volume::getGMount() const
{
    return m_gMount;
}

bool Volume::canEject() const{
    return m_canEject;
}

bool Volume::canStop() const
{
   return m_canStop;
}

bool Volume::canUnmount() const{
    return m_canUnmount;
}

bool Volume::canMount() const
{
    return m_canMount;
}

/*==================Mount property==============*/
QString Mount::name() const{
    return m_name;
}

QString Mount::uuid() const{
    return m_uuid;
}

QString Mount::icon() const
{
    return m_icon;
}

QString Mount::device() const{
    return m_device;
}

QString Mount::mountPoint() const{
    return m_mountPoint;
}

GMount *Mount::getGMount() const
{
    return m_mount;
}

bool Mount::canEject() const{
    return m_canEject;
}

bool Mount::canStop() const{
    return m_canStop;
}

bool Mount::canUnmount() const{
    return m_canUnmount;
}

static void unmount_force_cb(GMount* mount, GAsyncResult* result, QString* targetUri) {

    GError *err = nullptr;
    g_mount_unmount_with_operation_finish(mount, result, &err);
    if (err) {
        QMessageBox::warning(nullptr, QObject::tr("Force unmount failed"), QObject::tr("Error: %1\n").arg(err->message));
        g_error_free(err);
    } else {
        QString unmountNotify = QObject::tr("Data synchronization is complete,the device has been unmount successfully!");
        Peony::SyncThread::notifyUser(unmountNotify);
        QString uri = *targetUri;
        Q_EMIT VolumeManager::getInstance()->signal_unmountFinished(*targetUri);
    }
    if(targetUri){
        delete targetUri;
        targetUri = nullptr;
    }
}

static GAsyncReadyCallback unmount_finished(GMount *mount, GAsyncResult *result, QString* targetUri)
{
    g_autoptr (GError) err = nullptr;
    g_mount_unmount_with_operation_finish(mount, result, &err);
    if (err) {        
        if(!strcmp(err->message,"Not authorized to perform operation")){//umount /data need permissions.
            QMessageBox::warning(nullptr,QObject::tr("Eject failed"),QObject::tr("Not authorized to perform operation."), QMessageBox::Ok);           
            if(targetUri){
                delete targetUri;
                targetUri = nullptr;
            }
            return nullptr;
        }
        if(strstr(err->message,"umount: ")){
            if(targetUri){
                delete targetUri;
                targetUri = nullptr;
            }
            QMessageBox::warning(nullptr,QObject::tr("Unmount failed"),QObject::tr("Unable to unmount it, you may need to close some programs, such as: GParted etc."),QMessageBox::Yes);
            return nullptr;
        }
        auto button = QMessageBox::warning(nullptr, QObject::tr("Unmount failed"), QObject::tr("Error: %1\n"
                                            "Do you want to unmount forcely?").arg(err->message),QMessageBox::Yes, QMessageBox::No);
        if (button == QMessageBox::Yes) {
            g_mount_unmount_with_operation(mount,
                                           G_MOUNT_UNMOUNT_FORCE,
                                           nullptr,
                                           nullptr,
                                           GAsyncReadyCallback(unmount_force_cb),
                                           targetUri);
        } else {
            if(targetUri){
                delete targetUri;
                targetUri = nullptr;
            }
        }

    } else {
        /* 卸载完成信息提示 */
        QString unmountNotify = QObject::tr("Data synchronization is complete,the device has been unmount successfully!");
        Peony::SyncThread::notifyUser(unmountNotify);     
        Q_EMIT VolumeManager::getInstance()->signal_unmountFinished(*targetUri);
        if(targetUri){
            delete targetUri;
            targetUri = nullptr;
        }
    }
    return nullptr;
}

void Mount::unmount()
{
    if(m_canUnmount && m_mount){//gparted打开时 + 中文挂载点 => 卸载失败(转码后的挂载点目录找不到，与文件系统格式无关)               
        QString* targetUri = new QString(VolumeManager::getInstance()->getTargetUriFromUnixDevice(m_device));
        g_autoptr(GMountOperation) mount_op = g_mount_operation_new();
        g_mount_unmount_with_operation(m_mount, G_MOUNT_UNMOUNT_NONE, mount_op, nullptr, GAsyncReadyCallback(unmount_finished), targetUri);
    }
    //考虑使用udisks API udisks_filesystem_call_unmount 或者udisks2相关dbus做卸载处理
}


/*==================Drive property==============*/

#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QListWidget>
MessageDialog::MessageDialog(QWidget *parent):
    QDialog(parent)
{    
    //setWindowTitle("Volume is occupied");
    setWindowTitle(tr("Peony"));
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setMinimumSize(QSize(350,200));
}

void MessageDialog::init(std::map<QString, QIcon> &occupiedAppMap, const QString& message)
{
    QVBoxLayout *layout = new QVBoxLayout();
    QFont font;
    font.setBold(true);
    QLabel* massageLabel = new QLabel(message.toStdString().data());
    massageLabel->setFont(font);
    massageLabel->setContentsMargins(40,10,0,0);

    std::map<QString, QIcon>::iterator  it;
    QListWidget* listWidget         = new QListWidget(this);
    listWidget->setContentsMargins(QMargins(10,10,10,10));
    int i =0;
    for (it = occupiedAppMap.begin(); it != occupiedAppMap.end(); ++it,++i)
    {
        QListWidgetItem *newItem = new QListWidgetItem;
        newItem->setText(it->first);
        newItem->setIcon(it->second);
        listWidget->insertItem(i, newItem);
    }

    QLabel* hintLabel = new QLabel(tr("Forcibly pulling out the device may cause data\n loss or device exceptions!"));

    layout->addWidget(massageLabel);
    layout->addStretch(5);
    layout->addWidget(listWidget);
    layout->addStretch();
    layout->addWidget(hintLabel);
    setLayout(layout);
}

#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include"file-utils.h"
GetOccupiedAppsInfoThread::GetOccupiedAppsInfoThread(QObject *parent) : QThread(parent)
{

}

void GetOccupiedAppsInfoThread::run()
{
    this->m_mountOp=g_mount_operation_new();
    g_signal_connect (m_mountOp, "show-processes", G_CALLBACK(show_processes_cb), this);
}

void GetOccupiedAppsInfoThread::show_processes_cb(GMountOperation *MountOp, char *message, GArray *processes, char **choices, gpointer user_data)
{
    Q_UNUSED(MountOp)
    Q_UNUSED(choices)
    qDebug()<<"len of processes:"<<processes->len;
    std::map<QString,QIcon> occupiedAppMap;
    for(guint i=0; i< processes->len; i++)
    {
        GPid pid = g_array_index(processes, GPid ,i);
        QProcess *process =new QProcess();
        QString cmd =QString("/usr/bin/ps -p %1 o comm=").arg(pid);
        process->start(cmd);
        process->waitForFinished();
        QString application = QString(process->readAll()).replace("\n","");

        QString iconName = "application-x-executable";
        /* 通过应用名获取.desktop,再获取iconName,linkto bug#157362 【文件管理器】优盘占用提示弹窗中正在占用卷的应用程序图标显示为齿轮  */
        auto results = g_desktop_app_info_search(application.toUtf8().constData());
        if(results){
            for (int i = 0; results[i]; i++)
            {
                QString desktopFileName = *results[i];
                if("wps" == application){
                    desktopFileName = "wps-office-wps.desktop";
                }else if("explor" == application){
                    desktopFileName = "explor.desktop";
                }
                QString fileName = QString("/usr/share/applications/").append(desktopFileName);
                g_autoptr (GDesktopAppInfo) desktop_app_info = g_desktop_app_info_new_from_filename(fileName.toUtf8().constData());
                if (desktop_app_info) {
                    auto app_info = G_APP_INFO(desktop_app_info);
                    GIcon *icon = g_app_info_get_icon(app_info);
                    iconName = Peony::FileUtils::getIconStringFromGIcon(icon);
                }
                qDebug()<<application<<desktopFileName<<iconName;
                g_strfreev (results[i]);
                break;
            }
            g_free (results);
        }//end

        if(application=="bash")
            iconName = "utilities-terminal";

        if (application == "ffmpeg") {
            QProcess p;
            p.start(QString("/usr/bin/kill -9 %1").arg(pid));
            p.waitForFinished(-1);
            p.close();
        }

        auto icon = QIcon::fromTheme(iconName);
        occupiedAppMap.insert(std::pair<QString, QIcon>(application,icon));
        qDebug()<<application<<pid;
    }

    auto thread = static_cast<GetOccupiedAppsInfoThread *>(user_data);
    thread->signal_occupiedAppInfo(occupiedAppMap, message);
    g_mount_operation_reply(thread->getMountOp(), G_MOUNT_OPERATION_ABORTED);/* 解决一直弹出信息框问题，link to issue#I9VOWE. */
}

GMountOperation *GetOccupiedAppsInfoThread::getMountOp() const
{
    return m_mountOp;
}

#ifdef LINGMO_UDF_BURN
#include <libkyudfburn/disccontrol.h>
#include "ky-udf-format-dialog.h"
#include "udfAppendBurnDataDialog.h"

UdfBurn::UdfFormatDialogWrapper::UdfFormatDialogWrapper(const QString &uri, UdfBurn::DiscControl *discControl, QWidget *parent)
{
    m_dialog = new UdfFormatDialog(uri, discControl, parent);
}

UdfBurn::UdfFormatDialogWrapper::~UdfFormatDialogWrapper()
{
    delete m_dialog;
}

void UdfBurn::UdfFormatDialogWrapper::raise()
{
    m_dialog->raise();
}

void UdfBurn::UdfFormatDialogWrapper::show()
{
    m_dialog->show();
}

#endif


