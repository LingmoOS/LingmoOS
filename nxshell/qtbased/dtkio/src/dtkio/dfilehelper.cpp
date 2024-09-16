// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfilehelper.h"

#include <QDebug>

#include "dtkiotypes.h"

DIO_BEGIN_NAMESPACE

namespace LocalFunc {

bool isFile(GFileInfo *fileInfo)
{
    if (!fileInfo)
        return false;
    return g_file_info_get_file_type(fileInfo) == G_FILE_TYPE_REGULAR;
}

bool isDir(GFileInfo *fileInfo)
{
    if (!fileInfo)
        return false;
    return g_file_info_get_file_type(fileInfo) == G_FILE_TYPE_DIRECTORY;
}

bool isRoot(const QUrl &url)
{
    if (!url.isValid())
        return false;

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(url);
    g_autofree gchar *gpath = g_file_get_path(gfile);   // no blocking I/O

    return QString::fromStdString(gpath) == "/";
}

QString fileName(GFileInfo *fileInfo)
{
    if (!fileInfo)
        return QString();

    const char *name = g_file_info_get_name(fileInfo);
    return QString::fromStdString(name);
}

QString baseName(GFileInfo *fileInfo)
{
    const QString &fullName = fileName(fileInfo);

    if (isDir(fileInfo))
        return fullName;

    int pos2 = fullName.indexOf(".");
    if (pos2 == -1)
        return fullName;
    else
        return fullName.left(pos2);
}

QString completeBaseName(GFileInfo *fileInfo)
{
    const QString &fullName = fileName(fileInfo);

    if (isDir(fileInfo))
        return fullName;

    int pos2 = fullName.lastIndexOf(".");
    if (pos2 == -1)
        return fullName;
    else
        return fullName.left(pos2);
}

QString suffix(GFileInfo *fileInfo)
{
    // path
    if (isDir(fileInfo))
        return "";

    const QString &fullName = fileName(fileInfo);

    int pos2 = fullName.lastIndexOf(".");
    if (pos2 == -1)
        return "";
    else
        return fullName.mid(pos2 + 1);
}

QString completeSuffix(GFileInfo *fileInfo)
{
    if (isDir(fileInfo))
        return "";

    const QString &fullName = fileName(fileInfo);

    int pos2 = fullName.indexOf(".");
    if (pos2 == -1)
        return "";
    else
        return fullName.mid(pos2 + 1);
}

QString filePath(const QUrl &url)
{
    if (!url.isValid())
        return QString();

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(url);
    g_autofree gchar *gpath = g_file_get_path(gfile);   // no blocking I/O

    return QString::fromStdString(gpath);
}

QString parentPath(const QUrl &url)
{
    if (!url.isValid())
        return QString();

    g_autoptr(GFile) gfile = DFileHelper::fileNewForUrl(url);
    g_autoptr(GFile) fileParent = g_file_get_parent(gfile);   // no blocking I/O

    g_autofree gchar *gpath = g_file_get_path(fileParent);   // no blocking I/O
    return QString::fromStdString(gpath);
}
}   // LocalFunc

namespace DFileHelper {

std::map<AttributeID, AttributeDetails> attributeDetailsMap()
{
    static std::map<AttributeID, AttributeDetails> map = {
        { AttributeID::StandardType, { .id = AttributeID::StandardType, .key = "standard::type", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::StandardIsHidden, { .id = AttributeID::StandardIsHidden, .key = "standard::is-hidden", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::StandardIsBackup, { .id = AttributeID::StandardIsBackup, .key = "standard::is-backup", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::StandardIsSymlink, { .id = AttributeID::StandardIsSymlink, .key = "standard::is-symlink", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::StandardIsVirtual, { .id = AttributeID::StandardIsVirtual, .key = "standard::is-virtual", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::StandardIsVolatile, { .id = AttributeID::StandardIsVolatile, .key = "standard::is-volatile", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::StandardName, { .id = AttributeID::StandardName, .key = "standard::name", .defaultValue = "", .type = AttributeType::TypeByteString } },
        { AttributeID::StandardDisplayName, { .id = AttributeID::StandardDisplayName, .key = "standard::display-name", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::StandardEditName, { .id = AttributeID::StandardEditName, .key = "standard::edit-name", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::StandardCopyName, { .id = AttributeID::StandardCopyName, .key = "standard::copy-name", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::StandardIcon, { .id = AttributeID::StandardIcon, .key = "standard::icon", .defaultValue = {}, .type = AttributeType::TypeStringV } },
        { AttributeID::StandardSymbolicIcon, { .id = AttributeID::StandardSymbolicIcon, .key = "standard::symbolic-icon", .defaultValue = {}, .type = AttributeType::TypeStringV } },
        { AttributeID::StandardContentType, { .id = AttributeID::StandardContentType, .key = "standard::content-type", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::StandardFastContentType, { .id = AttributeID::StandardFastContentType, .key = "standard::fast-content-type", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::StandardSize, { .id = AttributeID::StandardSize, .key = "standard::size", .defaultValue = 0, .type = AttributeType::TypeUInt64 } },
        { AttributeID::StandardAllocatedSize, { .id = AttributeID::StandardAllocatedSize, .key = "standard::allocated-size", .defaultValue = 0, .type = AttributeType::TypeUInt64 } },
        { AttributeID::StandardSymlinkTarget, { .id = AttributeID::StandardSymlinkTarget, .key = "standard::symlink-target", .defaultValue = "", .type = AttributeType::TypeByteString } },
        { AttributeID::StandardTargetUri, { .id = AttributeID::StandardTargetUri, .key = "standard::target-uri", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::StandardSortOrder, { .id = AttributeID::StandardSortOrder, .key = "standard::sort-order", .defaultValue = 0, .type = AttributeType::TypeInt32 } },
        { AttributeID::StandardDescription, { .id = AttributeID::StandardDescription, .key = "standard::description", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::EtagValue, { .id = AttributeID::EtagValue, .key = "etag::value", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::IdFile, { .id = AttributeID::IdFile, .key = "id::file", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::IdFilesystem, { .id = AttributeID::IdFilesystem, .key = "id::filesystem", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::AccessCanRead, { .id = AttributeID::AccessCanRead, .key = "access::can-read", .defaultValue = true, .type = AttributeType::TypeBool } },
        { AttributeID::AccessCanWrite, { .id = AttributeID::AccessCanWrite, .key = "access::can-write", .defaultValue = true, .type = AttributeType::TypeBool } },
        { AttributeID::AccessCanExecute, { .id = AttributeID::AccessCanExecute, .key = "access::can-execute", .defaultValue = true, .type = AttributeType::TypeBool } },
        { AttributeID::AccessCanDelete, { .id = AttributeID::AccessCanDelete, .key = "access::can-delete", .defaultValue = true, .type = AttributeType::TypeBool } },
        { AttributeID::AccessCanTrash, { .id = AttributeID::AccessCanTrash, .key = "access::can-trash", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::AccessCanRename, { .id = AttributeID::AccessCanRename, .key = "access::can-rename", .defaultValue = true, .type = AttributeType::TypeBool } },
        { AttributeID::MountableCanMount, { .id = AttributeID::MountableCanMount, .key = "mountable::can-mount", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::MountableCanUnmount, { .id = AttributeID::MountableCanUnmount, .key = "mountable::can-unmount", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::MountableCanEject, { .id = AttributeID::MountableCanEject, .key = "mountable::can-eject", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::MountableUnixDevice, { .id = AttributeID::MountableUnixDevice, .key = "mountable::unix-device", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::MountableUnixDeviceFile, { .id = AttributeID::MountableUnixDeviceFile, .key = "mountable::unix-device-file", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::MountableHalUdi, { .id = AttributeID::MountableHalUdi, .key = "mountable::hal-udi", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::MountableCanPoll, { .id = AttributeID::MountableCanPoll, .key = "mountable::can-poll", .defaultValue = false, .type = AttributeType::TypeString } },
        { AttributeID::MountableIsMediaCheckAutomatic, { .id = AttributeID::MountableIsMediaCheckAutomatic, .key = "mountable::is-media-check-automatic", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::MountableCanStart, { .id = AttributeID::MountableCanStart, .key = "mountable::can-start", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::MountableCanStartDegraded, { .id = AttributeID::MountableCanStartDegraded, .key = "mountable::can-start-degraded", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::MountableCanStop, { .id = AttributeID::MountableCanStop, .key = "mountable::can-stop", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::MountableStartStopType, { .id = AttributeID::MountableStartStopType, .key = "mountable::start-stop-type", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::TimeModified, { .id = AttributeID::TimeModified, .key = "time::modified", .defaultValue = 0, .type = AttributeType::TypeUInt64 } },
        { AttributeID::TimeModifiedUsec, { .id = AttributeID::TimeModifiedUsec, .key = "time::modified-usec", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::TimeAccess, { .id = AttributeID::TimeAccess, .key = "time::access", .defaultValue = 0, .type = AttributeType::TypeUInt64 } },
        { AttributeID::TimeAccessUsec, { .id = AttributeID::TimeAccessUsec, .key = "time::access-usec", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::TimeChanged, { .id = AttributeID::TimeChanged, .key = "time::changed", .defaultValue = 0, .type = AttributeType::TypeUInt64 } },
        { AttributeID::TimeChangedUsec, { .id = AttributeID::TimeChangedUsec, .key = "time::changed-usec", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::TimeCreated, { .id = AttributeID::TimeCreated, .key = "time::created", .defaultValue = 0, .type = AttributeType::TypeUInt64 } },
        { AttributeID::TimeCreatedUsec, { .id = AttributeID::TimeCreatedUsec, .key = "time::created-usec", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::UnixDevice, { .id = AttributeID::UnixDevice, .key = "unix::device", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::UnixInode, { .id = AttributeID::UnixInode, .key = "unix::inode", .defaultValue = 0, .type = AttributeType::TypeUInt64 } },
        { AttributeID::UnixMode, { .id = AttributeID::UnixMode, .key = "unix::mode", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::UnixNlink, { .id = AttributeID::UnixNlink, .key = "unix::nlink", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::UnixUID, { .id = AttributeID::UnixUID, .key = "unix::uid", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::UnixGID, { .id = AttributeID::UnixGID, .key = "unix::gid", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::UnixRdev, { .id = AttributeID::UnixRdev, .key = "unix::rdev", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::UnixBlockSize, { .id = AttributeID::UnixBlockSize, .key = "unix::block-size", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::UnixBlocks, { .id = AttributeID::UnixBlocks, .key = "unix::blocks", .defaultValue = 0, .type = AttributeType::TypeUInt64 } },
        { AttributeID::UnixIsMountPoint, { .id = AttributeID::UnixIsMountPoint, .key = "unix::is-mountpoint", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::DosIsArchive, { .id = AttributeID::DosIsArchive, .key = "dos::is-archive", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::DosIsSystem, { .id = AttributeID::DosIsSystem, .key = "dos::is-system", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::OwnerUser, { .id = AttributeID::OwnerUser, .key = "owner::user", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::OwnerUserReal, { .id = AttributeID::OwnerUserReal, .key = "owner::user-real", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::OwnerGroup, { .id = AttributeID::OwnerGroup, .key = "owner::group", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::ThumbnailPath, { .id = AttributeID::ThumbnailPath, .key = "thumbnail::path", .defaultValue = "", .type = AttributeType::TypeByteString } },
        { AttributeID::ThumbnailFailed, { .id = AttributeID::ThumbnailFailed, .key = "thumbnail::failed", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::ThumbnailIsValid, { .id = AttributeID::ThumbnailIsValid, .key = "thumbnail::is-valid", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::PreviewIcon, { .id = AttributeID::PreviewIcon, .key = "preview::icon", .defaultValue = {}, .type = AttributeType::TypeStringV } },
        { AttributeID::FileSystemSize, { .id = AttributeID::FileSystemSize, .key = "filesystem::size", .defaultValue = 0, .type = AttributeType::TypeUInt64 } },
        { AttributeID::FileSystemFree, { .id = AttributeID::FileSystemFree, .key = "filesystem::free", .defaultValue = 0, .type = AttributeType::TypeUInt64 } },
        { AttributeID::FileSystemUsed, { .id = AttributeID::FileSystemUsed, .key = "filesystem::used", .defaultValue = 0, .type = AttributeType::TypeUInt64 } },
        { AttributeID::FileSystemType, { .id = AttributeID::FileSystemType, .key = "filesystem::type", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::FileSystemReadOnly, { .id = AttributeID::FileSystemReadOnly, .key = "filesystem::readonly", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::FileSystemUsePreview, { .id = AttributeID::FileSystemUsePreview, .key = "filesystem::use-preview", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::FileSystemRemote, { .id = AttributeID::FileSystemRemote, .key = "filesystem::remote", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::GvfsBackend, { .id = AttributeID::GvfsBackend, .key = "gvfs::backend", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::SelinuxContext, { .id = AttributeID::SelinuxContext, .key = "selinux::context", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::TrashItemCount, { .id = AttributeID::TrashItemCount, .key = "trash::item-count", .defaultValue = 0, .type = AttributeType::TypeUInt32 } },
        { AttributeID::TrashDeletionDate, { .id = AttributeID::TrashDeletionDate, .key = "trash::deletion-date", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::TrashOrigPath, { .id = AttributeID::TrashOrigPath, .key = "trash::orig-path", .defaultValue = "", .type = AttributeType::TypeByteString } },
        { AttributeID::RecentModified, { .id = AttributeID::RecentModified, .key = "recent::modified", .defaultValue = 0, .type = AttributeType::TypeInt64 } },
        { AttributeID::StandardIsFile, { .id = AttributeID::StandardIsFile, .key = "standard::is-file", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::StandardIsDir, { .id = AttributeID::StandardIsDir, .key = "standard::is-dir", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::StandardIsRoot, { .id = AttributeID::StandardIsRoot, .key = "standard::is-root", .defaultValue = false, .type = AttributeType::TypeBool } },
        { AttributeID::StandardSuffix, { .id = AttributeID::StandardSuffix, .key = "standard::suffix", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::StandardCompleteSuffix, { .id = AttributeID::StandardCompleteSuffix, .key = "standard::complete-suffix", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::StandardFilePath, { .id = AttributeID::StandardFilePath, .key = "standard::file-path", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::StandardParentPath, { .id = AttributeID::StandardParentPath, .key = "standard::parent-path", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::StandardBaseName, { .id = AttributeID::StandardBaseName, .key = "standard::base-name", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::StandardFileName, { .id = AttributeID::StandardFileName, .key = "standard::file-name", .defaultValue = "", .type = AttributeType::TypeString } },
        { AttributeID::StandardCompleteBaseName, { .id = AttributeID::StandardCompleteBaseName, .key = "standard::complete-base-name", .defaultValue = "", .type = AttributeType::TypeString } }
    };

    return map;
}

std::string attributeKey(AttributeID id)
{
    const auto &map = attributeDetailsMap();
    if (map.count(id) > 0)
        return map.at(id).key;
    return "";
}

QVariant attributeDefaultValue(AttributeID id)
{
    const auto &map = attributeDetailsMap();
    if (map.count(id) > 0)
        return map.at(id).defaultValue;
    return QVariant();
}

AttributeType attributeType(AttributeID id)
{
    const auto &map = attributeDetailsMap();
    if (map.count(id) > 0)
        return map.at(id).type;
    return AttributeType::TypeInvalid;
}

QVariant attribute(const QByteArray &key, const AttributeType type, GFileInfo *fileinfo)
{
    if (!g_file_info_has_attribute(fileinfo, key))
        return QVariant();
    if (type == AttributeType::TypeInvalid)
        return QVariant();

    switch (type) {
    case AttributeType::TypeString: {
        const char *ret = g_file_info_get_attribute_string(fileinfo, key);
        return QVariant(ret);
    }
    case AttributeType::TypeByteString: {
        const char *ret = g_file_info_get_attribute_byte_string(fileinfo, key);
        return QVariant(ret);
    }
    case AttributeType::TypeBool: {
        bool ret = g_file_info_get_attribute_boolean(fileinfo, key);
        return QVariant(ret);
    }
    case AttributeType::TypeUInt32: {
        uint32_t ret = g_file_info_get_attribute_uint32(fileinfo, key);
        return QVariant(ret);
    }
    case AttributeType::TypeInt32: {
        int32_t ret = g_file_info_get_attribute_int32(fileinfo, key);
        return QVariant(ret);
    }
    case AttributeType::TypeUInt64: {
        uint64_t ret = g_file_info_get_attribute_uint64(fileinfo, key);
        return QVariant(qulonglong(ret));
    }
    case AttributeType::TypeInt64: {
        int64_t ret = g_file_info_get_attribute_int64(fileinfo, key);
        return QVariant(qulonglong(ret));
    }
    case AttributeType::TypeStringV: {
        char **ret = g_file_info_get_attribute_stringv(fileinfo, key);
        QStringList retValue;
        for (int i = 0; ret && ret[i]; ++i) {
            retValue.append(QString::fromStdString(ret[i]));
        }
        return retValue;
    }
    default:
        return QVariant();
    }
}

QVariant attributeFromInfo(AttributeID id, GFileInfo *fileinfo, const QUrl &url)
{
    if (id > AttributeID::CustomStart)
        return customAttributeFromInfo(id, fileinfo, url);

    const std::string &key = attributeKey(id);
    const AttributeType &type = attributeType(id);

    return attribute(key.c_str(), type, fileinfo);
}

bool setAttribute(GFile *gfile, const char *key, AttributeType type, const QVariant &value, GFileQueryInfoFlags flags, GCancellable *cancellable, GError **error)
{
    switch (type) {
    case AttributeType::TypeString: {
        bool succ = g_file_set_attribute_string(gfile, key, value.toString().toStdString().c_str(), flags, cancellable, error);
        return succ;
    }
    case AttributeType::TypeByteString: {
        bool succ = g_file_set_attribute_byte_string(gfile, key, value.toString().toStdString().c_str(), flags, cancellable, error);
        return succ;
    }
    case AttributeType::TypeBool: {
        bool bvalue = value.toBool();
        gpointer gvalue = &bvalue;
        bool succ = g_file_set_attribute(gfile, key, G_FILE_ATTRIBUTE_TYPE_BOOLEAN, gvalue, flags, cancellable, error);
        return succ;
    }
    case AttributeType::TypeUInt32: {
        bool succ = g_file_set_attribute_uint32(gfile, key, value.toUInt(), flags, cancellable, error);
        return succ;
    }
    case AttributeType::TypeInt32: {
        bool succ = g_file_set_attribute_int32(gfile, key, value.toInt(), flags, cancellable, error);
        return succ;
    }
    case AttributeType::TypeUInt64: {
        bool succ = g_file_set_attribute_uint64(gfile, key, value.toULongLong(), flags, cancellable, error);
        return succ;
    }
    case AttributeType::TypeInt64: {
        bool succ = g_file_set_attribute_int64(gfile, key, value.toLongLong(), flags, cancellable, error);
        return succ;
    }
    case AttributeType::TypeStringV: {
        const QStringList &list = value.toStringList();
        gchar **values;
        values = g_new(gchar *, list.size() + 1);
        for (int i = 0, nEnd = list.size(); i < nEnd; ++i) {
            values[i] = g_strdup(list[i].toStdString().c_str());
        }
        values[list.size()] = nullptr;
        bool succ = g_file_set_attribute(gfile, key, G_FILE_ATTRIBUTE_TYPE_STRINGV, values, flags, cancellable, error);
        g_strfreev(values);
        return succ;
    }
    default:
        return false;
    }
}

QVariant customAttributeFromInfo(AttributeID id, GFileInfo *fileinfo, const QUrl &url)
{
    if (id < AttributeID::CustomStart)
        return attributeFromInfo(id, fileinfo, url);

    switch (id) {
    case AttributeID::StandardIsFile:
        return LocalFunc::isFile(fileinfo);
    case AttributeID::StandardIsDir:
        return LocalFunc::isDir(fileinfo);
    case AttributeID::StandardIsRoot:
        return LocalFunc::isRoot(url);
    case AttributeID::StandardSuffix:
        return LocalFunc::suffix(fileinfo);
    case AttributeID::StandardCompleteSuffix:
        return LocalFunc::completeSuffix(fileinfo);
    case AttributeID::StandardFilePath:
        return LocalFunc::filePath(url);
    case AttributeID::StandardParentPath:
        return LocalFunc::parentPath(url);
    case AttributeID::StandardBaseName:
        return LocalFunc::baseName(fileinfo);
    case AttributeID::StandardFileName:
        return LocalFunc::fileName(fileinfo);
    case AttributeID::StandardCompleteBaseName:
        return LocalFunc::completeBaseName(fileinfo);
    default:
        return QVariant();
    }
}

GFile *fileNewForUrl(const QUrl &url)
{
    return g_file_new_for_uri(url.toString().toStdString().c_str());
}

QSet<QString> hideListFromUrl(const QUrl &url)
{
    g_autofree char *contents = nullptr;
    g_autoptr(GError) error = nullptr;
    gsize len = 0;
    g_autoptr(GFile) hiddenFile = g_file_new_for_uri(url.toString().toLocal8Bit().data());
    const bool exists = g_file_query_exists(hiddenFile, nullptr);
    if (!exists)
        return {};

    const bool succ = g_file_load_contents(hiddenFile, nullptr, &contents, &len, nullptr, &error);
    if (succ) {
        if (contents && len > 0) {
            QString dataStr(contents);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            const QStringList strs { dataStr.split('\n', Qt::SkipEmptyParts) };
            return QSet<QString> { strs.begin(), strs.end() };
#else
            const QStringList strs { dataStr.split('\n', QString::SkipEmptyParts) };
            return QSet<QString>::fromList(strs);
#endif
        }
    } else {
        qWarning() << "load .hidden fail, url: " << url << " error: " << error->code << " " << QString::fromLocal8Bit(error->message);
    }
    return {};
}

bool fileIsHidden(const DFileInfo *dfileinfo, const QSet<QString> &hideList)
{
    if (!dfileinfo)
        return false;

    const QString &fileName = dfileinfo->attribute(AttributeID::StandardName, nullptr)->toString();
    if (fileName.startsWith(".")) {
        return true;
    } else {
        if (hideList.isEmpty()) {
            const QString &hiddenPath = dfileinfo->attribute(AttributeID::StandardParentPath, nullptr)->toString() + "/.hidden";
            const QSet<QString> &hideList = DFileHelper::hideListFromUrl(QUrl::fromLocalFile(hiddenPath));

            if (hideList.contains(fileName))
                return true;
        } else {
            return hideList.contains(fileName);
        }
    }

    return false;
}

}

TimeoutHelper::TimeoutHelper(int timeout)
{
    blocker = new QEventLoop();

    timer.reset(new QTimer());
    timer->setInterval(timeout);
    timer->setSingleShot(true);
    QObject::connect(timer.data(), &QTimer::timeout, blocker, [this] {
        blocker->exit(Timeout);
    });
}

TimeoutHelper::~TimeoutHelper()
{
    if (blocker) {
        blocker->exit();
        delete blocker;
        blocker = nullptr;
    }
    timer->stop();
}

// namespace DFileHelper

DIO_END_NAMESPACE
