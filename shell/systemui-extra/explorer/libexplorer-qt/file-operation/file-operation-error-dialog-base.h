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
#ifndef FILEOPERATIONERRORDIALOGBASE_H
#define FILEOPERATIONERRORDIALOGBASE_H

#include <QBoxLayout>
#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QProxyStyle>
#include <QPushButton>
#include "file-operation-error-handler.h"

namespace Peony {
class PEONYCORESHARED_EXPORT FileOperationErrorDialogBase : public QDialog, public FileOperationErrorHandler
{
    Q_OBJECT
    Q_INTERFACES(Peony::FileOperationErrorHandler)
public:
    explicit FileOperationErrorDialogBase(QDialog *parent);
    ~FileOperationErrorDialogBase() override;

    void adjustTextContent();

    void setText (QString text);
    void setIcon (QString iconName);
    QPushButton* addButton (QString name);
    QCheckBox* addCheckBoxLeft (QString name);

    bool event(QEvent *event) override;

Q_SIGNALS:
    void cancel();
    void fontChanged();

protected:
    FileOperationError*         m_error = nullptr;

private:
    QLabel*                     m_tipimage = nullptr;
    QLabel*                     m_tipcontent = nullptr;
    QHBoxLayout*                m_buttonLeft = nullptr;
    QHBoxLayout*                m_buttonRight = nullptr;
    QString                     m_iconName;
};
};


#endif // FILEOPERATIONERRORDIALOGBASE_H
