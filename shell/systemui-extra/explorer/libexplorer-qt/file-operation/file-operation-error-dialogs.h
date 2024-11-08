/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Jing Ding <dingjing@kylinos.cn>
 *
 */
#ifndef FILEOPERATIONERRORDIALOGREGULAR_H
#define FILEOPERATIONERRORDIALOGREGULAR_H

#include <QLabel>
#include <QDialog>
#include <QCheckBox>
#include <QLineEdit>
#include <QScrollArea>
#include "file-operation-error-dialog-base.h"

namespace Peony {

class FileRenameDialog;
class FileInformationLabel;
class FileOperationErrorDialogConflict;


/*!
 * \brief Factory class for error handling pop-up boxes
 *
 */
class PEONYCORESHARED_EXPORT FileOperationErrorDialogFactory
{
public:
    static FileOperationErrorHandler* getDialog (FileOperationError& errInfo);
};

/*!
 * \brief Error warning pop-up box
 * ED_WARNING
 */
class PEONYCORESHARED_EXPORT FileOperationErrorDialogWarning : public FileOperationErrorDialogBase
{
    Q_OBJECT
    Q_INTERFACES(Peony::FileOperationErrorHandler)
public:
    explicit FileOperationErrorDialogWarning(FileOperationErrorDialogBase *parent = nullptr);
    ~FileOperationErrorDialogWarning()override;

    virtual void handle (FileOperationError& error) override;

private:
    bool m_ok = false;
    bool m_cancel = false;

    QPushButton* m_cancel_btn = nullptr;
};

/*!
 * \brief Dialog box for handling file conflicts
 *
 */
class PEONYCORESHARED_EXPORT FileOperationErrorDialogConflict : public FileOperationErrorDialogBase
{
    Q_OBJECT
    Q_INTERFACES(Peony::FileOperationErrorHandler)
public:
    explicit FileOperationErrorDialogConflict(FileOperationErrorDialogBase *parent = nullptr);
    ~FileOperationErrorDialogConflict() override;

    void setTipFilename (QString name);
    void setTipFileicon (QString icon);

    virtual void handle (FileOperationError& error) override;

private:
    bool            m_ignore = false;
    bool            m_backup = false;
    bool            m_replace = false;
    bool            m_do_same = false;
};

/*!
 * \brief Error warning pop-up box
 * ED_NOT_SUPPORTED
 */
class PEONYCORESHARED_EXPORT FileOperationErrorDialogNotSupported : public FileOperationErrorDialogBase
{
    Q_OBJECT
    Q_INTERFACES(Peony::FileOperationErrorHandler)
public:
    explicit FileOperationErrorDialogNotSupported(FileOperationErrorDialogBase *parent = nullptr);
    ~FileOperationErrorDialogNotSupported()override;

    virtual void handle (FileOperationError& error) override;

private:
    bool m_ok = false;
    bool m_cancel = false;
    bool m_do_same = true;
};

};

#endif // FILEOPERATIONERRORDIALOGREGULAR_H
