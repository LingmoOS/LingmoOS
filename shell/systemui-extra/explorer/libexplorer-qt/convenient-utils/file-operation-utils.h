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

#ifndef FILEOPERATIONUTILS_H
#define FILEOPERATIONUTILS_H

#include <QStringList>
#include <memory>
#include "create-template-operation.h"

namespace Peony {

class FileInfo;
class FileOperation;

/*!
 * \brief The FileOperationUtils class
 * This is a class only provide static method for file operation.
 */
class PEONYCORESHARED_EXPORT FileOperationUtils
{
public:
    static FileOperation *move(const QStringList &srcUris, const QString &destUri, bool addHistory, bool copyMove = false);
    static FileOperation *copy(const QStringList &srcUris, const QString &destUri, bool addHistory);
    static FileOperation *trash(const QStringList &uris, bool addHistory);
    static FileOperation *trash(const QStringList &uris, bool addHistory, bool isFromSearchTab);
    static FileOperation *trashWithDialog(const QStringList &uris, bool addHistory);
    static FileOperation *remove(const QStringList &uris);
    static FileOperation *rename(const QString &uri, const QString &newName, bool addHistory);
    static FileOperation *link(const QString &srcUri, const QString &destUri, bool addHistory);
    static FileOperation *restore(const QString &uriInTrash);
    static FileOperation *restore(const QStringList &urisInTrash);
    static FileOperation *create(const QString &destDirUri, const QString &name = nullptr, CreateTemplateOperation::Type type = CreateTemplateOperation::EmptyFile);

    static void executeRemoveActionWithDialog(const QStringList &uris);

    static bool leftNameIsDuplicatedFileOfRightName(const QString &left, const QString &right);
    static bool leftNameLesserThanRightName(const QString &left, const QString &right);

    /*!
     * \brief queryFileInfo
     * \param uri
     * \return
     * \retval the file's latest info at current time.
     * \details
     * I provide a method for force querying a file's info.
     * \note
     * This info might be hold by other class instance yet,
     * that means other object share this info can recive the info's
     * updated signal when the file's info is updated.
     */
    static std::shared_ptr<FileInfo> queryFileInfo(const QString &uri);

    /*!
     * \brief move
     * \param srcUris
     * \param destUri
     * \param addHistory
     * \param action
     * Qt::MoveAction for default, it might cause a copy move while move a file to another file system.
     * Qt::CopyAction for copy, same as copy().
     * Qt::TargetMoveAction for force moving, delete source anyway.
     * \return
     */
    static FileOperation *moveWithAction(const QStringList &srcUris, const QString &destUri, bool addHistory, Qt::DropAction action = Qt::MoveAction);
    static FileOperation *moveWithAction(const QStringList &srcUris, const QString &destUri, bool addHistory, Qt::DropAction action, bool bMoveFromSearchTab = false);

    static FileOperation *clearRecycleBinWithDialog(const QStringList &list);

    static FileOperation *clearRecycleBinWithDialog(const QStringList &list, QWidget *parent);/* 增加一个重装接口,可设置父窗口，link to bug#22692 */

private:
    FileOperationUtils();
};

}

#endif // FILEOPERATIONUTILS_H
