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

#ifndef FILEINFOSJOB_H
#define FILEINFOSJOB_H

#endif // FILEINFOSJOB_H
#include "explor-core_global.h"

#include <QObject>

#include <memory>
#include <gio/gio.h>

namespace Peony {

class FileInfo;

class PEONYCORESHARED_EXPORT FileInfosJob : public QObject
{
    Q_OBJECT
public:
    explicit FileInfosJob(std::vector<std::shared_ptr<FileInfo> >infos, QObject *parent = nullptr);
    ~FileInfosJob();
    std::vector<std::shared_ptr<FileInfo> > batchQuerySync();

public Q_SLOTS:
    void batchCancel();/* 批量取消查询时使用 */

private:
    std::shared_ptr<FileInfo> queryFileType(std::shared_ptr<FileInfo>info, GFileInfo* new_info);
    std::shared_ptr<FileInfo> queryFileDisplayName(std::shared_ptr<FileInfo>info, GFileInfo* new_info);
    std::shared_ptr<FileInfo> refreshFileSystemInfo(std::shared_ptr<FileInfo>info, GFileInfo* new_info);
    std::shared_ptr<FileInfo> refreshInfoContents(std::shared_ptr<FileInfo>info, GFileInfo *new_info);

private:
    GCancellable *m_batchCanellable = nullptr;
    std::vector<std::shared_ptr<FileInfo> >m_infos;
};

}
