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
#include <gio/gunixmounts.h>
#include "file-info.h"

#include "file-info-manager.h"
#include "file-info-job.h"
#include "file-meta-info.h"
#include "file-utils.h"
#include "thumbnail-manager.h"
#include "emblem-provider.h"

#include <QUrl>
#include <QtDBus/QDBusConnection>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

using namespace Peony;

FileInfo::FileInfo(QObject *parent) : QObject (parent)
{
    m_cancellable = g_cancellable_new();
}

FileInfo::FileInfo(const QString &uri, QObject *parent) : QObject (parent)
{
    m_cancellable = g_cancellable_new();
    /*!
     * \note
     * In qt program we alwas handle file's uri format as unicode,
     * but in glib/gio it might be not.
     * I want to keep the uri strings format in explorer-qt,
     * this would help me avoid some problem, such as the uri path completion
     * bug in PathBarModel enumeration.
     */
    m_uri = uri;
    m_file = g_file_new_for_uri(uri.toUtf8().constData());
    m_parent = g_file_get_parent(m_file);
    m_is_remote = !g_file_is_native(m_file);
    m_is_dir = false;
    m_is_volume = false;
    m_meta_info = FileMetaInfo::fromGFileInfo(uri, nullptr);
    GFileType type = g_file_query_file_type(m_file, G_FILE_QUERY_INFO_NONE, nullptr);
    switch (type) {
    case G_FILE_TYPE_DIRECTORY:
        //qDebug()<<"dir";
        m_is_dir = true;
        break;
    case G_FILE_TYPE_MOUNTABLE:
        //qDebug()<<"mountable";
        m_is_volume = true;
        break;
    default:
        break;
    }
}

FileInfo::~FileInfo()
{
    //qDebug()<<"~FileInfo"<<m_uri;
    disconnect();

    g_object_unref(m_cancellable);
    g_object_unref(m_file);

    if (m_parent)
        g_object_unref(m_parent);

    m_uri = nullptr;
}

std::shared_ptr<FileInfo> FileInfo::fromUri(QString uri)
{
    FileInfoManager *info_manager = FileInfoManager::getInstance();
    // avoid using binding mount original uri. link to: #48982.
    if (info_manager->isAutoParted()) {
        if (uri.contains("file:///data/home")) {
            uri.replace("file:///data/home", "file:///home");
        }
    }
    info_manager->lock();
    std::shared_ptr<FileInfo> info = info_manager->findFileInfoByUri(uri);
    if (info != nullptr) {
        info_manager->unlock();
        return info;
    } else {
        std::shared_ptr<FileInfo> newly_info = std::make_shared<FileInfo>();

        newly_info->m_uri = uri;
        newly_info->m_file = g_file_new_for_uri(uri.toUtf8().constData());

        newly_info->m_parent = g_file_get_parent(newly_info->m_file);
        newly_info->m_is_remote = ! g_file_is_native(newly_info->m_file);
        newly_info->m_is_dir = false;
        newly_info->m_is_volume = false;
        newly_info->m_meta_info = FileMetaInfo::fromGFileInfo(uri, nullptr);
        if (! newly_info->m_is_remote && false) {
            GFileType type = g_file_query_file_type(newly_info->m_file, G_FILE_QUERY_INFO_NONE, nullptr);
            switch (type) {
            case G_FILE_TYPE_DIRECTORY:
                //qDebug()<<"dir";
                newly_info->m_is_dir = true;
                break;
            case G_FILE_TYPE_MOUNTABLE:
                //qDebug()<<"mountable";
                newly_info->m_is_volume = true;
                break;
            default:
                break;
            }
        }

        newly_info = info_manager->insertFileInfo(newly_info);
        info_manager->unlock();
        return newly_info;
    }
}

std::vector<std::shared_ptr<FileInfo> > FileInfo::fromUris(QStringList uris)
{
    std::vector<std::shared_ptr<FileInfo> > fileInfoVec;
    for (auto uri : uris) {
        // note that FileInfo::fromUri() will replace file:///data/home to file:///home
        // for auto parted installation, it't not good for model performance. however,
        // if we do not relpace uri here, will lead #206224.
//        FileInfoManager *info_manager = FileInfoManager::getInstance();
//        if (info_manager->isAutoParted()) {
//            if (uri.contains("file:///data/home")) {
//                uri.replace("file:///data/home", "file:///home");
//            }
//        }
        std::shared_ptr<FileInfo> newly_info = std::make_shared<FileInfo>();
        newly_info->m_uri = uri;
        newly_info->m_file = g_file_new_for_uri(uri.toUtf8().constData());
        fileInfoVec.push_back(newly_info);
    }
    return fileInfoVec;
}

std::shared_ptr<FileInfo> FileInfo::fromPath(QString path)
{
    QString uri = "file://" + path;
    return fromUri(uri);
}

std::shared_ptr<FileInfo> FileInfo::fromGFile(GFile *file)
{
    char *uri_str = g_file_get_uri(file);
    QString uri = uri_str;
    g_free(uri_str);
    return fromUri(uri);
}

/*******
函数功能：判断文件是否是视频文件
一般的视频文件都是 video/*,但是有些视频文件比较特殊
比如：
asf :   application/vnd.ms-asf
rmvb:   application/vnd.rn-realmedia
swf :   application/vnd.adobe.flash.movie
ts  :   text/vnd.trolltech.linguist
h264:   application/octet-stream
目前上面这些是已经测试到的视频文件类型，如果有没有覆盖到的情况后面再补充
**/
bool FileInfo::isVideoFile()
{
    if (nullptr != m_mime_type_string)
    {
        if (m_mime_type_string.startsWith("video")
            || m_mime_type_string.endsWith("vnd.trolltech.linguist")
            || m_mime_type_string.endsWith("vnd.adobe.flash.movie")
            || m_mime_type_string.endsWith("vnd.rn-realmedia")
            || m_mime_type_string.endsWith("vnd.ms-asf")
            || m_mime_type_string.endsWith("octet-stream"))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool FileInfo::isExecDisable()
{
    if (m_meta_info) {
        int nRet = m_meta_info->getMetaInfoInt("exec_disable");
        if(1 == nRet) {
            return true;
        }
    }

    return false;
}

bool FileInfo::isAudioFile()
{
    if (nullptr != m_mime_type_string) {
        if (m_mime_type_string.startsWith("audio")
            || m_mime_type_string.startsWith("application/x-smaf")) {
            return true;
        } else {
            return false;
        }
    }

    return false;
}


bool FileInfo::isOfficeFile()
{
    int idx = 0;
    QString mtype = nullptr;

    for (idx = 0; office_mime_types[idx] != "end"; idx++)
    {
        mtype = office_mime_types[idx];
        if (m_mime_type_string.contains(mtype))
        {
            return true;
        }
    }

    return false;
}

const QString FileInfo::targetUri()
{
    return m_target_uri;
}

const QString FileInfo::symlinkTarget()
{
    if (m_symlink_target == ".")
        return "";

    //fix soft link use relative path issue, link to bug#73529
    if (! m_symlink_target.startsWith("/"))
    {
        QString parentUri = FileUtils::getParentUri(m_uri);
        m_symlink_target = QUrl(parentUri).path() + "/" + m_symlink_target;
    }
    return m_symlink_target;
}

const QString FileInfo::customIcon()
{
    if (!m_meta_info)
        return nullptr;
    return m_meta_info.get()->getMetaInfoString("custom-icon");
}

quint64 FileInfo::getDeletionDateUInt64()
{
    return m_deletion_date_uint64;
}

guint64 FileInfo::getCreateTime() const
{
    return m_create_time;
}

QString FileInfo::getCreateDate() const
{
    return m_create_date;
}

const QString FileInfo::getFinalDisplayName()
{
    if (isEmptyInfo())
        return nullptr;

    bool isMountPoint;
    QString unixDevice,deviceName;

    unixDevice = unixDeviceFile();
    isMountPoint = FileUtils::isMountPoint(m_uri);

    if(m_uri == "file:///DATA"
            || m_uri == "file:///data"
            || m_target_uri == "file:///data")
    {
        return tr("data");
    }

    if((nullptr != m_display_name)
            && (!isMountPoint
                || unixDevice.isEmpty()  /*@m_uri is like "computer:///xxx"*/
                || !unixDevice.contains("/dev")  /*audio-cd*/
                || unixDevice.contains("/dev/sr"))) { /*blank-cd or blank-dvd*/
         return m_display_name;
    }

    if (m_uri.endsWith("/")) {
        QString uri = m_uri;
        if (!m_uri.endsWith(":///") && !m_uri.endsWith("://")) {
            uri.chop(1);
        }
        return uri.split("/").last();
    }

    //@deviceName transcoding
    deviceName = m_display_name;
    FileUtils::handleVolumeLabelForFat32(deviceName,unixDevice);
    return deviceName;
}

FileInfo &FileInfo::operator=(const FileInfo &other)
{
    if(this != &other){
        this->m_uri = other.m_uri;
        this->m_is_valid = other.m_is_valid;
        this->m_is_dir = other.m_is_dir;
        this->m_is_volume = other.m_is_volume;
        this->m_is_remote = other.m_is_remote;
        this->m_is_symbol_link = other.m_is_symbol_link;
        this->m_is_virtual = other.m_is_virtual;
        this->m_is_loaded = other.m_is_loaded;
        this->m_display_name = other.m_display_name;
        this->m_desktop_name = other.m_desktop_name;
        this->m_icon_name = other.updateIconName(other.m_uri, other.m_icon_name);
        this->m_symbolic_icon_name = other.m_symbolic_icon_name;
        this->m_file_id = other.m_file_id;
        this->m_path = other.m_path;
        this->m_content_type = other.m_content_type;
        this->m_size = other.m_size;
        this->m_modified_time = other.m_modified_time;
        this->m_access_time = other.m_access_time;
        this->m_deletion_date_uint64 = other.m_deletion_date_uint64;
        this->m_mime_type_string = other.m_mime_type_string;
        this->m_file_type = other.m_file_type;
        this->m_file_size = other.m_file_size;
        this->m_modified_date = other.m_modified_date;
        this->m_access_date = other.m_access_date;
        this->m_deletion_date = other.m_deletion_date;
        this->m_can_read = other.m_can_read;
        this->m_can_write = other.m_can_write;
        this->m_can_excute = other.m_can_excute;
        this->m_can_delete = other.m_can_delete;
        this->m_can_trash = other.m_can_trash;
        this->m_can_rename = other.m_can_rename;
        this->m_can_mount = other.m_can_mount;
        this->m_can_unmount = other.m_can_unmount;
        this->m_can_eject = other.m_can_eject;
        this->m_can_start = other.m_can_start;
        this->m_can_stop = other.m_can_stop;
        this->m_unix_device_file = other.m_unix_device_file;
        this->m_target_uri = other.m_target_uri;
        this->m_symlink_target = other.m_symlink_target;
        this->m_fs_type = other.m_fs_type;
        this->m_meta_info = std::make_shared<FileMetaInfo>(FileMetaInfo(other.m_meta_info.get()));
        this->m_colors = other.m_colors;
        this->m_finalDisplayName = other.m_finalDisplayName;
        this->m_create_time = other.m_create_time;
        this->m_create_date = other.m_create_date;
        this->setProperty("orig-path", other.property("orig-path"));
        this->setProperty(G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN, other.property(G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN));
    }
    return *this;
}

QString FileInfo::updateIconName(const QString &uri, const QString &iconName) const
{
    QIcon icon = QIcon::fromTheme(iconName);
    if (icon.isNull()) {
        return FileUtils::updateFileIconName(uri, true);
    }
    return iconName;
}

const QString FileInfo::unixDeviceFile()
{
    GFile* file;
    const char *path;
    bool isMountPoint;
    GUnixMountEntry* entry;
    char* device = nullptr;
    QString unixDevice = nullptr;

    isMountPoint = FileUtils::isMountPoint(m_uri);
    //return from here if @m_uri is like "computer:///xxx"
    if(!isMountPoint)
        return m_unix_device_file;

    //query device path if @m_uri is like "file:///media/user/xxx"
    file = g_file_new_for_uri(m_uri.toUtf8().constData());
    if(!file)
        return unixDevice;

    path = g_file_peek_path(file);
    if(path){
        entry = g_unix_mount_at(path,NULL);
        if(!entry)
            entry = g_unix_mount_for(path,NULL);

        if(entry){
            device = g_strescape(g_unix_mount_get_device_path(entry),NULL);
            g_unix_mount_free(entry);
        }
    }

    unixDevice = device;
    g_object_unref(file);
    if(device)
        g_free(device);

    return unixDevice;
}

const QString FileInfo::displayName()
{
    return m_finalDisplayName;
}

QString FileInfo::displayFileType()
{
    if (isDir()) {
        return tr("folder");
    } else if (isOfficeFile()) {
        return m_file_type;
    } else if (isDesktopFile()) {
        return "DESKTOP" + tr("file");
    } else {
        if (m_file_type.contains("plain text document")) {
            return tr("text file");
        } else {
            QString fileName = displayName();
            if (fileName.contains(".")) {
                QStringList strElement = fileName.split(".");
                return QString(strElement.last()).toUpper() + tr("file");
            } else {
                return tr("file");
            }
        }
    }

    return m_file_type;
}
