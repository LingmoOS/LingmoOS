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
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#ifndef FILEINFO_H
#define FILEINFO_H

#include "explor-core_global.h"

#include <memory>
#include <gio/gio.h>
#include <QString>
#include <QObject>

#include <QMutex>

#include <QIcon>
#include <QColor>

namespace Peony {

class FileInfoJob;
class FileMetaInfo;

static const char *office_mime_types[] = {
    "application/wps-office.doc",
    "application/msword",
    "application/vnd.ms-word",
    "application/x-msword",
    "application/vnd.ms-word.document.macroenabled.12",
    "application/wps-office.dot",
    "application/msword-template",
    "application/vnd.ms-word.template.macroenabled.12",
    "application/wps-office.dotx",
    "application/vnd.openxmlformats-officedocument.wordprocessingml.template",
    "application/wps-office.docx",
    "application/vnd.openxmlformats-officedocument.wordprocessingml.document",
    "application/wps-office.wps",
    "application/vnd.ms-works",

    "application/wps-office.wpt",
    "application/wps-office.ppt",
    "application/vnd.ms-powerpoint",
    "application/powerpoint",
    "application/mspowerpoint",
    "application/x-mspowerpoint",
    "application/vnd.ms-powerpoint.presentation.macroenabled.12",
    "application/wps-office.pot",
    "application/vnd.ms-powerpoint.template.macroenabled.12",
    "application/wps-office.potx",
    "application/vnd.openxmlformats-officedocument.presentationml.template",
    "application/wps-office.pptx",
    "application/vnd.openxmlformats-officedocument.presentationml.presentation",
    "application/vnd.openxmlformats-officedocument.presentationml.slideshow",
    "application/wps-office.dps",
    "application/wps-office.dpt",

    "application/wps-office.xls",
    "application/vnd.ms-excel",
    "application/msexcel",
    "application/x-msexcel",
    "application/vnd.ms-excel.sheet.macroenabled.12",
    "application/vnd.ms-excel.template.macroenabled.12",
    "application/wps-office.xlt",
    "application/wps-office.xltx",
    "application/vnd.openxmlformats-officedocument.spreadsheetml.template",
    "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
    "application/wps-office.et",
    "application/wps-office.ett",
    "end"
};

/*!
 * \brief The FileInfo class
 * <br>
 * This class represents the information of this file and file itself.
 * Although you can use constructor new a FileInfo instance,
 * you should use FileInfo::fromUri(). Because it provides an shared data,
 * and FileInfoJob need hold a shared_ptr reference, too.
 * This will help to reduce the risk of memory leaks.
 * </br>
 */
class PEONYCORESHARED_EXPORT FileInfo : public QObject
{
    friend class FileInfoJob;
    friend class FileMetaInfo;
    friend class FileInfosJob;

    Q_OBJECT
public:
    enum Access {
        Readable,
        Writeable,
        Executable,
        Deleteable,
        Trashable,
        Renameable
    };
    Q_DECLARE_FLAGS(AccessFlags, Access)

    explicit FileInfo(QObject *parent = nullptr);
    explicit FileInfo(const QString &uri, QObject *parent = nullptr);
    ~FileInfo();
    /*!
     * \brief fromUri
     * \param uri
     * \param addToHash
     * \return
     * \deprecated
     */
    static std::shared_ptr<FileInfo> fromUri(QString uri);
    static std::vector<std::shared_ptr<FileInfo> > fromUris(QStringList uris);
    /*!
     * \brief fromPath
     * \param path
     * \param addToHash
     * \return
     * \deprecated
     */
    static std::shared_ptr<FileInfo> fromPath(QString path);
    /*!
     * \brief fromGFile
     * \param file
     * \param addToHash
     * \return
     * \deprecated
     */
    static std::shared_ptr<FileInfo> fromGFile(GFile *file);

    std::shared_ptr<FileMetaInfo> metainfo() {
        return m_meta_info;
    }
    QString uri() {
        return m_uri;
    }
    bool isDir() {
        return m_is_dir || m_content_type == "inode/directory";
    }
    bool isVolume() {
        return m_is_volume;
    }
    bool isSymbolLink() {
        return m_is_symbol_link;
    }
    bool isVirtual() {
        return m_is_virtual;
    }
    bool isValid()
    {
        return m_is_valid;
    }

    QString desktopName(){
        return m_desktop_name;
    }

    QString iconName() {
        return m_icon_name;
    }
    QString symbolicIconName() {
        return m_symbolic_icon_name;
    }
    QString fileID() {
        return m_file_id;
    }
    QString mimeType() {
        return m_mime_type_string;
    }

    QString displayFileType();

    QString fileType() {
        return m_file_type;
    }

    QString filePath() {
        return m_path;
    }

    QString fileSize() {
        return m_file_size;
    }
    QString modifiedDate() {
        return m_modified_date;
    }
    QString accessDate() {
        return m_access_date;
    }
    QString deletionDate() {
        return m_deletion_date;
    }
    QString createDate() {
        return m_create_date;
    }

    QString type() {
        return m_content_type;
    }

    QString fileSystemType() {
        return m_fs_type;
    }

    quint64 size() {
        return m_size;
    }
    quint64 modifiedTime() {
        return m_modified_time;
    }
    quint64 accessTime() {
        return m_access_time;
    }
    quint64 deletionTime() {
        return m_deletion_date_uint64;
    }
    quint64 createTime() {
        return m_create_time;
    }

    QList<QColor> getColors() {
        return m_colors;
    }
    bool canRead() {
        return m_can_read;
    }
    bool canWrite() {
        return m_can_write;
    }
    bool canExecute() {
        return m_can_excute;
    }
    bool canDelete() {
        return m_can_delete;
    }
    bool canTrash() {
        return m_can_trash;
    }
    bool canRename() {
        return m_can_rename;
    }

    bool canMount() {
        return m_can_mount;
    }
    bool canUnmount() {
        return m_can_unmount;
    }
    bool canEject() {
        return m_can_eject;
    }
    bool canStart() {
        return m_can_start;
    }
    bool canStop() {
        return m_can_stop;
    }

    bool isDesktopFile() {
        return m_can_excute && m_uri.endsWith(".desktop");
    }

    bool isPdfFile(){
        return m_mime_type_string.contains("pdf");
    }

    bool isImageFile(){
        return m_mime_type_string.startsWith("image/");
    }

    bool isImagePdfFile(){
        return m_mime_type_string.contains("djvu");
    }

    bool isVideoFile();

    bool isAudioFile();

    bool isOfficeFile();

    bool isEmptyInfo() {
        return m_display_name == nullptr || m_display_name == "";
    }
    // 是否禁止执行程序
    bool isExecDisable();

    AccessFlags accesses() {
        auto flags = AccessFlags();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
        flags.setFlag(Readable, m_can_read);
        flags.setFlag(Writeable, m_can_write);
        flags.setFlag(Executable, m_can_excute);
        flags.setFlag(Deleteable, m_can_delete);
        flags.setFlag(Trashable, m_can_trash);
        flags.setFlag(Renameable, m_can_rename);
        return flags;
#else
        flags = 0;
        if (m_can_read)
            flags |= Readable;
        if (m_can_write)
            flags |= Writeable;
        if (m_can_excute)
            flags |= Executable;
        if (m_can_delete)
            flags |= Deleteable;
        if (m_can_trash)
            flags |= Trashable;
        if (m_can_rename)
            flags |= Renameable;
#endif
    }

    GFile *gFileHandle() {
        return m_file;
    }

    const QString targetUri();
    const QString displayName();/* 返回最终的显示名 */
    const QString symlinkTarget();
    const QString unixDeviceFile();

    const QString customIcon();

    quint64 getDeletionDateUInt64();
    const QString getFinalDisplayName();/* 通过m_display_name获取最终的显示名 */

    guint64 getCreateTime() const;
    QString getCreateDate() const;

    //const QIcon thumbnail() {return m_thumbnail;}
    //void setThumbnail(const QIcon &thumbnail) {m_thumbnail = thumbnail;}
    FileInfo &operator=(const FileInfo &other);
    QString updateIconName(const QString& uri, const QString& iconName) const;

Q_SIGNALS:
    void updated();

private:
    /* 如果新增了成员变量，需按需更新“&operator=、fileInfoJob::refreshInfoContents、fileInfosJob::refreshInfoContents”等接口！！！后续考虑优化。 */
    QString m_uri = nullptr;
    bool m_is_valid = false;
    bool m_is_dir = false;
    bool m_is_volume = false;
    bool m_is_remote = false;
    bool m_is_symbol_link = false;
    bool m_is_virtual = false;

    bool m_is_loaded = false;

    QString m_display_name = nullptr;
    QString m_desktop_name = nullptr;
    QString m_icon_name = nullptr;
    QString m_symbolic_icon_name = nullptr;
    QString m_file_id = nullptr;
    QString m_path = nullptr;
    QString m_content_type = nullptr;
    guint64 m_size = 0;
    guint64 m_modified_time = 0;
    guint64 m_access_time = 0;
    guint64 m_deletion_date_uint64 = 0;


    /*!
     * \deprecated
     * \brief m_mime_type_string
     * \see m_content_type
     */
    QString m_mime_type_string = nullptr;
    QString m_file_type = nullptr;
    QString m_file_size = nullptr;
    QString m_modified_date = nullptr;
    QString m_access_date = nullptr;
    QString m_deletion_date = nullptr;

    //access
    bool m_can_read = true;
    bool m_can_write = false;
    bool m_can_excute = false;
    bool m_can_delete = false;
    bool m_can_trash = false;
    bool m_can_rename = false;

    bool m_can_mount = false;
    bool m_can_unmount = false;
    bool m_can_eject = false;

    bool m_can_start = false;
    bool m_can_stop = false;
    QString m_unix_device_file;

    //FIXME: should i use smart pointer wrap these data?
    GFile *m_file = nullptr;
    GFile *m_parent = nullptr;

    QString m_target_uri;
    QString m_symlink_target;

    // filesystem
    QString m_fs_type;

    /*!
     * \brief m_cancellable
     * This cancellable is used in async query file info in FileInfoJob instance.
     */
    GCancellable *m_cancellable = nullptr;

    //QIcon m_thumbnail;
    std::shared_ptr<FileMetaInfo> m_meta_info = nullptr;

    QList<QColor> m_colors;

    QMutex m_mutex;

    QString m_finalDisplayName; /* 最终的显示名，查询获取后缓存起来 */

    guint64 m_create_time = 0;
    QString m_create_date = nullptr;
};

}

#endif // FILEINFO_H
