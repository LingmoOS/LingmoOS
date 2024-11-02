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
 * Authors: Ding Jing <dingjing@kylinos.cn>
 *
 */
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/mman.h>
#include "file-copy.h"
#include "file-utils.h"
#include <stdio.h>
#include <fcntl.h>
#include <cstring>
#include <QString>
#include <mutex>
#include <mntent.h>
#include <QProcess>
#include "file-info.h"
#include "file-info-job.h"
#include "file-operation-manager.h"

#define BUF_SIZE        1024000
#define SYNC_INTERVAL   10
#define BIG_FILE_SIZE   300 * 1024 * 1024
const size_t BUFFER_SIZE = 4096;
std::mutex fileMutex;

using namespace Peony;

static gchar* get_fs_type (char* path);

FileCopy::FileCopy (QString srcUri, QString destUri, GFileCopyFlags flags, GCancellable* cancel, GFileProgressCallback cb, gpointer pcd, GError** error, QObject* obj) : QObject (obj)
{
    mSrcUri = FileUtils::urlEncode(srcUri);
    mDestUri = FileUtils::urlEncode(destUri);
    QString destUrit = nullptr;

    mCopyFlags = flags;

    mCancel = cancel;
    mProgress = cb;
    mProgressData = pcd;
    mError = error;
}

FileCopy::~FileCopy()
{

}

void FileCopy::pause ()
{
    mStatus = PAUSE;
    mPause.tryLock();
}

void FileCopy::resume ()
{
    mStatus = RESUME;
    mPause.unlock();
}

void FileCopy::cancel()
{
    if (mCancel) {
        g_cancellable_cancel (mCancel);
    }
    mPause.unlock();
}

FileCopy::Status FileCopy::getStatus()
{
    return mStatus;
}

void FileCopy::detailError (GError** error)
{
    g_clear_error (mError);

    if (nullptr == error || nullptr == *error) {
        return;
    }

    g_set_error(mError, (*error)->domain, (*error)->code, "%s", (*error)->message);
    g_clear_error(error);
}

void FileCopy::sync(const GFile* destFile)
{
    g_autofree char*    uri = g_file_get_uri(const_cast<GFile*>(destFile));
    g_autofree char*    path = g_file_get_path(const_cast<GFile*>(destFile));

    // it's not possible
    g_return_if_fail(uri && path);

    // uri is start with "file://"
    QString gvfsPath = QString("/run/user/%1/gvfs/").arg(getuid());
    if (0 != g_ascii_strncasecmp(uri, "file:///", 8) || g_strstr_len(uri, strlen(uri), gvfsPath.toUtf8().constData())) {
        return;
    }

    if(mTotalSize < BUF_SIZE * SYNC_INTERVAL || mIsDestFileLocal) {
        return;
    }

    // execute sync
    int fromfd = open(path, O_SYNC);
    if (-1 != fromfd) {
        FileOperationManager::getInstance()->setFsyncStatus(true);
        fsync(fromfd);
        FileOperationManager::getInstance()->setFsyncStatus(false);
        close(fromfd);
    } else {
        auto lastError = strerror(errno);
        qWarning() << "Failed to open the source file, path:" << path
                   << "error code:" << errno
                   << "error msg:" << lastError;
    }
}


void FileCopy::updateProgress () const
{
    if (nullptr == mProgress) {
        return;
    }

    mProgress(mOffset, mTotalSize, mProgressData);
}

void FileCopy::run ()
{
    int                     syncTim = 0;
    GError*                 error = nullptr;
    gsize                  readSize = 0;
    gsize                  writeSize = 0;
    GFileInputStream*       readIO = nullptr;
    GFileOutputStream*      writeIO = nullptr;
    GFile*                  srcFile = nullptr;
    GFile*                  destFile = nullptr;
    GFileInfo*              srcFileInfo = nullptr;
    GFileInfo*              destFileInfo = nullptr;
    GFileType               srcFileType = G_FILE_TYPE_UNKNOWN;
    GFileType               destFileType = G_FILE_TYPE_UNKNOWN;
    qint64                  blockSize = 0;
    g_autofree gchar*       buf = nullptr;

    g_autoptr (GFile)       destDir = NULL;

    srcFile = g_file_new_for_uri(FileUtils::urlEncode(mSrcUri).toUtf8());
    destFile = g_file_new_for_uri(FileUtils::urlEncode(mDestUri).toUtf8());

    bool notSupportInputStream = false;
    bool notSupportOutputStream = false;

    // it's impossible
    if (nullptr == srcFile || nullptr == destFile) {
        error = g_error_new (1, G_IO_ERROR_INVALID_ARGUMENT,"%s", tr("Error in source or destination file path!").toUtf8().constData());
        detailError(&error);
        goto out;
    }

    // impossible
    srcFileType = g_file_query_file_type(srcFile, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr);
    if (G_FILE_TYPE_DIRECTORY == srcFileType) {
        error = g_error_new (1, G_IO_ERROR_INVALID_ARGUMENT,"%s", tr("Error in source or destination file path!").toUtf8().constData());
        detailError(&error);
        goto out;
    }

    //fix copy special file stuck issue, will skip these type files: socket, fifo, blockdev, chardev
    //fix bug#162130, 参照nautilus提示修改
    if (G_FILE_TYPE_SPECIAL == srcFileType) {
        //qWarning() << "skip G_FILE_TYPE_SPECIAL type file copy srcFile: " << mSrcUri;
        error = g_error_new(1, G_IO_ERROR_NOT_REGULAR_FILE, "%s", tr("Error when copy file: %1, can not copy special files, skip this file and continue?").arg(mSrcUri.replace("file://", "")).toUtf8().constData());
        detailError(&error);
        goto out;
    }

    destFileType = g_file_query_file_type(destFile, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr);
    if (G_FILE_TYPE_DIRECTORY == destFileType) {
        mDestUri = mDestUri + "/" + mSrcUri.split("/").last();
        g_object_unref(destFile);
        destFile = g_file_new_for_uri(FileUtils::urlEncode(mDestUri).toUtf8());
        if (nullptr == destFile) {
            error = g_error_new (1, G_IO_ERROR_INVALID_ARGUMENT,"%s", tr("Error in source or destination file path!").toUtf8().constData());
            detailError(&error);
            goto out;
        }
    }

    // check file status
    if (FileUtils::isFileExsit(mDestUri)) {
        bool src_file_exists = g_file_query_exists(srcFile, mCancel);
        if (!src_file_exists) {
            qWarning()<<__FUNCTION__<<"query src file doesn't exist"<<mSrcUri;
            g_clear_error(&error);
            error = g_error_new_literal(g_io_error_quark(), G_IO_ERROR_NOT_FOUND, tr("Can not copy %1, file doesn't exist. Has the file been renamed or moved?").arg(mSrcUri).toUtf8().constData());
            detailError(&error);
            return;
        }
        if (mCopyFlags & G_FILE_COPY_OVERWRITE) {
            g_file_delete(destFile,  nullptr, &error);
            if (nullptr != error) {
                detailError(&error);
                goto out;
            }
        } else if (mCopyFlags & G_FILE_COPY_BACKUP) {
            do {
                QStringList newUrl = mDestUri.split("/");
                newUrl.pop_back();
                newUrl.append(FileUtils::handleDuplicateName(FileUtils::urlDecode(mDestUri)));
                mDestUri = newUrl.join("/");
            } while (FileUtils::isFileExsit(mDestUri));
            if (nullptr != destFile) {
                g_object_unref(destFile);
            }
            destFile = g_file_new_for_uri(FileUtils::urlEncode(mDestUri).toUtf8());
        } else {
            error = g_error_new (1, G_IO_ERROR_EXISTS, "%s", QString(tr("The dest file \"%1\" has existed!")).arg(mDestUri).toUtf8().constData());
            detailError(&error);
            goto out;
        }
    }

    srcFileInfo = g_file_query_info(srcFile, "standard::*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, mCancel ? mCancel : nullptr, &error);
    if (nullptr != error) {
        mTotalSize = 0;
        qInfo() << "srcFile: " << mSrcUri << " querry info error: " << error->message << "  code:" << error->code;
        detailError(&error);
    } else {
        mTotalSize = g_file_info_get_size(srcFileInfo);
    }

    // check dest filesystem
    destDir = g_file_get_parent (destFile);
    mIsDestFileLocal = isFileOnLocal(destDir);
    if (destDir && mTotalSize >= 4294967296) {
        g_autoptr (GMount) destMount = g_file_find_enclosing_mount (destDir, NULL, NULL);
        if (destMount) {
            g_autoptr (GFile) destMountFile = g_mount_get_default_location (destMount);
            if (destMountFile) {
                g_autofree gchar* mountPath = g_file_get_path (destMountFile);
                if (mountPath) {
                    g_autofree gchar* fstype = get_fs_type (mountPath);
                    if (!g_strcmp0 (fstype, "vfat") || !g_strcmp0 (fstype, "fat32")) {
                        error = g_error_new (1, G_IO_ERROR_NOT_SUPPORTED, "%s", QString(tr("Vfat/FAT32 file systems do not support a single file that occupies more than 4 GB space!")).toUtf8 ().constData ());
                        detailError(&error);
                        goto out;
                    }
                }
            }
        }
    }

    if (mTotalSize > BIG_FILE_SIZE) {
        if (!mSrcUri.startsWith("ftp://") && !mSrcUri.startsWith("sftp://")) {
            auto SrcPath = g_file_get_path(srcFile);
            auto destPath = g_file_get_path(destFile);

            if(-1 != doCopyBigFile(SrcPath, destPath)){
                if (CANCEL == mStatus) {
                    error = g_error_new(1, G_IO_ERROR_CANCELLED, "%s", tr("operation cancel").toUtf8().constData());
                    detailError(&error);
                    g_file_delete(destFile, nullptr, nullptr);
                } else {
                    mStatus = FINISHED;
                }
                goto out;
            } else {
                if (ERROR == mStatus) {
                    error = g_error_new(1, G_IO_ERROR_FAILED, "%s", tr("Error writing to file: Input/output error").toUtf8().constData());
                    detailError(&error);
                    goto out;
                }
            }
        }
    }
    blockSize = mTotalSize > BUF_SIZE ? BUF_SIZE : (mTotalSize + 1);
    buf = (char*)g_malloc0(sizeof (char) * blockSize);
    // read io stream
    readIO = g_file_read(srcFile, mCancel ? mCancel : nullptr, &error);
    if (nullptr != error) {
        if (g_error_matches(error, g_io_error_quark(), G_IO_ERROR_NOT_SUPPORTED)) {
            notSupportInputStream = true;
            g_error_free(error);
            error = nullptr;
        } else {
            detailError(&error);
            goto out;
        }
    }

    // write io stream
    writeIO = g_file_create(destFile, G_FILE_CREATE_REPLACE_DESTINATION, mCancel ? mCancel : nullptr, &error);
    if (nullptr != error) {
        // it's impossible for 'buf' is null
        if (!buf || G_IO_ERROR_NOT_SUPPORTED == error->code) {
            qWarning() << "g_file_copy " << error->code << " -- " << error->message;
            g_error_free(error);
            error = nullptr;
            notSupportOutputStream = true;
        } else {
            detailError(&error);
            qInfo() << "create dest file error!" << mDestUri << " == " << g_file_get_uri(destFile);
            goto out;
        }
    }

    if (notSupportInputStream || notSupportOutputStream) {
        qInfo()<<"stream io not supported, use g_file_copy instead";
        g_file_copy(srcFile, destFile, (writeIO? GFileCopyFlags(mCopyFlags|G_FILE_COPY_OVERWRITE): mCopyFlags), mCancel, mProgress, mProgressData, &error);
        sync(destFile);
        if (error) {
            qWarning() << "g_file_copy error:" << error->code << " -- " << error->message;
            mStatus = ERROR;
            detailError(&error);
        } else {
            mStatus = FINISHED;
        }
        goto out;
    }

    while (true) {
        if (RUNNING == mStatus) {
            if (nullptr != mCancel && g_cancellable_is_cancelled(mCancel)) {
                mStatus = CANCEL;
                continue;
            }

            memset(buf, 0, blockSize);
            if (++syncTim > SYNC_INTERVAL) {
                syncTim = 0;
                sync(destFile);
            }

            mPause.lock();
            // read data
            // readSize = g_input_stream_read(G_INPUT_STREAM(readIO), buf, BUF_SIZE - 1, mCancel ? mCancel : nullptr, &error);
            g_input_stream_read_all(G_INPUT_STREAM(readIO), buf, blockSize - 1, &readSize, mCancel ? mCancel : nullptr, &error);
            if (0 == readSize && nullptr == error) {
                mStatus = FINISHED;
                mPause.unlock();
                if (mTotalSize == 0) {
                    mTotalSize = 1024;
                    mOffset = mTotalSize;
                    updateProgress ();
                }
                continue;
            } else if (nullptr != error) {
                detailError(&error);
                mStatus = ERROR;
                mPause.unlock();
                continue;
            }
            mPause.unlock();

            // write data
            // writeSize = g_output_stream_write(G_OUTPUT_STREAM(writeIO), buf, readSize, mCancel ? mCancel : nullptr, &error);
            g_output_stream_write_all(G_OUTPUT_STREAM(writeIO), buf, readSize, &writeSize, mCancel ? mCancel : nullptr, &error);
            if (nullptr != error) {
                qInfo() << "write destfile: " << mDestUri << " error: " << error->message;
                detailError(&error);
                mStatus = ERROR;
                continue;
            }

            if (readSize != writeSize) {
                // it's impossible
                qCritical() << "read file: " << mSrcUri << "  --- write file: " << mDestUri << " size not inconsistent";

                // check files existed again for ensure error message, related to #120721, #120973
                bool existed = g_file_query_exists(srcFile, nullptr) && g_file_query_exists(destFile, nullptr);
                if (!existed) {
                    error = g_error_new (1, G_IO_ERROR_FAILED, "%s", tr("Please check whether the device has been removed!").toUtf8().constData());
                } else {
                    error = g_error_new(1, G_IO_ERROR_FAILED, "%s", tr("Write file error: There is no available disk space for device!").toUtf8().constData());
                }

                detailError(&error);
                mStatus = ERROR;
                continue;
            }

            if (mOffset <= mTotalSize) {
                mOffset += writeSize;
            }

            updateProgress ();

        } else if (CANCEL == mStatus) {
            error = g_error_new(1, G_IO_ERROR_CANCELLED, "%s", tr("operation cancel").toUtf8().constData());
            detailError(&error);
            g_file_delete (destFile, nullptr, nullptr);
            break;
        } else if (ERROR == mStatus) {
            // 在一些特殊场景下可能会导致数据丢失问题，所以屏蔽
            //g_file_delete (destFile, nullptr, nullptr);
            break;
        } else if (FINISHED == mStatus) {
            break;
        } else if (PAUSE == mStatus) {
            if (mPause.tryLock(3000)) {
                if (RESUME == mStatus) {
                    mPause.unlock();
                }
                mStatus = RUNNING;
            }
        } else {
            mStatus = RUNNING;
        }
    }

out:
    // if copy sucessed, flush all data
    if (FINISHED == mStatus && g_file_query_exists(destFile, nullptr)) {
        // copy file attribute
        // It is possible that some file systems do not support file attributes
        //从只读文件系统复制文件，默认给与文件可写权限，hgzs项目前场反馈需求,task#138082
        g_file_copy_attributes(srcFile, destFile, mParentFlags, nullptr, &error);
        if (nullptr != error) {
            qWarning() << "copy attribute error:" << error->code << "  ---  " << error->message;
            g_error_free(error);
            error = nullptr;
        }
        sync(destFile);
    } else {
        // some special detail for mtp,  gphoto2, or other cases.
        if (mSrcUri.startsWith("mtp://") || mDestUri.startsWith("mtp://")) {
            if (mError) {
                if ((*mError)->code != G_IO_ERROR_EXISTS) {
                    g_error_free(*mError);
                    *mError = nullptr;
                    g_set_error(mError, 1, G_IO_ERROR_FAILED, "%s", tr("File opening failure").toUtf8().constData());
                }
            }else if (mDestUri.startsWith("gphoto2://")) {
                if (mError) {
                    g_error_free(*mError);
                    *mError = nullptr;
                    g_set_error(mError, 1, G_IO_ERROR_FAILED, "%s", tr("Failed to create %1. Please ensure if it is in root directory, or if the device supports gphoto2 protocol correctly.").arg(mDestUri).toUtf8().constData());
                }
            } else {
                if (mError) {
                    g_error_free(*mError);
                    *mError = nullptr;
                    g_set_error(mError, 1, G_IO_ERROR_FAILED, "%s", tr("Failed to create %1.").arg(mDestUri).toUtf8().constData());
                }
            }
        }
    }

    if (nullptr != readIO) {
        g_input_stream_close (G_INPUT_STREAM(readIO), nullptr, nullptr);
        g_object_unref(readIO);
    }

    if (nullptr != writeIO) {
        g_output_stream_close (G_OUTPUT_STREAM(writeIO), nullptr, nullptr);
        g_object_unref(writeIO);
    }

    if (nullptr != error) {
        g_error_free(error);
    }

    if (nullptr != srcFile) {
        g_object_unref(srcFile);
    }

    if (nullptr != destFile) {
        g_object_unref(destFile);
    }

    if (nullptr != srcFileInfo) {
        g_object_unref(srcFileInfo);
    }

    if (nullptr != destFileInfo) {
        g_object_unref(destFileInfo);
    }
}

int FileCopy::doCopyBigFile(const char *srcPath, const char *destPath)
{
    mStatus = RUNNING;
    int in_fd, out_fd, ret(-1);
    struct stat stat_buf;
    off_t offset = 0;
    int syncCount = 0;

    std::unique_lock<std::mutex> lock(fileMutex, std::defer_lock);
    lock.lock();
    in_fd = open(srcPath, O_RDONLY);
    if (in_fd == -1) {
        qDebug() << "Failed to open the source file";
        return ret;
    }

    if  (fstat(in_fd, &stat_buf) == -1) {
        qDebug() << "Failed to get the source file status";
        return ret;
    }
    lock.unlock();
    out_fd = open(destPath, O_WRONLY | O_CREAT, stat_buf.st_mode);
    if (out_fd == -1) {
        qDebug() << "Failed to open the destination file";
        return ret;
    }

    off_t buf = 10 * 1024 * 1024;
    ssize_t sendSize = 0;
    ssize_t size = 0;
    while(size < stat_buf.st_size) {
        if (nullptr != mCancel && g_cancellable_is_cancelled(mCancel)) {
            mStatus = CANCEL;
            break;
        }

        if (RUNNING == mStatus) {
            sendSize = sendfile(out_fd, in_fd, &offset, buf);
            if (sendSize == -1) {
                size = sendSize;
                if (0 != offset) {
                    mStatus = ERROR;
                }
                qWarning() << "send file error";
                break;
            }
            size += sendSize;
            if (mOffset <= mTotalSize) {
                mOffset = size;
            }
            if (!mIsDestFileLocal) {
                if (++syncCount > SYNC_INTERVAL) {
                    syncCount = 0;
                    FileOperationManager::getInstance()->setFsyncStatus(true);
                    fsync(out_fd);
                    FileOperationManager::getInstance()->setFsyncStatus(false);
                }
            }
            updateProgress ();
        } else if (PAUSE == mStatus) {
            if (mPause.tryLock(3000)) {
                if (RESUME == mStatus) {
                    mPause.unlock();
                }
                mStatus = RUNNING;
            }
        } else {
            mStatus = RUNNING;
        }
    }
    close(in_fd);
    close(out_fd);
    return size;
}

bool FileCopy::isFileOnLocal(const GFile* destFile)
{
    GMount* mount = g_file_find_enclosing_mount(const_cast<GFile*>(destFile), NULL, NULL);
    if (mount) {
        return !g_mount_can_unmount(mount);
    }
    return true;
}

void FileCopy::setParentFlags(GFileCopyFlags parentFlags)
{
    if (parentFlags != mParentFlags) {
        mParentFlags = parentFlags;
    }
}

/**
 * @note
 *  同步了 glib 针对vfat/fat32 单文件大于4g，提前提醒用户的补丁，
 *  由于 gio 获取文件系统类型不准确(据康哥反馈会把某些设备vfat/fat32、ntfs文件系统都识别为msdos)，
 *  所以使用linux接口获取。
 */
static gchar* get_fs_type (char* path)
{
    char*           fstype = NULL;
    struct mntent*  m = NULL;
    FILE*           f = NULL;

    f = setmntent ("/etc/mtab", "r");
    if (!f) {
        qWarning() << "get filesystem type error";
    }

    while ((m = getmntent(f))) {
        if (!path) continue;
        if (!m->mnt_dir) continue;
        if (!strcmp (path, m->mnt_dir)) {
            fstype = g_strdup_printf("%s", m->mnt_type);
            break;
        }
    }

    endmntent (f);

    return fstype;
}
