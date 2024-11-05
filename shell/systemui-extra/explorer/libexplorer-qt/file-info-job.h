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

#ifndef FileInfoJob_H
#define FileInfoJob_H

#define HAVE_SET_QUERY_DISPLAY_NAME_WITH_VOLUMES

#include "explorer-core_global.h"

#include <QObject>

#include <memory>
#include <gio/gio.h>

namespace Peony {

class FileInfo;

/*!
 * \brief The FileInfoJob class
 * <br>
 * FileInfoJob provide both sync and async method querying of an FileInfo
 * instance. Both of them will send the info's FileInfo::updated() signal.
 * For query async, it aslo send FileInfoJob::queryAsyncFinished() signal
 * when job finished.
 * </br>
 * \note
 * Operations that querying a file's information might be very frequent,
 * so I recommend querying in asynchronous and cancellable ways.
 * Using queryAsync() will cancel the all existing querying job about this shared info.
 * Alternatively, the share info data will be refresh when last async job finished.
 * This desgin is similar to explorer(caja).
 */
class PEONYCORESHARED_EXPORT FileInfoJob : public QObject
{
    friend class FileInfo;
    friend class FileEnumerator;

    Q_OBJECT
public:
    /*!
     * \brief FileInfoJob
     * \param info
     * \param parent
     * \deprecated
     * This constructor would be deprecated, use
     * FileInfoJob(const QString &uri, QObject *parent) instead in your
     * newly writen code.
     */
    explicit FileInfoJob(std::shared_ptr<FileInfo> info, QObject *parent = nullptr);
    explicit FileInfoJob(const QString &uri, QObject *parent = nullptr);

    std::shared_ptr<FileInfo> getInfo() {
        return m_info;
    }
    ~FileInfoJob();

    bool querySync();

    void setAutoDelete(bool deleteWhenJobFinished = true) {
        m_auto_delete = deleteWhenJobFinished;
    }

    static void setQueryDisplayNameWithVolumes(bool queryWithVolumes);

Q_SIGNALS:
    /*!
     * \brief queryAsyncFinished
     * \param successed
     * \retval true if no error happened in queryAsync() and callback
     * \retval false if error happened, it might be cancelled, or others.
     * \note If you just want to get the info states, I recommend you connect to
     * Peony::FileInfo::updated signal, every query job of a info will send this signal
     * if query successfully.
     * \see Peony::FileInfo::updated(), Peony::FileInfoJob::refreshInfoContents().
     * \deprecated use FileInfoJob::infoUpdated()
     */
    void queryAsyncFinished(bool successed);

    /*!
     * \brief infoUpdated
     * <br>
     * As we used shared data of FileInfo, a job might be cancelled frequently by other job
     * which has same FileInfo handle. So async callback might return an cancelled error.
     * This signal is triggered when a FileInfo::updated() send.
     * \see query_info_async_callback(), Peony::FileInfoJob::refreshInfoContents().
     * </br>
     */
    void infoUpdated();

public Q_SLOTS:
    void queryAsync();
    /*!
     * \brief cancel
     * <br>
     * cancel is an useful method to control the content refresh frequency.
     * there may be many query job for one same info at In a very short period of time
     * from different info holders.
     * </br>
     */
    void cancel();

    QString getAppName(QString desktopfp);

protected:
    static GAsyncReadyCallback query_info_async_callback(GFile *file,
            GAsyncResult *res,
            FileInfoJob *thisJob);

private:
    void queryFileType(GFileInfo* new_info);
    void queryFileDisplayName(GFileInfo* new_info);
    void refreshFileSystemInfo (GFileInfo* new_info);
    void refreshInfoContents (GFileInfo *new_info);

private:
    std::shared_ptr<FileInfo> m_info;

    quint64 m_file_size_uint = 0;
    quint64 m_file_modify_uint = 0;

    QString m_file_name = nullptr;
    QString m_file_size = nullptr;
    QString m_file_modify = nullptr;
    QString m_file_location = nullptr;

    bool m_auto_delete = false;

    GCancellable *m_cancellable = nullptr;
    GCancellable *m_fs_cancellable = nullptr;
};

}

#endif // FileInfoJob_H
