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
 * Authors: ZhuoAn Li <lizhuoan@kylinos.cn>
 *
 */

#ifndef FILEBATCHRENAMEOPERATION_H
#define FILEBATCHRENAMEOPERATION_H

#include "explor-core_global.h"
#include "file-operation.h"
#include <QMutex>

namespace Peony {

class PEONYCORESHARED_EXPORT FileBatchRenameOperation : public FileOperation
{
    Q_OBJECT
public:
    explicit FileBatchRenameOperation(QStringList uris, QString newName);
    ~FileBatchRenameOperation();

    void run() override;

    std::shared_ptr<FileOperationInfo> getOperationInfo() override {
        return m_info;
    }

private:
    GFileCopyFlags m_default_copy_flag = GFileCopyFlags(G_FILE_COPY_NOFOLLOW_SYMLINKS | G_FILE_COPY_ALL_METADATA);
    QStringList m_uris ;
    QString m_new_name = nullptr;
    QStringList m_old_names;
    QStringList m_new_names;
    std::shared_ptr<FileOperationInfo> m_info = nullptr;
    ExceptionResponse m_apply_all = Other;

    goffset m_current_offset = 0;
    goffset m_total_size = 0 ;

    QString getFileExtensionOfFile(const QString& file); /* 获取文件的文件扩展名 */
    ExceptionResponse prehandle(GError *err);
    QString handleDuplicate(const QString uri);
    void rollback(std::shared_ptr<FileOperationInfo> info);

};

class FileBatchRenameInternalOperation : public FileOperation
{
    Q_OBJECT
public:
    explicit FileBatchRenameInternalOperation(std::shared_ptr<FileOperationInfo> info);

    void run() override;

    std::shared_ptr<FileOperationInfo> getOperationInfo() override {
        return m_info;
    }

private:
    std::shared_ptr<FileOperationInfo> m_info = nullptr;
};

}
#endif // FILEBATCHRENAMEOPERATION_H
