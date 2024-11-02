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

#ifndef FILEDELETEOPERATION_H
#define FILEDELETEOPERATION_H

#include "file-operation.h"

#include "explor-core_global.h"

namespace Peony {

class FileNode;
class FileNodeReporter;

class PEONYCORESHARED_EXPORT FileDeleteOperation : public FileOperation
{
    Q_OBJECT
public:
    explicit FileDeleteOperation(QStringList sourceUris, QObject *parent = nullptr);
    ~FileDeleteOperation() override;

    std::shared_ptr<FileOperationInfo> getOperationInfo() override;

    void deleteRecursively(FileNode *node);
    void run() override;

    void cancel() override;

private:

    int m_current_count = 0;
    int m_total_count = 0;
    QString m_current_src_uri = nullptr;
    QString m_disc_media_type = nullptr;
    bool m_is_disk_work = false;

    goffset m_current_offset = 0;
    goffset m_total_szie = 0;

    FileNodeReporter *m_reporter = nullptr;

    /*!
     * \brief m_prehandle_hash
     * \details
     * Once a move operation get into error, this class might cache the specific response
     * for next prehandleing.
     */
    QHash<int, ExceptionResponse> m_prehandle_hash;

    std::shared_ptr<FileOperationInfo> m_info = nullptr;
};

}

#endif // FILEDELETEOPERATION_H
