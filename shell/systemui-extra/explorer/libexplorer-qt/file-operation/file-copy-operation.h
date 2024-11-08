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

#ifndef FILECOPYOPERATION_H
#define FILECOPYOPERATION_H

#include "explorer-core_global.h"

#include "file-operation.h"

namespace Peony {

class FileNodeReporter;
class FileNode;

/*!
 * \brief The FileCopyOperation class
 * \todo
 * implment duplicated copy. this should be consumed as the backup handler.
 */
class PEONYCORESHARED_EXPORT FileCopyOperation : public FileOperation
{
    Q_OBJECT
public:
    explicit FileCopyOperation(QStringList sourceUris, QString destDirUri, QObject *parent = nullptr);
    ~FileCopyOperation() override;

    void run() override;
    std::shared_ptr<FileOperationInfo> getOperationInfo() override {
        return m_info;
    }

public Q_SLOTS:
    void cancel() override;

protected:
    ExceptionResponse prehandle(GError *err);
    static void progress_callback(goffset current_num_bytes,
                                  goffset total_num_bytes,
                                  FileCopyOperation *p_this);
    /*!
     * \brief copyRecursively
     * \param node
     * \see FileMoveOperation::copyRecursively()
     */
    void copyRecursively(FileNode *node);
    /*!
     * \brief rollbackNodeRecursively
     * \param node
     * \details
     * This function is similar to FileMoveOperation::rollbackNodeRecursively(),
     * but it is more simple. The copy operation's rollbacking logic is easier to
     * understand.
     */
    void rollbackNodeRecursively(FileNode *node);

    bool copyLinkedFile(FileNode *node, GFileInfo *info, GFileWrapperPtr file);

    ExceptionResponse udfCopyWarningDialog();

    bool saveAsOtherPath();
private:
    /*!
     * \brief m_is_duplicated_copy
     * \details
     * In explorer-qt, file copy operation has 2 types.
     * 1. The file is duplicated in same folder.
     * 2. Other.
     * If case 1, the explorer-qt will trying to copy a file with special suffix automaticly.
     * For example, a file abc.xyz will be copied and renamed to abc(1).xyz, abc(2).xyz...
     * If case 2, there will be nothing special action in operation except it went into error.
     * \deprecated
     * use FileDuplicateOperation instead.
     */
    bool m_is_duplicated_copy = false;
    QStringList m_source_uris;
    QString m_dest_dir_uri = nullptr;
    QString m_last_src_parent = nullptr;

    QString m_disc_media_type = nullptr;
    bool m_is_disk_work = false;
    int m_current_count = 0;
    int m_total_count = 0;
    QSet<QString> m_conflict_files;
    QString m_current_src_uri = nullptr;
    QString m_current_dest_dir_uri = nullptr;
    QString m_save_as_other_uri= nullptr;

    goffset m_current_offset = 0;
    goffset m_total_size = 0;

    GFileCopyFlags m_default_copy_flag = GFileCopyFlags(G_FILE_COPY_NOFOLLOW_SYMLINKS);
    GFileCopyFlags m_parent_flags = GFileCopyFlags(G_FILE_COPY_NONE);

    FileNodeReporter *m_reporter = nullptr;

    bool m_is_udf_burn_work = false;
    bool m_is_udf_warning = false;
    bool m_is_long_name_file_operation = false;
    bool m_is_long_name_error_msg_show = false;
    int m_cate_type = 0;
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

#endif // FILECOPYOPERATION_H
