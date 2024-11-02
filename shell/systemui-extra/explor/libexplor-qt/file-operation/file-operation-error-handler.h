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

#ifndef FILEOPERATIONERRORHANDLER_H
#define FILEOPERATIONERRORHANDLER_H

#include <QMap>
#include <QString>
#include <QVariant>
#include <QMetaType>
#include "gerror-wrapper.h"
#include "explor-core_global.h"

#define ErrorHandlerIID "org.lingmo.explor-qt.FileOperationErrorHandler"

namespace Peony {

enum ExceptionType
{
    ET_GIO,
    ET_CUSTOM,
};

/*!
 * \brief Type of error handling
 * \li ED_CONFLICT: General conflict handling for file operations
 */
enum ExceptionDialogType {
    ED_WARNING,
    ED_CONFLICT,
    ED_NOT_SUPPORTED,
    ED_RENAME           // usually used for long name handling, etc...
};

/*!
 * \brief All possible results of error handling
 */
enum ExceptionResponse {
    Other,
    Retry,
    Cancel,
    Rename,
    Invalid,
    IgnoreOne,
    IgnoreAll,
    BackupOne,
    BackupAll,
    OverWriteOne,
    OverWriteAll,
    Force,          // Force operation
    ForceAll,       //
    RenameOne,      // usually used for long name handling, etc...
    RenameAll,      // not used yet
    TruncateOne,    // truncate
    TruncateAll,
    SaveOne,        //long name file save as other paht
    SaveAll,
};

typedef enum{
    FileOpInvalid,  //invalid operation
    FileOpMove,     //file or dir move
    FileOpCopy,     //file or dir copy
    FileOpLink,     //file or dir create link
    FileOpRename,   //file or dir rename
    FileOpTrash,    //file or dir delete to trash
    FileOpUntrash,  //file or dir restore to origin from trash
    FileOpDelete,   //file or dir delete forever
    FileOpCount,    //file or dir file count
    FileOpCreateTemp, //create file or dir
    FileOpRenameToHideFile, // file or dir rename to a hide file
    FileOpRenameChangeType, // change file suffix cause type change
    FileOpNum,
}FileOpsType;

/*!
 * \brief The format of the data that needs to be transferred for error handling operations
 * \li errorCode: Error code
 * \li isCritical: is critical
 * \li title: The title that appears in the error handling window, indicating what operation went wrong
 * \li srcUri: The file/folder being operated on
 * \li destDirUri: The target folder
 * \li errorType: Error types, gio errors and custom errors
 * \li respCode: The action selected by the user
 * \li respValue: Data entered by the user
 */
typedef struct _FileOperationError
{
    int                         errorCode;
    bool                        isCritical;
    FileOpsType                 op;
    QString                     title;
    QString                     srcUri;
    QString                     destDirUri;
    QString                     errorStr;
    ExceptionType               errorType;
    ExceptionDialogType         dlgType;

    ExceptionResponse           respCode;
    QMap<QString, QVariant>     respValue;
} FileOperationError;

class PEONYCORESHARED_EXPORT FileOperationErrorHandler {
public:
    virtual ~FileOperationErrorHandler() = 0;
    virtual void handle (FileOperationError& error) = 0;
};
}

Q_DECLARE_INTERFACE(Peony::FileOperationErrorHandler, ErrorHandlerIID)

#endif // FILEOPERATIONERRORHANDLER_H
