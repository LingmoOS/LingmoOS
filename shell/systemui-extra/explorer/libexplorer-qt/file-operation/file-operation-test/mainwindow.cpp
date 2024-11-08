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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "mainwindow.h"
#include "file-move-operation.h"
#include "file-node.h"
#include "file-node-reporter.h"

#include "connect-server-dialog.h"
#include "gerror-wrapper.h"

#include "file-operation/file-operation-progress-wizard.h"
#include "file-operation-error-dialog.h"

#include <QDebug>
#include <QMessageBox>

#include <QToolBar>
#include <QLabel>
#include <QThread>
#include <QThreadPool>

#include <QProgressBar>
#include <QProgressDialog>

#include <QDialog>
#include <QFileDialog>
#include <QUrl>

#include <gerror-wrapper.h>

#include "file-copy-operation.h"
#include "file-delete-operation.h"
#include "file-link-operation.h"
#include "file-trash-operation.h"
#include "file-untrash-operation.h"
#include "file-rename-operation.h"

#include "file-enumerator.h"

#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    qDebug()<<QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    qDebug()<<g_get_user_special_dir(G_USER_DIRECTORY_TEMPLATES);
    QToolBar *t = new QToolBar(this);
    addToolBar(t);
    //quint64 *offset_value = new quint64(0);
    QLabel *offset_label = new QLabel(t);
    QLabel *current_uri_label = new QLabel(t);
    t->setOrientation(Qt::Vertical);
    t->addWidget(current_uri_label);
    t->addWidget(offset_label);
    QAction *startAction = new QAction("start", t);
    t->addAction(startAction);

    connect(startAction, &QAction::triggered, [=] {
        QMessageBox::question(nullptr, tr("source files"), tr("choose source files，"
                              "use cancel to finish the choices."));
        QFileDialog srcdlg;
        srcdlg.setFileMode(QFileDialog::ExistingFiles);
        srcdlg.setAcceptMode(QFileDialog::AcceptSave);
        srcdlg.setViewMode(QFileDialog::List);

        QStringList srcUris;
        srcdlg.exec();
        for (auto uri : srcdlg.selectedUrls()) {
            srcUris<<uri.toString();
        }

        QMessageBox::question(nullptr, tr("dest dir"), tr("choose dest dir,"
                              "use cancel to finish the choices."));
        QFileDialog destdlg;
        destdlg.setAcceptMode(QFileDialog::AcceptSave);
        destdlg.setFileMode(QFileDialog::Directory);

        QString destUri;
        destdlg.exec();
        destUri = destdlg.selectedUrls().at(0).url();

        //move
        Peony::FileMoveOperation *moveOp = new Peony::FileMoveOperation(srcUris, destUri);
        moveOp->setForceUseFallback();

        //copy
        //Peony::FileCopyOperation *moveOp = new Peony::FileCopyOperation(srcUris, destUri);

        //delete
        //Peony::FileDeleteOperation *moveOp = new Peony::FileDeleteOperation(srcUris);

        //link
        //Peony::FileLinkOperation *moveOp = new Peony::FileLinkOperation(srcUris.isEmpty()? nullptr: srcUris.at(0), destUri);

        //trash
        //Peony::FileTrashOperation *moveOp = new Peony::FileTrashOperation(srcUris);

        //untrash
        /*
        Peony::FileEnumerator e;
        e.setEnumerateDirectory("trash:///");
        e.enumerateSync();
        auto infos = e.getChildren();
        QStringList uris;
        for (auto info : infos) {
            uris<<info->uri();
        }
        Peony::FileUntrashOperation *moveOp = new Peony::FileUntrashOperation(uris);
        */

        //rename
        //Peony::FileRenameOperation *moveOp = new Peony::FileRenameOperation(srcUris.isEmpty()? nullptr: srcUris.at(0), "RenameSample");

        moveOp->connect(moveOp, &Peony::FileOperation::errored,
                        this, &MainWindow::handleError,
                        Qt::BlockingQueuedConnection);

        /*
        moveOp->connect(moveOp, &Peony::FileOperation::invalidOperation, [=](const QString &message){
            QMessageBox::critical(nullptr, "Error", message);
        });
        */

        Peony::FileOperationProgressWizard *wizard = new Peony::FileOperationProgressWizard;
        wizard->connect(moveOp, &Peony::FileOperation::operationStarted,
                        wizard, &Peony::FileOperationProgressWizard::show, Qt::BlockingQueuedConnection);
        wizard->connect(moveOp, &Peony::FileOperation::operationPreparedOne,
                        wizard, &Peony::FileOperationProgressWizard::onElementFoundOne);
        wizard->connect(moveOp, &Peony::FileOperation::operationPrepared,
                        wizard, &Peony::FileOperationProgressWizard::switchToProgressPage);
        wizard->connect(moveOp, &Peony::FileOperation::operationProgressedOne,
                        wizard, &Peony::FileOperationProgressWizard::onFileOperationProgressedOne);
        //operationFinished has a few time delay because there are many resources need be released and deconstructor.
        wizard->connect(moveOp, &Peony::FileOperation::operationProgressed,
                        wizard, &Peony::FileOperationProgressWizard::onFileOperationProgressedAll);
        wizard->connect(moveOp, &Peony::FileOperation::operationAfterProgressedOne,
                        wizard, &Peony::FileOperationProgressWizard::onElementClearOne);

        connect(wizard, &Peony::FileOperationProgressWizard::cancelled,
                moveOp, &Peony::FileOperation::cancel);
        connect(moveOp, &Peony::FileOperation::operationStartRollbacked,
                wizard, &Peony::FileOperationProgressWizard::switchToRollbackPage);
        connect(moveOp, &Peony::FileOperation::operationRollbackedOne,
                wizard, &Peony::FileOperationProgressWizard::onFileRollbacked);

        wizard->connect(moveOp, &Peony::FileOperation::operationFinished,
                        wizard, &QDialog::accepted);
        connect(wizard, &QDialog::accepted, wizard, &Peony::FileOperationProgressWizard::deleteLater);

        QThreadPool::globalInstance()->start(moveOp);
    });
}

MainWindow::~MainWindow()
{

}

QVariant MainWindow::handleError(const QString &srcUri,
                                 const QString &destDirUri,
                                 const Peony::GErrorWrapperPtr &err)
{
    Peony::FileOperationErrorDialog dlg;

    if (err.get()->code() == G_IO_ERROR_INVALID_FILENAME) {
        QMessageBox::critical(nullptr, "Critical:", err.get()->message());
        return QVariant(Peony::FileOperation::ResponseType::IgnoreAll);
    }

    return dlg.handleError(srcUri, destDirUri, err);
}
