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

#include "file-operation-error-dialog.h"

#include <QFormLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QButtonGroup>

#include <QDesktopWidget>
#include <QApplication>

#include <QUrl>

using namespace Peony;

FileOperationErrorDialog::FileOperationErrorDialog(QWidget *parent) : QDialog(parent)
{
    //center to desktop.
    setParent(QApplication::desktop());
    //setWindowFlag(Qt::Dialog);
    //use WindowStaysOnTopHint flag to make sure this dialog always stay on top
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    setWindowTitle(tr("File Operation Error"));
    setWindowIcon(QIcon::fromTheme("system-error"));
    m_layout = new QFormLayout(this);
    m_layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    m_layout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    m_layout->setLabelAlignment(Qt::AlignRight);
    m_layout->setFormAlignment(Qt::AlignLeft);

    m_err_line = new QLabel(tr("unkwon"), this);
    m_src_line = new QLabel(tr("null"), this);
    m_dest_line = new QLabel(tr("null"), this);

    m_layout->addRow(tr("Error message:"), m_err_line);
    m_layout->addRow(tr("Source File:"), m_src_line);
    m_layout->addRow(tr("Dest File:"), m_dest_line);

    m_button_box = new QDialogButtonBox(this);
    m_button_box2 = new QDialogButtonBox(this);
    QPushButton *ignoreOneBt = new QPushButton(tr("Ignore"), m_button_box);
    QPushButton *ignoreAllBt = new QPushButton(tr("Ignore All"), m_button_box);
    QPushButton *overwriteOneBt = new QPushButton(tr("Overwrite"), m_button_box);
    QPushButton *overwriteAllBt = new QPushButton(tr("Overwrite All"), m_button_box);
    QPushButton *backupOneBt = new QPushButton(tr("Backup"), m_button_box);
    QPushButton *backupAllBt = new QPushButton(tr("Backup All"), m_button_box);
    QPushButton *retryBt = new QPushButton(tr("&Retry"), m_button_box);
    QPushButton *cancelBt = new QPushButton(tr("&Cancel"), m_button_box);

    btGroup = new QButtonGroup(this);
    btGroup->addButton(ignoreOneBt, 1);
    btGroup->addButton(ignoreAllBt, 2);
    btGroup->addButton(overwriteOneBt, 3);
    btGroup->addButton(overwriteAllBt, 4);
    btGroup->addButton(backupOneBt, 5);
    btGroup->addButton(backupAllBt, 6);
    btGroup->addButton(retryBt, 7);
    btGroup->addButton(cancelBt, 8);

    connect(btGroup, SIGNAL(buttonClicked(int)), this, SLOT(done(int)));

    m_button_box->addButton(ignoreOneBt, QDialogButtonBox::ActionRole);
    m_button_box->addButton(ignoreAllBt, QDialogButtonBox::ActionRole);
    m_button_box->addButton(overwriteOneBt, QDialogButtonBox::ActionRole);
    m_button_box->addButton(overwriteAllBt, QDialogButtonBox::ActionRole);
    m_button_box2->addButton(backupOneBt, QDialogButtonBox::ActionRole);
    m_button_box2->addButton(backupAllBt, QDialogButtonBox::ActionRole);
    m_button_box2->addButton(retryBt, QDialogButtonBox::ActionRole);
    m_button_box2->addButton(cancelBt, QDialogButtonBox::ActionRole);

    m_layout->addWidget(m_button_box);
    m_layout->addWidget(m_button_box2);

    setLayout(m_layout);
}

FileOperationErrorDialog::~FileOperationErrorDialog()
{

}

FileOperationErrorHandler::~FileOperationErrorHandler()
{

}

void FileOperationErrorDialog::handle (FileOperationError& error)
{
    Q_UNUSED(error);
}

int FileOperationErrorDialog::handleError(const QString &srcUri,
        const QString &destDirUri,
        const GErrorWrapperPtr &err,
        bool isCritical)
{
    QUrl srcUrl = srcUri;
    QUrl destDirUrl = destDirUri;
    for (int i = 3; i < 8; i++) {
        btGroup->button(i)->setVisible(!isCritical);
    }

    pfontMetrics = new QFontMetrics(m_src_line->font());
    int charWidth = pfontMetrics->averageCharWidth();
    QString edit_srcUri = pfontMetrics->elidedText(srcUrl.toDisplayString(), Qt::ElideRight, ELIDE_ERROR_TEXT_LENGTH * charWidth);
    QString edit_destDirUri = pfontMetrics->elidedText(destDirUrl.toDisplayString(), Qt::ElideRight, ELIDE_ERROR_TEXT_LENGTH * charWidth);
    QString edit_err_text = pfontMetrics->elidedText(err.get()->message(), Qt::ElideRight, ELIDE_ERROR_TEXT_LENGTH * charWidth);
    delete pfontMetrics;
    pfontMetrics = nullptr;

    m_src_line->setText(edit_srcUri);
    m_dest_line->setText(edit_destDirUri);
    m_err_line->setText(edit_err_text);

    //FIXME: how to uniform the button with dynamically?
    //the default size won't change before the dialog has shown.
    //do i need compute the longest text width? but the text is 18in...
    for (int i = 1; i < 9; i++) {
        btGroup->button(i)->setFixedWidth(100);
    }

    int val = exec();
    switch (val) {
    case 1: {
        return ExceptionResponse::IgnoreOne;
    }
    case 2: {
        return ExceptionResponse::IgnoreAll;
    }
    case 3: {
        return ExceptionResponse::OverWriteOne;
    }
    case 4: {
        return ExceptionResponse::OverWriteAll;
    }
    case 5: {
        return ExceptionResponse::BackupOne;
    }
    case 6: {
        return ExceptionResponse::BackupAll;
    }
    case 7: {
        return ExceptionResponse::Retry;
    }
    case 8: {
        return ExceptionResponse::Cancel;
    }
    default: {
        return ExceptionResponse::IgnoreAll;
    }
    }
}
