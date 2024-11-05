/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include "intel-side-bar-file-system-item.h"
#include "sync-thread.h"
#include "file-info.h"
#include "file-utils.h"
#include "file-watcher.h"
#include "file-info-job.h"
#include "volume-manager.h"
#include "volumeManager.h"
#include "intel-side-bar-model.h"
#include "file-enumerator.h"
#include "gobject-template.h"
#include "linux-pwd-helper.h"
#include "intel-side-bar-separator-item.h"

#include <QIcon>
#include <QThread>
#include <QMessageBox>
#include <QPushButton>
#include <udisks/udisks.h>
#include <sys/stat.h>
#include <libnotify/notify.h>

using namespace Peony::Intel;
using namespace Peony;

void notifyUser(QString notifyContent);

SideBarFileSystemItem::SideBarFileSystemItem(QString uri,
        SideBarFileSystemItem *parentItem,
        SideBarModel *model,
        QObject *parent) : SideBarAbstractItem (model, parent)
{
    m_parent = parentItem;

    if (parentItem == nullptr) {
        m_is_root_child = true;
        m_uri = "computer:///";
        m_display_name = tr("Computer");
        //m_icon_name = "computer";
        //top dir don't show icon
        m_icon_name = "";

        //m_watcher->setMonitorChildrenChange();
        //connect(m_watcher.get(), &FileWatcher::fileChanged, [=]())
    } else {
        m_uri = uri;
        m_info = FileInfo::fromUri(uri);
        //FIXME: replace BLOCKING api in ui thread.
        if (m_info.get()->isEmptyInfo()) {
            FileInfoJob j(m_info);
            j.querySync();
        }
        m_display_name = FileUtils::getFileDisplayName(uri);
        m_icon_name = FileUtils::getFileIconName(uri);

        // display name is a read only attribute， so i change it here
        if (m_display_name == tr("文件系统")) {
            m_display_name = tr("System Disk");
        } else {
            FileUtils::queryVolumeInfo(m_uri, m_volume_name, m_unix_device, m_display_name);
        }
    }
}

QString SideBarFileSystemItem::displayName()
{
    QString displayName;
    if (!m_volume_name.isEmpty()) {
        displayName += m_volume_name;
    }
    if (!m_unix_device.isEmpty() && !m_uri.contains("root.link")) {
        displayName += QString(" (%1)").arg(m_unix_device);
    }
    if (!displayName.isEmpty())
        return displayName;

    return m_display_name;
}

QModelIndex SideBarFileSystemItem::firstColumnIndex()
{
    return m_model->firstColumnIndex(this);
}

QModelIndex SideBarFileSystemItem::lastColumnIndex()
{
    return m_model->lastColumnIndex(this);
}

void SideBarFileSystemItem::clearChildren()
{
    stopWatcher();
    SideBarAbstractItem::clearChildren();
}

/*!
 * \brief SideBarFileSystemItem::findChildren
 * \bug root doesn't support gvfs, so computer:/// cannot be enumerated.
 * to avoid the bug, I forbided find filesystem item children in root.
 * I should use another way to display the devices/volumes.
 */
void SideBarFileSystemItem::findChildren()
{
    auto pwdItem = LinuxPWDHelper::getCurrentUser();
    if (pwdItem.userId() == 0) {
        return;
    }
    clearChildren();

    FileEnumerator *e = new FileEnumerator;
    e->setEnumerateDirectory(m_uri);
    connect(e, &FileEnumerator::prepared, this, [=](const GErrorWrapperPtr &err, const QString &targetUri) {
        if (targetUri != nullptr) {
            if (targetUri != this->uri()) {
                e->setEnumerateDirectory(targetUri);
            }
        }

        e->enumerateSync();
        //bug#116016 收起计算机目录后展开计算机下方只显示一个盘
        auto parentItem = qobject_cast<SideBarFileSystemItem *>(m_parent);
        if (!parentItem) {
            auto userDiskItem = new SideBarUserDiskItem(this, m_model);
            m_children->append(userDiskItem);
        }

        auto infos = e->getChildren();
        bool isEmpty = true;
        int real_children_count = infos.count();
        if (infos.isEmpty()) {
            auto separator = new SideBarSeparatorItem(SideBarSeparatorItem::EmptyFile, this, m_model);
            this->m_children->prepend(separator);
            m_model->insertRows(0, 1, this->firstColumnIndex());
            goto end;
        }

        for (auto info: infos) {
            FileInfoJob j(info);
            j.querySync();

            if (!info->displayName().startsWith(".") && (info->isDir() || info->isVolume())) {
                isEmpty = false;
            }
            //skip the independent files
            if (!(info->isDir() || info->isVolume())) {
                real_children_count--;
                continue;
            }

            SideBarFileSystemItem *item = new SideBarFileSystemItem(info->uri(),
                    this,
                    m_model,
                    this);

            // skip local device
            if (!item->isEjectable() && !item->isRemoveable() && item->uri() != "computer:///root.link") {
                real_children_count--;
                delete item;
                continue;
            }

            //check is mounted.
            //FIXME: replace BLOCKING api in ui thread.
            auto targetUri = FileUtils::getTargetUri(info->uri());
            bool isUmountable = FileUtils::isFileUnmountable(info->uri());
            item->m_is_mounted = (!targetUri.isEmpty() && (targetUri != "file:///")) || isUmountable;
            m_children->append(item);
            //qDebug()<<info->uri();
        }
        m_model->insertRows(0, m_children->count(), firstColumnIndex());

        if (isEmpty) {
            auto separator = new SideBarSeparatorItem(SideBarSeparatorItem::EmptyFile, this, m_model);
            this->m_children->prepend(separator);
            m_model->insertRows(0, 1, this->firstColumnIndex());
        }
end:
        Q_EMIT this->findChildrenFinished();
        if (err != nullptr) {
            //qDebug()<<"prepared:"<<err.get()->message();
        }
        delete e;

        //NOTE: init watcher after prepared.
        this->initWatcher();
        this->m_watcher->setMonitorChildrenChange();
        /*
        if (this->uri() == "computer:///") {
            this->m_watcher->setMonitorChildrenChange();
        }
        */

        //start listening.
        connect(m_watcher.get(), &FileWatcher::fileCreated, this, [=](const QString &uri) {
            //qDebug()<<"created:"<<uri;
            for (auto item : *m_children) {
                if (item->uri() == uri) {
                    return;
                }
            }

            SideBarFileSystemItem *item = new SideBarFileSystemItem(uri,
                    this,
                    m_model);
            m_model->beginInsertRows(this->firstColumnIndex(), m_children->count(), m_children->count());
            m_children->append(item);
            m_model->endInsertRows();
            m_model->indexUpdated(this->firstColumnIndex());
            m_model->dataChanged(item->firstColumnIndex(), item->lastColumnIndex());
        });

        connect(m_watcher.get(), &FileWatcher::fileDeleted, this, [=](const QString &uri) {
            //qDebug()<<"deleted:"<<uri;
            for (auto child : *m_children) {
                if (child->uri() == uri) {
                    int index = m_children->indexOf(child);
                    m_model->beginRemoveRows(firstColumnIndex(), index, index);
                    m_children->removeOne(child);
                    m_model->endRemoveRows();
                    child->deleteLater();
                    break;
                }
            }
            m_model->indexUpdated(this->firstColumnIndex());
        });

        connect(m_watcher.get(), &FileWatcher::fileChanged, this, [=](const QString &uri) {
            //FIXME: maybe i have to remove this changed item then add it again to avoid
            //qt's view expander cannot show correctly after the volume item unmounted.
            //qDebug()<<"side bar fs item changed:"<<uri;
            for (auto child : *m_children) {
                if (child->uri() == uri) {
                    SideBarFileSystemItem *changedItem = static_cast<SideBarFileSystemItem*>(child);
                    updateFileInfo(changedItem);
                    //FIXME: replace BLOCKING api in ui thread.
                    if (FileUtils::getTargetUri(uri).isEmpty()) {
                        changedItem->m_is_mounted = false;
                        changedItem->clearChildren();
                    } else {
                        changedItem->m_is_mounted = true;
                    }

                    //why it would failed when send changed signal for newly mounted item?
                    //m_model->dataChanged(changedItem->firstColumnIndex(), changedItem->firstColumnIndex());
                    m_model->dataChanged(changedItem->firstColumnIndex(), changedItem->lastColumnIndex());
                    break;
                }
            }
        });

        this->startWatcher();
        //m_model->setData(lastColumnIndex(), QVariant(QIcon::fromTheme("media-eject")), Qt::DecorationRole);

        connect(Peony::VolumeManager::getInstance(), &VolumeManager::volumeChanged, this, [=](const std::shared_ptr<Volume> &volume){
            qDebug()<<__func__<<__LINE__<<"volume changed"<<volume.get();
            if(!volume)
                return;
            GVolume *gvolume = volume.get()->getGVolume();
            if(!gvolume)
                return;

            char* gdevice = g_volume_get_identifier(gvolume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
            char* gname = g_volume_get_name(gvolume);
            QString device = gdevice;
            QString name = gname;
            g_free(gdevice);
            g_free(gname);

            qDebug()<<__func__<<__LINE__<<"volume changed"<<device<<name;
            for (auto child : *m_children) {/* update display name */
                qDebug()<<__func__<<__LINE__<<"volume changed"<<child->m_unix_device<<device<<child->m_volume_name<<name;
                if (child->m_unix_device == device && child->m_volume_name != name) {
                    SideBarFileSystemItem *changedItem = static_cast<SideBarFileSystemItem*>(child);
                    updateFileInfo(changedItem);
                    qDebug()<<__func__<<__LINE__<<"volume changed"<<changedItem->m_unix_device<<device<<changedItem->m_volume_name<<name;
                    m_model->dataChanged(changedItem->firstColumnIndex(), changedItem->lastColumnIndex());
                    break;
                }
            }
        });
    });

    e->prepare();

    Q_EMIT findChildrenFinished();
}

void SideBarFileSystemItem::findChildrenAsync()
{
    //TODO add async method.
    findChildren();
}

bool SideBarFileSystemItem::isRemoveable()
{
    if (m_uri.contains("computer:///") && m_uri != "computer:///") {
        auto info = FileInfo::fromUri(m_uri);
        if (info->displayName().isEmpty()) {
            FileInfoJob j(info);
            j.querySync();
        }
        return info->canEject() || info->canStop();
    }
    return false;
}

bool SideBarFileSystemItem::isEjectable()
{
    if (m_uri.contains("computer:///") && m_uri != "computer:///") {
        auto info = FileInfo::fromUri(m_uri);
        if (info->displayName().isEmpty()) {
            FileInfoJob j(info);
            j.querySync();
        }
        return info->canEject();
    }
    return false;
}

bool SideBarFileSystemItem::isMountable()
{
    if (m_uri.contains("computer:///") && m_uri != "computer:///") {
        //some mountable item can be unmounted but can't be mounted.
        //the most of them is remote servers and shared directories.
        //they should be seemed as mountable items.
        auto info = FileInfo::fromUri(m_uri);
        if (info->displayName().isEmpty()) {
            FileInfoJob j(info);
            j.querySync();
        }
        return info->canMount() || info->canUnmount();
    }
    return false;
}

bool SideBarFileSystemItem::isMounted()
{
    QString targetUri;
    if (m_uri.contains("computer:///") && m_uri != "computer:///") {
        auto info = FileInfo::fromUri(m_uri);
        if(info.get()){
            if (info->targetUri().isEmpty()) {
                FileInfoJob j(info);
                j.querySync();
            }
            targetUri = info->targetUri();
        }
    }else{
        targetUri = FileUtils::getTargetUri(m_uri);
    }
    if(!targetUri.isEmpty())
        m_is_mounted = true;

    return m_is_mounted;
}

void SideBarFileSystemItem::unmount()
{
    Peony::SyncThread *syncThread = new Peony::SyncThread(uri());
    QThread* currentThread = new QThread();
    syncThread->moveToThread(currentThread);
    connect(currentThread,&QThread::started,syncThread,&Peony::SyncThread::parentStartedSlot);
    connect(syncThread,&Peony::SyncThread::syncFinished,this,[=](){
        realUnmount();
        syncThread->disconnect(this);
        syncThread->deleteLater();
        currentThread->disconnect(SIGNAL(started()));
        //currentThread->deleteLater();
    });
    currentThread->start();
}

void SideBarFileSystemItem::eject(GMountUnmountFlags ejectFlag)
{
    Peony::SyncThread *syncThread = new Peony::SyncThread(uri());
    QThread* currentThread = new QThread();
    syncThread->moveToThread(currentThread);
    connect(currentThread,&QThread::started,syncThread,&Peony::SyncThread::parentStartedSlot);
    connect(syncThread,&Peony::SyncThread::syncFinished,this,[=](){
        realEject(ejectFlag);
        syncThread->disconnect(this);
        syncThread->deleteLater();
        currentThread->disconnect(SIGNAL(started()));
        //currentThread->deleteLater();
    });
    currentThread->start();
}

void SideBarFileSystemItem::realEject(GMountUnmountFlags ejectFlag)
{
    //FIXME: replace BLOCKING api in ui thread.
    VolumeManager *volumeManager;
    std::shared_ptr<Drive> drive;
    GDrive *gdrive;

    auto file = wrapGFile(g_file_new_for_uri(this->uri().toUtf8().constData()));
    auto target = FileUtils::getTargetUri(m_uri);

    volumeManager = VolumeManager::getInstance();
    drive = volumeManager->getDriveFromUri(target);
    if(!drive)
        drive = volumeManager->getDriveFromSystemByPath(m_unix_device);

    if(!drive)
        return;

    gdrive = drive->getGDrive();
    /* hotfix bug#118518 【文件管理器】弹出u盘后，文件管理器侧边栏仍显示u盘（必现）,所以U盘使用g_drive_stop弹出 */
    if(g_drive_can_eject(gdrive) && !m_unix_device.startsWith("/dev/sd")){//for DVD.
        g_file_eject_mountable_with_operation(file.get()->get(),
                                          ejectFlag,
                                          nullptr,
                                          nullptr,
                                          GAsyncReadyCallback(eject_cb),
                                          this);
    }else if(g_drive_can_stop(gdrive) || g_drive_is_removable(gdrive)){//for udisk or mobile harddisk.
        g_drive_stop(gdrive,ejectFlag,NULL,NULL,
                     GAsyncReadyCallback(ejectDevicebyDrive),
                     this);
    }
}

static void unmount_force_cb(GFile* file, GAsyncResult* result, gpointer udata) {
    auto targetUri = static_cast<QString *>(udata);
    QString unmountNotify;
    GError *err = nullptr;
    g_file_unmount_mountable_with_operation_finish (file, result, &err);
    if (err) {
        QMessageBox::warning(nullptr, QObject::tr("Force unmount failed"), QObject::tr("Error: %1\n").arg(err->message));
        g_error_free(err);
    } else {
        VolumeManager::getInstance()->fileUnmounted(*targetUri);
        unmountNotify = QObject::tr("Data synchronization is complete,the device has been unmount successfully!");
        notifyUser(unmountNotify);
    }
    delete targetUri;
}

static void unmount_finished(GFile* file, GAsyncResult* result, gpointer udata)
{
    auto targetUri = static_cast<QString *>(udata);
    GError *err = nullptr;
    QString unmountNotify;
    g_file_unmount_mountable_with_operation_finish (file, result, &err);
    if (err) {
        if(!strcmp(err->message,"Not authorized to perform operation")){//umount /data need permissions.
            g_error_free(err);
            return;
        }
        if(strstr(err->message,"umount: ")){
            QMessageBox::warning(nullptr,QObject::tr("Unmount failed"),QObject::tr("Unable to unmount it, you may need to close some programs, such as: GParted etc."),QMessageBox::Yes);
            g_error_free(err);
            return;
        }

        auto button = QMessageBox::warning(nullptr, QObject::tr("Unmount failed"), QObject::tr("Error: %1\n"
                                                                                               "Do you want to unmount forcely?").arg(err->message),
                                           QMessageBox::Yes, QMessageBox::No);
        if (button == QMessageBox::Yes) {
            QString *string = new QString;
            *string = *targetUri;
            g_file_unmount_mountable_with_operation(file,
                                                    G_MOUNT_UNMOUNT_FORCE,
                                                    nullptr,
                                                    nullptr,
                                                    GAsyncReadyCallback(unmount_force_cb),
                                                    string);
        }
        g_error_free(err);
    } else {
        VolumeManager::getInstance()->fileUnmounted(*targetUri);
        unmountNotify = QObject::tr("Data synchronization is complete,the device has been unmount successfully!");
        notifyUser(unmountNotify);
    }
    delete targetUri;

    /*!
      \note

      the user data repesent current sidebar item in callback function here is unsafe.
      because once the item unmounted, the item itself will be deleted automaticly, too.
      so, never try using it, especially when unmount operation successed.

      */
    // dangerous data, it might be delete while unmounting, do not use it.

    //SideBarFileSystemItem* th = (SideBarFileSystemItem*)udata;

//    if (FileUtils::isMountPoint(FileUtils::getTargetUri(th->uri()))) {
//        QMessageBox::warning(nullptr, QObject::tr("eject device failed"),
//                             QObject::tr("Please check whether the device is occupied and then eject the device again"),
//                             QMessageBox::Ok);
//    }
}

static UDisksObject *get_object_from_block_device (UDisksClient *client,const gchar *block_device)
{
    struct stat statbuf;
    const gchar *crypto_backing_device;
    UDisksObject *object, *crypto_backing_object;
    UDisksBlock *block;

    object = NULL;

    if (stat (block_device, &statbuf) != 0)
    {
        return object;
    }

    block = udisks_client_get_block_for_dev (client, statbuf.st_rdev);
    if (block == NULL)
    {
        return object;
    }

    object = UDISKS_OBJECT (g_dbus_interface_dup_object (G_DBUS_INTERFACE (block)));
    g_object_unref (block);

    crypto_backing_device = udisks_block_get_crypto_backing_device ((udisks_object_peek_block (object)));
    crypto_backing_object = udisks_client_get_object (client, crypto_backing_device);
    if (crypto_backing_object != NULL)
    {
        g_object_unref (object);
        object = crypto_backing_object;
    }
    return object;
}


void SideBarFileSystemItem::realUnmount()
{
    /*!
      \note

      unmount operation of gio will handle both native and remote mount point.
      so we must not use udisk's api directly, because it will lead to crash while
      unmounting a remote directory.

      */

//    UDisksObject *object;
//    UDisksClient *client;
//    UDisksBlock *block;
//    client = udisks_client_new_sync(NULL,NULL);

//    object = get_object_from_block_device(client,m_unix_device.toUtf8().constData());
//    block = udisks_object_get_block(object);

//    // if device type is disc , Eject optical drive
//    if(g_strcmp0(udisks_block_get_id_type(block),"iso9660")==0 || strlen(udisks_block_get_id_type(block))==0){

//        // if can eject ,eject it
//        if(isEjectable()){
//            this->eject();
//        }

//        return;
//    }

    auto file = wrapGFile(g_file_new_for_uri(this->uri().toUtf8().constData()));
    QString *targetUri = new QString;
    *targetUri = FileUtils::getTargetUri(m_uri);
    g_file_unmount_mountable_with_operation(file.get()->get(),
                                            G_MOUNT_UNMOUNT_NONE,
                                            nullptr,
                                            nullptr,
                                            GAsyncReadyCallback(unmount_finished),
                                            targetUri);
}

void SideBarFileSystemItem::ejectOrUnmount()
{
    if (isRemoveable())
        eject(G_MOUNT_UNMOUNT_NONE);

    else if (isMountable())
        unmount();
}

void SideBarFileSystemItem::initWatcher()
{
    if (!m_watcher) {
        m_watcher = std::make_shared<FileWatcher>(m_uri);
    }
}

void SideBarFileSystemItem::startWatcher()
{
    initWatcher();
    m_watcher->startMonitor();
}

void SideBarFileSystemItem::stopWatcher()
{
    initWatcher();
    m_watcher->stopMonitor();
}

GAsyncReadyCallback SideBarFileSystemItem::eject_cb(GFile *file, GAsyncResult *res, SideBarFileSystemItem *p_this)
{
    GError *err = nullptr;
    QString ejectNotify;
    bool successed = g_file_eject_mountable_with_operation_finish(file, res, &err);
    qDebug()<<successed;
    if (err) {
        qDebug()<<err->message;
        /*fix #18957*/
        QMessageBox warningBox(QMessageBox::Warning,QObject::tr("Eject failed"),QString(err->message));
        QPushButton *cancelBtn = (warningBox.addButton(QObject::tr("Cancel"),QMessageBox::RejectRole));
        QPushButton *ensureBtn = (warningBox.addButton(QObject::tr("Eject Anyway"),QMessageBox::YesRole));
        warningBox.exec();
        if(warningBox.clickedButton() == ensureBtn)
            p_this->realEject(G_MOUNT_UNMOUNT_FORCE);

        g_error_free(err);
    } else {
        char *uri = g_file_get_uri(file);
        VolumeManager::getInstance()->fileUnmounted(uri);
        if (uri)
            g_free(uri);

        ejectNotify = QObject::tr("Data synchronization is complete and the device can be safely unplugged!");
        notifyUser(ejectNotify);
        // remove item anyway
        /*int index = p_this->parent()->m_children->indexOf(p_this);
        p_this->m_model->beginRemoveRows(p_this->parent()->firstColumnIndex(), index, index);
        p_this->parent()->m_children->removeOne(p_this);
        p_this->m_model->endRemoveRows();
        p_this->deleteLater();*/
    }
    return nullptr;
}

//update udisk file info
void SideBarFileSystemItem::updateFileInfo(SideBarFileSystemItem *pThis){
        auto fileInfo = FileInfo::fromUri(pThis->m_uri);
        FileInfoJob fileJob(fileInfo);
        fileJob.querySync();

        QString tmpName = FileUtils::getFileDisplayName(pThis->m_uri);

        //old's drive name -> now's volume name. fix #17968
        FileUtils::queryVolumeInfo(pThis->m_uri,pThis->m_volume_name,pThis->m_unix_device,tmpName);
        //icon name.
        pThis->m_icon_name = FileUtils::getFileIconName(pThis->m_uri);

        qDebug()<<"update file info"<<pThis->m_uri<<pThis->m_volume_name<<pThis->m_unix_device<<pThis->displayName()<<tmpName;
}

/* Eject some device by stop it's drive. Such as: mobile harddisk.
 */
void SideBarFileSystemItem::ejectDevicebyDrive(GObject* object,GAsyncResult* res,SideBarFileSystemItem *pThis)
{
    GError *error;
    QString errorMsg;

    error = NULL;
    if(!g_drive_poll_for_media_finish(G_DRIVE(object),res,&error)){
         if((NULL != error) && (G_IO_ERROR_FAILED_HANDLED != error->code)){
             errorMsg = QObject::tr("Unable to eject %1").arg(pThis->m_display_name);

             QMessageBox warningBox(QMessageBox::Warning,QObject::tr("Eject failed"),errorMsg);
             QPushButton *cancelBtn = (warningBox.addButton(QObject::tr("Cancel"),QMessageBox::RejectRole));
             QPushButton *ensureBtn = (warningBox.addButton(QObject::tr("Eject Anyway"),QMessageBox::YesRole));
             warningBox.exec();
             if(warningBox.clickedButton() == ensureBtn)
                 pThis->eject(G_MOUNT_UNMOUNT_FORCE);

             g_error_free(error);
         }
    }
}

void notifyUser(QString notifyContent)
{
    NotifyNotification* notify;

    notify_init(QObject::tr("PeonyNotify").toUtf8().constData());
    notify  = notify_notification_new(QObject::tr("File Manager").toUtf8().constData(),
                                      notifyContent.toUtf8().constData(),
                                      "system-file-manager");
    notify_notification_show(notify,nullptr);

    notify_uninit();
    g_object_unref(G_OBJECT(notify));
}

SideBarUserDiskItem::SideBarUserDiskItem(SideBarFileSystemItem *item, SideBarModel *model, QObject *parent) : SideBarFileSystemItem(nullptr, item, model, parent)
{
    m_parent = item;
}

QString SideBarUserDiskItem::uri()
{
    return "file:///home";
}

QString SideBarUserDiskItem::displayName()
{
    return tr("User Disk");
}

QString SideBarUserDiskItem::iconName()
{
    return "drive-harddisk";
}

SideBarAbstractItem *SideBarUserDiskItem::parent()
{
    return m_parent;
}

void SideBarUserDiskItem::findChildren()
{

}

void SideBarUserDiskItem::findChildrenAsync()
{
    findChildren();
}

void SideBarUserDiskItem::clearChildren()
{
    SideBarAbstractItem::clearChildren();
}
