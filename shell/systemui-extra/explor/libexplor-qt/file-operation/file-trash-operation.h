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

#ifndef FILETRASHOPERATION_H
#define FILETRASHOPERATION_H

#include "explor-core_global.h"
#include "file-operation.h"
#include "file-node.h"
#include "file-info.h"

namespace Peony {

class PEONYCORESHARED_EXPORT FileTrashOperation : public FileOperation
{
    Q_OBJECT
public:
    explicit FileTrashOperation(QStringList srcUris, QObject *parent = nullptr);

    std::shared_ptr<FileOperationInfo> getOperationInfo() override {
        return m_info;
    }
    void run() override;

    void setSearchOperation(const bool &isSearch);

Q_SIGNALS:
    void deleteRequest(const QStringList &uris);

private:
    void forceDelete (QString uri);
    void setErrorMessage (GError** err);
    void deleteRecursively(FileNode *node);

private:
    int m_current_count = 0;
    int m_total_count = 0;
    bool m_is_search = false;
    std::shared_ptr<FileOperationInfo> m_info = nullptr;

    // record source file infos before trashed, only used in native fs
    QList<std::shared_ptr<FileInfo>> m_src_infos;
};

}

#endif // FILETRASHOPERATION_H
