/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */
#include "computer-volume-item.h"
#include <explor-qt/volumeManager.h>
#include <explor-qt/file-utils.h>
#include <explor-qt/datacdrom.h>
#include <udisks/udisks.h>
#include <sys/stat.h>
#include <explor-qt/global-fstabdata.h>
#include <explor-qt/sync-thread.h>
#include "computer-model.h"
#include "computer-user-share-item.h"
#include <QMessageBox>
#include <QDebug>
#include <QApplication>
#include <QPushButton>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDir>
#include <QStringList>
#include <QFileInfoList>

#ifdef signals
#undef signals
#endif

#include <gio/gunixmounts.h>

ComputerVolumeItem::ComputerVolumeItem(GVolume *volume, ComputerModel *model, AbstractComputerItem *parentNode, QObject *parent) : AbstractComputerItem(model, parentNode, parent)
{
    m_model->beginInsertItem(parentNode->itemIndex(), parentNode->m_children.count());
    parentNode->m_children<<this;

    if (parentNode->itemType() != Volume) {
        m_displayName = tr("Volume");
        m_model->endInsterItem();
        return;
    }

    m_cancellable = g_cancellable_new();
    if (!volume) {
        m_icon = QIcon::fromTheme("drive-harddisk-system");
        m_uri = "file:///";
        m_displayName = tr("File System");
        auto file = g_file_new_for_uri("file:///");
        g_file_query_filesystem_info_async(file, "*", 0, m_cancellable,
                                           GAsyncReadyCallback(query_root_info_async_callback), this);
        m_model->endInsterItem();
        return;
    }

    m_volume = std::make_shared<Peony::Volume>(volume, true);
    m_volumeChangedHandle = g_signal_connect(volume, "changed", G_CALLBACK(volume_changed_callback), this);
    m_volumeRemovedHandle = g_signal_connect(volume, "removed", G_CALLBACK(volume_removed_callback), this);

    m_volumeMonitor = g_volume_monitor_get();
    m_mountChangedHandle = g_signal_connect(m_volumeMonitor, "mount_changed", G_CALLBACK(mount_changed_callback), this);
    m_mountAddedHandle = g_signal_connect(m_volumeMonitor, "mount_added", G_CALLBACK(mount_added_callback), this);

    updateInfoAsync();

    m_model->endInsterItem();
}

//ComputerVolumeItem::ComputerVolumeItem(const QString uri,ComputerModel *model,AbstractComputerItem *parentNode,QObject *parent) : AbstractComputerItem(model,parentNode,parent){
//    if(uri.isNull() || uri.isEmpty())
//        return;

//    //collectInfoWhenGpartedOpen(uri);
//}

ComputerVolumeItem::~ComputerVolumeItem()
{
    if(m_volumeMonitor){
        g_signal_handler_disconnect(g_volume_monitor_get(), m_mountChangedHandle);
        g_signal_handler_disconnect(g_volume_monitor_get(), m_mountAddedHandle);
    }
    if(m_volume){
        g_signal_handler_disconnect(m_volume->getGVolume(), m_volumeChangedHandle);
        g_signal_handler_disconnect(m_volume->getGVolume(), m_volumeRemovedHandle);
    }
    g_cancellable_cancel(m_cancellable);
    g_object_unref(m_cancellable);

    if(m_watcher){
        m_watcher->stopMonitor();
        delete m_watcher;
    }
}

void ComputerVolumeItem::updateInfoAsync()
{
    if (!m_volume) {
        m_icon = QIcon::fromTheme("drive-harddisk-system");
        m_uri = "file:///";
        m_displayName = tr("File System");
        auto file = g_file_new_for_uri("file:///");
        g_file_query_filesystem_info_async(file, "*", 0, m_cancellable,
                                           GAsyncReadyCallback(query_root_info_async_callback), this);
        return;
    }

    char *deviceName;

    m_displayName = m_volume->name();
    //Handle the Chinese name of fat32 udisk.
    deviceName = g_volume_get_identifier(m_volume->getGVolume(),G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
    if(deviceName){
        m_unixDeviceName = QString(deviceName);
        qDebug()<<"unix Device Name"<< m_unixDeviceName;
        Peony::FileUtils::handleVolumeLabelForFat32(m_displayName, m_unixDeviceName);
        g_free(deviceName);
    }

    //fix u-disk show as hard-disk icon issue, task#25343
    updateBlockIcons();
    //qDebug()<<m_displayName;

    check();

    auto mount = g_volume_get_mount(m_volume->getGVolume());
    if (mount) {
        m_mount = std::make_shared<Peony::Mount>(mount, true);
        auto active_root = g_mount_get_root(mount);
        if (active_root) {
            auto uri = g_file_get_uri(active_root);
            if (uri) {
                m_uri = uri;
                g_free(uri);
            }
            if (m_uri == "file:///data") {
                if (Peony::FileUtils::isFileExsit("file:///data/usershare")) {
                    m_isHidden = true;
                }
            }
            g_file_query_filesystem_info_async(active_root, "*",
                                               0, m_cancellable, GAsyncReadyCallback(qeury_info_async_callback), this);
            g_object_unref(active_root);
        }
    } else {
        //m_mount = nullptr;
        //mount first
        //FIXME: check auto mount
//        this->mount();
    }

    bool isData = false;
    Peony::GlobalFstabData *globalFstabData = Peony::GlobalFstabData::getInstance();
    if(!globalFstabData->getUuidState()){
        if(globalFstabData->isMountPoints(m_unixDeviceName.toUtf8(), "/data")){
            isData = true;
        }
    }else{
        if(globalFstabData->isMountPoints(getDeviceUUID(m_unixDeviceName.toUtf8()), "/data")){
            isData = true;
        }
    }

    if(m_uri == "file:///data" || isData){
        m_displayName = tr("Data");
    }

    auto index = this->itemIndex();
    m_model->dataChanged(index, index);
    m_model->invalidateRequest();
}

const QString ComputerVolumeItem::displayName()
{
    return m_displayName;
}

const QIcon ComputerVolumeItem::icon()
{
    return m_icon.isNull()? AbstractComputerItem::icon(): m_icon;
}

bool ComputerVolumeItem::isMount()
{
    return Peony::FileUtils::isMountPoint(m_uri);
}

void ComputerVolumeItem::findChildren()
{
    //add root
    auto root = new ComputerVolumeItem(nullptr, m_model, this);

    //enumerate
    auto volume_monitor = g_volume_monitor_get();
    auto current_volumes = g_volume_monitor_get_volumes(volume_monitor);
    GList *l = current_volumes;
    while (l) {
        auto volume = G_VOLUME(l->data);
        auto item = new ComputerVolumeItem(volume, m_model, this);
        l = l->next;
    }

    //comment gparted process code to fix duplicated volume issue, bug#41623
//    if(!current_volumes)
//        findChildrenWhenGPartedOpen();

    //monitor
    auto volumeManager = Peony::VolumeManager::getInstance();
    connect(volumeManager, &Peony::VolumeManager::volumeAdded, this, &ComputerVolumeItem::onVolumeAdded);

    //watcher
//    m_watcher = new Peony::FileWatcher("computer:///",this);
//    connect(m_watcher, &Peony::FileWatcher::fileCreated, this, &ComputerVolumeItem::onFileAdded);
//    connect(m_watcher, &Peony::FileWatcher::fileDeleted, this, &ComputerVolumeItem::onFileRemoved);
//    m_watcher->startMonitor();

    if (Peony::FileUtils::isFileExsit("file:///data/usershare")) {
        new ComputerUserShareItem(nullptr, m_model, this);
    }
}

void ComputerVolumeItem::check()
{
    if (!m_volume)
        return;

    auto active_root = g_volume_get_activation_root(m_volume->getGVolume());
    if (active_root) {
        auto uri = g_file_get_uri(active_root);
        auto path = g_file_get_path(active_root);
        if (QString(uri) == "file:///data") {
            if (Peony::FileUtils::isFileExsit("file:///data/usershare")) {
                m_isHidden = true;
            }
        }
        //QMessageBox::information(0, 0, QString("%1 has active root %2").arg(m_displayName).arg(uri));
        if (uri) {
            /*!
              \bug
              when a volume added, it could have active root represond with a uri,
              such as mtp://, etc...
              however it could not get the local path of active root correctly at the
              time. that makes some volumes location changement meet troubles.
              */
            m_uri = uri;

            g_free(uri);
        }

        if (path) {
            //try to fix bug#174666, part phone mtp mode access wrong issue
            qDebug() << "use path update uri:"<<path<<uri;
            if (! QString(path).startsWith("mtp://") && !QString(path).startsWith("gphoto2://"))
               m_uri = QString("file://%1").arg(path);

            g_free(path);
        }
        g_object_unref(active_root);
    }

    if (m_uri.isNull()) {
        GFile* file = nullptr;
        GMount* mount = g_volume_get_mount(m_volume->getGVolume());
        if (nullptr != mount) {
            file = g_mount_get_root (mount);
            if (nullptr != file) {
                m_uri = g_file_get_uri(file);
            }
        }
        if (nullptr != file) {
            g_object_unref(file);
        }
        if (nullptr != mount) {
            g_object_unref (mount);
        }
    }
}

bool ComputerVolumeItem::canEject()
{
    GVolume *gvolume;
    GDrive  *gdrive;
    bool ejectAble = false;

    if("file:///" == m_uri || m_volume == nullptr)    /*The root File System cannot eject*/
        return false;

    if(NULL != m_volume->getGVolume()){
        gvolume = (GVolume*)g_object_ref(m_volume->getGVolume());
        gdrive = g_volume_get_drive(gvolume);
        if(gdrive){
            qDebug() <<"uri ejectAble:"<<m_uri<<g_drive_can_eject(gdrive)<<g_drive_can_stop(gdrive)<<g_drive_is_removable(gdrive);
            //try to fix bug#149182, SD card option not the same with side bar menu
            ejectAble = g_drive_can_eject(gdrive) || g_drive_can_stop(gdrive) /*|| g_drive_is_removable(gdrive)*/;
            g_object_unref(gdrive);
        }
        g_object_unref(gvolume);
    }

    return ejectAble;
}

void ComputerVolumeItem::eject(GMountUnmountFlags ejectFlag)
{
    GVolume *g_volume;
    GMount *g_mount;
    QString ejectNotify = QObject::tr("It need to synchronize before operating the device,place wait!");
    g_autoptr(GMountOperation) mount_op = g_mount_operation_new();

    /*If udisk device is mounted,eject it from here*/
    if (m_mount && (g_mount = m_mount->getGMount())) {
        if (g_mount_can_eject(g_mount) && !m_unixDeviceName.startsWith("/dev/sd")) {/* 由于要显示异常U盘，U盘弹出用stop */
            Peony::SyncThread::notifyUser(ejectNotify);
            g_mount_eject_with_operation(g_mount, ejectFlag, mount_op, m_cancellable,
                                         GAsyncReadyCallback(eject_async_callback), this);
        } else {
            auto g_drive = g_mount_get_drive(g_mount);
            if (!g_drive)
                return;

            if (g_drive_can_stop(g_drive)|| (g_drive_is_removable(g_drive) && !m_unixDeviceName.startsWith("/dev/mmc"))){// for mobile harddisk.
                /* 加"(g_drive_is_removable(m_drive) && !m_device.startsWith("/dev/mmc"))"这个判断是为了解决bug#184111和bug#149182；有些U盘的can-stop为false,其中为一款sd卡的devicename */
                Peony::SyncThread::notifyUser(ejectNotify);
                g_drive_stop(g_mount_get_drive(g_mount), ejectFlag, mount_op, nullptr, GAsyncReadyCallback(stop_async_callback), this);/*m_cancellable改为nullptr，linkto bug145072【HWE】弹出成功后提示操作被取消 */
            }else if(g_drive_is_removable(g_drive)){
                //fix bug#141782, SD card eject can not recgonize issue
                g_mount_eject_with_operation(g_mount, ejectFlag, mount_op, m_cancellable,
                                             GAsyncReadyCallback(eject_async_callback), this);
            }

            g_object_unref(g_drive);
        }

        return;
    }

    /*If udisk device is unmounted,eject it from here*/
    if (m_volume && (g_volume = m_volume->getGVolume())) {
        if (g_volume_can_eject(g_volume)) {
            Peony::SyncThread::notifyUser(ejectNotify);
            g_volume_eject_with_operation(g_volume, ejectFlag, mount_op, m_cancellable,
                                         GAsyncReadyCallback(eject_async_callback), this);
        } else {
            auto g_drive = g_mount_get_drive(g_mount);
            if (!g_drive)
                return;

            if (g_drive_can_stop(g_drive)){
                Peony::SyncThread::notifyUser(ejectNotify);
                g_drive_stop(g_mount_get_drive(g_mount), ejectFlag, mount_op, nullptr, GAsyncReadyCallback(stop_async_callback), this);/*m_cancellable改为nullptr，linkto bug145072【HWE】弹出成功后提示操作被取消 */
            }

            g_object_unref(g_drive);
        }
    }
}

bool ComputerVolumeItem::canUnmount()
{
    if(m_uri.endsWith(".mount") || m_uri.endsWith(".volume"))
        return true;
    if("file:///data" == m_uri)
        return false;
    return m_mount != nullptr;
}

void ComputerVolumeItem::unmount(GMountUnmountFlags unmountFlag)
{
    GMount *g_mount = nullptr;
    GFile *file = nullptr;

    g_autoptr(GMountOperation) mount_op = g_mount_operation_new();

    m_vfs_uri = m_model->m_volumeTargetMap.key(m_uri);
    QString unmountNotify = QObject::tr("It need to synchronize before operating the device,place wait!");
    if (!m_vfs_uri.isEmpty()) {
        file = g_file_new_for_uri(m_vfs_uri.toUtf8().constData());
        if(file){
            Peony::SyncThread::notifyUser(unmountNotify);
            g_file_unmount_mountable_with_operation(file,unmountFlag,
                                                    mount_op,nullptr,
                                                    GAsyncReadyCallback(unmount_async_callback),
                                                    this);
        }
        g_object_unref(file);
    } else if (m_mount) {
        if (g_mount = m_mount->getGMount()){
            Peony::SyncThread::notifyUser(unmountNotify);
                g_mount_unmount_with_operation(g_mount, unmountFlag, mount_op, m_cancellable,
                                           GAsyncReadyCallback(unmount_async_callback), this);
        }
    } else if (!m_uri.isEmpty()) {
        file = g_file_new_for_uri(m_uri.toUtf8().constData());
        if(file){
            Peony::SyncThread::notifyUser(unmountNotify);
            g_file_unmount_mountable_with_operation(file,unmountFlag,
                                                    mount_op,nullptr,
                                                    GAsyncReadyCallback(unmount_async_callback),
                                                    this);
        }
        g_object_unref(file);
    }
}

void ComputerVolumeItem::mount()
{
    if (m_uri == "file:///") {
        return;
    }

    if (m_mount) {
        auto root = g_mount_get_root(m_mount->getGMount());
        if (root) {
            auto uri = g_file_get_uri(root);
            if (uri) {
                m_uri = uri;
                g_free(uri);
            }
            g_file_query_filesystem_info_async(root, "*",
                                               0, m_cancellable, GAsyncReadyCallback(qeury_info_async_callback), this);
            g_object_unref(root);
        }
    } else {
        g_autoptr (GMountOperation) op = g_mount_operation_new();
        g_volume_mount(m_volume->getGVolume(),
                       G_MOUNT_MOUNT_NONE,
                       op,
                       m_cancellable,
                       GAsyncReadyCallback(mount_async_callback),
                       this);
    }
}

QModelIndex ComputerVolumeItem::itemIndex()
{
    if (!m_parentNode) {
        return m_model->createItemIndex(0, this);
    } else {
        return m_model->createItemIndex(m_parentNode->m_children.indexOf(this), this);
    }
}

bool ComputerVolumeItem::isHidden()
{
    return m_isHidden;
}

QString ComputerVolumeItem::getDeviceUUID(const char *device)
{
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

void ComputerVolumeItem::volume_changed_callback(GVolume *volume, ComputerVolumeItem *p_this)
{
    //QMessageBox::information(0, 0, tr("Volume Changed"));
    if(!p_this)
        return;
    p_this->m_mount = nullptr;
    p_this->m_uri = nullptr;
    p_this->m_icon = QIcon();
    p_this->m_displayName = nullptr;
    p_this->m_usedSpace = 0;
    p_this->m_totalSpace = 0;
    p_this->updateInfo();
}

void ComputerVolumeItem::volume_removed_callback(GVolume *volume, ComputerVolumeItem *p_this)
{
    auto parentNode = p_this->m_parentNode;
    if (!parentNode)
        return;

    auto row = parentNode->m_children.indexOf(p_this);
    parentNode->m_model->beginRemoveItem(parentNode->itemIndex(), row);
    parentNode->m_children.removeAt(row);
    p_this->deleteLater();
    parentNode->m_model->endRemoveItem();
}

void ComputerVolumeItem::mount_changed_callback(GVolumeMonitor *volumeMonitor, GMount *gmount, ComputerVolumeItem *p_this)
{
    if (p_this){
        p_this->m_usedSpace = 0;
        p_this->m_totalSpace = 0;
        p_this->updateInfo();
        qDebug()<<"mount changed uri: "<<p_this->uri();
    }
}

void ComputerVolumeItem::mount_added_callback(GVolumeMonitor *volumeMonitor, GMount *gmount, ComputerVolumeItem *p_this)
{
    if (p_this){
        p_this->updateInfo();
        qDebug()<<"mount added uri: "<<p_this->uri();
    }
}

QString iconFileFromMountpoint(const QString& mountpoint){
    bool isReadOnly;
    QDir mountDir;
    QString iconPath;
    GUnixMountEntry* entry;

    if(mountpoint.isEmpty())
        return iconPath;

    entry = g_unix_mount_for(mountpoint.mid(7).toUtf8().constData(),NULL);
    if(entry){
       isReadOnly = g_unix_mount_is_readonly(entry);
       g_unix_mount_free(entry);
       if(!isReadOnly)//is not a boot disk
           return iconPath;
    }

    mountDir.setPath(mountpoint.mid(7));//remove 'file://'
    if(mountDir.exists()){
        QStringList filters;
        filters << "*.ico";
        mountDir.setNameFilters(filters);

        QFileInfoList list = mountDir.entryInfoList();
        if(0 != list.length())
            iconPath = list.at(0).absoluteFilePath();
	}

    return iconPath;
}

bool isMountReadOnly(const QString& mountpoint)
{
    bool isReadOnly = false;
    GUnixMountEntry* entry;

    if(mountpoint.isEmpty() || mountpoint.length() < 8)
        return false;

    entry = g_unix_mount_for(mountpoint.mid(7).toUtf8().constData(),NULL);
    if(entry){
       isReadOnly = g_unix_mount_is_readonly(entry);
       g_unix_mount_free(entry);
    }

    qDebug() << "isMountReadOnly:"<<mountpoint<<isReadOnly;
    return isReadOnly;
}

void ComputerVolumeItem::qeury_info_async_callback(GFile *file, GAsyncResult *res, ComputerVolumeItem *p_this)
{
    GError *err = nullptr;
    auto info = g_file_query_filesystem_info_finish(file, res, &err);
    if (info) {
        /*
        *由于对DVD+RW或者是DVD-RW类型的光盘，无法通过cdromdata类获取到使用的容量，所以使用的容量统一使用gio函数获取
        */
        quint64 used = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_USED);
        bool bNotDisk = true;
        //ignore disk update when the disk deviece is busy,related to bug#143293
        if (p_this->m_unixDeviceName.startsWith("/dev/sr") && ! Peony::FileUtils::isBusyDevice(p_this->m_unixDeviceName)) {
            Peony::DataCDROM *cdrom = new Peony::DataCDROM(p_this->m_unixDeviceName);
            if (cdrom) {
                cdrom->getCDROMInfo();
                //p_this->m_usedSpace = used;
                //used无法正确获取追加刻录后光盘的使用容量，getCDROMUsedCapacity()无法获取可擦除光盘的使用容量
                p_this->m_usedSpace = cdrom->getCDROMUsedCapacity();
                if((cdrom->getCDROMType()).contains("DVD+RW") || (cdrom->getCDROMType()).contains("DVD-RW")){
                       p_this->m_usedSpace  = used;
                }
                p_this->m_totalSpace = cdrom->getCDROMCapacity();
                delete cdrom;
                cdrom = nullptr;
                bNotDisk = false;
            }
        }

        //fix block not update volume issue, link to bug#63326
        if (bNotDisk || 0 == p_this->m_totalSpace) {
            quint64 total = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
            quint64 free = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_FREE);
            char *fs_type = g_file_info_get_attribute_as_string(info, G_FILE_ATTRIBUTE_FILESYSTEM_TYPE);
            QString type(fs_type);
            if (total > 0 && (used > 0 || free > 0)) {
                if (used > 0 && used <= total) {
                    p_this->m_usedSpace = used;
                    p_this->m_totalSpace = total;
                } else if(free > 0 && free <= total) {
                    p_this->m_usedSpace = total - free;
                    p_this->m_totalSpace = total;
                }

                p_this->updateBlockIcons();
             }
            if (type.contains("ext")) {
                p_this->m_usedSpace = total - free;
            }
         }

        qWarning()<<"udisk name"<<p_this->m_volume->name();
        qWarning()<<"udisk used space"<<p_this->m_usedSpace;
        qWarning()<<"udisk total space"<<p_this->m_totalSpace;
        /***************************collect info when gparted open*************************/
//        if(p_this->m_icon.name().isEmpty()){
//            QString iconName = Peony::FileUtils::getFileIconName(p_this->m_uri);
//            if(iconName.isNull())               /*Some startup disks cannot get the icon*/
//                iconName = "drive-harddisk-usb";
//            p_this->m_icon = QIcon::fromTheme(iconName);
//        }
//        if(p_this->m_displayName.isEmpty()){
//            p_this->m_displayName = Peony::FileUtils::getFileDisplayName(p_this->m_uri);

//            if(!p_this->m_targetUri.isEmpty()){
//               char *realMountPoint = g_filename_from_uri(p_this->m_targetUri.toUtf8().constData(),NULL,NULL);
//               const char *unixDev = Peony::VolumeManager::getUnixDeviceFileFromMountPoint(realMountPoint);
//               QString unixDeviceName = unixDev;
//               Peony::FileUtils::handleVolumeLabelForFat32(p_this->m_displayName,unixDeviceName);
//               g_free(realMountPoint);
//            }
//        }
        /**********************************************************************************/

        auto index = p_this->itemIndex();
        p_this->m_model->dataChanged(index, index);
        //p_this->m_model->dataChanged(p_this->itemIndex(), p_this->itemIndex());

        g_object_unref(info);
    }
    if (err) {
        //QMessageBox::critical(0, 0, err->message);
        g_error_free(err);
    }
}

void ComputerVolumeItem::query_root_info_async_callback(GFile *file, GAsyncResult *res, ComputerVolumeItem *p_this)
{
    GError *err = nullptr;
    GFileInfo *info = g_file_query_info_finish(file, res, &err);
    if (info) {
        quint64 total = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
        quint64 used = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_USED);
        quint64 available = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_FILESYSTEM_FREE);
        char *fs_type = g_file_info_get_attribute_as_string(info, G_FILE_ATTRIBUTE_FILESYSTEM_TYPE);
        QString type(fs_type);
        if(type.contains("ext")) {
            used = total - available;
        }
        p_this->m_totalSpace = total;
        p_this->m_usedSpace = used;
        auto index = p_this->itemIndex();
        p_this->m_model->dataChanged(index, index);
        //p_this->m_model->dataChanged(p_this->itemIndex(), p_this->itemIndex());

        g_object_unref(info);
    }
    if (err) {
        //QMessageBox::critical(0, 0, err->message);
        g_error_free(err);
    }
}

void ComputerVolumeItem::mount_async_callback(GVolume *volume, GAsyncResult *res, ComputerVolumeItem *p_this)
{
    GError *err = nullptr;
    bool successed = g_volume_mount_finish(volume, res, &err);
    if (err) {
        //QMessageBox::critical(0, 0, err->message);
        if (g_error_matches(err, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED)) {
            Experimental_Peony::Volume v(volume);
            v.mount();
        }
        g_error_free(err);
    }

    if (successed) {
        QString unmountNotify = QObject::tr("The device has been mount successfully!");
        Peony::SyncThread::notifyUser(unmountNotify);
        p_this->updateInfoAsync();
    }
}

void ComputerVolumeItem::unmount_async_callback(GObject* object,GAsyncResult *res,ComputerVolumeItem *p_this)
{
    GError *err = nullptr;
    QString errorMsg;
    bool successed;

    if(G_IS_MOUNT(object)){
        if(successed)
             p_this->m_mount = nullptr;
        successed = g_mount_unmount_with_operation_finish(G_MOUNT(object),res,&err);
    }else if(G_IS_FILE(object)){
        successed = g_file_unmount_mountable_with_operation_finish(G_FILE(object),res,&err);
    }

    if(successed){
        QString unmountNotify = QObject::tr("Data synchronization is complete,the device has been unmount successfully!");
        Peony::SyncThread::notifyUser(unmountNotify);
    }

    if(err){
        errorMsg = err->message;
        if(strstr(err->message,"target is busy")){
            errorMsg = QObject::tr("One or more programs prevented the unmount operation.");
            QMessageBox::warning(nullptr, QObject::tr("Unmount failed"), QObject::tr("Error: %1\n").arg(errorMsg), QMessageBox::Yes);
        }else if(strstr(err->message,"umount: /media/")){
            //chinese name need to be converted,this may be a error that from glib2/gio2.
            errorMsg = QObject::tr("Unable to unmount it, you may need to close some programs, such as: GParted etc.");
            QMessageBox::warning(nullptr, QObject::tr("Unmount failed"), QObject::tr("%1").arg(errorMsg), QMessageBox::Yes);
        } else if (err->code == G_IO_ERROR_PERMISSION_DENIED || errorMsg.contains("authorized", Qt::CaseInsensitive)) {
            // do nothing because we have requested polkit dialog yet.
        } else {
            QMessageBox::warning(nullptr, QObject::tr("Unmount failed"), QObject::tr("Error: %1\n").arg(err->message), QMessageBox::Yes);
        }

        g_error_free(err);
    }
}

void ComputerVolumeItem::eject_async_callback(GObject *object, GAsyncResult *res, ComputerVolumeItem *p_this)
{
    GError *err = nullptr;
    bool successed;

    if(G_IS_MOUNT(object))
        successed = g_mount_eject_with_operation_finish(G_MOUNT(object), res, &err);
    else if(G_IS_VOLUME(object))
        successed = g_volume_eject_with_operation_finish(G_VOLUME(object), res, &err);

    if (successed) {
        //QMessageBox::information(0, 0, "Volume Ejected");
        QString unmountNotify = QObject::tr("Data synchronization is complete,the device has been unmount successfully!");
        Peony::SyncThread::notifyUser(unmountNotify);
    }
    if (err) {
        qDebug()<<"error code of drive stop:"<<err->code<<", error message:"<<err->message;
        QString errMsg = err->message;
        if (!errMsg.contains("authorized", Qt::CaseInsensitive)) {
            //fix bug#104482, pull device immediately when eject it, error message not translated issue
            if (err->code == G_IO_ERROR_FAILED){
                errMsg = QObject::tr("Eject device failed, the reason may be that the device has been removed, etc.");
            }
            QMessageBox warningBox(QMessageBox::Warning, QObject::tr("Eject failed"), errMsg, QMessageBox::Ok);
            warningBox.exec();
        }

        g_error_free(err);
    }
}

void ComputerVolumeItem::stop_async_callback(GDrive *drive, GAsyncResult *res, ComputerVolumeItem *p_this)
{
    GError *err = nullptr;
    bool successed;

    successed = g_drive_stop_finish(drive, res, &err);
    if (!successed) {
        qDebug()<<"error code of drive stop:"<<err->code<<", error message:"<<err->message;
        if(!strcmp(err->message, "Not authorized to perform operation")){/* gmountOperation会弹出授权框，防止二次弹框,linkto bug#101075 */
            g_error_free(err);
            return;
        }

        //fix bug#104482, pull device immediately when eject it, error message not translated issue
        QString errMessage = err->message;
        if (err->code == G_IO_ERROR_FAILED){
            errMessage = QObject::tr("Eject device failed, the reason may be that the device has been removed, etc.");
        }
        QMessageBox warningBox(QMessageBox::Warning, QObject::tr("Eject failed"), errMessage, QMessageBox::Ok);
        warningBox.exec();
        g_error_free(err);
    } else {
        QString ejectNotify = QObject::tr("Data synchronization is complete,the device has been unmount successfully!");
        Peony::SyncThread::notifyUser(ejectNotify);
    }
}

void ComputerVolumeItem::onVolumeAdded(const std::shared_ptr<Peony::Volume> volume)
{
    auto g_volume = volume->getGVolume();
    auto item = new ComputerVolumeItem(g_volume, m_model, this);
}

/*
 * 根据反馈的bug做逻辑调整：
 * 1.默认使用 m_volume->iconName() 图标；
 * 2.只读系统，系统盘优先使用gmount icon，关联bug#120459
 * 3.需要区分部分型号的U盘显示成移动硬盘图标问题，关联bug#182809, #57660, #70014 #96652
 * 4.在Windows系统使用工具，比如软碟通制作的系统盘，不是只读格式，获取iconName后缀带'.ico'做判断
 * */
void ComputerVolumeItem::updateBlockIcons()
{
    //qDebug() << "updateBlockIcons volume icon:"<<m_volume->iconName()<<m_uri;
    m_icon = QIcon::fromTheme(m_volume->iconName());
    //fix bug#182809, U disk show mobile-SSD icon issue, only readonly device use gmount icon
    bool isReadOnlyDevice = isMountReadOnly(m_uri);
    if(m_mount && !m_mount->iconName().isEmpty()){/* 优先使用gmount的icon，hotfix bug#120459. */
        //只读系统或者带系统盘图标，使用gmount的icon
        if (isReadOnlyDevice || m_mount->iconName().endsWith(".ico")){
            m_icon = QIcon::fromTheme(m_mount->iconName());
            qDebug() << "updateBlockIcons mount icon readonly:"<<m_mount->iconName();
            return;
        }
    }

    //fix bug#182809, U disk show mobile-SSD icon issue, only readonly device use gmount icon
    if (m_volume->iconName() == "drive-harddisk-usb"){
        //用设备容量区分U盘和移动硬盘，大于128GiB为移动硬盘
        //FIXME with a better solution, fix bug#57660, #70014 #96652
        GDrive* gdrive = g_volume_get_drive(m_volume->getGVolume());
        QString tmpDevice;
        double size = 0.0;
        if(gdrive){
            g_autofree char* gdevice = g_drive_get_identifier(gdrive, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
            tmpDevice = gdevice;
            g_object_unref(gdrive);
        }

        if(!tmpDevice.isEmpty()){
            size = Peony::FileUtils::getDeviceSize(tmpDevice.toUtf8().constData());
        }
        if (m_totalSpace/(1024 * 1024 *1024) > 128 || size > 128)
            m_icon = QIcon::fromTheme("drive-harddisk-usb");
        else
            m_icon = QIcon::fromTheme("drive-removable-media-usb");
    }
}

/* Usage scenarios:
 *   -> open Peony first,then open Gparted.
 *   -> when computer:///xxx.mount or computer:///xxx.volume is generated,
 *      it will query their information from here.
 */
//comment gparted process code to fix duplicated volume issue, bug#41623
//void ComputerVolumeItem::collectInfoWhenGpartedOpen(QString uri)
//{
//    GFile *file = NULL;

//    m_uri = uri;
//    m_displayName = "";
//    m_icon = QIcon();
//    m_cancellable = g_cancellable_new();
//    m_targetUri = Peony::FileUtils::getTargetUri(m_uri);
//    file = g_file_new_for_uri(m_targetUri.toUtf8().constData());

//    g_file_query_filesystem_info_async(file,"*",0,
//                            m_cancellable,
//                            GAsyncReadyCallback(qeury_info_async_callback),this);
//    g_object_unref(file);
//}

/* monitor file generation for computer:///xxx.mount or computer:///xxx.volume
 */
//void ComputerVolumeItem::onFileAdded(const QString &uri)
//{
//    QString targetUri = Peony::FileUtils::getTargetUri(uri);
//    if(!targetUri.contains("file:///") || targetUri.isEmpty())
//        return;

//    for (auto item : m_children) {
//        qDebug() << "ComputerVolumeItem onFileAdded uri:"<<uri<<item->uri();
//        if (item->uri() == uri)
//            return;
//    }

//    m_model->beginInsertItem(itemIndex(), m_children.count());
//    auto item = new ComputerVolumeItem(uri, m_model, this);
//    m_children<<item;
//    m_model->endInsterItem();
//}

//void ComputerVolumeItem::onFileRemoved(const QString &uri)
//{
//    int row = -1;
//    for (auto item : m_children) {
//        qDebug() << "ComputerVolumeItem onFileRemoved uri:"<<uri<<item->uri();
//        if (item->uri() == uri) {
//            row = m_children.indexOf(item);
//            break;
//        }
//    }

//    if (row < 0)
//        return;

//    m_model->beginRemoveItem(itemIndex(), row);
//    auto item = m_children.takeAt(row);
//    item->deleteLater();
//    m_model->endRemoveItem();
//}

/* Usage scenarios:
 *   -> open Gparted first,then open Peony.
 *   -> when Gparted is open,it is used to find all partition devices
 *      except the root partition.
 */
//void ComputerVolumeItem::findChildrenWhenGPartedOpen()
//{
//    GFile *file;

//    file = g_file_new_for_uri("computer:///");
//    m_tmpCancellable = g_cancellable_new();
//    g_file_enumerate_children_async(file, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, 0,
//                                    m_tmpCancellable, GAsyncReadyCallback(enumerate_async_callback), this);
//    //g_object_unref(file);
//}

//void ComputerVolumeItem::enumerate_async_callback(GFile *file, GAsyncResult *res, ComputerVolumeItem *p_this)
//{
//    GError *err = nullptr;
//    auto enumerator = g_file_enumerate_children_finish(file, res, &err);
//    if (enumerator) {
//        g_file_enumerator_next_files_async(enumerator, 9999, 0, p_this->m_tmpCancellable,
//                                           GAsyncReadyCallback(find_children_async_callback), p_this);
//    }
//    if (err)
//        g_error_free(err);
//}

//void ComputerVolumeItem::find_children_async_callback(GFileEnumerator *enumerator, GAsyncResult *res, ComputerVolumeItem *p_this)
//{
//    GError *err = nullptr;
//    auto infos = g_file_enumerator_next_files_finish(enumerator, res, &err);
//    GList *l = infos;
//    while (l) {
//        auto info = G_FILE_INFO(l->data);
//        l = l->next;
//        if (!info)
//            continue;

//        auto file = g_file_enumerator_get_child(enumerator, info);
//        if (!file)
//            continue;

//        auto uri = g_file_get_uri(file);
//        if (!uri)
//            continue;

//        //not include ftp://xxx etc.
//        QString targetUri;
//        targetUri = Peony::FileUtils::getTargetUri(uri);
//        if(targetUri.isEmpty())
//            continue;
//        if("file:///" == targetUri)
//            continue;
//        if(!targetUri.contains("file:///"))
//            continue;

//        p_this->m_model->beginInsertItem(p_this->itemIndex(), p_this->m_children.count());
//        auto item = new ComputerVolumeItem(uri, p_this->m_model, p_this);
//        qDebug() << "find_children_async_callback uri:" <<uri<<targetUri;
//        p_this->m_children<<item;
//        p_this->m_model->endInsterItem();
//        g_free(uri);
//        g_object_unref(file);
//    }

//    if (infos)
//        g_list_free_full(infos, g_object_unref);

//    if (enumerator) {
//        g_file_enumerator_close(enumerator, nullptr, nullptr);
//        g_object_unref(enumerator);
//    }

//    if(p_this->m_tmpCancellable){
//        g_cancellable_cancel(p_this->m_tmpCancellable);
//        g_object_unref(p_this->m_tmpCancellable);
//    }

//    if (err) {
//        g_error_free(err);
//    }
//}

quint64 calcVolumeCapacity(ComputerVolumeItem* pThis)
{
    char* tmpDevice;
    GVolume* gVolume;
    quint64 capacityBytes;
    QString unixDevice,dbusPath;

    if(!pThis->m_mount && pThis->m_targetUri.isEmpty())
        return 0;

    if(pThis->m_mount){
        if(gVolume = pThis->m_volume->getGVolume()){
            tmpDevice = g_volume_get_identifier(gVolume,G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
            if (tmpDevice) {
                unixDevice = QString(tmpDevice+5);
                //fix crash issue
                if (tmpDevice)
                   g_free(tmpDevice);
            } else {
                unixDevice = Peony::FileUtils::getUnixDevice(pThis->m_uri);
                unixDevice = unixDevice.section('/',-1);
            }
        }
    }else{
        unixDevice = Peony::FileUtils::getUnixDevice(pThis->m_uri);
        unixDevice = unixDevice.section('/',-1);
    }

    if (unixDevice.isEmpty()) {
        return 0;
    }

    dbusPath = "/org/freedesktop/UDisks2/block_devices/" + unixDevice;
    QDBusInterface blockInterface("org.freedesktop.UDisks2",
                                  dbusPath,
                                  "org.freedesktop.UDisks2.Block",
                                  QDBusConnection::systemBus());

    capacityBytes = 0;
    if(blockInterface.isValid())
        capacityBytes = blockInterface.property("Size").toULongLong();

    return capacityBytes;
}
