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

#include "file-utils.h"
#include "file-info.h"
#include "file-info-job.h"
#include "volume-manager.h"
#include "linux-pwd-helper.h"
#include <QUrl>
#include <QFileInfo>
#include <QFileInfoList>
#include <QTextCodec>
#include <QByteArray>
#include <QStandardPaths>
#include <QDir>
#include <QIcon>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <udisks/udisks.h>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <gio/gdesktopappinfo.h>
#include <gio/gunixmounts.h>
#include <QCoreApplication>
#include <QThread>

using namespace Peony;


FileUtils::FileUtils()
{
}

QString FileUtils::getQStringFromCString(char *c_string, bool free)
{
    QString value = c_string;
    if (free)
        g_free(c_string);
    return value;
}

QString FileUtils::getFileUri(const GFileWrapperPtr &file)
{
    if (!G_IS_FILE (file.get()->get())) {
        return nullptr;
    }
    g_autofree char *uri = g_file_get_uri(file.get()->get());
    QString urlString = QString(uri);
    QUrl url = urlString;
    //g_free(uri);
    g_autofree char *path = g_file_get_path(file.get()->get());
    if (path && !url.isLocalFile()) {
        QString urlString = QString("file://%1").arg(path);
        //g_free(path);
        return urlString;
    }

    return urlString;
}

QString FileUtils::getFileBaseName(const GFileWrapperPtr &file)
{
    char *basename = g_file_get_basename(file.get()->get());
    return getQStringFromCString(basename);
}

QString FileUtils::getUriBaseName(const QString &uri)
{
    QUrl url = uri;
    return url.fileName();
}

GFileWrapperPtr FileUtils::getFileParent(const GFileWrapperPtr &file)
{
    return wrapGFile(g_file_get_parent(file.get()->get()));
}

QString FileUtils::getRelativePath(const GFileWrapperPtr &dir, const GFileWrapperPtr &file)
{
    char *relative_path = g_file_get_relative_path(dir.get()->get(), file.get()->get());
    return getQStringFromCString(relative_path);
}

GFileWrapperPtr FileUtils::resolveRelativePath(const GFileWrapperPtr &dir, const QString &relativePath)
{
    return wrapGFile(g_file_resolve_relative_path(dir.get()->get(),
                                                  relativePath.toUtf8().constData()));
}

QString FileUtils::urlEncode(const QString& url)
{
    QString decodeUrl = urlDecode(url);

    if (!decodeUrl.isEmpty()) {
        g_autofree gchar* encodeUrl = g_uri_escape_string (decodeUrl.toUtf8().constData(), ":/", true);
//        qDebug() << "encode url from:'" << url <<"' to '" << encodeUrl << "'";
        return encodeUrl;
    }

    g_autofree gchar* encodeUrl = g_uri_escape_string (url.toUtf8().constData(), ":/", true);

//    qDebug() << "encode url from:'" << url <<"' to '" << encodeUrl << "'";

    return encodeUrl;
}

QString FileUtils::urlDecode(const QString &url)
{
    g_autofree gchar* decodeUrl = g_uri_unescape_string(url.toUtf8().constData(), ":/");
    if (!decodeUrl) {
//        qDebug() << "decode url from:'" << url <<"' to '" << url << "'";
        return url;
    }

//    qDebug() << "decode url from:'" << url <<"' to '" << decodeUrl << "'";

    return decodeUrl;
}

QString FileUtils::getBaseNameOfFile(const QString &file)
{
    /* 一些常见扩展名处理，特殊情况以后待完善 */
    QFileInfo qFileInfo(file);
    QString suffix = qFileInfo.suffix();
    QString fileBaseName = file.left(file.length() - suffix.length() - 1);
     if (fileBaseName.isEmpty())
         return file;
     else if(fileBaseName.endsWith(".tar"))
         return fileBaseName.remove(".tar");
     else if(fileBaseName.endsWith(".7z"))
         return fileBaseName.remove(".7z");
     else
         return fileBaseName;
}

QString FileUtils::handleDuplicateName(const QString& uri)
{
    QString handledName = nullptr;
    QString name = QUrl(uri).toDisplayString().split("/").last();

    QRegExp regExpNum("\\(\\d+\\)");
    QRegExp regExp (QString("\\ -\\ %1\\(\\d+\\)(\\.[0-9a-zA-Z\\.]+|)$").arg(QObject::tr("duplicate")));

    QString dupReg = nullptr;

    if (name.contains(regExp)) {
        int num = 0;
        QString numStr = "";

        QString ext = regExp.cap(0);
        if (ext.contains(regExpNum)) {
            numStr = regExpNum.cap(0);
        }

        numStr.remove(0, 1);
        numStr.chop(1);
        num = numStr.toInt();
        ++num;
        handledName = name.replace(regExp, ext.replace(regExpNum, QString("(%1)").arg(num)));
    } else {
        if (name.contains(".")) {
            auto list = name.split(".");
            if (list.count() <= 1) {
                handledName = name + QString(" - %1(1)").arg(QObject::tr("duplicate"));
            } else {
                int pos = list.count() - 1;
                if (list.last() == "gz"
                        || list.last() == "xz"
                        || list.last() == "Z"
                        || list.last() == "sit"
                        || list.last() == "bz"
                        || list.last() == "bz2") {
                    --pos;
                }
                if (pos < 0) {
                    pos = 0;
                }
                auto tmp = list;
                QStringList suffixList;
                for (int i = 0; i < list.count() - pos; i++) {
                    suffixList.prepend(tmp.takeLast());
                }
                auto suffix = suffixList.join(".");

                auto basename = tmp.join(".");
                name = basename + QString(" - %1(1)").arg(QObject::tr("duplicate")) + "." + suffix;
                if (name.endsWith(".")) {
                    name.chop(1);
                }
                handledName = name;
            }
        } else {
            handledName = name + QString(" - %1(1)").arg(QObject::tr("duplicate"));
        }
    }

    return handledName;
}

QString FileUtils::handleDesktopFileName(const QString& uri, const QString& displayName)
{
    //no need self handle, add return to fix bug#72642
    return displayName;

    QString name = QUrl(uri).toDisplayString().split("/").last();
    QRegExp regExpNum("\\(\\d+\\)");
    auto showName = displayName;

    if (!name.contains(QObject::tr("duplicate")))
        return displayName;

    QStringList matchList;
    int pos=0;
    while((pos=regExpNum.indexIn(name,pos))!=-1)
    {
       pos+=regExpNum.matchedLength();
       QString result=regExpNum.cap(0);
       matchList<<result;
    }

    for(auto match : matchList)
    {
        showName = showName + match;
    }

    return showName;
}

bool FileUtils::getFileHasChildren(const GFileWrapperPtr &file)
{
    GFileType type = g_file_query_file_type(file.get()->get(),
                                            G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                            nullptr);
    return type == G_FILE_TYPE_DIRECTORY || type == G_FILE_TYPE_MOUNTABLE;
}

bool FileUtils::getFileIsFolder(const GFileWrapperPtr &file)
{
    GFileType type = g_file_query_file_type(file.get()->get(),
                                            G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                            nullptr);
    return type == G_FILE_TYPE_DIRECTORY;
}

bool FileUtils::getFileIsFolder(const QString &uri)
{
    auto info = FileInfo::fromUri(uri);
    return info.get()->isDir();
}

bool FileUtils::getFileIsSymbolicLink(const QString &uri)
{
    auto info = FileInfo::fromUri(uri);
    return info.get()->isSymbolLink();
}

QStringList FileUtils::getChildrenUris(const QString &directoryUri)
{
    QStringList uris;

    GError *err = nullptr;
    GFile *top = g_file_new_for_uri(directoryUri.toUtf8().constData());
    GFileEnumerator *e = g_file_enumerate_children(top, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                   nullptr, &err);

    if (err) {
        g_error_free(err);
    }
    g_object_unref(top);
    if (!e)
        return uris;

    auto child_info = g_file_enumerator_next_file(e, nullptr, nullptr);
    while (child_info) {
        auto child = g_file_enumerator_get_child(e, child_info);

        auto uri = g_file_get_uri(child);
        QString urlString = FileUtils::urlEncode(uri);
        // BUG: 65889
//        auto path = g_file_get_path(child);
//        QUrl url = urlString;
//        if (path && !url.isLocalFile()) {
//            urlString = QString("file://%1").arg(path);
//            g_free(path);
//        } else {
//            urlString = uri;
//        }

        uris<<urlString;
        g_free(uri);
        g_object_unref(child);
        g_object_unref(child_info);
        child_info = g_file_enumerator_next_file(e, nullptr, nullptr);
    }

    g_file_enumerator_close(e, nullptr, nullptr);
    g_object_unref(e);

    return uris;
}

QString FileUtils::getNonSuffixedBaseNameFromUri(const QString &uri)
{
    QUrl url = uri;
    if (url.isLocalFile()) {
        QFileInfo info(url.path());
        return info.baseName();
    } else {
        QString suffixedBaseName = url.fileName();
        int index = suffixedBaseName.lastIndexOf(".");
        if (index != -1) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            QString suffix = suffixedBaseName.chopped(suffixedBaseName.size() - index);
            if (suffix == ".gz" || suffix == ".xz" || suffix == ".bz"
                    || suffix == ".bz2" || suffix == ".Z" ||
                    suffix == ".sit") {
                int secondIndex = suffixedBaseName.lastIndexOf('.');
                suffixedBaseName.chop(suffixedBaseName.size() - secondIndex);
            }
#else
            suffixedBaseName.chop(suffixedBaseName.size() - index);
#endif
        }
        return suffixedBaseName;
    }
}

#include "file-label-model.h"
QString FileUtils::getFileDisplayName(const QString &uri)
{
    auto fileInfo = FileInfo::fromUri(uri);
    if (uri == "file:///data")
        return QObject::tr("data");
    //fix bug#47597, show as root.link issue. 125255, file system show tip "/" issue
    if (uri == "file:///")
        return QObject::tr("File System");

    //fix bug#139600，替换windows共享名称, “172.17.123.173上的Windows共享” 显示为 "172.17.123.173上的共享"
    bool isSmbPath = uri.startsWith("smb://");
    QString showName = fileInfo.get()->displayName();
    //设置尽量苛刻的条件减少错误的替换
    if (isSmbPath && showName.length()>=18 && showName.contains("Windows") && showName.split(".").length() ==4){
        showName = showName.replace("Windows", "");
        //only replaced one "Windows" to specific match
        if (fileInfo.get()->displayName().length() - showName.length() == 7){
            return showName;
        }
    }
    if(uri.startsWith("label://")){/* 标记模式uri的displayName */
        if("label:///" == uri){
            showName = QObject::tr("label");
        }else{
            showName = uri.section("/", -1,-1).replace("?schema=file","");
        }
        return showName;
    }

    return fileInfo.get()->displayName();
}

QString FileUtils::getApplicationName(const QString &uri)
{
    QString displayName = getFileDisplayName(uri);
    if (uri.endsWith(".desktop")){
        g_autoptr(GFile) gfile = g_file_new_for_uri(uri.toUtf8().constData());
        g_autofree gchar *desktop_file_path = g_file_get_path(gfile);
        g_autoptr(GDesktopAppInfo) gdesktopappinfo = g_desktop_app_info_new_from_filename(desktop_file_path);
        if (gdesktopappinfo) {
            g_autofree gchar *desktop_name = g_desktop_app_info_get_locale_string(gdesktopappinfo, "Name");
            if (!desktop_name) {
                desktop_name = g_desktop_app_info_get_string(gdesktopappinfo, "Name");
            }
            if (desktop_name) {
                displayName = desktop_name;
            }
        }
    }

    return displayName;
}

QString FileUtils::getFileIconName(const QString &uri, bool checkValid)
{
    if (checkValid) {
        //FIXME: replace BLOCKING api in ui thread.
        auto fileInfo = FileInfo::fromUri(uri);
        return fileInfo.get()->iconName();
        if (!fileInfo.get()->isEmptyInfo()) {
            return fileInfo.get()->iconName();
        }
    }

    auto file = wrapGFile(g_file_new_for_uri(uri.toUtf8().constData()));
    auto info = wrapGFileInfo(g_file_query_info(file.get()->get(),
                              G_FILE_ATTRIBUTE_STANDARD_ICON,
                              G_FILE_QUERY_INFO_NONE,
                              nullptr,
                              nullptr));
    if (!G_IS_FILE_INFO (info.get()->get()))
        return nullptr;
    GIcon *g_icon = g_file_info_get_icon (info.get()->get());
    QString icon_name;
    //do not unref the GIcon from info.
    if (G_IS_ICON(g_icon)) {
        const gchar* const* icon_names = g_themed_icon_get_names(G_THEMED_ICON (g_icon));
        if (icon_names) {
            auto p = icon_names;
            if (*p)
                icon_name = QString (*p);
            if (checkValid) {
                while (*p) {
                    QIcon icon = QIcon::fromTheme(*p);
                    if (!icon.isNull()) {
                        icon_name = QString (*p);
                        break;
                    } else {
                        p++;
                    }
                }
            }
        }else {
            //if it's a bootable-media,maybe we can get the icon from the mount directory.
            char *bootableIcon = g_icon_to_string(g_icon);
            if(bootableIcon){
                icon_name = QString(bootableIcon);
                g_free(bootableIcon);
            }
        }
    }
    return icon_name;
}

GErrorWrapperPtr FileUtils::getEnumerateError(const QString &uri)
{
    auto file = wrapGFile(g_file_new_for_uri(uri.toUtf8().constData()));
    GError *err = nullptr;
    auto enumerator = wrapGFileEnumerator(g_file_enumerate_children(file.get()->get(),
                                          G_FILE_ATTRIBUTE_STANDARD_NAME,
                                          G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                          nullptr,
                                          &err));
    if (err) {
        return GErrorWrapper::wrapFrom(err);
    }
    return nullptr;
}

QString FileUtils::getTargetUri(const QString &uri)
{
    auto fileInfo = FileInfo::fromUri(uri);
    return fileInfo.get()->targetUri();
}


QString FileUtils::getEncodedUri(const QString &uri)
{
    GFile *file = g_file_new_for_uri(uri.toUtf8().constData());
    g_autofree gchar *tmp_uri = g_file_get_uri(file);
    QString encodedUri = tmp_uri;
    g_object_unref(file);

    return encodedUri;
}

QString FileUtils::getSymbolicTarget(const QString &uri)
{
    auto fileInfo = FileInfo::fromUri(uri);
    return fileInfo.get()->symlinkTarget();
}

bool FileUtils::isMountPoint(const QString &uri)
{
    bool flag = false;                      // The uri is a mount point

    GFile* file = g_file_new_for_uri(uri.toUtf8().constData());

    GList* mounts = nullptr;
    GVolumeMonitor* vm = g_volume_monitor_get();
    if (nullptr != vm) {
        mounts = g_volume_monitor_get_mounts(vm);
        if (nullptr != mounts) {
            for (GList* l = mounts; nullptr != l; l = l->next) {
                GMount* m = (GMount*)l->data;
                GFile* f = g_mount_get_root(m);
                if (g_file_equal(file, f)) {
                    flag = true;
                    g_object_unref(f);
                    break;
                }
                g_object_unref(f);
            }
        }
    }

    if (nullptr != vm) {
        g_object_unref(vm);
    }

    if (nullptr != mounts) {
        g_list_free_full(mounts, g_object_unref);
    }

    if (nullptr != file) {
        g_object_unref(file);
    }

    return flag;
}

bool FileUtils::stringStartWithChinese(const QString &string)
{
    if (string.isEmpty())
        return false;

    auto firstStrUnicode = string.at(0).unicode();
    return (firstStrUnicode <=0x9FA5 && firstStrUnicode >= 0x4E00);
}

bool FileUtils::stringLesserThan(const QString &left, const QString &right)
{
    bool leftStartWithChinese = stringStartWithChinese(left);
    bool rightStartWithChinese = stringStartWithChinese(right);
    if (!(!leftStartWithChinese && !rightStartWithChinese)) {
        return leftStartWithChinese;
    }
    return left.toLower() < right.toLower();
}

const QString FileUtils::getParentUri(const QString &uri)
{
    auto file = wrapGFile(g_file_new_for_uri(uri.toUtf8().constData()));
    auto parent = getFileParent(file);
    auto parentUri = getFileUri(parent);

    return parentUri == uri? nullptr: parentUri;
}

const QString FileUtils::getOriginalUri(const QString &uri)
{
    auto file = wrapGFile(g_file_new_for_uri(uri.toUtf8().constData()));
    auto originalUri = getFileUri(file);
    return originalUri;
}

QStringList FileUtils::standardPathList()
{
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString documentPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString picturePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString videoPath= QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    QString downloadPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QString musicPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    QString templatePath = g_get_user_special_dir(G_USER_DIRECTORY_TEMPLATES);
    QString publicPath = g_get_user_special_dir(G_USER_DIRECTORY_PUBLIC_SHARE);
    //qDebug() << "isStandardPath :" <<templatePath;
    QStringList standardPathList;
    standardPathList<<desktopPath <<documentPath <<picturePath <<videoPath
                  <<downloadPath <<musicPath <<templatePath<<publicPath;
    return standardPathList;
}

bool FileUtils::isStandardPath(const QString &uri)
{
    QStringList standardPaths = standardPathList();
    QUrl url = uri;
    if (standardPaths.contains(url.path()))
        return true;

    return false;
}

/* @func: 使用设备是否可卸载的方式判断是否为移动设备
 * 可移动设备是可以卸载的，可卸载的不一定是移动设备
 * 排除掉网络地址，如ftp,sftp,smb挂载
*/
bool FileUtils::isMobileDeviceFile(const QString &uri)
{
    if (uri.isEmpty() || uri.startsWith("ftp:///") || uri.startsWith("sftp:///") || uri.startsWith("smb:///"))
        return false;

    bool isMobile = false;
    //多次测试发现不准确，使用canEject和canStop属性来做判断
//    GFile *dest_dir_file = g_file_new_for_path(uri.toUtf8().constData());
//    GMount *dest_dir_mount = g_file_find_enclosing_mount(dest_dir_file, nullptr, nullptr);
//    if (dest_dir_mount) {
//        isMobile = g_mount_can_unmount(dest_dir_mount);
//        g_object_unref(dest_dir_mount);
//    }
//    g_object_unref(dest_dir_file);
    auto dev = VolumeManager::getDriveFromUri(getParentUri(uri));
    if(dev != nullptr){
        bool canEject = g_drive_can_eject(dev.get()->getGDrive());
        bool canStop = g_drive_can_stop(dev.get()->getGDrive());
        if(canEject || canStop){
            isMobile = true;
        }
        qDebug() << "isMobile :" << isMobile;
    }
    return isMobile;
}

bool FileUtils::isSamePath(const QString &uri, const QString &targetUri)
{
    //computer:/// and file:///, favorite:/// path check
    if (uri.endsWith(":///") && targetUri.endsWith(":///"))
        return uri == targetUri;

    //return QUrl(uri).path() == QUrl(targetUri).path();
    //fix bug#84324
    return QUrl(urlEncode(uri)).path() == QUrl(urlEncode(targetUri)).path();
}

bool FileUtils::containsStandardPath(const QStringList &list)
{
    QStringList standardPaths = standardPathList();
    for(auto& uri:list){
        QUrl url = uri;
        if (standardPaths.contains(url.path())){
            return true;
        }
    }
    return false;
}

bool FileUtils::containsStandardPath(const QList<QUrl> &urls)
{
    QStringList standardPaths = standardPathList();
    for (QUrl url : urls) {
        if (standardPaths.contains(url.toDisplayString())) {
            return true;
        }
    }

    return false;
}

bool FileUtils::isFileExsit(const QString &uri)
{
    //FIXME: replace BLOCKING api in ui thread.
    bool exist = false;
    GFile *file = g_file_new_for_uri(FileUtils::urlEncode(uri).toUtf8().constData());
    exist = g_file_query_exists(file, nullptr);
    g_object_unref(file);
    return exist;
}

const QStringList FileUtils::toDisplayUris(const QStringList &args)
{
    QStringList uris;
    for (QString path : args) {
        QUrl url = path;
        if (url.scheme().isEmpty()) {
            if (path.startsWith ("/")) {
                uris << ("file://" + path);
            } else {
                g_autofree gchar* currentDir = g_get_current_dir();
                g_autofree gchar* file = g_strdup_printf ("file://%s/%s", currentDir, path.toUtf8 ().constData ());
                uris << file;
            }
        } else if (path.startsWith("mtp://") || path.startsWith("gphoto2://")) {
            uris << path;
        } else {
            uris << FileUtils::urlEncode (path);
        }
    }

    return uris;
}

bool FileUtils::isMountRoot(const QString &uri)
{
    //FIXME: replace BLOCKING api in ui thread.
    return false;
    GFile *file = g_file_new_for_uri(uri.toUtf8().constData());
    GFileInfo *info = g_file_query_info(file,
                                        "unix::is-mountpoint",
                                        G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                        nullptr,
                                        nullptr);
    g_object_unref(file);
    if (info) {
        bool isMount = g_file_info_get_attribute_boolean(info, "unix::is-mountpoint");
        g_object_unref(info);
        return isMount;
    }
    return false;
}

bool FileUtils::queryVolumeInfo(const QString &volumeUri, QString &volumeName, QString &unixDeviceName, const QString &volumeDisplayName)
{
    char *unix_dev_file = nullptr;

    if (!volumeUri.startsWith("computer:///"))
        return false;

    GFile *file = g_file_new_for_uri(volumeUri.toUtf8().constData());
    GFileInfo *info = g_file_query_info(file,
                                        G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE_FILE","G_FILE_ATTRIBUTE_STANDARD_TARGET_URI,
                                        G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                        nullptr,
                                        nullptr);
    g_object_unref(file);

    if (!info) {
        return false;
    }

    auto displayName = volumeDisplayName;
    if (displayName.isNull()) {
        displayName = getFileDisplayName(volumeUri);
    }

    unix_dev_file = g_file_info_get_attribute_as_string(info, G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE_FILE);//for computer:///xxx.drive
    if(!unix_dev_file){//for computer:///xxx.mount
        char *targetUri = g_file_info_get_attribute_as_string(info, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
        if(targetUri){
            char *realMountPoint = g_filename_from_uri(targetUri,NULL,NULL);
            const char *unix_dev = Peony::VolumeManager::getUnixDeviceFileFromMountPoint(realMountPoint);
            unixDeviceName = unix_dev;
            g_free(targetUri);
            g_free(realMountPoint);
        }
    }else{
        unixDeviceName = unix_dev_file;
        g_free(unix_dev_file);
    }

    auto list = displayName.split(":");
    if (list.count() > 1) {
        auto last = list.last();
        if (last.startsWith(" "))
            last.remove(0, 1);
        volumeName = last;
    } else {
        volumeName = displayName;
    }

    handleVolumeLabelForFat32(volumeName,unixDeviceName);
    return true;
}

bool FileUtils::isReadonly(const QString& uri)
{
    GFile *file = g_file_new_for_uri(uri.toUtf8().constData());
    GFileInfo *info = g_file_query_info(file, "access::*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
    g_object_unref(file);
    if (info) {
        bool read = g_file_info_get_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_READ);
        bool write = g_file_info_get_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
        bool execute = g_file_info_get_attribute_boolean(info, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE);
        if (read && !write && !execute) {
            return true;
        }
    }

    return false;
}

bool FileUtils::isFileDirectory(const QString &uri)
{
    bool isFolder = false;
    GFile *file = g_file_new_for_uri(uri.toUtf8().constData());
    isFolder = g_file_query_file_type(file,
                                      G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                      nullptr) == G_FILE_TYPE_DIRECTORY;
    g_object_unref(file);
    return isFolder;
}

bool FileUtils::isFileUnmountable(const QString &uri)
{
    GFile *file = g_file_new_for_uri(uri.toUtf8().constData());
    GFileInfo *info = g_file_query_info(file, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_UNMOUNT, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
    g_object_unref(file);
    if (info) {
        bool unmountable = g_file_info_get_attribute_boolean(info, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_UNMOUNT);
        g_object_unref(info);
        return unmountable;
    }
    return false;
}

/* @func:        convert a ascii string to unicode string. 将一个ascii字符串转换为unicode字符串
 * @gbkName      a string that needs to be converted from ascii to  Unicode. eg:"\\xb8\\xfc\\xd0\\xc2CODE"
 */
QString transcodeForGbkCode(QByteArray gbkName, QString &volumeName)
{
    int i;
    QByteArray dest,tmp;
    QString name;
    int len = gbkName.size();

    for(i = 0x0; i < len; ++i){
        if(92 == gbkName.at(i)){
            if(4 == tmp.size())
                dest.append(QByteArray::fromHex(tmp));
            else{
                if(tmp.size() > 4){
                    dest.append(QByteArray::fromHex(tmp.left(4)));
                    dest.append(tmp.mid(4));
                }else
                    dest.append(tmp);
            }
            tmp.clear();
            tmp.append(gbkName.at(i));
            continue;
        }else if(tmp.size() > 0){
            tmp.append(gbkName.at(i));
            continue;
        }else
            dest.append(gbkName.at(i));
    }

    if(4 == tmp.size())
        dest.append(QByteArray::fromHex(tmp));
    else{
        if(tmp.size() > 4){
            dest.append(QByteArray::fromHex(tmp.left(4)));
            dest.append(tmp.mid(4));
        }else
            dest.append(tmp);
    }

    /*
    * gio的api获取的卷名和/dev/disk/by-label先的名字不一致，有可能是卷名
    * 中含有特殊字符，导致/dev/disk/label下的卷名含有转义字符，导致二者的名字不一致
    * 而不是编码格式的不一致导致的，比如卷名：“数据光盘(2020-08-22)”，在/dev/disk/by-label
    * 写的名字:"数据光盘\x282020-08-22\x29",经过上述处理之后可以去除转义字符，在判断一次
    * 是否相等。比较完美的解决方案是找到能够判断字符串的编码格式，目前还没有找到实现方式，需要进一步完善
    */
    name = QString(dest);
    if (name == volumeName){
        return name;
    }

    name = QTextCodec::codecForName("GBK")->toUnicode(dest);
    //name = QTextCodec::codecForLocale()->toUnicode(dest);
    return name;
}

/* @func:           calculate all files size, recursively calculate folder files size
 * @uri             file uri 文件uri, 如果是文件夹将会递归计算子文件大小
 */
quint64 FileUtils::getFileTotalSize(const QString &uri)
{
    auto info = FileInfo::fromUri(uri);
    if (info->isDir())
    {
        guint64 disk_usage = 0, num_dirs = 0, num_files = 0;
        GFile          *m_file = g_file_new_for_uri(uri.toUtf8().constData());
        GCancellable   *m_cancel = g_cancellable_new();
        GError         *err = nullptr;
        g_file_measure_disk_usage (m_file,
                                   G_FILE_MEASURE_NONE,
                                   m_cancel,
                                   nullptr,
                                   nullptr,
                                   &disk_usage,
                                   &num_dirs,
                                   &num_files,
                                   &err);

        if (err){
            qWarning()<< "getFileTotalSize has error:" <<err->code<<err->message<<disk_usage;
        }

        g_object_unref(m_file);
        g_object_unref(m_cancel);

        return disk_usage;
    }

    //if not have size info, need update it
    if (info->isEmptyInfo())
    {
        auto infoJob = new FileInfoJob(info);
        infoJob->setAutoDelete();
        infoJob->querySync();
    }

    return info->size();
}

/* @func:           determines whether the @volumeName needs to be transcoded. 判断字符串是否需要转码.
 * @volumeName      a string that needs to be converted from ascii to  Unicode. eg:"\\xb8\\xfc\\xd0\\xc2CODE"
 * @unixDeviceName  a device name. eg: /dev/sdb
 */
void FileUtils::handleVolumeLabelForFat32(QString &volumeName,const QString &unixDeviceName){
    if (unixDeviceName.isEmpty())
        return;

    GVolumeMonitor *vm = g_volume_monitor_get();
    GList *volumes = g_volume_monitor_get_volumes(vm);
    GList *l = volumes;
    while (l) {
        GVolume *volume = static_cast<GVolume *>(l->data);
        g_autofree char *volume_unix_dev = g_volume_get_identifier(volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        if (unixDeviceName == volume_unix_dev) {
            g_autofree char *volume_name = g_volume_get_name(volume);
            volumeName = volume_name;
            break;
        }
        l = l->next;
    }
    g_list_free_full(volumes, g_object_unref);
    g_object_unref(vm);
    return;

    QFileInfoList diskList;
    QFileInfo diskLabel;
    QDir diskDir;
    QString partitionName,linkTarget;
    QString tmpName,finalName;
    int i;
    QRegExp rx("[^\u4e00-\u9fa5]");		//non-chinese characters

    diskDir.setPath("/dev/disk/by-label");
    if(!diskDir.exists())               //this means: volume has no name.
        return;                         //            or there no mobile devices.

    diskList = diskDir.entryInfoList(); //all file from dir.
    /* eg: unixDeviceName == "/dev/sdb4"
     *     partitionName == "sdb4"
     */
    partitionName = unixDeviceName.mid(unixDeviceName.lastIndexOf('/')+1);

    for(i = 0; i < diskList.size(); ++i){
        diskLabel = diskList.at(i);
        linkTarget = diskLabel.symLinkTarget();
        linkTarget = linkTarget.mid(linkTarget.lastIndexOf('/')+1);
        if(linkTarget == partitionName)
            break;
        linkTarget.clear();
    }

    if(!linkTarget.isEmpty())
        tmpName = diskLabel.fileName();//可能带有乱码的名字

    if(!tmpName.isEmpty()){
        if(tmpName == volumeName || !tmpName.contains(rx)){      //ntfs、exfat格式或者非纯中文名的fat32设备,这个设备的名字不需要转码
            volumeName = tmpName;
            return;
        } else {
            finalName = transcodeForGbkCode(tmpName.toLocal8Bit(), volumeName);
            if(!finalName.isEmpty())
                volumeName = finalName;
        }
    }
}


/* @Func: return abstract device path
 * @uri : such as "computer:///xxx.drive"
 * @return: nullptr or such as "/dev/sdb1"
 */
QString FileUtils::getUnixDevice(const QString &uri)
{
    GFile* file;
    GFileInfo* fileInfo;
    GCancellable* cancel;
    QString devicePath,targetUri;
    const char *tmpPath;
    char *mountPoint;

    if(uri.isEmpty())
        return nullptr;

    cancel = g_cancellable_new();
    file = g_file_new_for_uri(uri.toUtf8().constData());
    if(!file ||!cancel)
        return nullptr;

    //query device path by "mountable::unix-device-file"
    fileInfo = g_file_query_info(file,"*",G_FILE_QUERY_INFO_NONE,cancel,NULL);
    g_autofree gchar* tmp_path = g_file_info_get_attribute_as_string(fileInfo,G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE_FILE);
    tmpPath = tmp_path;
    devicePath = tmpPath;
    if(!devicePath.isEmpty()){
        g_object_unref(fileInfo);
        g_cancellable_cancel(cancel);
        g_object_unref(cancel);
        g_object_unref(file);
        return devicePath;
    }

    //query device path by "standard::target-uri"
    targetUri = getTargetUri(uri);
    if(targetUri.isEmpty())
        return nullptr;

    mountPoint = g_filename_from_uri(targetUri.toUtf8().constData(),NULL,NULL);
    if(mountPoint)
        tmpPath = Peony::VolumeManager::getUnixDeviceFileFromMountPoint(mountPoint);
    devicePath = tmpPath;

    g_free(mountPoint);
    g_object_unref(fileInfo);
    g_cancellable_cancel(cancel);
    g_object_unref(cancel);
    g_object_unref(file);

    return devicePath;
}

double FileUtils::getDeviceSize(const gchar * device_name)
{
    struct stat statbuf;
    const gchar *crypto_backing_device;
    UDisksObject *object, *crypto_backing_object;
    UDisksBlock *block;
    UDisksClient *client =udisks_client_new_sync (NULL,NULL);
    object = NULL;
    if (stat (device_name, &statbuf) != 0)
    {
        return -1;
    }

    block = udisks_client_get_block_for_dev (client, statbuf.st_rdev);
    if (block == NULL)
    {
        return -1;
    }

    object = UDISKS_OBJECT (g_dbus_interface_dup_object (G_DBUS_INTERFACE (block)));
    g_object_unref (block);

    crypto_backing_device = udisks_block_get_crypto_backing_device ((udisks_object_get_block (object)));
    crypto_backing_object = udisks_client_get_object (client, crypto_backing_device);
    if (crypto_backing_object != NULL)
    {
        g_object_unref (object);
        object = crypto_backing_object;
    }

    block = udisks_object_get_block (object);
    guint64 size = udisks_block_get_size(block);
    double volume_size =(double)size/1024/1024/1024;

    g_clear_object(&client);
    g_object_unref(object);
    g_object_unref(block);

    return volume_size;
}

quint64 FileUtils::getDiskFreeSpace(const gchar * path, bool &isState)
{
    struct statvfs vfs;
    quint64 freeSize = 0;
    auto state = statvfs(path, &vfs);
    if(0 > state) {
        isState = false;
        qWarning() << "read statvfs error";
    } else {
        freeSize = vfs.f_bavail * vfs.f_bsize;
    }
    return freeSize;
}

quint64 FileUtils::getFileSystemSize(QString uri)
{
    QString unixDevice,dbusPath;
    quint64 total = 0;

    unixDevice = FileUtils::getUnixDevice(uri);

    //related bug#95731, encrypted data disk show property crash issue
    if (unixDevice.isEmpty() ||
        ! (unixDevice.startsWith("/dev/sd") || unixDevice.startsWith("/dev/sr"))) {
        return total;
    }
    dbusPath = "/org/freedesktop/UDisks2/block_devices/" + unixDevice.split("/").last();
    if (! QDBusConnection::systemBus().isConnected())
        return total;
    QDBusInterface blockInterface("org.freedesktop.UDisks2",
                                  dbusPath,
                                  "org.freedesktop.UDisks2.Filesystem",
                                  QDBusConnection::systemBus());

    if(blockInterface.isValid())
        total = blockInterface.property("Size").toULongLong();

    return total;
}

QString FileUtils::getFileSystemType(QString uri)
{
    QString unixDevice,dbusPath;
    QString fsType = "";

    unixDevice = getUnixDevice(uri);

    //fix bug#95731, encrypted data disk show property crash issue
    //encrypted disk unixDevice name is like /dev/mapper/lingmo--vg-data
    if (unixDevice.isEmpty() ||
        ! (unixDevice.startsWith("/dev/sd") || unixDevice.startsWith("/dev/sr"))) {
        return fsType;
    }
    dbusPath = "/org/freedesktop/UDisks2/block_devices/" + unixDevice.split("/").last();
    if (! QDBusConnection::systemBus().isConnected())
        return fsType;
    QDBusInterface blockInterface("org.freedesktop.UDisks2",
                                  dbusPath,
                                  "org.freedesktop.UDisks2.Block",
                                  QDBusConnection::systemBus());

    if(blockInterface.isValid())
        fsType = blockInterface.property("IdType").toString();

    //if need diff FAT16 and FAT32, should use IdVersion
//    if(fsType == "" && blockInterface.isValid())
//        fsType = blockInterface.property("IdVersion").toString();

    return fsType;
}

QString FileUtils::getMobieDataPath()
{
    //path like "file:///var/lib/kmre/data/kmre-1000-lingmo/KmreData"
    //file:///var/lib/kmre/data/kmre-1000-hemh/KmreData
    //1000 is uuid, lingmo is username
    QString prePath = "/var/lib/kmre/data";
    //user infos
    auto user = LinuxPWDHelper::getCurrentUser();
    QString sufPath =  QString("/kmre-%1-%2/KmreData").arg(user.userId()).arg(user.userName());
    QString completePath = prePath + sufPath;
    qDebug() <<"getMobieDataPath:" <<completePath;
    if (QFile::exists(completePath))
        return "file://" + completePath;
    else
        return "";
}

QString FileUtils::getFileSystemId(QString uri)
{
    if (nullptr == uri) return "";

    QString systemId = "";
    auto file = wrapGFile(g_file_new_for_uri(uri.toUtf8().constData()));
    auto info = wrapGFileInfo(g_file_query_info(file.get()->get(),
        G_FILE_ATTRIBUTE_ID_FILESYSTEM,
        G_FILE_QUERY_INFO_NONE,
        nullptr,
        nullptr));
    if (!G_IS_FILE_INFO (info.get()->get()))
        return systemId;

    if (info) {
        systemId = g_file_info_get_attribute_string(info.get()->get(), G_FILE_ATTRIBUTE_ID_FILESYSTEM);
    }

    return systemId;
}

bool FileUtils::isRemoteServerUri(const QString &uri)
{
    if(uri.startsWith("smb://") || uri.startsWith("ftp://") || uri.startsWith("sftp://"))
        return true;

    return false;
}

bool FileUtils::isEmptyDisc(const QString &unixDevice)
{
    if (unixDevice.isEmpty()) //没有设备时不做后续处理
        return false;

    if (!QDBusConnection::systemBus().isConnected())
        return false;

    /* 通过Properties获取Drive的path */
    QString  dbusPath = "/org/freedesktop/UDisks2/block_devices/" + unixDevice.split("/").last();
    QDBusInterface PropertiesIf("org.freedesktop.UDisks2",
                                  dbusPath,
                                  "org.freedesktop.DBus.Properties",
                                  QDBusConnection::systemBus());
    if(!PropertiesIf.isValid())
        return false;

    QDBusReply<QDBusVariant> reply = PropertiesIf.call("Get", "org.freedesktop.UDisks2.Block", "Drive");
    if(!reply.isValid())
        return false;

    QDBusObjectPath* busObjectPath = (QDBusObjectPath*)(reply.value().variant().data());
    if(!busObjectPath)
        return false;

    QString drivePath = busObjectPath->path();//end

    /* 获取Drive的"OpticalBlank"属性，判断是否是空光盘 */
    QDBusInterface driveInterface("org.freedesktop.UDisks2",
                                  drivePath,
                                  "org.freedesktop.UDisks2.Drive",
                                  QDBusConnection::systemBus());

    bool isBlank = false;
    if(driveInterface.isValid()){
        isBlank = driveInterface.property("OpticalBlank").toBool(); /* 获取"OpticalBlank"属性值 */
    }

    return isBlank;
}

bool FileUtils::isBusyDevice(const QString &unixDevice)
{
    if (unixDevice.isEmpty()) /* 没有设备时不做后续处理 */
        return false;

    QDBusMessage msg = QDBusMessage::createMethodCall("com.lingmo.burner.manager", "/com/lingmo/burner/manager",
                     "com.lingmo.burner.manager", "isBusyDevice");
    QList<QVariant> args;
    args.append(QVariant(unixDevice));
    msg.setArguments(args);
    QDBusMessage response = QDBusConnection::sessionBus().call(msg);

    bool isBusy = false;
    if (response.type() == QDBusMessage::ReplyMessage){
        isBusy = response.arguments().takeFirst().toBool();
    }
    return isBusy;
}

QString FileUtils::getIconStringFromGIcon(GIcon *gicon, QString deviceFile)
{
    QString iconName;
    if (G_IS_THEMED_ICON (gicon)) {
        const char * const * icon_names = g_themed_icon_get_names((GThemedIcon *)gicon);
        if(icon_names) {
            //iconName = *icon_names;
            auto p = icon_names;
            while (*p) {
                QIcon icon = QIcon::fromTheme(*p);
                if (!icon.isNull()) {
                    iconName = QString (*p);
                    break;
                } else {
                    p++;
                }
            }

            // fix #81852, refer to #57660, #70014, #96652, task #25343
            if (QString(iconName) == "drive-harddisk-usb") {
                double size = 0.0;
                if(!deviceFile.isEmpty()){
                    size = Peony::FileUtils::getDeviceSize(deviceFile.toUtf8().constData());
                    if (size < 128) {
                        iconName = "drive-removable-media-usb";
                    }
                }
            }
        }
    } else if (G_IS_FILE_ICON (gicon)) {
        g_autofree gchar *icon_name = g_icon_to_string(gicon);
        iconName = icon_name;
    } else if (G_IS_EMBLEMED_ICON (gicon)) {
        GIcon *icon_emblemed = g_emblemed_icon_get_icon((GEmblemedIcon *)(gicon));
        const char * const * icon_names = g_themed_icon_get_names((GThemedIcon *)icon_emblemed);
        if(icon_names) {
            iconName = *icon_names;
        }
    }
    return iconName;
}

QString FileUtils::getIconStringFromGIconThreadSafety(GIcon *gicon, QString deviceFile)
{
    QString iconName;
    if (G_IS_THEMED_ICON (gicon)) {
        const char * const * icon_names = g_themed_icon_get_names((GThemedIcon *)gicon);
        if(icon_names) {
            //iconName = *icon_names;

            static QThread *uiThread = qApp->thread();
            if (uiThread == QThread::currentThread()) {
                auto p = icon_names;
                while (*p) {
                    QIcon icon = QIcon::fromTheme(*p);
                    if (!icon.isNull()) {
                        iconName = QString (*p);
                        break;
                    } else {
                        p++;
                    }
                }
            } else {
                iconName = *icon_names;
            }

            // fix #81852, refer to #57660, #70014, #96652, task #25343
            if (QString(iconName) == "drive-harddisk-usb") {
                double size = 0.0;
                if(!deviceFile.isEmpty()){
                    size = Peony::FileUtils::getDeviceSize(deviceFile.toUtf8().constData());
                    if (size < 128) {
                        iconName = "drive-removable-media-usb";
                    }
                }
            }
        }
    } else if (G_IS_FILE_ICON (gicon)) {
        g_autofree gchar *icon_name = g_icon_to_string(gicon);
        iconName = icon_name;
    } else if (G_IS_EMBLEMED_ICON (gicon)) {
        GIcon *icon_emblemed = g_emblemed_icon_get_icon((GEmblemedIcon *)(gicon));
        const char * const * icon_names = g_themed_icon_get_names((GThemedIcon *)icon_emblemed);
        if(icon_names) {
            iconName = *icon_names;
        }
    }
    return iconName;
}

QString FileUtils::handleSpecialSymbols(const QString &displayName)
{
    QString tmpStr = displayName;
    if (displayName.contains("&")) {
        tmpStr = tmpStr.replace("&", "&&");
    }
    return tmpStr;
}

QString FileUtils::getFsTypeFromFile(const QString &fileUri)
{
    QString fsType = "ext";

    g_autoptr (GFile) file = g_file_new_for_uri(fileUri.toUtf8().constData());
    g_autoptr (GMount) mount = g_file_find_enclosing_mount(file, nullptr, nullptr);
    if (!mount)
        return "ext";

    g_autoptr (GVolume) volume = g_mount_get_volume(mount);
    if (!volume)
        return fsType;

    g_autofree gchar* unix_file = g_volume_get_identifier(volume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
    if (!unix_file)
        return fsType;

    QString unixDevice = unix_file;

    // try fix #179725
    if (unixDevice.startsWith("/dev/dm")) {
        return "ext";
    }

    QString dbusPath = "/org/freedesktop/UDisks2/block_devices/" + unixDevice.split("/").last();
    if (! QDBusConnection::systemBus().isConnected())
        return fsType;
    QDBusInterface blockInterface("org.freedesktop.UDisks2",
                                  dbusPath,
                                  "org.freedesktop.UDisks2.Block",
                                  QDBusConnection::systemBus());

    if(blockInterface.isValid())
        fsType = blockInterface.property("IdType").toString();

    //if need diff FAT16 and FAT32, should use IdVersion
//    if(fsType == "" && blockInterface.isValid())
//        fsType = blockInterface.property("IdVersion").toString();

    return fsType;
}

bool FileUtils::isFuseFileSystem(const QString &fileUri)
{
    g_autoptr (GFile) file = g_file_new_for_uri(fileUri.toUtf8().constData());
    g_autoptr (GFile) parent = g_file_get_parent(file);
    auto path = g_file_peek_path(parent);
    if (!path) {
        return false;
    }
    g_autoptr (GUnixMountEntry) entry = g_unix_mount_at(path, NULL);
    if (!entry) {
        entry = g_unix_mount_for(path, NULL);
        if (!entry) {
            return false;
        }
    }
    auto fsType = g_unix_mount_get_fs_type(entry);
    if (QString(fsType).contains("fuse.kyfs")) {
        return true;
    } else {
        return false;
    }
}
bool FileUtils::isLongNameFileOfNotDel2Trash(const QString &fileUri)
{
    /* 存放长文件名目录的下，判断文件名超过224字符的，右键删除选项改成永久删除，不删除到回收站。link bug#188864  */
    QString extendDir = "file://" +  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/扩展";/* 长文件名文件存放在家目录/下载/扩展目录下 */
    QString fileDecodeUri = urlDecode(fileUri);
    if(!fileDecodeUri.startsWith(extendDir))
        return false;

    QString baseName = Peony::FileUtils::getUriBaseName(fileDecodeUri);
    qDebug()<<"file decodeUri:"<<fileDecodeUri<<";base name:"<<baseName<<";length of base name:"<<baseName.length();
    if(224 < baseName.length())
        return true;

    return false;
}

QString FileUtils::getActualDirFromSearchUri(const QString &searchUri)
{
    QString actualDir =  searchUri;
    if(actualDir.startsWith("search:///search_uris=")){
        QUrl url(actualDir);
        QString uri = url.path().section("&", 0, 0).replace("/search_uris=", "");
        actualDir = FileUtils::getEncodedUri(uri);
    }
    return actualDir;
}

QString FileUtils::updateFileIconName(const QString &uri, bool checkValid)
{
    if (nullptr == uri) return "";

    auto file = wrapGFile(g_file_new_for_uri(uri.toUtf8().constData()));
    auto info = wrapGFileInfo(g_file_query_info(file.get()->get(),
                                                G_FILE_ATTRIBUTE_STANDARD_ICON,
                                                G_FILE_QUERY_INFO_NONE,
                                                nullptr,
                                                nullptr));
    if (!G_IS_FILE_INFO (info.get()->get()))
        return nullptr;
    GIcon *g_icon = g_file_info_get_icon (info.get()->get());
    QString icon_name;
    //do not unref the GIcon from info.
    if (g_icon && G_IS_ICON(g_icon)) {
        const gchar* const* icon_names = g_themed_icon_get_names(G_THEMED_ICON (g_icon));
        if (icon_names) {
            auto p = icon_names;
            icon_name = QString(icon_names[0]);
            if (checkValid) {
                while (*p) {
                    QIcon icon = QIcon::fromTheme(*p);
                    if (!icon.isNull()) {
                        icon_name = QString (*p);
                        break;
                    } else {
                        p++;
                    }
                }
            }
        } else {
            //if it's a bootable-media,maybe we can get the icon from the mount directory.
            char *bootableIcon = g_icon_to_string(g_icon);
            if(bootableIcon){
                icon_name = QString(bootableIcon);
                g_free(bootableIcon);
            }
        }
    }

    return icon_name;
}

QString FileUtilsPrivate::getFileIconName(const QString &uri)
{
    if (nullptr == uri) return "";

    auto file = wrapGFile(g_file_new_for_uri(uri.toUtf8().constData()));
    auto info = wrapGFileInfo(g_file_query_info(file.get()->get(),
        G_FILE_ATTRIBUTE_STANDARD_ICON,
        G_FILE_QUERY_INFO_NONE,
        nullptr,
        nullptr));
    if (!G_IS_FILE_INFO (info.get()->get()))
        return nullptr;
    GIcon *g_icon = g_file_info_get_icon (info.get()->get());
    QString icon_name;
    //do not unref the GIcon from info.
    if (g_icon && G_IS_ICON(g_icon)) {
        const gchar* const* icon_names = g_themed_icon_get_names(G_THEMED_ICON (g_icon));
        if (icon_names) {
            icon_name = QString(icon_names[0]);
        } else {
            //if it's a bootable-media,maybe we can get the icon from the mount directory.
            char *bootableIcon = g_icon_to_string(g_icon);
            if(bootableIcon){
                icon_name = QString(bootableIcon);
                g_free(bootableIcon);
            }
        }
    }

    return icon_name;
}

void FileUtils::saveCreateTime(const QString &url)
{
    g_autoptr (GError) error = NULL;
    g_autoptr (GFile) file = url.startsWith ("file://") ? g_file_new_for_uri (url.toUtf8 ().constData ()) : g_file_new_for_path (url.toUtf8 ().constData ());
    g_autofree gchar* currentTime = g_strdup_printf ("%ld", g_get_real_time ());

    g_return_if_fail (G_IS_FILE (file) && g_file_query_exists (file, NULL));
    g_file_set_attribute (file, "metadata::CreateTime", G_FILE_ATTRIBUTE_TYPE_STRING, currentTime, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);

    if (error)      qDebug () << "set create time error: " << error->message;
}

gint64 FileUtils::getCreateTimeOfMicro(const QString &url)
{
    g_autoptr (GError) error = NULL;
    g_autoptr (GFile) file = g_file_new_for_uri (url.toUtf8 ().constData ());
    g_autoptr (GFileInfo) fileInfo = g_file_query_info (file, G_FILE_ATTRIBUTE_TIME_CHANGED "," G_FILE_ATTRIBUTE_TIME_CREATED "," "metadata::CreateTime", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, &error);
    g_return_val_if_fail (G_IS_FILE (file) && G_IS_FILE_INFO (fileInfo) && g_file_query_exists (file, NULL), 0);

    if (g_file_info_has_attribute (fileInfo, G_FILE_ATTRIBUTE_TIME_CREATED)) {
        gint64 createTime = g_file_info_get_attribute_uint64 (fileInfo, G_FILE_ATTRIBUTE_TIME_CREATED);
        gint64 modifyTime = g_file_info_get_attribute_uint64 (fileInfo, G_FILE_ATTRIBUTE_TIME_CHANGED);
        if (createTime != 0 && createTime <= modifyTime) {
            return createTime;
        }
    }

    if (g_file_info_has_attribute (fileInfo, "metadata::CreateTime")) {
        const gchar* createTimeStr = g_file_info_get_attribute_string (fileInfo, "metadata::CreateTime");
        if (createTimeStr) {
            g_autofree char* createTime10 = g_strndup (createTimeStr, 10);
            return atoll (createTime10);
        }
    }

    return 0;
}
