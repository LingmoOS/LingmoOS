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
 * Authors: Wenfei He <hewenfei@kylinos.cn>
 *
 */

#include "side-bar-net-work-item.h"
#include "side-bar-model.h"
#include "global-settings.h"
#include "file-info.h"
#include "file-info-job.h"
#include "connect-to-server-dialog.h"
#include "sync-thread.h"
#include "volumeManager.h"
#include "gobject-template.h"
#include "file-utils.h"
#include "libnotify/notification.h"
#include <sys/stat.h>
#include <libnotify/notify.h>
#include "file-watcher.h"
#include "file-enumerator.h"

#include <QMessageBox>
#include <QDebug>
#include <QProcess>
#include <QUrl>

using namespace Peony;

void notifyUser(QString notifyContent);

SideBarNetWorkItem::SideBarNetWorkItem(const QString &uri,
                                       const QString &iconName,
                                       const QString &displayName,
                                       SideBarNetWorkItem *parentItem,
                                       SideBarModel *model,
                                       QObject *parent) :
        SideBarAbstractItem(model, parent),
        m_parentItem(parentItem)
{
    m_uri = uri;
    m_iconName = iconName;
    m_displayName = displayName;

    qRegisterMetaType<QHash<QString,QString> >("QHash<QString,QString>");
}

SideBarNetWorkItem::~SideBarNetWorkItem()
{
    if(m_enumerator)
    {
        m_enumerator->cancel();
        delete m_enumerator;
        m_enumerator = nullptr;
    }
}

QString SideBarNetWorkItem::uri()
{
    return m_uri;
}

QString SideBarNetWorkItem::displayName()
{
    return m_displayName;
}

QString SideBarNetWorkItem::iconName()
{
    return m_iconName;
}

bool SideBarNetWorkItem::hasChildren()
{
    return (m_parentItem == nullptr);
}


bool SideBarNetWorkItem::isUnmountable()
{
    /* 远程服务器返回true */
    if (!m_uri.startsWith("file://")&& m_uri!="network:///"){
        return true;
    }
    return false;
}

bool SideBarNetWorkItem::isMounted()
{
    if(FileUtils::isMountPoint(m_uri))
        return true;
    return false;
}

QModelIndex SideBarNetWorkItem::firstColumnIndex()
{
    return m_model->firstColumnIndex(this);
}

QModelIndex SideBarNetWorkItem::lastColumnIndex()
{
    return m_model->lastColumnIndex(this);
}

static void unmount_finished(GMount *mount, GAsyncResult *result, QString *mountPath)
{
    GError *err = nullptr;   
    g_mount_unmount_with_operation_finish(mount, result, &err);
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

        QMessageBox::warning(nullptr, QObject::tr("Unmount failed"), QObject::tr("Error: %1\n").arg(err->message), QMessageBox::Yes);
        g_error_free(err);

    } else {
        /* 卸载完成信息提示 */
        Q_EMIT Experimental_Peony::VolumeManager::getInstance()->signal_unmountFinished(*mountPath);
        QString unmountNotify = QObject::tr("Data synchronization is complete,the device has been unmount successfully!");
        SyncThread::notifyUser(unmountNotify);
    }
    if(mountPath){
        delete mountPath;
        mountPath = nullptr;
    }
}

void SideBarNetWorkItem::realUnmount()
{
    GFile *gFile= g_file_new_for_uri(this->uri().toUtf8().constData());
    auto mountPath = new QString;
    *mountPath = m_uri;

    /* 远程服务进入内部目录后卸载,link to bug#98623 */
    QUrl url(m_uri);
    if(url.port() > 0)
        *mountPath = m_uri.section(":",0,-2);
    if(!url.path().isEmpty() && m_uri.endsWith("/"))
        *mountPath = m_uri.section("/",0,-2);
    //end

    GMount *gMount = g_file_find_enclosing_mount(gFile, nullptr, nullptr);
    g_mount_unmount_with_operation(gMount,
                    G_MOUNT_UNMOUNT_NONE,
                    nullptr,
                    nullptr,
                    GAsyncReadyCallback(unmount_finished),
                    mountPath);
    g_object_unref(gFile);
    g_object_unref(gMount);
}

void SideBarNetWorkItem::ejectOrUnmount()
{
    unmount();
}

void SideBarNetWorkItem::unmount()
{
    SyncThread *syncThread = new SyncThread(m_uri);
    QThread* currentThread = new QThread();
    syncThread->moveToThread(currentThread);
    connect(currentThread,&QThread::started,syncThread,&SyncThread::parentStartedSlot);
    connect(syncThread,&SyncThread::syncFinished,this,[=](){
        realUnmount();
        syncThread->disconnect(this);
        syncThread->deleteLater();
        currentThread->disconnect(SIGNAL(started()));
    });
    currentThread->start();
}

void SideBarNetWorkItem::clearChildren()
{
    stopWatcher();
    SideBarAbstractItem::clearChildren();
}

void SideBarNetWorkItem::findChildrenAsync()
{
    findChildren();
}

#include "file-utils.h"
#include <QUrl>
void SideBarNetWorkItem::findChildren()
{
    //只有根节点才设置子节点
    if (m_parentItem == nullptr) {
        clearChildren();
        findRemoteServers();
        querySharedFolders();
    }

    /* 设备动态增减处理 */
    if("network:///" == m_uri && m_parentItem == nullptr){
        auto userShareManager = UserShareInfoManager::getInstance();
        connect(userShareManager, &UserShareInfoManager::signal_addSharedFolder, this, &SideBarNetWorkItem::slot_addSharedFolder);
        connect(userShareManager, &UserShareInfoManager::signal_deleteSharedFolder, this, &SideBarNetWorkItem::slot_deleteSharedFolder);
        connect(GlobalSettings::getInstance(), &GlobalSettings::signal_updateRemoteServer,this,&SideBarNetWorkItem::slot_updateRemoteServer, Qt::UniqueConnection);

        /* samba的子项 */
        auto volumeManager = Experimental_Peony::VolumeManager::getInstance();
        connect(volumeManager, &Experimental_Peony::VolumeManager::mountAdd, this, &SideBarNetWorkItem::slot_serverMount, Qt::UniqueConnection);
        connect(volumeManager, &Experimental_Peony::VolumeManager::mountRemove, this, &SideBarNetWorkItem::slot_unmountedRemoteServerCallBack, Qt::UniqueConnection);
    }

    /* 计算机视图卸载时，侧边栏item状态也要响应 */
    this->initWatcher();
    this->m_watcher->setMonitorChildrenChange();
    connect(m_watcher.get(), &FileWatcher::fileDeleted, this, [=](const QString &uri) {
        for (auto item : *m_children) {
            if(QUrl(item->uri()).host()==QUrl(uri).host())
                m_model->dataChanged(item->firstColumnIndex(), item->lastColumnIndex());
        }
    });

    this->startWatcher();
}

void SideBarNetWorkItem::getMountedServers()
{
    /* 枚举操作 */
    if(!m_enumerator)
        m_enumerator= new FileEnumerator();
    else {
        m_enumerator->cancel();
        m_enumerator->deleteLater();
        m_enumerator = new FileEnumerator();
    }
    m_enumerator->setEnumerateDirectory("computer:///");
    connect(m_enumerator, &Peony::FileEnumerator::enumerateFinished, this, [=](bool successed){
        if(!successed)
            return;

        auto infos = m_enumerator->getChildren();
        if (infos.isEmpty()) {
            return;
        }

        for (auto info: infos)
        {
            /* 更新fileinfo */
            FileInfoJob job(info);
            job.querySync();

            auto targetUri = FileUtils::getTargetUri(info->uri());
            //smb server
            if(targetUri.startsWith("smb://")){
                addItemForUri(targetUri, "network-workgroup-symbolic", info->displayName(), this, m_model, true);
            }
        }
    });
    m_enumerator->enumerateAsync();
}

void SideBarNetWorkItem::findRemoteServers()
{
    if (m_parentItem == nullptr) {
        /* 获取已经挂载的远程服务 */
        getMountedServers();

        //获取连接过的服务器
        QMap <QString, QVariant> remoteServer = GlobalSettings::getInstance()->getValue(REMOTE_SERVER_REMOTE_IP).toMap ();
        for (const QString& remoteServer : remoteServer.keys()) {
            if (!remoteServer.isEmpty()) {
                addItemForUri(remoteServer, "network-workgroup-symbolic", remoteServer, this, m_model, true);
            }
        }
    }
}

void SideBarNetWorkItem::querySharedFolders()
{
    if (m_parentItem != nullptr)
        return;

    //获取共享文件夹很慢，所以使用单独的线程处理 - Obtaining shared folders is slow, so use a separate thread for processing
    SharedDirectoryInfoThread *thread = new SharedDirectoryInfoThread();

    connect(thread, &SharedDirectoryInfoThread::querySharedInfoFinish, this, [=](QHash<QString,QString> sharedFolderInfoMap)
    {
        delete thread;
        QHash<QString, QString>::iterator iter;
        for(iter = sharedFolderInfoMap.begin();iter != sharedFolderInfoMap.end();iter++){
            QString sharePath=iter.value();
            QString shareName=iter.key();
            if (!sharePath.isEmpty()) {
                addItemForUri("file://" + sharePath,"folder", shareName, this, m_model);
            }
        }
    });

    thread->start();
}

void SideBarNetWorkItem::addItemForUri(const QString &uri, const QString &iconName, const QString &displayName, SideBarNetWorkItem *parentItem,
                                      SideBarModel *model, bool isVolume, QObject *parent)
{
    /* 防止一个远程服务器多次添加 */
    for (auto& item : *m_children) {
        if (FileUtils::urlDecode(item->uri()) == FileUtils::urlDecode(uri)) {
            return;
        }
    }

    SideBarNetWorkItem *item = new SideBarNetWorkItem(uri, iconName, displayName, parentItem, model, parent);
    item->m_isVolume = isVolume;
    m_model->beginInsertRows(this->firstColumnIndex(), m_children->count(), m_children->count());
    m_children->append(item);
    m_model->endInsertRows();
    qDebug()<<"add item for uri "<<FileUtils::urlDecode(uri);

}

void SideBarNetWorkItem::removeItemForUri(const QString uri)
{
    for (auto item : *m_children){
        if(FileUtils::urlDecode(item->uri()) != FileUtils::urlDecode(uri))
            continue;
        int index = m_children->indexOf(item);
        m_model->beginRemoveRows(firstColumnIndex(), index, index);
        m_children->removeOne(item);
        m_model->endRemoveRows();
        item->deleteLater();
        qDebug()<<"remove item for uri:"<<FileUtils::urlDecode(uri);
        break;
    }
}


void SideBarNetWorkItem::slot_addSharedFolder(const ShareInfo &shareInfo, bool successed)
{
    if (!successed)
        return;

    if (!shareInfo.originalPath.isEmpty()) {
        addItemForUri("file://" + shareInfo.originalPath, "folder", shareInfo.name, this, m_model);
    }
    return;
}

void SideBarNetWorkItem::slot_deleteSharedFolder(const QString& originalPath, bool successed)
{
    if(!successed)
        return;

    removeItemForUri("file://" + originalPath);
    return;
}

void SideBarNetWorkItem::slot_updateRemoteServer(const QString& server,bool add)
{
   if(add){
       addItemForUri(server, "network-workgroup-symbolic", server, this, m_model, true);

   }else{
       //删除时如果已挂载则先卸载后删除
       for (auto item : *m_children){
           if(item->uri()!= server)
               continue;
           QUrl serverUrl = QUrl(server);
           if(!item->isMounted() ||"smb"==serverUrl.scheme().toLower() && serverUrl.path().isEmpty()){/* 例如：smb://127.0.0.1:445 */
               removeItemForUri(server);
           }else{/* sftp、ftp */
               item->unmount();
               m_canDeleteServer = true;
               connect(Experimental_Peony::VolumeManager::getInstance(), &Experimental_Peony::VolumeManager::signal_unmountFinished, this, [=](const QString& server){
                   if(m_canDeleteServer){
                       /* hotfix bug#162858 【文件管理器】ftp已挂载，删除记录后侧边栏仍显示ftp服务器记录 */
                       QUrl serverUrl(server);/* ftp:///127.0.0.1 */
                       QUrl itemUrl(item->uri());/* ftp:///127.0.0.1:21 */
                       if(serverUrl.scheme() == itemUrl.scheme() && serverUrl.host() == itemUrl.host()){
                           removeItemForUri(item->uri());
                       }
                       m_canDeleteServer = false;
                   }
               });
           }
           break;
       }
   }
}

void SideBarNetWorkItem::slot_unmountedRemoteServerCallBack(const QString &server)
{
    QUrl serverUrl = QUrl(server);
    if("smb"==serverUrl.scheme().toLower() && !serverUrl.path().isEmpty()){/* samba的子项卸载成功后需从侧边栏上移除 */
        removeItemForUri(server);
    }
}

void SideBarNetWorkItem::slot_serverMount(const Experimental_Peony::Volume &volume)
{
    QUrl serverUrl = QUrl(volume.mountPoint());
    if("smb"==serverUrl.scheme().toLower() && !serverUrl.path().isEmpty()){/* samba的子项挂载成功后需添加到侧边栏 */
        addItemForUri(volume.mountPoint(), "network-workgroup-symbolic", volume.name(), this, m_model, true);
    }
}

void SideBarNetWorkItem::initWatcher()
{
    /* 监听计算机视图的卸载信号 */
    if (!m_watcher) {
        m_watcher = std::make_shared<FileWatcher>("computer:///");
    }
}

void SideBarNetWorkItem::startWatcher()
{initWatcher();
    m_watcher->startMonitor();

}

void SideBarNetWorkItem::stopWatcher()
{
    initWatcher();
    m_watcher->stopMonitor();
}

SharedDirectoryInfoThread::SharedDirectoryInfoThread()
{

}

void SharedDirectoryInfoThread::run()
{
    /**
     * \brief 获取全部共享文件夹的共享名称
     * 在编写代码时的输出格式为：
     * 888
     *
     * 如果输出发生改变，视情况修改
     */
    QStringList args;
    args.append("usershare");
    args.append("list");
    args.append("");
    bool ret = false;

    auto userShareManager = UserShareInfoManager::getInstance();
    QString shareNames=userShareManager->exectueCommand(args,&ret);

    QHash<QString,QString> sharedFolderInfoMap;/* key:shareName,value: sharePath */

    for (QString shareName : shareNames.split(QRegExp("\n"))) {
        if (!shareName.isEmpty()) {
            /**
             * \brief 根据共享名称获取详细信息
             * 在编写代码时的输出格式为：
             * [888]
             * path=/home/hxf/888
             * comment=Peony-Qt-Share-Extension
             * usershare_acl=Everyone:R,
             * guest_ok=n
             *
             * 如果输出发生改变，视情况修改
             */

            const Peony::ShareInfo* shareInfo = userShareManager->getShareInfo(shareName);
            userShareManager->getUserShareAcl(shareName);
            //修复共享新建文件夹副本后，闪退问题(命令行设置共享成功，属性中共享设置无法成功)
            if (! shareInfo)
                continue;
            QString sharePath = shareInfo->originalPath;
            if (!sharePath.isEmpty())
                sharedFolderInfoMap.insert(shareName,sharePath);
        }
    }

    Q_EMIT querySharedInfoFinish(sharedFolderInfoMap);
}
