/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2013 Dominik Haumann <dhaumann.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateprojecttreeviewcontextmenu.h"
#include "filehistorywidget.h"
#include "git/gitutils.h"
#include "kateproject.h"
#include "kateprojectinfoviewterminal.h"
#include "kateprojectviewtree.h"

#include <KApplicationTrader>
#include <KIO/ApplicationLauncherJob>
#include <kio_version.h>
#if KIO_VERSION >= QT_VERSION_CHECK(5, 98, 0)
#include <KIO/JobUiDelegateFactory>
#else
#include <KIO/JobUiDelegate>
#endif
#include <KIO/OpenFileManagerWindowJob>
#include <KLocalizedString>
#include <KMoreTools>
#include <KMoreToolsMenuFactory>
#include <KPropertiesDialog>
#include <KTerminalLauncherJob>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QMimeType>
#include <QStandardPaths>

#include <ktexteditor/application.h>
#include <ktexteditor/editor.h>

static QString getName(QWidget *parent)
{
    QInputDialog dlg(parent);
    dlg.setLabelText(i18n("Enter name:"));
    dlg.setOkButtonText(i18n("Add"));
    dlg.setInputMode(QInputDialog::TextInput);

    int res = dlg.exec();
    bool suc = res == QDialog::Accepted;
    if (!suc || dlg.textValue().isEmpty()) {
        return {};
    }
    return dlg.textValue();
}

void KateProjectTreeViewContextMenu::exec(const QString &filename, const QModelIndex &index, const QPoint &pos, KateProjectViewTree *parent)
{
    /**
     * Create context menu
     */
    QMenu menu(parent);

    /**
     * Copy Path, always available, put that to the top
     */
    QAction *copyAction = menu.addAction(QIcon::fromTheme(QStringLiteral("edit-copy-path")), i18n("Copy Location"));

    QAction *addFile = nullptr;
    QAction *addFolder = nullptr;
    if (index.data(KateProjectItem::TypeRole).toInt() == KateProjectItem::Directory) {
        addFile = menu.addAction(QIcon::fromTheme(QStringLiteral("document-new")), i18n("Add File"));
        addFolder = menu.addAction(QIcon::fromTheme(QStringLiteral("folder-new")), i18n("Add Folder"));
    }

    // we can ATM only handle file renames
    QAction *rename = nullptr;
    QAction *fileDelete = nullptr;
    if (index.data(KateProjectItem::TypeRole).toInt() == KateProjectItem::File) {
        rename = menu.addAction(QIcon::fromTheme(QStringLiteral("edit-rename")), i18n("&Rename"));
        fileDelete = menu.addAction(QIcon::fromTheme(QStringLiteral("delete")), i18n("Delete"));
    }

    /**
     * File Properties Dialog
     */
    auto filePropertiesAction = menu.addAction(QIcon::fromTheme(QStringLiteral("dialog-object-properties")), i18n("Properties"));

    /**
     * Handle "open with",
     * find correct mimetype to query for possible applications
     */
    menu.addSeparator();
    QMenu *openWithMenu = menu.addMenu(i18n("Open With"));
    openWithMenu->setIcon(QIcon::fromTheme(QStringLiteral("system-run")));
    QMimeType mimeType = QMimeDatabase().mimeTypeForFile(filename);
    const KService::List offers = KApplicationTrader::queryByMimeType(mimeType.name());
    // For each one, insert a menu item...
    for (const auto &service : offers) {
        if (service->name() == QLatin1String("Kate")) {
            continue; // omit Kate
        }
        QAction *action = openWithMenu->addAction(QIcon::fromTheme(service->icon()), service->name());
        action->setData(service->entryPath());
    }
    // Perhaps disable menu, if no entries
    openWithMenu->setEnabled(!openWithMenu->isEmpty());

    /**
     * Open external terminal here
     */
    if (KateProjectInfoViewTerminal::isLoadable()) {
        menu.addAction(QIcon::fromTheme(QStringLiteral("terminal")), i18n("Open Internal Terminal Here"), [parent, &filename]() {
            QFileInfo checkFile(filename);
            if (checkFile.isFile()) {
                parent->openTerminal(checkFile.absolutePath());
            } else {
                parent->openTerminal(filename);
            }
        });
    }
    QAction *terminal = menu.addAction(QIcon::fromTheme(QStringLiteral("utilities-terminal")), i18n("Open External Terminal Here"));

    /**
     * Open Containing folder
     */
    auto openContaingFolderAction = menu.addAction(QIcon::fromTheme(QStringLiteral("document-open-folder")), i18n("&Open Containing Folder"));

    /**
     * Git menu
     */
    QAction *fileHistory = nullptr;
    KMoreToolsMenuFactory menuFactory(QStringLiteral("kate/addons/project/git-tools"));
    QMenu gitMenu; // must live as long as the maybe filled menu items should live
    if (GitUtils::isGitRepo(QFileInfo(filename).absolutePath())) {
        menu.addSeparator();
        fileHistory = menu.addAction(i18n("Show Git History"));
        menuFactory.fillMenuFromGroupingNames(&gitMenu, {QLatin1String("git-clients-and-actions")}, QUrl::fromLocalFile(filename));
        const auto gitActions = gitMenu.actions();
        for (auto action : gitActions) {
            menu.addAction(action);
        }
    }

    auto handleOpenWith = [parent](QAction *action, const QString &filename) {
        KService::Ptr app = KService::serviceByDesktopPath(action->data().toString());
        // If app is null, ApplicationLauncherJob will invoke the open-with dialog
        auto *job = new KIO::ApplicationLauncherJob(app);
        job->setUrls({QUrl::fromLocalFile(filename)});
#if KIO_VERSION >= QT_VERSION_CHECK(5, 98, 0)
        job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, parent));
#else
        job->setUiDelegate(new KIO::JobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, parent));
#endif
        job->start();
    };

    auto handleDeleteFile = [parent, index](const QString &path) {
        // message box
        const QString title = i18n("Delete File");
        const QString text = i18n("Do you want to delete the file '%1'?", path);
        if (QMessageBox::Yes == QMessageBox::question(parent, title, text, QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes)) {
            const QList<KTextEditor::Document *> openDocuments = KTextEditor::Editor::instance()->application()->documents();

            // if is open, close
            for (auto doc : openDocuments) {
                if (doc->url().adjusted(QUrl::RemoveScheme) == QUrl(path).adjusted(QUrl::RemoveScheme)) {
                    KTextEditor::Editor::instance()->application()->closeDocument(doc);
                    break;
                }
            }
            parent->removeFile(index, path);
        }
    };

    /**
     * run menu and handle the triggered action
     */
    if (QAction *const action = menu.exec(pos)) {
        if (action == copyAction) {
            QApplication::clipboard()->setText(filename);
        } else if (action == terminal) {
            // handle "open terminal here"
            QFileInfo checkFile(filename);
            auto *job = new KTerminalLauncherJob(QString());
            if (checkFile.isFile()) {
                job->setWorkingDirectory(checkFile.absolutePath());
            } else {
                job->setWorkingDirectory(filename);
            }
            job->start();
        } else if (action->parentWidget() == openWithMenu) {
            // handle "open with"
            handleOpenWith(action, filename);
        } else if (action == openContaingFolderAction) {
            KIO::highlightInFileManager({QUrl::fromLocalFile(filename)});
        } else if (fileDelete && action == fileDelete) {
            handleDeleteFile(filename);
        } else if (action == filePropertiesAction) {
            // code copied and adapted from frameworks/kio/src/filewidgets/knewfilemenu.cpp
            KFileItem fileItem(QUrl::fromLocalFile(filename));
            QDialog *dlg = new KPropertiesDialog(fileItem, parent);
            dlg->setAttribute(Qt::WA_DeleteOnClose);
            dlg->show();
        } else if (rename && action == rename) {
            /**
             * hack:
             * We store a reference to project in the item so that
             * after rename we can update file2Item map properly.
             */
            KateProjectItem *item = parent->project()->itemForFile(index.data(Qt::UserRole).toString());
            if (!item) {
                return;
            }
            item->setData(QVariant::fromValue(parent->project()), KateProjectItem::ProjectRole);

            /** start the edit */
            parent->edit(index);
        } else if (action == fileHistory) {
            FileHistory::showFileHistory(index.data(Qt::UserRole).toString());
        } else if (addFile && action == addFile) {
            QString name = getName(parent);
            if (!name.isEmpty()) {
                parent->addFile(index, name);
            }
        } else if (addFolder && action == addFolder) {
            QString name = getName(parent);
            if (!name.isEmpty()) {
                parent->addDirectory(index, name);
            }
        } else {
            // One of the git actions was triggered
        }
    }
}
