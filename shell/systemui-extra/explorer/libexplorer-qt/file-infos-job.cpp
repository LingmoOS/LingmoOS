/*
 * Peony-Qt's Library
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
 * Authors: Ling Yang <yangling@kylinos.cn>
 *
 */

#include "file-infos-job.h"
#include "file-info-job.h"
#include "file-info.h"
#include "file-meta-info.h"

#include "file-info-manager.h"
#include "file-label-model.h"

#include "emblem-provider.h"

#include <gio/gdesktopappinfo.h>
#include <global-settings.h>

#include <QDebug>
#include <QDateTime>
#include <QIcon>
#include <QUrl>
#include <QLocale>
#include <QFileInfo>

#include <QCoreApplication>

using namespace Peony;

FileInfosJob::FileInfosJob(std::vector<std::shared_ptr<FileInfo> > infos, QObject *parent)
{
    //sync bug#181067 change to batch file process
    connect(qApp, &QCoreApplication::aboutToQuit, this, &FileInfosJob::batchCancel);

    m_infos = infos;
    m_batchCanellable = g_cancellable_new();
}

FileInfosJob::~FileInfosJob()
{
    g_object_unref(m_batchCanellable);
}

std::vector<std::shared_ptr<FileInfo> > FileInfosJob::batchQuerySync()
{
    std::vector<std::shared_ptr<FileInfo> > fileInfoVec;
    for (auto& fileInfo : m_infos) {
        std::shared_ptr<FileInfo> info;
        if (auto data = fileInfo.get()) {
            info = fileInfo;
        } else {
            continue;
        }

        GError *err = nullptr;
        auto _info = g_file_query_info(info->m_file,
                                       "standard::*," "time::*," "access::*," "mountable::*," "metadata::*," "trash::*," G_FILE_ATTRIBUTE_ID_FILE,
                                       G_FILE_QUERY_INFO_NONE,
                                       m_batchCanellable,
                                       &err);

        if (err) {
            qDebug()<<err->code<<err->message<<info->uri();
            g_error_free(err);
            if(g_cancellable_is_cancelled(m_batchCanellable))
                break;
            continue;
        }

        /* 暂时没用到，为了提升速度，先注释掉 */
//        auto _fs_info = g_file_query_filesystem_info(info->m_file, "filesystem::*,", m_batchCanellable, &err);
//        if (err) {
//            qDebug()<<err->code<<err->message;
//            g_error_free(err);
//            if(g_cancellable_is_cancelled(m_batchCanellable))
//                break;
//        }
//        if (_fs_info) {
//            info = refreshFileSystemInfo(info, _fs_info);
//            g_object_unref(_fs_info);

//        }

        info = refreshInfoContents(info, _info);
        g_object_unref(_info);
        fileInfoVec.push_back(info);
    }
    return fileInfoVec;
}

void FileInfosJob::batchCancel()
{
    g_cancellable_cancel(m_batchCanellable);

}

std::shared_ptr<FileInfo> FileInfosJob::queryFileType(std::shared_ptr<FileInfo> info, GFileInfo *new_info)
{
    if (!info)
        return info;

    info->m_is_dir = false;
    info->m_is_volume = false;
    GFileType type = g_file_info_get_file_type (new_info);
    switch (type) {
    case G_FILE_TYPE_DIRECTORY:
        //qDebug()<<"dir";
        info->m_is_dir = true;
        break;
    case G_FILE_TYPE_MOUNTABLE:
        //qDebug()<<"mountable";
        info->m_is_volume = true;
        break;
    default:
        break;
    }
    return info;
}

std::shared_ptr<FileInfo> FileInfosJob::queryFileDisplayName(std::shared_ptr<FileInfo> info, GFileInfo *new_info)
{
    if (!g_file_info_get_display_name(new_info)) {
        qWarning()<<info.get()->uri()<<"query file display name failed";
        return info;
    }

    if (!info)
        return info;

    info->m_display_name = QString (g_file_info_get_display_name(new_info));
    info->m_finalDisplayName = info->getFinalDisplayName();
    if (info->isDesktopFile()) {
        info->m_desktop_name = info->displayName();
        QUrl url = info->uri();
        GDesktopAppInfo *desktop_info = g_desktop_app_info_new_from_filename(url.path().toUtf8());
        if (!desktop_info) {
            //info->m_mutex.unlock();
            info->updated();
            return info;
        }
#if GLIB_CHECK_VERSION(2, 56, 0)
        g_autofree gchar* string = g_desktop_app_info_get_locale_string(desktop_info, "Name");
#else
        //FIXME: should handle locale?
        //change "Name" to QLocale::system().name(),
        //try to fix Qt5.6 untranslated desktop file issue
        auto key = "Name[" +  QLocale::system().name() + "]";
        g_autofree gchar* string = g_desktop_app_info_get_string(desktop_info, key.toUtf8().constData());
#endif
        qDebug() << "get name string:"<<string <<info->uri()<<info->displayName();
        QString path = "/usr/share/applications/" + info->displayName();
        if(QFileInfo::exists(url.path().toUtf8()) && QFileInfo::exists(path))
        {
            url = path;
            desktop_info = g_desktop_app_info_new_from_filename(url.path().toUtf8());
            string = g_desktop_app_info_get_locale_string(desktop_info, "Name");
            info->m_display_name = string;
        }
        else{
            info->m_display_name = string;
        }
        info->m_finalDisplayName = info->getFinalDisplayName();

        g_object_unref(desktop_info);
    }/* else if (!info->uri().startsWith("file:///")) {
        if (info->uri() == "trash:///") {
            info->m_display_name = tr("Trash");
        } else if (info->uri() == "computer:///") {
            info->m_display_name = tr("Computer");
        } else if (info->uri() == "network:///") {
            info->m_display_name = tr("Network");
        } else if (info->uri() == "recent:///") {
            info->m_display_name = tr("Recent");
        }
    }*/
    return info;
}

std::shared_ptr<FileInfo> FileInfosJob::refreshFileSystemInfo(std::shared_ptr<FileInfo> info, GFileInfo *new_info)
{
    if (!info)
        return info;

    // fs type
    info->m_fs_type = g_file_info_get_attribute_string (new_info, G_FILE_ATTRIBUTE_FILESYSTEM_TYPE);
    return info;
}

std::shared_ptr<FileInfo> FileInfosJob::refreshInfoContents(std::shared_ptr<FileInfo> info, GFileInfo *new_info)
{
    if (!info)
        return info;

    info = queryFileType(info, new_info);

    /* 获取info的G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN字段的值 */
    info->setProperty(G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN, g_file_info_get_attribute_boolean(new_info, G_FILE_ATTRIBUTE_STANDARD_IS_HIDDEN));

    info->m_is_symbol_link = g_file_info_get_attribute_boolean(new_info, G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK);
    if (g_file_info_has_attribute(new_info, G_FILE_ATTRIBUTE_ACCESS_CAN_READ)) {
        info->m_can_read = g_file_info_get_attribute_boolean(new_info, G_FILE_ATTRIBUTE_ACCESS_CAN_READ);
    } else {
        // we assume an unknow access file is readable.
        info->m_can_read = true;
    }

    if (g_file_info_has_attribute(new_info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE)) {
        info->m_can_write = g_file_info_get_attribute_boolean(new_info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
    } else {
        info->m_can_write = true;
    }

    if (g_file_info_has_attribute(new_info, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE)) {
        info->m_can_excute = g_file_info_get_attribute_boolean(new_info, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE);
    } else {
        info->m_can_excute = true;
    }

    info->m_can_delete = g_file_info_get_attribute_boolean(new_info, G_FILE_ATTRIBUTE_ACCESS_CAN_DELETE);
    info->m_can_trash = g_file_info_get_attribute_boolean(new_info, G_FILE_ATTRIBUTE_ACCESS_CAN_TRASH);
    info->m_can_rename = g_file_info_get_attribute_boolean(new_info, G_FILE_ATTRIBUTE_ACCESS_CAN_RENAME);

    info->m_can_mount = g_file_info_get_attribute_boolean(new_info, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_MOUNT);
    info->m_can_unmount = g_file_info_get_attribute_boolean(new_info, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_UNMOUNT);
    info->m_can_eject = g_file_info_get_attribute_boolean(new_info, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_EJECT);
    info->m_can_start = g_file_info_get_attribute_boolean(new_info, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_START);
    info->m_can_stop = g_file_info_get_attribute_boolean(new_info, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_STOP);

    info->m_is_virtual = g_file_info_get_attribute_boolean(new_info, G_FILE_ATTRIBUTE_STANDARD_IS_VIRTUAL);
    if(g_file_info_has_attribute(new_info,G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE_FILE))
        info->m_unix_device_file = g_file_info_get_attribute_string(new_info,G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE_FILE);

    info->m_target_uri = g_file_info_get_attribute_string(new_info, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
    info->m_symlink_target = g_file_info_get_symlink_target(new_info);

    GIcon *g_icon = g_file_info_get_icon (new_info);
    if (G_IS_THEMED_ICON(g_icon)) {
        const gchar* const* icon_names = g_themed_icon_get_names(G_THEMED_ICON (g_icon));
        if (icon_names) {
            info->m_icon_name = QString(*icon_names);
        }
        //g_object_unref(g_icon);
    }

    //fix bug#163449, trash desktop file icon issue
    auto uri = info.get()->uri();
    QUrl targetUrl = info->m_target_uri;
    if (uri.startsWith("trash:///") && uri != "trash:///" && QFile::exists(targetUrl.path())){
        if (info->m_icon_name == "application-x-desktop"){
            auto _desktop_file = g_desktop_app_info_new_from_filename(QUrl(targetUrl).path().toUtf8().constData());
            if (_desktop_file) {
                info->m_icon_name = g_desktop_app_info_get_string(_desktop_file, "Icon");
            }
        }
    }

    //qDebug()<<"refreshInfoContents:"<<info->m_uri <<info->m_icon_name;
    GIcon *g_symbolic_icon = g_file_info_get_symbolic_icon (new_info);
    if (G_IS_THEMED_ICON(g_symbolic_icon)) {
        const gchar* const* symbolic_icon_names = g_themed_icon_get_names(G_THEMED_ICON (g_symbolic_icon));
        if (symbolic_icon_names)
            info->m_symbolic_icon_name = QString (*symbolic_icon_names);
        //g_object_unref(g_symbolic_icon);
    }

    char* name = g_file_get_path(info->m_file);
    info->m_path = name;
    if (NULL != name) {
        g_free(name);
    }
    info->m_file_id = g_file_info_get_attribute_string(new_info, G_FILE_ATTRIBUTE_ID_FILE);

    info->m_content_type = g_file_info_get_content_type (new_info);
    if (info->m_content_type == nullptr) {
        if (g_file_info_has_attribute(new_info, "standard::fast-content-type")) {
            info->m_content_type = g_file_info_get_attribute_string(new_info, "standard::fast-content-type");
        }
    }

    info->m_size = g_file_info_get_attribute_uint64(new_info, G_FILE_ATTRIBUTE_STANDARD_SIZE);
    info->m_modified_time = g_file_info_get_attribute_uint64(new_info, G_FILE_ATTRIBUTE_TIME_MODIFIED);
    info->m_access_time = g_file_info_get_attribute_uint64(new_info, G_FILE_ATTRIBUTE_TIME_ACCESS);
    info->m_create_time = g_file_info_get_attribute_uint64(new_info, G_FILE_ATTRIBUTE_TIME_CREATED);

    info->m_mime_type_string = info->m_content_type;
    if (!info->m_mime_type_string.isEmpty()) {
        char *content_type = g_content_type_get_description (info->m_mime_type_string.toUtf8().constData());
        info->m_file_type = content_type;
        g_free (content_type);
        content_type = nullptr;
    }

    if (info->m_size) {
//        char *size_full = strtok(g_format_size_full(info->m_size, G_FORMAT_SIZE_IEC_UNITS),"iB");
        //列表视图显示改为GB - List view display changed to GB
        char *size_full = g_format_size_full(info->m_size, G_FORMAT_SIZE_IEC_UNITS);
        info->m_file_size = QString(size_full).replace("iB", "B");;
        g_free(size_full);
    } else {
        //fix not show file size issue, link to bug#104721
        if (g_file_info_has_attribute(new_info, G_FILE_ATTRIBUTE_STANDARD_SIZE))
           info->m_file_size = "0 KB";
        else
           info->m_file_size = nullptr;
    }

//    auto systemTimeFormat = GlobalSettings::getInstance()->getSystemTimeFormat();
//    QDateTime date = QDateTime::fromMSecsSinceEpoch(info->m_modified_time*1000);
    if (info->m_modified_time) {
        //info->m_modified_date = date.toString(systemTimeFormat);
        info->m_modified_date = GlobalSettings::getInstance()->transToSystemTimeFormat(info->m_modified_time);
    } else {
        info->m_modified_date = nullptr;
    }

    if (info->m_access_time) {
//        date = QDateTime::fromMSecsSinceEpoch(info->m_access_time*1000);
//        info->m_access_date = date.toString(systemTimeFormat);
        info->m_access_date = GlobalSettings::getInstance()->transToSystemTimeFormat(info->m_access_time);
    } else {
        info->m_access_date = nullptr;
    }

    if (info->m_create_time) {
        info->m_create_date = GlobalSettings::getInstance()->transToSystemTimeFormat(info->m_create_time);
    } else {
        info->m_create_date = nullptr;
    }

    if (g_file_info_has_attribute(new_info, "trash::deletion-date"))
    {
       QString deletionDate = g_file_info_get_attribute_as_string(new_info, G_FILE_ATTRIBUTE_TRASH_DELETION_DATE);
       info->m_deletion_date = deletionDate.replace("T", " ");
       QDateTime dateTime = QDateTime::fromString (deletionDate, "yyyy-MM-dd HH:mm:ss");
       info->m_deletion_date_uint64 = dateTime.toMSecsSinceEpoch ();
       //time already processed, need /1000 to origin state
       info->m_deletion_date = GlobalSettings::getInstance()->transToSystemTimeFormat(info->m_deletion_date_uint64/1000);
    }
    if (g_file_info_has_attribute(new_info, G_FILE_ATTRIBUTE_TRASH_ORIG_PATH)) {
        auto origPath = g_file_info_get_attribute_byte_string(new_info, G_FILE_ATTRIBUTE_TRASH_ORIG_PATH);
        info->setProperty("orig-path", origPath);
    }

    // update explorer qt color list after meta info updated.
    info->m_meta_info = FileMetaInfo::fromGFileInfo(info->uri(), new_info);
    QList<QColor> l;
    auto labels = info->m_meta_info->getMetaInfoStringList(PEONY_FILE_LABEL_IDS);
    for (auto& label : labels) {
        auto id = label.toInt();
        auto item = FileLabelModel::getGlobalModel()->itemFromId(id);
        if (item) {
            l<<item->color();
        }
    }
    info->m_colors = l;

    auto customIconName = info->m_meta_info.get()->getMetaInfoString("custom-icon");
    if (!customIconName.isEmpty()/* && !customIconName.startsWith("/")*/) {
        info->m_icon_name = customIconName;
    }

    info = queryFileDisplayName(info, new_info);

    // fix #81862
    //fix bug#126974, related to trash link files, use this code when target exists
    if (uri.startsWith("trash:///") && uri != "trash:///" && QFile::exists(targetUrl.path())) {
        std::shared_ptr<FileInfo> targetInfo = std::make_shared<FileInfo>();
        targetInfo->m_uri = info->m_target_uri;
        targetInfo->m_file = g_file_new_for_uri(info->m_target_uri.toUtf8().constData());
        GError *err = nullptr;
        auto gFileInfo = g_file_query_info(targetInfo->m_file,
                                       "standard::*," "time::*," "access::*," "mountable::*," "metadata::*," "trash::*," G_FILE_ATTRIBUTE_ID_FILE,
                                       G_FILE_QUERY_INFO_NONE,
                                       m_batchCanellable,
                                       &err);

        if (err) {
            qDebug()<<err->code<<err->message;
            g_error_free(err);
        }else{
            if (g_file_info_has_attribute(gFileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE)) {
                targetInfo->m_can_excute = g_file_info_get_attribute_boolean(gFileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE);
            } else {
                targetInfo->m_can_excute = true;
            }
            targetInfo = queryFileDisplayName(targetInfo, gFileInfo);
            info->m_finalDisplayName = targetInfo.get()->getFinalDisplayName();
            info->m_display_name = targetInfo.get()->displayName();
        }
    }

    return info;
}

