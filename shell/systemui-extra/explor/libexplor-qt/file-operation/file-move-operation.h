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

#ifndef FILEMOVEOPERATION_H
#define FILEMOVEOPERATION_H

#include "file-operation.h"
#include "file-info.h"

namespace Peony {

class FileNodeReporter;
class FileNode;

class FileOperationInfo;

/*!
 * \brief The FileMoveOperation class
 * \todo
 * implement the backup option of err handling.
 */
class PEONYCORESHARED_EXPORT FileMoveOperation : public FileOperation
{
    Q_OBJECT
public:
    /*!
     * \brief FileMoveOperation
     * \param sourceUris, list of source files' uris.
     * \param destUri, dest folder uri.
     * \param parent
     * <br>
     * This class is derived from FileOperation class, implement the virtual
     * run() function for moving files to a target directory.
     * </br>
     * \note
     * Unlike the g_file_move function, the target file of this operation must
     * be a directory.
     */
    explicit FileMoveOperation(QStringList sourceUris, QString destDirUri, QObject *parent = nullptr);

    ~FileMoveOperation() override;

    /*!
     * \brief setCopyMove
     * \param copyMove
     * \details
     * If a move operation is not native move, it will do a copy and delete operation
     * as a 'move'. setCopyMove will tell the operation do not delete the source files.
     */
    void setCopyMove(bool copyMove = true);

    void setAction(Qt::DropAction action);

    /*!
     * \brief setForceUseFallback
     * \param useFallback
     * \value true, force using copy and delete replace native move function provided by file system.
     * \value false, try using native move function first.
     * <br>
     * The reason why we have a optional of force-using fallback is that we might want to get
     * more details of a move, which is the native one doesn't support.
     * If we force using fallback function, we will trying to get infos of files that waiting for
     * moving. So that we can count the total size, and dynamicly get the progress of operation.
     * </br>
     */
    void setForceUseFallback(bool useFallback = true) {
        m_force_use_fallback = useFallback;
    }

    /*!
     * \brief rollbackNodeRecursively
     * \param node, the parent node need rollback
     * <br>
     * This is a recursive method for internal operation rollback.
     * recursive rollback function is more complex than recursive copy and delete function,
     * because we need decied the order of rollback and recursion by the node's states.
     * If the node need rollback state is Handled (cancelled in progressing, such as copy),
     * we need do recursion first, so that we can delete sub nodes successfully.
     * If the node state is Cleared, this will be the opposite of the previous one.
     * The rollback file will be created first.
     * <\br>
     * \note A native move operation does not support recursive rollback.
     */
    void rollbackNodeRecursively(FileNode *node);

    void run() override;

    std::shared_ptr<FileOperationInfo> getOperationInfo() override {
        return m_info;
    }
    bool isCopyMove() {
        return m_copy_move;
    }

    void setUriSort(bool isSort, int type = 0);

    void setSearchOperation(const bool &isSearch);

public Q_SLOTS:
    void cancel() override;

protected:
    static void progress_callback(goffset current_num_bytes,
                                  goffset total_num_bytes,
                                  FileMoveOperation *p_this);

    void copyRecursively(FileNode *node);
    void deleteRecursively(FileNode *node);

    bool isValid();
    void move();
    void moveForceUseFallback();
    void moveForceUseFallback(FileNode* node);
    bool copyLinkedFile(FileNode *node, GFileInfo *info, GFileWrapperPtr file);
    ExceptionResponse udfCopyWarningDialog();
    bool saveAsOtherPath();
    /*!
     * \brief prehandle
     * \param err
     * \return
     * \retval true if error handled.
     * \retval false if error not handled.
     * \details
     * There's something complicate handling the error in file operation.
     * User might handle the same kind of errors unified.
     * But different errors actually need different way to solve.
     * this function will help us do some thing prehandling if user
     * choose IgnoreAll, BackupAll or OverWriteAll option when a
     * specific type error occured.
     */
    ExceptionResponse prehandle(GError *err);

private:
    //QStringList m_source_uris;
    QString m_dest_dir_uri = nullptr;

    /*!
     * \brief m_current_count, used in progress_callback
     */
    int m_current_count = 0;
    /*!
     * \brief m_total_count, used in progress_callback
     */
    int m_total_count = 0;
    /*!
     * \brief m_current_src_uri, used in progress_callback.
     */
    QString m_current_src_uri = nullptr;
    /*!
     * \brief m_current_dest_dir_uri, used in progress_callback.
     */
    QString m_current_dest_dir_uri = nullptr;
    QString m_save_as_other_uri= nullptr;
    QString m_last_src_parent = nullptr;

    goffset m_current_offset = 0;
    goffset m_total_size = 0;

    /*!
     * \brief m_force_use_callback
     * \value true, the move operation will use copy + delete fallback anyway.
     * \value false, the move operation will priority using native move operations
     * if supported. The native implementation may support moving directories
     * (for instance on moves inside the same filesystem).
     */
    bool m_force_use_fallback = false;

    /*!
     * \brief m_copy_move
     * \value true, if native move operation is not supported.
     * just copy source files and do not delete source files.
     * \deprecated
     * should be dropped. use m_move_action instead.
     */
    bool m_copy_move = false;

    bool m_is_same_fs = false;

    /*!
     * \brief m_move_action
     * replacing m_copy_move flag.
     */
    Qt::DropAction m_move_action = Qt::MoveAction;

    GFileCopyFlags m_default_copy_flag = GFileCopyFlags(G_FILE_COPY_NOFOLLOW_SYMLINKS|
                                         G_FILE_COPY_NO_FALLBACK_FOR_MOVE);
    GFileCopyFlags m_parent_flags = GFileCopyFlags(G_FILE_COPY_NONE);

    FileNodeReporter *m_reporter = nullptr;

    bool m_is_long_name_file_operation = false;

    bool m_is_long_name_error_msg_show = false;

    bool m_is_udf_burn_work = false;

    bool m_is_udf_warning = false;

    bool m_is_search = false;

    QStringList m_burn_uris;

    int m_cate_type = 0;

    bool m_is_sort = true;

    int m_sort_type = 0;

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

#endif // FILEMOVEOPERATION_H
