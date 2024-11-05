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

#ifndef FILENODE_H
#define FILENODE_H

#include <QString>
#include <QList>
#include <gio/gio.h>
#include <memory>

#include "file-operation.h"

namespace Peony {

enum TurnCateType {
    Post = 0,
    Front,
    AllPost,
    ALLFront,
};

class FileNodeReporter;

/*!
 * \brief The FileNode class
 * <br>
 * This class contains 3 parts:
 * 1. the self info, such as uri, basename, etc,
 * 2. the parent node and list of children,
 * 3. the reporter handle.
 * </br>
 * <br>
 * FileNode is smilar to FileItem, but it is more simple. It is used with many limitation.
 * Cause it method is Synchronized, you should never try use this class in the ui-thread.
 * This class is desgined for file operation classes which running in other threads.
 * For example, a copy operation might need enumerate all children for get the total size
 * of the source files.
 * </br>
 * <br>
 * The FileNodeReporter handle is used for send signal to tell other object the current state
 * of file node enumeration. Actually, a FileNode instance always be with a FileNodeReproter
 * instance at its initialization.
 * </br>
 * \see FileNodeReporter.
 */
class PEONYCORESHARED_EXPORT FileNode
{
    friend class FileNodeReporter;
public:
    enum State {
        Unhandled,
        Handling,
        Handled,
        Cleared, /*! \deprecated */
        Invalid
    };

    FileNode(QString uri, FileNode* parent, FileNodeReporter *reporter = nullptr);
    ~FileNode();

    //FIXME: do i need add cancel function?
    void findChildrenRecursively();
    void computeTotalSize(goffset *offset);

    QString uri() {
        return m_uri;
    }
    QString destUri() {
        return m_dest_uri;
    }
    State state() {
        return m_state;
    }
    Peony::ExceptionResponse responseType() {
        return m_err_response;
    }
    QString baseName() {
        return m_basename;
    }
    const QString destBaseName() {
        return m_dest_basename;
    }
    FileNode *parent() {
        return m_parent;
    }
    QList<FileNode*> *children() {
        return m_children;
    }
    qint64 size() {
        return m_size;
    }
    bool isFolder() {
        return m_is_folder;
    }

    /*!
     * \brief getRelativePath
     * \return
     * \deprecated
     */
    QString getRelativePath();

    /*!
     * \brief setDestUri
     * \param uri, the raw source uri of this file.
     * \deprecated
     * this method should not be used in newly writen code.
     *
     * \details
     * <br>
     * In explorer-qt, operantion is cancellable. That means we need add rollback function
     * for handling cancelled operation, to make sure that the operation is completed.
     * We should create a list of node trees for the files which have been copied or moved
     * for 'recover' them to the previous uri.
     * Dest uri is set when the file has been copied or moved to dest location.
     * This will aslo changed the node states. The rollback function will determine how to roll back
     * based on the status of dest uri and current states.
     * </br>
     * \see setState().
     */
    void setDestUri(QString uri) {
        m_dest_uri = uri;
    }
    /*!
     * \brief setState
     * \param state
     * <br>
     * State represent the current state of file.
     * </br>
     * \details
     * States of file has 3 types, there are: Unhandled, Handled, and Cleared.
     * Usually a file operaion of a file just have 2 states, Unhandled and Handled.
     * But some multi-step operation, such as fallback move operation, could have
     * 3 states. The file state will be changed when the file has been moved, copied
     * or deleted. That will guide the application how to roll back if the operation
     * was cancelled.
     */
    void setState(State state) {
        m_state = state;
    }
    /*!
     * \brief setErrorResponse
     * \param type
     * \details
     * When a file get into error in executing g_file operation, it will get the error handle response form
     * FileOperationErrorHandler. the when the file operation start clearing and rollbacking, the response type
     * will guide them how to do that.
     * For example, if a g_file move operation is ignored, it will not be cleared when clearing.
     */
    void setErrorResponse(ExceptionResponse type) {
        m_err_response = type;
    }
    void setDestFileName(const QString &name) {
        m_dest_basename = name;

        if (nullptr != m_dest_uri) {
            QStringList ls = m_dest_uri.split("/");
            ls.pop_back();
            ls.push_back(name);
            m_dest_uri = ls.join("/");
        }
    }
    const QString resolveDestFileUri(const QString &destRootDir);

    void truncateDestFileName(const int cateType);

private:
    QString m_uri = nullptr;
    QString m_basename = nullptr;
    QString m_dest_basename = nullptr;

    goffset m_size = 0;
    bool m_is_folder = false;
    FileNode *m_parent = nullptr;
    QList<FileNode*> *m_children = nullptr;

    QString m_dest_uri = nullptr;
    State m_state = Unhandled;
    ExceptionResponse m_err_response = Other;

    FileNodeReporter *m_reporter = nullptr;
};

}

#endif // FILENODE_H
