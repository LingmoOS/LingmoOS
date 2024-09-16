// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dgiofileinfo.h"
#include "dgiohelper.h"

#include <giomm/fileinfo.h>

using namespace Gio;

class DGioFileInfoPrivate
{
public:
    DGioFileInfoPrivate(DGioFileInfo *qq, FileInfo *gmmFileInfoPtr);

    Glib::RefPtr<FileInfo> getGmmFileInfoInstance() const;

    Glib::RefPtr<const Icon> icon() const;
    bool getAttributeBoolean(const std::string &attribute) const;
    quint64 getAttributeUint64(const std::string &attribute) const;
    QString getAttributeString(const std::string &attribute) const;

private:
    Glib::RefPtr<FileInfo> m_gmmFileInfoPtr;

    QString uri() const;

    DGioFileInfo *q_ptr;

    Q_DECLARE_PUBLIC(DGioFileInfo)
};

DGioFileInfoPrivate::DGioFileInfoPrivate(DGioFileInfo *qq, FileInfo *gmmFileInfoPtr)
    : m_gmmFileInfoPtr(gmmFileInfoPtr)
    , q_ptr(qq)
{

}

Glib::RefPtr<FileInfo> DGioFileInfoPrivate::getGmmFileInfoInstance() const
{
    return m_gmmFileInfoPtr;
}

Glib::RefPtr<const Icon> DGioFileInfoPrivate::icon() const
{
    return getGmmFileInfoInstance()->get_icon();
}

bool DGioFileInfoPrivate::getAttributeBoolean(const std::string &attribute) const
{
    return m_gmmFileInfoPtr->get_attribute_boolean(attribute);
}

quint64 DGioFileInfoPrivate::getAttributeUint64(const std::string &attribute) const
{
    return m_gmmFileInfoPtr->get_attribute_uint64(attribute);
}

QString DGioFileInfoPrivate::getAttributeString(const std::string &attribute) const
{
    return QString::fromStdString(m_gmmFileInfoPtr->get_attribute_string(attribute));
}

// -------------------------------------------------------------

DGioFileInfo::DGioFileInfo(FileInfo *gmmFileInfoInfoPtr, QObject *parent)
    : QObject(parent)
    , d_ptr(new DGioFileInfoPrivate(this, gmmFileInfoInfoPtr))
{

}

DGioFileInfo::~DGioFileInfo()
{

}

/*!
 * \brief Gets a display name for a file.
 *
 * A display name is guaranteed to be in UTF8 and can thus be displayed in the UI.
 *
 * Wrapper of Gio::FileInfo::get_display_name(), internally it returns the
 * G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME attribute value.
 */
QString DGioFileInfo::displayName() const
{
    Q_D(const DGioFileInfo);

    return QString::fromStdString(d->getGmmFileInfoInstance()->get_display_name());
}

/*!
 * \brief DGioFileInfo::fileType
 *
 * Wrapper of Gio::FileInfo::get_file_type(), internally it returns the
 * G_FILE_ATTRIBUTE_STANDARD_TYPE attribute value.
 * \return
 */
DGioFileType DGioFileInfo::fileType() const
{
    Q_D(const DGioFileInfo);

    return static_cast<DGioFileType>(d->getGmmFileInfoInstance()->get_file_type());
}

/*!
 * \brief DGioFileInfo::fileSize
 *
 * Wrapper of Gio::FileInfo::get_size()
 *
 * \return the file size in bytes.
 */
quint64 DGioFileInfo::fileSize() const
{
    Q_D(const DGioFileInfo);

    return static_cast<quint64>(d->getGmmFileInfoInstance()->get_size());
}

/*!
 * \brief get the file content type (MimeType).
 *
 * Wrapper of Gio::FileInfo::get_content_type()
 *
 * \return a single MimeType of the file, such as "text/plain".
 */
QString DGioFileInfo::contentType() const
{
    Q_D(const DGioFileInfo);

    return QString::fromStdString(d->getGmmFileInfoInstance()->get_content_type());
}

/*!
 * \brief DGioFileInfo::iconString
 *
 * Wrapper of Gio::FileInfo::get_icon() and then Gio::Icon::to_string().
 *
 * Notice the returned value can be in two different forms:
 *
 *  - a native path (such as "/path/to/my icon.png") without escaping if the file for icon is a native
 *    file. If the file is not native, the returned string is the result of g_file_get_uri()
 *    (such as "sftp://path/to/my%20icon.png").
 *
 *  - a icon name if icon is a ThemedIcon with exactly one name, the encoding is simply the name (such as
 *    network-server), or a list of theme names (such as ". GThemedIcon text-plain text-x-generic text-plain-symbolic").
 *
 * \return the icon string as described above.
 */
QString DGioFileInfo::iconString() const
{
    Q_D(const DGioFileInfo);

    return QString::fromStdString(d->icon()->to_string());
}

QStringList DGioFileInfo::themedIconNames() const
{
    Q_D(const DGioFileInfo);

    return DGioPrivate::getThemedIconNames(d->icon());
}

bool DGioFileInfo::fsReadOnly() const
{
    Q_D(const DGioFileInfo);

    return d->getAttributeBoolean(G_FILE_ATTRIBUTE_FILESYSTEM_READONLY);
}

quint64 DGioFileInfo::fsTotalBytes() const
{
    Q_D(const DGioFileInfo);

    return d->getAttributeUint64(G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
}

quint64 DGioFileInfo::fsUsedBytes() const
{
    Q_D(const DGioFileInfo);

    return d->getAttributeUint64(G_FILE_ATTRIBUTE_FILESYSTEM_USED);
}

quint64 DGioFileInfo::fsFreeBytes() const
{
    Q_D(const DGioFileInfo);

    return d->getAttributeUint64(G_FILE_ATTRIBUTE_FILESYSTEM_FREE);
}

QString DGioFileInfo::fsType() const
{
    Q_D(const DGioFileInfo);

    return d->getAttributeString(G_FILE_ATTRIBUTE_FILESYSTEM_TYPE);
}
