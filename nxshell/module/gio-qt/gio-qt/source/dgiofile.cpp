// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dgiofile.h"
#include "dgiofileinfo.h"
#include "dgiofileiterator.h"
#include "dgiomount.h"

#include <glibmm/refptr.h>

#include <giomm/init.h>
#include <giomm/file.h>
#include <giomm/fileenumerator.h>

#include <QDebug>
#include <QWaitCondition>
#include <QMutex>
#include <QElapsedTimer>
#include <QtConcurrent/QtConcurrentRun>
#include <dgiomountoperation.h>

using namespace Gio;

class DGioFilePrivate
{
public:
    DGioFilePrivate(DGioFile *qq, File *gmmFilePtr);

    Glib::RefPtr<File> getGmmFileInstance() const;

private:
    Glib::RefPtr<File> m_gmmFilePtr;

    QString uri() const;

    DGioFile *q_ptr;

    void slot_enumerateChildrenAsyncResult(const Glib::RefPtr<Gio::AsyncResult>& result);

    void slot_mountEnclosingVolumeResult(const Glib::RefPtr<Gio::AsyncResult>& result);

    Q_DECLARE_PUBLIC(DGioFile)
};

DGioFilePrivate::DGioFilePrivate(DGioFile *qq, File *gmmFilePtr)
    : m_gmmFilePtr(gmmFilePtr)
    , q_ptr(qq)
{

}

Glib::RefPtr<File> DGioFilePrivate::getGmmFileInstance() const
{
    return m_gmmFilePtr;
}

QString DGioFilePrivate::uri() const
{
    return QString::fromStdString(m_gmmFilePtr->get_uri());
}

void DGioFilePrivate::slot_enumerateChildrenAsyncResult(const Glib::RefPtr<AsyncResult> &result)
{
    Q_Q(DGioFile);

    try {
        Glib::RefPtr<Gio::FileEnumerator> iter = m_gmmFilePtr->enumerate_children_finish(result);
        QExplicitlySharedDataPointer<DGioFileIterator> fileIterPtr(new DGioFileIterator(iter.release()));

        Q_EMIT q->createFileIteratorReady(fileIterPtr);
    } catch (const Glib::Error & error) {
        qDebug() << QString::fromStdString(error.what().raw());
    }
}

void DGioFilePrivate::slot_mountEnclosingVolumeResult(const Glib::RefPtr<AsyncResult> &result)
{
    Q_Q(DGioFile);

    try {
        bool res = m_gmmFilePtr->mount_enclosing_volume_finish(result);

        Q_EMIT q->mountEnclosingVolumeReady(res, "");
    } catch (const Glib::Error & error) {
        qDebug() << QString::fromStdString(error.what().raw());
        Q_EMIT q->mountEnclosingVolumeReady(false, QString::fromStdString(error.what().raw()));
    }
}

// -------------------------------------------------------------

DGioFile::DGioFile(File* gmmFilePtr, QObject *parent)
    : QObject(parent)
    , d_ptr(new DGioFilePrivate(this, gmmFilePtr))
{
    // gmountPtr must be valid;
    Q_CHECK_PTR(gmmFilePtr);
}

DGioFile::~DGioFile()
{

}

/*!
 * \brief Create a DGioFile instance by given \a path
 *
 * This operation never fails since Gio::File::create_for_path never fails, but the returned
 * object might not support any I/O operation if path is malformed.
 *
 * Caller take the ownership of the created object, you can also provide a \a parent object.
 *
 * \return the created DGioFile instance
 */
DGioFile *DGioFile::createFromPath(QString path, QObject *parent)
{
    // ensure GIO got initialized
    Gio::init();

    // File::create_for_path never falls.
    Glib::RefPtr<File> gmmFile = File::create_for_path(path.toStdString());

    return new DGioFile(gmmFile.release(), parent);
}

/*!
 * \brief Create a DGioFile instance by given \a uri
 *
 * This operation never fails since Gio::File::create_for_uri never fails, but the returned
 * object might not support any I/O operation if uri is malformed.
 *
 * Caller take the ownership of the created object, you can also provide a \a parent object.
 *
 * \return the created DGioFile instance
 */
DGioFile *DGioFile::createFromUri(QString uri, QObject *parent)
{
    // ensure GIO got initialized
    Gio::init();

    // File::create_for_uri never falls.
    Glib::RefPtr<File> gmmFile = File::create_for_uri(uri.toStdString());

    return new DGioFile(gmmFile.release(), parent);
}

/*!
 * \brief Create a DGioFile instance for a given argument from the command line.
 *
 * The value of \a arg can be either a URI, an absolute path or a relative path resolved relative
 * to the current working directory. This operation never fails, but the returned object might
 * not support any I/O operation if \a arg points to a malformed path.
 *
 * \param arg A string containing either a URI, a relative or absolute path.
 * \return the created DGioFile instance
 */
DGioFile *DGioFile::createFromCmdArg(QString arg, QObject *parent)
{
    // ensure GIO got initialized
    Gio::init();

    // File::create_for_uri never falls.
    Glib::RefPtr<File> gmmFile = File::create_for_commandline_arg(arg.toStdString());

    return new DGioFile(gmmFile.release(), parent);
}

/*!
 * \brief Gets the base name (the last component of the path) of the DGioFile
 *
 * Wrapper of Gio::File::get_basename(), normally return filename with suffix (without path).
 *
 * If called for the top level of a system (such as the filesystem root or a uri like sftp://host/)
 * it will return a single directory separator (and on Windows, possibly a drive letter).
 *
 * If you want to use filenames in a user interface you should use DGioFileInfo::displayName() instead.
 */
QString DGioFile::basename() const
{
    Q_D(const DGioFile);

    return QString::fromStdString(d->getGmmFileInstance()->get_basename());
}

/*!
 * \brief Gets the local pathname of the DGioFile, if one exists.
 *
 * Wrapper of Gio::File::get_path(). For local file it gets the local pathname with filename included,
 * for filesystem it gets the mount point path.
 *
 * If valid, this is guaranteed to be an absolute, canonical path. It might contain symlinks.
 */
QString DGioFile::path() const
{
    Q_D(const DGioFile);

    return QString::fromStdString(d->getGmmFileInstance()->get_path());
}

QString DGioFile::uri() const
{
    Q_D(const DGioFile);

    return d->uri();
}

/*!
 * \brief Gets the requested information about the file.
 *
 * Wrapper of Gio::File::query_info(const std::string& attributes = "*", FileQueryInfoFlags flags = FILE_QUERY_INFO_NONE).
 *
 * \return the created file info object, or nullptr if create failed.
 */
QExplicitlySharedDataPointer<DGioFileInfo> DGioFile::createFileInfo(QString attr, DGioFileQueryInfoFlags queryInfoFlags, unsigned long timeout_msec)
{
    Q_D(DGioFile);

    try {
        unsigned int flagValue = queryInfoFlags;
        FileQueryInfoFlags flags = static_cast<FileQueryInfoFlags>(flagValue);
        Glib::RefPtr<FileInfo> gmmFileInfo;
        QSharedPointer<QWaitCondition> cond(new QWaitCondition);
        QSharedPointer<QMutex> m(new QMutex);
        QtConcurrent::run([&, cond, m, timeout_msec] {
            Glib::RefPtr<FileInfo> localret;
            QElapsedTimer t;
            t.start();
            try {
                localret = d->getGmmFileInstance()->query_info(attr.toStdString(), flags);
            } catch (const Glib::Error &error) {
                qDebug() << QString::fromStdString(error.what().raw());
                return;
            }

            if (t.elapsed() < timeout_msec) {
                gmmFileInfo = localret;
                cond->wakeAll();
            }
        });
        m->lock();
        bool finished = gmmFileInfo || cond->wait(m.data(), timeout_msec);
        m->unlock();
        if (finished && gmmFileInfo) {
            QExplicitlySharedDataPointer<DGioFileInfo> fileInfoPtr(new DGioFileInfo(gmmFileInfo.release()));
            return fileInfoPtr;
        }
    } catch (const Glib::Error &error) {
        qDebug() << QString::fromStdString(error.what().raw());
    }

    return QExplicitlySharedDataPointer<DGioFileInfo>(nullptr);
}

/*!
 * \brief Obtains information about the filesystem the file is on.
 *
 * Wrapper of Gio::File::query_filesystem_info("filesystem::*").
 *
 * \return the created file system info object, or nullptr if create failed.
 */
QExplicitlySharedDataPointer<DGioFileInfo> DGioFile::createFileSystemInfo(QString attr)
{
    Q_D(DGioFile);

    try {
        Glib::RefPtr<FileInfo> gmmFileInfo = d->getGmmFileInstance()->query_filesystem_info(attr.toStdString());
        if (gmmFileInfo) {
            QExplicitlySharedDataPointer<DGioFileInfo> fileInfoPtr(new DGioFileInfo(gmmFileInfo.release()));
            return fileInfoPtr;
        }
    } catch (const Glib::Error &error) {
        qDebug() << QString::fromStdString(error.what().raw());
    }

    return QExplicitlySharedDataPointer<DGioFileInfo>(nullptr);
}

/*!
 * \brief Gets the requested information about the files in a directory.
 *
 * Wrapper of Gio::File::enumerate_children()
 *
 * The attribute value is a string that specifies the file attributes that should be gathered.
 * It is not an error if its not possible to read a particular requested attribute from a file,
 * it just won't be set. attribute should be a comma-separated list of attribute or attribute
 * wildcards. The wildcard "*" means all attributes, and a wildcard like "standard::*" means all
 * attributes in the standard namespace. An example attribute query be "standard::*,owner::user".
 * The standard attributes are available as defines, like FILE_ATTRIBUTE_STANDARD_NAME.
 *
 * \param attr An attribute query string.
 * \param queryInfoFlags
 * \return the created DGioFileIterator object, or nullptr if failed.
 */
QExplicitlySharedDataPointer<DGioFileIterator> DGioFile::createFileIterator(QString attr, DGioFileQueryInfoFlags queryInfoFlags)
{
    Q_D(DGioFile);

    unsigned int flagValue = queryInfoFlags;
    FileQueryInfoFlags flags = static_cast<FileQueryInfoFlags>(flagValue);

    try {
        Glib::RefPtr<FileEnumerator> gmmFileEnumerator = d->getGmmFileInstance()->enumerate_children(attr.toStdString(), flags);
        QExplicitlySharedDataPointer<DGioFileIterator> fileIterPtr(new DGioFileIterator(gmmFileEnumerator.release()));

        return fileIterPtr;
    } catch (const Glib::Error &error) {
        qDebug() << QString::fromStdString(error.what().raw());
    }

    return QExplicitlySharedDataPointer<DGioFileIterator>(nullptr);
}

void DGioFile::createFileIteratorAsync(QString attr, DGioFileQueryInfoFlags queryInfoFlags)
{
    Q_D(DGioFile);

    unsigned int flagValue = queryInfoFlags;
    FileQueryInfoFlags flags = static_cast<FileQueryInfoFlags>(flagValue);
    d->getGmmFileInstance()->enumerate_children_async(sigc::mem_fun(d, &DGioFilePrivate::slot_enumerateChildrenAsyncResult),
                                                      attr.toStdString(), flags);
}

void DGioFile::mountEnclosingVolume(DGioMountOperation *dgioMountOperation)
{
    Q_D(DGioFile);

    d->getGmmFileInstance()->mount_enclosing_volume(dgioMountOperation->getGIOMountOperationObj(),
                                                    sigc::mem_fun(d, &DGioFilePrivate::slot_mountEnclosingVolumeResult));
}

QExplicitlySharedDataPointer<DGioMount> DGioFile::findEnclosingMount()
{
    Q_D(DGioFile);
    QExplicitlySharedDataPointer<DGioMount> ret;
    try {
        ret = new DGioMount(d->getGmmFileInstance()->find_enclosing_mount().release());
    } catch (const Glib::Error &error) {
        qDebug() << QString::fromStdString(error.what().raw());
    }
    return ret;
}
