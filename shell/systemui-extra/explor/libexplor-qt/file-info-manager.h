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

#ifndef FILEINFOMANAGER_H
#define FILEINFOMANAGER_H

#include "file-info.h"
#include "file-item.h"

#include <QMutex>

namespace Peony {

/*!
 * \brief The FileInfoManager class
 * <br>
 * This is a class used to share FileInfo instances acrossing various members.
 * It is a single instance class with a hash table that cached all infos.
 * We generally would not operate directly on instance of this class,
 * because FileInfo class provides an interface for this class.
 * use FileInfo::fromUri(), FileInfo::fromPath() or FileInfo::fromGFile()
 * for getting the corresponding shared data.
 * </br>
 * \note The memory management is based on std smart pointer, but it is not regular.
 * Because hash table always hold a use count of shared data. If you want to use
 * shared data of this class instance, you should remenmber this point:
 * When releasing your info resources, you aslo need to add an additional judgment that whether
 * there is no other member but you and manager instance hold this shared data.
 * If true, you should aslo remove the element in manager's hash for really releasing resources.
 * Otherwise, it might cause an one-time memory leak.
 * \see FileInfo, FileInfoJob, FileEnumerator; FileInfo::~FileInfo(), FileInfoJob::~FileInfoJob(),
 * FileEnumerator::~FileEnumerator().
 * \bug
 * Even though I try my best to make share the file info data, it seems that the info is not be shared
 * sometimes. Maybe there were some wrong in other classes?
 */
class PEONYCORESHARED_EXPORT FileInfoManager
{
    friend class FileInfo;
    friend class FileItem;

public:
    static FileInfoManager *getInstance();
    std::shared_ptr<FileInfo> findFileInfoByUri(const QString &uri); //{return global_info_list->value(uri);}

    void lock() {
        m_mutex.lock();
    }
    void unlock() {
        m_mutex.unlock();
    }

    /*!
     * \brief isAutoParted
     * true if using lingmo-installer's default pareted strategy.
     */
    bool isAutoParted();
    void showState();

protected:
    std::shared_ptr<FileInfo> insertFileInfo(std::shared_ptr<FileInfo> info); //{global_info_list->insert(info->uri(), info);}
    void updateFileInfo(std::shared_ptr<FileInfo>& info); //{global_info_list->insert(info->uri(), info);}
    std::shared_ptr<ExtraInfoRecorder> getExtraInfoRecorderByUri(const QString &uri); /* <uri, ExtraInfoRecorder>,暂时只允许在fileitem中使用！ */


private:
    FileInfoManager();
    ~FileInfoManager();

    QMutex m_mutex;
};

}

#endif // FILEINFOMANAGER_H
