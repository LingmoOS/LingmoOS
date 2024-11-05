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

#ifndef FILERENAMEOPERATION_H
#define FILERENAMEOPERATION_H

#include "explorer-core_global.h"
#include "file-operation.h"

namespace Peony {

class PEONYCORESHARED_EXPORT FileRenameOperation : public FileOperation
{
    Q_OBJECT
public:
    /*!
     * \brief FileRenameOperation
     * \param uri
     * \param newName
     * \details
     * In most filemanagers, files always show their display name at directory view,
     * but there were a special kind of files not, the .desktop files might show
     * their names based on their contents.
     * Rename Operation have to both effect at the normal files and .desktop files(executable).
     * \note
     * Rename a .desktop file is very complex. Because it would change the contents of the file.
     * In GLib/GIO's api, it will lost some attribute if change the file contents.
     */
    explicit FileRenameOperation(QString uri, QString newName);

    void setAutoOverwrite () {
//        m_auto_overwrite = true;
        m_apply_all = OverWriteAll;
    }

    void setAutoBackup () {
        m_apply_all = BackupAll;
    }

    void setAutoIgnore () {
        m_apply_all = IgnoreAll;
    }

    void run() override;
    std::shared_ptr<FileOperationInfo> getOperationInfo() override {
        return m_info;
    }

private:
    QString getFileExtensionOfFile(const QString& file); /* 获取文件的文件扩展名 */

private:
    ExceptionResponse prehandle(GError *err);
    GFileCopyFlags m_default_copy_flag = GFileCopyFlags(G_FILE_COPY_NOFOLLOW_SYMLINKS | G_FILE_COPY_ALL_METADATA);
    QString m_uri = nullptr;
    QString m_new_name = nullptr;
    QString m_old_name = nullptr;

    std::shared_ptr<FileOperationInfo> m_info = nullptr;

    ExceptionResponse m_apply_all = Other;
};

}

#endif // FILERENAMEOPERATION_H
