/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "desktop-menu.h"

#include "file-delete-operation.h"
#include "sound-effect.h"
#include "directory-view-plugin-iface.h"
#include "file-info-job.h"
#include "file-info.h"
#include "file-utils.h"
#include "file-launch-action.h"
#include "file-launch-manager.h"
#include "file-lauch-dialog.h"

#include "clipboard-utils.h"
#include "file-operation-utils.h"
#include "file-enumerator.h"

#include "desktop-menu-plugin-manager.h"

#include "global-settings.h"
#include "audio-play-manager.h"

#include <QProcess>
#include <QStandardPaths>
#include <QMessageBox>
#include <QtConcurrent>

#include <QUrl>
#include <QDir>
#include <QStandardPaths>

using namespace Peony;

DesktopMenu::DesktopMenu(DirectoryViewIface *view, QWidget *parent) : QMenu(parent)
{
    //comment old highlight effect
//    setProperty("useIconHighlightEffect", true);
//    setProperty("iconHighlightEffectMode", 1);
    setProperty("fillIconSymbolicColor", true);

    m_view = view;
    m_directory = view->getDirectoryUri();
    m_selections = view->getSelections();

    fillActions();
}

void DesktopMenu::fillActions()
{
    //add open actions
    auto openActions = constructOpenOpActions();
    if (!openActions.isEmpty())
        addSeparator();

    auto createTemplateActions = constructCreateTemplateActions();
    if (!createTemplateActions.isEmpty())
        addSeparator();

    //add view actions
    auto viewActions = constructViewOpActions();
    if (!viewActions.isEmpty())
        addSeparator();

    //add operation actions
    auto fileOpActions = constructFileOpActions();
    if (!fileOpActions.isEmpty())
        addSeparator();

    //add plugin actions
    auto pluginActions = constructMenuPluginActions();
    if (!pluginActions.isEmpty())
        addSeparator();

    //add propertries actions
    //if this is a background menu, then it should not add propertries actions
    bool isBackgroundMenu = m_selections.isEmpty();
    if(!isBackgroundMenu) {
        auto propertiesAction = constructFilePropertiesActions();
    }
}

const QList<QAction *> DesktopMenu::constructOpenOpActions()
{
    QList<QAction *> l;

    bool isBackgroundMenu = m_selections.isEmpty();
    if (isBackgroundMenu) {
        l<<addAction(QIcon::fromTheme("window-new-symbolic"), tr("Open in new Window"));
        connect(l.last(), &QAction::triggered, this, [=]() {
            this->openWindow(m_directory);
        });

        l<<addAction(tr("Select All"));
        connect(l.last(), &QAction::triggered, this, [=]() {
            //qDebug() << "select all";
            m_view->invertSelections();
        });
    } else {
        if (m_selections.count() == 1
                && !m_selections.first().startsWith("trash://")
                && !m_selections.first().startsWith("computer://")
                && (m_selections.first() != ("file://" + QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first()))) {
            auto info = FileInfo::fromUri(m_selections.first());
            auto displayName = info->displayName();
            //FIXME: replace BLOCKING api in ui thread.
            if (displayName.isEmpty())
                displayName = FileUtils::getFileDisplayName(info->uri());
            //when name is too long, show elideText
            //qDebug() << "charWidth:" <<charWidth;
            if (displayName.length() > ELIDE_TEXT_LENGTH)
            {
                int  charWidth = fontMetrics().averageCharWidth();
                displayName = fontMetrics().elidedText(displayName, Qt::ElideRight, ELIDE_TEXT_LENGTH * charWidth);
            }
            if (info->isDir()) {
                l<<addAction(QIcon::fromTheme("document-open-symbolic"), tr("Open"));
                connect(l.last(), &QAction::triggered, this, [=]() {
                    this->openWindow(m_selections);
                });

                auto recommendActions = FileLaunchManager::getRecommendActions(m_selections.first());
                if (recommendActions.count() > 1)
                {
                    auto openWithAction = addAction(tr("Open with..."));
                    QMenu *openWithMenu = new QMenu(this);

                    for (auto action : recommendActions) {
                        action->setParent(openWithMenu);
                        openWithMenu->addAction(static_cast<QAction*>(action));
                    }
                    auto fallbackActions = FileLaunchManager::getFallbackActions(m_selections.first());
                    for (auto action : fallbackActions) {
                        action->setParent(openWithMenu);
                        openWithMenu->addAction(static_cast<QAction*>(action));
                    }
                    openWithMenu->addSeparator();
                    openWithMenu->addAction(tr("More applications..."), this, [=]() {
                        FileLauchDialog d(m_selections.first());
                        d.exec();
                    });
                    openWithAction->setMenu(openWithMenu);
                }
            } else if (!info->isVolume()) {
                l<<addAction(QIcon::fromTheme("document-open-symbolic"), tr("Open"));
                connect(l.last(), &QAction::triggered, this, [=]() {
                    auto uri = m_selections.first();
                    FileLaunchManager::openAsync(uri);
                });
                auto openWithAction = addAction(tr("Open with..."));
                //FIXME: add sub menu for open with action.
                QMenu *openWithMenu = new QMenu(this);
                auto recommendActions = FileLaunchManager::getRecommendActions(m_selections.first());
                auto fallbackActions = FileLaunchManager::getFallbackActions(m_selections.first());
                //fix has default open app but no recommend actions issue, link to bug#61365
                //fix open options has two same app issue, linkto bug#74480, 69977
                if (recommendActions.count() == 0 && fallbackActions.count() ==0)
                {
                    auto action = FileLaunchManager::getDefaultAction(m_selections.first());
                    if (action != NULL && action->getAppInfoDisplayName().length() > 0)
                        recommendActions.append(action);
                }
                for (auto action : recommendActions) {
                    action->setParent(openWithMenu);
                    openWithMenu->addAction(static_cast<QAction*>(action));
                }

                for (auto action : fallbackActions) {
                    action->setParent(openWithMenu);
                    openWithMenu->addAction(static_cast<QAction*>(action));
                }
                openWithMenu->addSeparator();
                openWithMenu->addAction(tr("More applications..."), this, [=]() {
                    FileLauchDialog d(m_selections.first());
                    d.exec();
                });
                openWithAction->setMenu(openWithMenu);
            } else {
                l<<addAction(tr("Open"));
                connect(l.last(), &QAction::triggered, this, [=]() {
                    auto uri = m_selections.first();
                    //FIXME:
                });
            }
        } else {
            if (m_selections.count() == 1)
            {
                auto info = FileInfo::fromUri(m_selections.first());
                auto displayName = info->displayName();
                l<<addAction(QIcon::fromTheme("document-open-symbolic"), tr("Open"));
            }
            else
                l<<addAction(QIcon::fromTheme("document-open-symbolic"), tr("Open %1 selected files").arg(m_selections.count()));

            connect(l.last(), &QAction::triggered, this, [=]() {
                qDebug()<<"triggered:"<<m_selections.count();
                QStringList dirs;
                QStringList files;
                for (auto uri : m_selections) {
                    auto info = FileInfo::fromUri(uri);
                    if (info->isDir() || info->isVolume()) {
                        dirs<<uri;
                    } else {
                        files<<uri;
                    }
                }
                if (!dirs.isEmpty())
                    this->openWindow(dirs);
                if (!files.isEmpty()) {
                    for (auto uri : files) {
                        FileLaunchManager::openAsync(uri);
                    }
                }
            });
        }

        l<<addAction(tr("Reverse Select"));
        connect(l.last(), &QAction::triggered, this, [=]() {
            //qDebug() << "Reverse select";
            m_view->invertSelections();
        });
    }

    return l;
}

const QList<QAction *> DesktopMenu::constructCreateTemplateActions()
{
    QList<QAction *> l;
    if (m_selections.isEmpty()) {
        auto createAction = new QAction(tr("New"), this);
        l<<createAction;
        QMenu *subMenu = new QMenu(this);
        createAction->setMenu(subMenu);
        addAction(createAction);

        //enumerate template dir
        QDir templateDir(g_get_user_special_dir(G_USER_DIRECTORY_TEMPLATES));
        QString templatePath = templateDir.path();
        qWarning()<<"tempalte Path is"<<templatePath;
        if(!templatePath.isEmpty())
        {
            QDir templateDir(templatePath);
            auto templates = templateDir.entryList(QDir::AllEntries|QDir::NoDotAndDotDot);
            if (!templates.isEmpty()) {
                for (auto t : templates) {
                    QFileInfo qinfo(templateDir, t);
                    qWarning()<<"template entry is"<<qinfo.filePath();
                    GFile *gtk_file = g_file_new_for_path(qinfo.filePath().toUtf8().constData());
                    char *uri_str = g_file_get_uri(gtk_file);
                    //FIXME: replace BLOCKING api in ui thread.
                    std::shared_ptr<FileInfo> info = FileInfo::fromUri(uri_str);
                    QIcon tmpIcon;
                    if (info->uri().endsWith(".desktop")) {
                        QUrl url = info->uri();
                        auto path = url.path();
                        auto key_file = g_key_file_new();
                        if (g_key_file_load_from_file(key_file, path.toUtf8().constData(), G_KEY_FILE_NONE, 0)) {
                            g_autofree gchar* icon_name = g_key_file_get_value(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, 0);
                            if (icon_name) {
                                QString iconName = icon_name;
                                tmpIcon = QIcon::fromTheme(iconName);
                            }
                            g_key_file_free(key_file);
                        }
                    } else {
                        QString iconName = FileUtils::updateFileIconName(info->uri(), true);
                        tmpIcon = QIcon::fromTheme(iconName);
                    }

//                    QString mimeType = info->mimeType();
//                    if (mimeType.isEmpty()) {
//                        FileInfoJob job(info);
//                        job.querySync();
//                        mimeType = info->mimeType();
//                    }

//                    QIcon tmpIcon;
//                    GList *app_infos = g_app_info_get_recommended_for_type(mimeType.toUtf8().constData());
//                    GList *l = app_infos;
//                    QList<FileLaunchAction *> actions;
//                    bool isOnlyUnref = false;
//                    while (l) {
//                        auto app_info = static_cast<GAppInfo*>(l->data);
//                        if (!isOnlyUnref) {
//                            GThemedIcon *icon = G_THEMED_ICON(g_app_info_get_icon(app_info));
//                            const char * const * icon_names = g_themed_icon_get_names(icon);
//                            if (icon_names)
//                                tmpIcon = QIcon::fromTheme(*icon_names);
//                            if(!tmpIcon.isNull())
//                                isOnlyUnref = true;
//                        }
//                        l = l->next;
//                    }
//                    g_list_free_full(app_infos, g_object_unref);

                    QAction *action = new QAction(tmpIcon, qinfo.baseName(), this);
                    connect(action, &QAction::triggered, this, [=]() {
                        CreateTemplateOperation op(m_directory, CreateTemplateOperation::Template, t);
                        op.run();
                        auto target = op.target();
                        //clear old data,fix bug#164160, not enter edit new file issue
                        m_uris_to_edit.clear();
                        m_uris_to_edit<<target;
                    });
                    subMenu->addAction(action);
                    g_free(uri_str);
                    g_object_unref(gtk_file);
                }
                subMenu->addSeparator();
            } else {
                qWarning()<<"template entries is empty";
            }
        } else {
            qWarning()<<"template path is empty";
        }

        QList<QAction *> actions;
        auto createEmptyFileAction = new QAction(QIcon::fromTheme("document-new-symbolic"), tr("Empty File"), this);
        actions<<createEmptyFileAction;
        connect(actions.last(), &QAction::triggered, this, [=]() {
            //FileOperationUtils::create(m_directory);
            CreateTemplateOperation op(m_directory);
            op.run();
            auto targetUri = op.target();
            qDebug()<<"target:"<<targetUri;
            //clear old data,fix bug#164160, not enter edit new file issue
            m_uris_to_edit.clear();
            m_uris_to_edit<<targetUri;
        });
        auto createFolderActions = new QAction(QIcon::fromTheme("folder-new-symbolic"), tr("Folder"), this);
        actions<<createFolderActions;
        connect(actions.last(), &QAction::triggered, this, [=]() {
            //FileOperationUtils::create(m_directory, nullptr, CreateTemplateOperation::EmptyFolder);
            CreateTemplateOperation op(m_directory, CreateTemplateOperation::EmptyFolder, tr("New Folder"));
            op.run();
            auto targetUri = op.target();
            qDebug()<<"target:"<<targetUri;
            //clear old data,fix bug#164160, not enter edit new file issue
            m_uris_to_edit.clear();
            m_uris_to_edit<<targetUri;
        });
        subMenu->addActions(actions);
    }

    return l;
}

const QList<QAction *> DesktopMenu::constructViewOpActions()
{
    QList<QAction *> l;

    if (m_selections.isEmpty()) {
        auto viewTypeAction = addAction(tr("View Type"));
        l<<viewTypeAction;
        QMenu *viewTypeSubMenu = new QMenu(this);
        auto desktopView = dynamic_cast<DesktopIconView*>(m_view);
        auto zoomLevel = desktopView->zoomLevel();

        auto smallAction = viewTypeSubMenu->addAction(tr("Small"), this, [=]() {
            Q_EMIT setDefaultZoomLevel(DesktopIconView::Small);
        });
        auto normalAction = viewTypeSubMenu->addAction(tr("Normal"), this, [=]() {
            Q_EMIT setDefaultZoomLevel(DesktopIconView::Normal);
        });
        auto largeAction = viewTypeSubMenu->addAction(tr("Large"), this, [=]() {
            Q_EMIT setDefaultZoomLevel(DesktopIconView::Large);
        });
        auto hugeAction = viewTypeSubMenu->addAction(tr("Huge"), this, [=]() {
            Q_EMIT setDefaultZoomLevel(DesktopIconView::Huge);
        });

        switch (zoomLevel) {
        case DesktopIconView::Small:
            smallAction->setCheckable(true);
            smallAction->setChecked(true);
            break;
        case DesktopIconView::Normal:
            normalAction->setCheckable(true);
            normalAction->setChecked(true);
            break;
        case DesktopIconView::Large:
            largeAction->setCheckable(true);
            largeAction->setChecked(true);
            break;
        case DesktopIconView::Huge:
            hugeAction->setCheckable(true);
            hugeAction->setChecked(true);
            break;
        default:
            break;
        }

        viewTypeAction->setMenu(viewTypeSubMenu);

        //sort type
        auto sortTypeAction = addAction(tr("Sort By"));
        l<<sortTypeAction;
        QMenu *sortTypeMenu = new QMenu(this);

        QList<QAction *> tmp;
        tmp<<sortTypeMenu->addAction(tr("Name"));
        tmp<<sortTypeMenu->addAction(tr("Modified Date"));
        tmp<<sortTypeMenu->addAction(tr("File Type"));
        tmp<<sortTypeMenu->addAction(tr("File Size"));

        int sortType = m_view->getSortType();
        qDebug() << "sortType:" <<sortType <<tmp.count();
        if (sortType < 0 || sortType >= tmp.count()) {
            sortType = GlobalSettings::getInstance()->getValue(LAST_DESKTOP_SORT_ORDER).toInt();
        }
        tmp.at(sortType)->setCheckable(true);
        tmp.at(sortType)->setChecked(true);


        for (int i = 0; i < tmp.count(); i++) {
            connect(tmp.at(i), &QAction::triggered, this, [=]() {
                qDebug() << "setSortType in menu:" <<i;
                Q_EMIT setSortType(i);
                GlobalSettings::getInstance()->setValue(LAST_DESKTOP_SORT_ORDER, i);
            });
        }

        sortTypeAction->setMenu(sortTypeMenu);

        /*
        //sort order
        auto sortOrderAction = addAction(tr("Sort Order..."));
        l<<sortOrderAction;
        QMenu *sortOrderMenu = new QMenu(this);
        tmp.clear();
        tmp<<sortOrderMenu->addAction(tr("Ascending Order"));
        tmp<<sortOrderMenu->addAction(tr("Descending Order"));
        //        int sortOrder = m_view->getSortOrder();
        //        tmp.at(sortOrder)->setCheckable(true);
        //        tmp.at(sortOrder)->setChecked(true);

        for (int i = 0; i < tmp.count(); i++) {
            connect(tmp.at(i), &QAction::triggered, this, [=](){
                m_view->setSortOrder(i);
            });
        }

        sortOrderAction->setMenu(sortOrderMenu);

        l<<addAction(QIcon::fromTheme("zoom-in-symbolic"), tr("Zoom &In"), this, [=](){
            auto desktopView = dynamic_cast<DesktopIconView*>(m_view);
            desktopView->zoomIn();
        });
        l<<addAction(QIcon::fromTheme("zoom-out-symbolic"), tr("Zoom &Out"), this, [=](){
            auto desktopView = dynamic_cast<DesktopIconView*>(m_view);
            desktopView->zoomOut();
        });
        */
    }

    return l;
}

const QList<QAction *> DesktopMenu::constructFileOpActions()
{
    QList<QAction *> l;

    if (!m_selections.isEmpty()) {
        //don't show file operations when select home path
        QString homeUri = "file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        if (m_selections.count() == 1 && m_selections.first() == "trash:///") {
            FileEnumerator e;
            e.setEnumerateDirectory("trash:///");
            e.enumerateSync();
            auto trashChildren = e.getChildrenUris();
            //comment to avoid uncorrect operation in desktop, only in trash can do it
//            l<<addAction(QIcon::fromTheme("view-refresh-symbolic"), tr("&Restore all"), [=]() {
//                FileOperationUtils::restore(trashChildren);
//            });
//            l.last()->setEnabled(!trashChildren.isEmpty());
            l<<addAction(QIcon::fromTheme("edit-clear-symbolic"), tr("Clean the trash"), this, [=]() {
                auto removeop = Peony::FileOperationUtils::clearRecycleBinWithDialog(trashChildren);
                qApp->setProperty("clearTrash",true);
//                if(removeop){
//                    removeop->connect(removeop,&Peony::FileDeleteOperation::operationFinished, [=](){
//                        Peony::SoundEffect::getInstance()->recycleBinClearMusic();
//                    });
//                }
//                Peony::AudioPlayManager::getInstance()->playWarningAudio();
//                auto result = QMessageBox::question(nullptr, tr("Delete Permanently"), tr("Are you sure that you want to delete these files? "
//                                                    "Once you start a deletion, the files deleting will never be "
//                                                    "restored again."));
//                if (result == QMessageBox::Yes) {
//                    FileEnumerator e;
//                    FileOperationUtils::remove(trashChildren);
//                }
            });
            l.last()->setEnabled(!trashChildren.isEmpty());
        } else if (m_selections.count() == 1 && m_selections.first() == "computer:///") {

        } else if (! m_selections.contains(homeUri)) {
            l<<addAction(QIcon::fromTheme("edit-copy-symbolic"), tr("Copy"));
            connect(l.last(), &QAction::triggered, this, [=]() {
                ClipboardUtils::setClipboardFiles(m_selections, false);
                //auto desktopView = dynamic_cast<DesktopIconView*>(m_view);
                //desktopView->viewport()->update();
            });
            l<<addAction(QIcon::fromTheme("edit-cut-symbolic"), tr("Cut"));
            connect(l.last(), &QAction::triggered, this, [=]() {
                ClipboardUtils::setClipboardFiles(m_selections, true);
                //auto desktopView = dynamic_cast<DesktopIconView*>(m_view);
                //desktopView->viewport()->update();
            });

            if (!m_selections.contains("trash:///")) {
                bool canTrash = true;
                bool canDelete = true;
                for (auto uri : m_selections) {
                    auto info = FileInfo::fromUri(uri);
                    if (! info->canTrash())
                        canTrash = false;

                    if (! info->canDelete())
                        canDelete = false;
                }

                if (canTrash)
                {
                    l<<addAction(QIcon::fromTheme("edit-delete-symbolic"), tr("Delete to trash"));
                    connect(l.last(), &QAction::triggered, this, [=]() {
                        FileOperationUtils::trash(m_selections, true);
                    });
                }
                else if(canDelete)
                {
                    //comment delete forever right menu option,reference to mac and Windows
                    //add delete forever option
                    l<<addAction(QIcon::fromTheme("edit-clear-symbolic"), tr("Delete forever"));
                    connect(l.last(), &QAction::triggered, this, [=]() {
                        FileOperationUtils::executeRemoveActionWithDialog(m_selections);
                    });
                }
            }

            if (m_selections.count() == 1) {
                l<<addAction(QIcon::fromTheme("document-edit-symbolic"), tr("Rename"));
                connect(l.last(), &QAction::triggered, this, [=]() {
                    m_view->editUri(m_selections.first());
                });
            } else if (m_selections.count() > 1) {
                l<<addAction(QIcon::fromTheme("document-edit-symbolic"), tr("Rename"));
                connect(l.last(), &QAction::triggered, this, [=]() {
                    m_view->editUris(m_selections);
                });
            }
        }
    } else {
        auto pasteAction = addAction(QIcon::fromTheme("edit-paste-symbolic"), tr("Paste"));
        l<<pasteAction;
        pasteAction->setEnabled(ClipboardUtils::isClipboardHasFiles());
        connect(l.last(), &QAction::triggered, this, [=]() {
            ClipboardUtils::pasteClipboardFiles(m_directory);
        });
        l<<addAction(QIcon::fromTheme("view-refresh-symbolic"), tr("Refresh"));
        connect(l.last(), &QAction::triggered, this, [=]() {
            auto desktopView = dynamic_cast<DesktopIconView*>(m_view);
            desktopView->refresh();
        });
    }

    return l;
}

const QList<QAction *> DesktopMenu::constructFilePropertiesActions()
{
    QList<QAction *> l;

    l<<addAction(QIcon::fromTheme("preview-file"), tr("Properties"));
    connect(l.last(), &QAction::triggered, this, [=]() {
        //FIXME:
        if (m_selections.isEmpty()) {
            this->showProperties(m_directory);
        } else {
            this->showProperties(m_selections);
        }
    });

    return l;
}

void DesktopMenu::openWindow(const QString &uri)
{
    QUrl url = uri;
    QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    p.setProgram("/usr/bin/explorer");
    p.setArguments(QStringList()<<"--show-folders"<<url.toEncoded());
    p.startDetached();
#else
    p.startDetached("/usr/bin/explorer", QStringList()<<"--show-folders"<<uri);
#endif
}

const QList<QAction *> DesktopMenu::constructMenuPluginActions()
{
    QList<QAction *> l;
    //FIXME:
    auto mgr = DesktopMenuPluginManager::getInstance();
    if (mgr->isLoaded()) {
        //add debug log to check open-terminal not show in first time issue
        qDebug() << "DesktopMenu PluginManager is loaded";
        //sort plugiins by name, so the menu option orders is relatively fixed
        auto pluginIds = mgr->getPluginIds();
        qSort(pluginIds.begin(), pluginIds.end());

        for (auto id : pluginIds) {
            qDebug() << "desktop luginIds:" <<id;
            auto plugin = mgr->getPlugin(id);
            auto actions = plugin->menuActions(MenuPluginInterface::DesktopWindow,
                                               m_directory,
                                               m_selections);
            for (auto action : actions) {
                action->setParent(this);
            }
            l<<actions;
        }
    }
    addActions(l);
    return l;
}

void DesktopMenu::openWindow(const QStringList &uris)
{
    QStringList args;
    for (auto arg : uris) {
        QUrl url = arg;
        args<<QString(url.toEncoded());
    }
    QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    p.setProgram("/usr/bin/explorer");
    p.setArguments(QStringList()<<"--show-folders"<<args);
    p.startDetached();
#else
    p.startDetached("/usr/bin/explorer", QStringList()<<"--show-folders"<<args);
#endif
}

void DesktopMenu::showProperties(const QString &uri)
{
    //qDebug() << "showProperties uri:" <<uri;
    if (uri == "computer:///" || uri == "//")
    {
        gotoAboutComputer();
        return;
    }

    QUrl url = uri;
    QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    p.setProgram("/usr/bin/explorer");
    p.setArguments(QStringList()<<"--show-properties"<<url.toEncoded());
    p.startDetached();
#else
    p.startDetached("/usr/bin/explorer", QStringList()<<"--show-properties"<<url.toEncoded());
#endif
}

void DesktopMenu::showProperties(const QStringList &uris)
{
    QStringList args;
    for (auto arg : uris) {
        QUrl url = arg;
        args << url.toEncoded();
    }

    args.removeDuplicates();

    //注释掉这里以修复BUG#17809
    if (uris.contains("computer:///")) {
        args.removeAt(args.indexOf("computer:///"));
        QtConcurrent::run([=]() {
            gotoAboutComputer();
        });
    }

    if (uris.contains("trash:///") && uris.count() > 1) {
        args.removeAt(args.indexOf("trash:///"));
        QtConcurrent::run([=]() {
            QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            p.setProgram("/usr/bin/explorer");
            p.setArguments(QStringList() << "--show-properties" << "trash:///");
            p.startDetached();
#else
            p.startDetached("/usr/bin/explorer", QStringList()<<"--show-properties"<<args);
#endif
        });
    }

    QProcess p;
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    p.setProgram("/usr/bin/explorer");
    p.setArguments(QStringList() << "--show-properties" << args);
    p.startDetached();
#else
    p.startDetached("/usr/bin/explorer", QStringList()<<"--show-properties"<<args);
#endif
}

void DesktopMenu::gotoAboutComputer()
{
    QProcess p;
    p.setProgram("/usr/bin/lingmo-control-center");
    //-m About para to show about computer infos, related to bug#88258
    p.setArguments(QStringList()<<"-m" << "About");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    p.startDetached();
#else
    p.startDetached("/usr/bin/lingmo-control-center", QStringList()<<"-m" << "About");
#endif
    p.waitForFinished(-1);
}
