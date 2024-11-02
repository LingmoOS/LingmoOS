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

#include "directory-view-menu.h"
#include "directory-view-plugin-iface2.h"
#include "directory-view-widget.h"

#include "fm-window.h"
#include "directory-view-container.h"

#include "menu-plugin-manager.h"
#include "file-info-job.h"
#include "file-info.h"

#include "directory-view-factory-manager.h"
#include "view-factory-model.h"
#include "view-factory-sort-filter-model.h"

#include "clipboard-utils.h"
#include "file-operation-utils.h"
#include "file-operation-manager.h" //FileOpInfo
#include "audio-play-manager.h"
#include "file-untrash-operation.h"
#include "file-delete-operation.h"
#include "sound-effect.h"
#ifdef LINGMO_SDK_SOUND_EFFECTS
#include "ksoundeffects.h"
#endif

#include "file-utils.h"
#include "bookmark-manager.h"

#include "volume-manager.h"

//#include "properties-window.h"
#include "properties-window-factory-plugin-manager.h"
#include "windows/format_dialog.h"
#include "file-launch-manager.h"
#include "file-launch-action.h"
#include "file-lauch-dialog.h"
#include "file-operation-error-dialog.h"
#include "file-enumerator.h"
#include "gerror-wrapper.h"
#include "format-dlg-create-delegate.h"

#include "global-settings.h"
#include "sound-effect.h"
#include "directoryviewhelper.h"
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>

#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

#include <QLocale>
#include <QStandardPaths>
#include <recent-vfs-manager.h>

#include <QDebug>

#include <QApplication>

using namespace Peony;
#ifdef LINGMO_SDK_SOUND_EFFECTS
using namespace kdk;
#endif

#define DEBUG qDebug() << "[" << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << "]"

DirectoryViewMenu::DirectoryViewMenu(DirectoryViewWidget *directoryView, QWidget *parent) : QMenu(parent)
{
    m_top_window = nullptr;
    m_view = directoryView;

    m_directory = directoryView->getDirectoryUri();
    m_selections = directoryView->getSelections();

    fillActions();
}

DirectoryViewMenu::DirectoryViewMenu(FMWindowIface *window, QWidget *parent) : QMenu(parent)
{
    m_top_window = window;
    m_view = window->getCurrentPage()->getView();
    //setParent(dynamic_cast<QWidget*>(m_view));

    m_directory = window->getCurrentUri();
    m_selections = window->getCurrentSelections();

    fillActions();
}

void DirectoryViewMenu::setHiddenActionsByObjectName(const QStringList &actionNames)
{
    for (auto action : actions()) {
        if (actionNames.contains(action->objectName())) {
            action->setVisible(false);
        }
    }
}

void DirectoryViewMenu::fillActions()
{
    m_version = qApp->property("version").toString();
    QString actualDir =  m_directory;
    if(actualDir.startsWith("search://")){
        m_is_search = true;
        actualDir = FileUtils::getActualDirFromSearchUri(m_directory);
    }

    QString tmpUri;
    g_autoptr (GFile) tmp_file = g_file_new_for_uri(actualDir.toUtf8().constData());
    g_autofree gchar* tmp_uri = g_file_get_uri(tmp_file);
    tmpUri = tmp_uri;

    if (actualDir == "computer:///") {
        m_is_computer = true;
    }

    if (actualDir == "network:///") {
        m_is_network = true;
    }

    if (actualDir == "trash:///") {
        m_is_trash = true;
    }

    if (actualDir.startsWith("burn://")) {
        m_is_cd = true;
    }

    if (actualDir.startsWith("recent://")) {
        m_is_recent = true;
    }

    if (actualDir.startsWith("favorite://")) {
        m_is_favorite = true;
    }

    if (actualDir.startsWith("kydroid:///") || actualDir.startsWith("kmre:///") || actualDir.startsWith("mult://")) {
        m_is_kydroid = true;
    }

    if (actualDir.startsWith("ftp://")
            || actualDir.startsWith("sftp://") || tmpUri.startsWith("ftp://") || tmpUri.startsWith("sftp://")) {
        m_is_ftp = true;
    }

    if (actualDir.startsWith("filesafe:///")){
        m_is_filebox_file = true;
    }

    if(actualDir == "filesafe:///" || m_directory.startsWith("search:///search_uris=filesafe:///&")) {
        m_is_filesafe = true;
    }

    if(actualDir.startsWith("smb://") || tmpUri.startsWith("smb://")){
        m_is_smb_file = true;
    }

    QString boxpath = "file://"+QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.box";
    if(actualDir == boxpath) {
        m_is_boxpath = true;
    }

    if (actualDir.startsWith("label://") && m_version != "lingmo3.0"){
        m_is_label_model = true;
    }

    if (actualDir.startsWith("mtp://") || actualDir.startsWith("gphoto2://")){
        m_is_mtp_ptp = true;
    }

    isMobileFile(actualDir);/* 判断是否为移动文件或目录 */
//    auto dev = VolumeManager::getDriveFromUri(actualDir);
//    if(dev != nullptr){
//        bool canEject = g_drive_can_eject(dev.get()->getGDrive());
//        bool canStop = g_drive_can_stop(dev.get()->getGDrive());
//        if(canEject || canStop){
//            m_is_mobile_file = true;
//        }
//        qDebug() << "canEject :" << canEject;
//    }
    //task#147972 【删除回收站】选项需求
    //如果是长城的机器并且带了9215控制器，不再区分移动设备，统一右键删除到回收站
    bool trashSettings = GlobalSettings::getInstance()->getValue(TRASH_MOBILE_FILES).toBool();
    if (trashSettings)
       m_is_mobile_file = false;

    QString homeUri = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString musicUri = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString videoUri = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    QString docUri = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString pictureUri = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString downloadUri = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

    for (auto uriIndex = 0; uriIndex < m_selections.count(); ++uriIndex) {
        //qDebug() << desktop;
        if (m_selections.at(uriIndex) == "favorite:///?schema=trash"
                || m_selections.at(uriIndex) == "favorite:///?schema=kmre"
                || m_selections.at(uriIndex) == "favorite:///?schema=recent"
                || m_selections.at(uriIndex) == "favorite:///data/usershare?schema=file"
                || m_selections.at(uriIndex) == "favorite://" + homeUri + "?schema=file"
                || m_selections.at(uriIndex) == "favorite://" + desktop + "?schema=file"
                /*|| m_selections.at(uriIndex) == "favorite://" + musicUri + "?schema=file"
                || m_selections.at(uriIndex) == "favorite://" + videoUri + "?schema=file"
                || m_selections.at(uriIndex) == "favorite://" + docUri + "?schema=file"
                || m_selections.at(uriIndex) == "favorite://" + pictureUri + "?schema=file"
                || m_selections.at(uriIndex) == "favorite://" + downloadUri + "?schema=file"*/) {
            m_can_delete = false;
            break;
        }
    }

    QList<QAction *> l;

    //add open actions
    auto openActions = constructOpenOpActions();
    if (!openActions.isEmpty()) {
        l<<addSeparator();
        l.last()->setObjectName(OPEN_ACTIONS_SEPARATOR);
    }

    //netwotk items not show operation menu
    if (m_is_network)
        return;

    if (! m_is_kydroid){
        //create template actions
        auto templateActions = constructCreateTemplateActions();
        if (!templateActions.isEmpty()) {
            l<<addSeparator();
            l.last()->setObjectName(CREATE_ACTIONS_SEPARATOR);
        }

        //add view actions
        auto viewActions = constructViewOpActions();
        if (!viewActions.isEmpty()) {
            l<<addSeparator();
            l.last()->setObjectName(VIEW_ACTIONS_SEPARATOR);
        }
    }

    //add multiselect action
    auto multiselectAction = constructMultiSelectActions();
    if(!multiselectAction.isEmpty())
        addSeparator();

    //add operation actions
    auto fileOpActions = constructFileOpActions();
    if (!fileOpActions.isEmpty()) {
        l<<addSeparator();
        l.last()->setObjectName(FILE_OPERATION_ACTIONS_SEPARATOR);
    }

    if (! m_is_kydroid){
        //add plugin actions
        auto pluginActions = constructMenuPluginActions();
        if (!pluginActions.isEmpty()) {
            l<<addSeparator();
            l.last()->setObjectName(PLUGIN_ACTIONS_SEPARATOR);
        }
    }

    //add propertries actions
    auto propertiesAction = constructFilePropertiesActions();
    if (!propertiesAction.isEmpty()) {
        l<<addSeparator();
        l.last()->setObjectName(PROPERTIES_ACTIONS_SEPARATOR);
    }

    if (! m_is_kydroid){
        //add actions in computer:///
        auto computerActions = constructComputerActions();
        if (!computerActions.isEmpty()) {
            l<<addSeparator();
            l.last()->setObjectName(COMPUTER_ACTIONS_SEPARATOR);
        }

        //add actions in trash:///
        auto trashActions = constructTrashActions();
        if (!trashActions.isEmpty()) {
            l<<addSeparator();
            l.last()->setObjectName(TRASH_ACTIONS_SEPARATOR);
        }

        //add actions in search:///
        auto searchActions = constructSearchActions();
    }
}

const QList<QAction *> DirectoryViewMenu::constructOpenOpActions()
{
    QList<QAction *> l;
    if (m_is_trash)
        return l;

    bool isBackgroundMenu = m_selections.isEmpty();
    if (isBackgroundMenu) {
        l<<addAction(QIcon::fromTheme("window-new-symbolic"), tr("Open in New Window"));
        l.last()->setObjectName(OPEN_IN_NEW_WINDOW_ACTION);
        connect(l.last(), &QAction::triggered, this, [=]() {
            auto windowIface = m_top_window->create(m_directory);
            auto newWindow = dynamic_cast<QWidget *>(windowIface);
            newWindow->setAttribute(Qt::WA_DeleteOnClose);
            //FIXME: show when prepared?
            newWindow->show();
        });
        if (!qApp->property("tabletMode").toBool()) {
            l<<addAction(QIcon::fromTheme("tab-new-symbolic"), tr("Open in New Tab"));
            l.last()->setObjectName(OPEN_IN_NEW_TAB_ACTION);
            connect(l.last(), &QAction::triggered, this, [=]() {
                if (!m_top_window)
                    return;
                QStringList uris;
                uris<<m_directory;
                m_top_window->addNewTabs(uris);
            });
        }
    } else {
        if (m_selections.count() == 1) {
            auto info = FileInfo::fromUri(m_selections.first());
            auto displayName = info->displayName();

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
                //add to bookmark option
                if (!info->isVirtual() &&  !info->uri().startsWith("smb://") && !m_is_kydroid && !m_is_filesafe && !m_is_filebox_file)
                {
                    l<<addAction(QIcon::fromTheme("bookmark-add-symbolic"), tr("Add to bookmark"));
                    l.last()->setObjectName(ADD_TO_BOOKMARK_ACTION);

                    if(BookMarkManager::getInstance()->existsInBookMarks(info->uri()))
                    {
                        l.last()->setEnabled(false);
                    }
                    else
                    {
                        connect(l.last(), &QAction::triggered, this, [=]() {

                            //qDebug() <<"add to bookmark:" <<info->uri();
                            auto bookmark = BookMarkManager::getInstance();
                            if (bookmark->isLoaded()) {
                                bookmark->addBookMark(info->uri());
                            }
                        });
                    }
                }

                l<<addAction(QIcon::fromTheme("document-open-symbolic"), tr("Open"));
                l.last()->setObjectName(OPEN_ACTION);
                connect(l.last(), &QAction::triggered, this, [=]() {
                    if (!m_top_window)
                        return;
                    m_top_window->goToUri(m_selections.first(), true);
                });

                auto recommendActions = FileLaunchManager::getRecommendActions(m_selections.first());
                if (recommendActions.count() >1)
                {
                    auto openWithAction = addAction(tr("Open with..."));
                    openWithAction->setObjectName(OPEN_WITH_ACTION);
                    QMenu *openWithMenu = new QMenu(this);
                    // do not highlight application icons.
                    openWithMenu->setProperty("skipHighlightIconEffect", true);
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
                    openWithMenu->addAction(tr("More applications..."), [=]() {
                        FileLauchDialog d(m_selections.first());
                        d.exec();
                    });
                    openWithAction->setMenu(openWithMenu);
                }

                l<<addAction(QIcon::fromTheme("window-new-symbolic"), tr("Open in New Window"));
                l.last()->setObjectName(OPEN_IN_NEW_WINDOW_ACTION);
                connect(l.last(), &QAction::triggered, this, [=]() {
                    auto windowIface = m_top_window->create(m_selections.first());
                    auto newWindow = dynamic_cast<QWidget *>(windowIface);
                    newWindow->setAttribute(Qt::WA_DeleteOnClose);
                    //FIXME: show when prepared?
                    newWindow->show();
                });
                if (!qApp->property("tabletMode").toBool()) {
                    l<<addAction(QIcon::fromTheme("tab-new-symbolic"), tr("Open in New Tab"));
                    l.last()->setObjectName(OPEN_IN_NEW_TAB_ACTION);
                    connect(l.last(), &QAction::triggered, this, [=]() {
                        if (!m_top_window)
                            return;
                        m_top_window->addNewTabs(m_selections);
                    });
                }
            } else if (!info->isVolume()) {
#ifdef MULTI_DISABLE
                bool needDisable = isMultFile(info);
#endif
                l<<addAction(QIcon::fromTheme("document-open-symbolic"), tr("Open"));
                l.last()->setObjectName(OPEN_ACTION);
                connect(l.last(), &QAction::triggered, this, [=]() {
                    auto uri = m_selections.first();
                    if (m_is_network)
                        m_top_window->goToUri(uri, true);
                    else
                        FileLaunchManager::openAsync(uri, false, false);
                });
#ifdef MULTI_DISABLE
                if (needDisable) {
                    l.last()->setEnabled(false);
                }
#endif
                if (m_is_network)
                    return l;
                auto openWithAction = addAction(tr("Open with..."));
                openWithAction->setObjectName(OPEN_WITH_ACTION);
                QMenu *openWithMenu = new QMenu(this);
                // do not highlight application icons.
                openWithMenu->setProperty("skipHighlightIconEffect", true);

                QString uri = m_selections.first();
                //fix bug#101386, can not open file in filesafe path
                //fix bug#125679, can not open file in recent
                if (uri.startsWith("filesafe:///") || uri.startsWith("recent:///"))
                {
                   auto targetUri = FileUtils::getTargetUri(uri);
                   uri = targetUri.isEmpty() ? uri : targetUri;
                }

                //use origin uri instead of target uri, fix recommand menu not same with desktop issue
                //link to bug#80207
                auto recommendActions = FileLaunchManager::getRecommendActions(uri);
                auto fallbackActions = FileLaunchManager::getFallbackActions(uri);
                //fix has default open app but no recommend actions issue, link to bug#61365
                //fix open options has two same app issue, linkto bug#74480
                if (recommendActions.count() == 0 && fallbackActions.count() == 0)
                {
                    auto action = FileLaunchManager::getDefaultAction(uri);
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
                    FileLauchDialog d(uri);
                    d.exec();
                });
                openWithAction->setMenu(openWithMenu);
#ifdef MULTI_DISABLE
                if (needDisable) {
                    openWithAction->setEnabled(false);
                }
#endif
            } else {
                l<<addAction(tr("Open"));
                l.last()->setObjectName(OPEN_ACTION);
                connect(l.last(), &QAction::triggered, this, [=]() {
                    auto uri = m_selections.first();
                    //FIXME:
                    m_top_window->goToUri(uri, true);
                });
            }
        } else {
            l<<addAction(QIcon::fromTheme("document-open-symbolic"), tr("Open %1 selected files").arg(m_selections.count()));
            l.last()->setObjectName(OPEN_SELECTED_FILES_ACTION);
            connect(l.last(), &QAction::triggered, this, [=]() {
                qDebug()<<"triggered";
                QStringList dirs;
                QMap<QString, QStringList> fileMap;
                /**step 1: Categorize files according to type.
                 * step 2: Open files in batches to avoid loss of asynchronous messages due to program startup.
                **/
                for (auto uri : m_selections) {
                    auto info = FileInfo::fromUri(uri);
                    if (info->isDir() || info->isVolume()) {
                        dirs<<uri;
                    } else {
                        QString defaultAppName = FileLaunchManager::getDefaultAction(uri)->getAppInfoName();
                        QStringList list;
                        if (fileMap.contains(defaultAppName)) {
                            list = fileMap[defaultAppName];
                            list << uri;
                            fileMap.insert(defaultAppName, list);
                        } else {
                            list << uri;
                            fileMap.insert(defaultAppName, list);
                        }
                    }
                }
                if (!dirs.isEmpty())
                    m_top_window->addNewTabs(dirs);

                if(!fileMap.empty()) {
                    QMap<QString, QStringList>::iterator iter = fileMap.begin();
                    while (iter != fileMap.end())
                    {
                        FileLaunchManager::openAsync(iter.value());
                        iter++;
                    }
                }
            });
        }
    }

    return l;
}

const QList<QAction *> DirectoryViewMenu::constructCreateTemplateActions()
{
    QList<QAction *> l;
    if (!m_is_search && !m_is_favorite && m_selections.isEmpty() && !m_is_filesafe && !m_is_trash && !m_is_label_model) {
        auto createAction = new QAction(tr("New"), this);
        createAction->setObjectName(CREATE_ACTION);
        if (m_is_cd) {
            createAction->setEnabled(false);
        }
        if (m_is_boxpath) {
            createAction->setEnabled(false);
        }
        //related bug#191108, huawei/android phone can paste file success,also can create
//        if (m_is_mtp_ptp) {
//            createAction->setEnabled(false);
//        }
        //fix create folder fail issue in special path
        auto info = FileInfo::fromUri(m_directory);
        if (info.get()->isEmptyInfo()) {
            //FIXME: replace BLOCKING api in ui thread.
            FileInfoJob job(info);
            job.querySync();
        }

        if (!info->canWrite() && !m_is_ftp) {
            createAction->setEnabled(false);
        }

        if(m_top_window->getFilterWorking())
        {
            createAction->setEnabled(false);
        }
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
//                            GIcon *icon = g_app_info_get_icon(app_info);
//                            QString iconName = FileUtils::getIconStringFromGIcon(icon);
//                            if (iconName.startsWith("/")) {
//                                tmpIcon.addFile(iconName);
//                            } else {
//                                tmpIcon = QIcon::fromTheme(iconName);
//                            }
//                            if(!tmpIcon.isNull())
//                                isOnlyUnref = true;
//                        }
//                        l = l->next;
//                    }
//                    g_list_free_full(app_infos, g_object_unref);

                    QAction *action = new QAction(tmpIcon, qinfo.baseName(), this);
                    connect(action, &QAction::triggered, this, [=]() {
                        // automatically check for conficts
                        CreateTemplateOperation op(m_directory, CreateTemplateOperation::Template, t);
                        Peony::FileOperationErrorDialogWarning dlg;
                        connect(&op, &Peony::FileOperation::errored, &dlg, &Peony::FileOperationErrorDialogWarning::handle);
                        op.run();
                        auto target = op.target();
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
            Peony::FileOperationErrorDialogWarning dlg;
            connect(&op, &Peony::FileOperation::errored, &dlg, &Peony::FileOperationErrorDialogWarning::handle);
            op.run();
            auto targetUri = op.target();
            qDebug()<<"target:"<<targetUri;
            m_uris_to_edit<<targetUri;
        });
        auto createFolderActions = new QAction(QIcon::fromTheme("folder-new-symbolic"), tr("Folder"), this);
        actions<<createFolderActions;
        connect(actions.last(), &QAction::triggered, this, [=]() {
            //FileOperationUtils::create(m_directory, nullptr, CreateTemplateOperation::EmptyFolder);
            CreateTemplateOperation op(m_directory, CreateTemplateOperation::EmptyFolder, tr("New Folder"));
            Peony::FileOperationErrorDialogWarning dlg;
            connect(&op, &Peony::FileOperation::errored, &dlg, &Peony::FileOperationErrorDialogWarning::handle);
            op.run();
            auto targetUri = op.target();
            qDebug()<<"target:"<<targetUri;
            m_uris_to_edit<<targetUri;
        });
        subMenu->addActions(actions);
    }

    return l;
}

const QList<QAction *> DirectoryViewMenu::constructViewOpActions()
{
    QList<QAction *> l;

    if (m_selections.isEmpty()) {
        ViewFactorySortFilterModel2 model;
        model.setDirectoryUri(m_directory);
        auto viewNames = model.supportViewIds();
        auto viewFactorysManager = DirectoryViewFactoryManager2::getInstance();

        if (!viewNames.isEmpty()) {
            //view type;
            auto viewTypeAction = addAction(tr("View Type"));
            viewTypeAction->setObjectName(VIEW_TYPE_ACTION);
            l<<viewTypeAction;
            QMenu *viewTypeSubMenu = new QMenu(this);
            for (auto viewId : viewNames) {
                auto viewType = viewTypeSubMenu->addAction(viewFactorysManager->getFactory(viewId)->viewIcon(), viewFactorysManager->getFactory(viewId)->viewName());
                viewType->setData(viewId);
                if (m_view->viewId() == viewId) {
                    viewType->setCheckable(true);
                    viewType->setChecked(true);
                } else {
                    connect(viewType, &QAction::triggered, this, [=]() {
                        m_top_window->beginSwitchView(viewType->data().toString());
                    });
                }
            }
            viewTypeAction->setMenu(viewTypeSubMenu);
        }

        //sort type
        auto sortTypeAction = addAction(tr("Sort By"));
        sortTypeAction->setObjectName(SORT_TYPE_ACTION);
        l<<sortTypeAction;
        QMenu *sortTypeMenu = new QMenu(this);

        QList<QAction *> tmp;
        tmp<<sortTypeMenu->addAction(tr("Name"));
        tmp<<sortTypeMenu->addAction(tr("Modified Date"));
        tmp<<sortTypeMenu->addAction(tr("File Type"));
        tmp<<sortTypeMenu->addAction(tr("File Size"));
        tmp<<sortTypeMenu->addAction(tr("Original Path"));

        if (m_top_window->getCurrentUri() != "trash:///") {
            tmp.last()->setVisible(false);
        }
        int sortType = m_view->getSortType();
        if (sortType >= 0) {
            tmp.at(sortType)->setCheckable(true);
            tmp.at(sortType)->setChecked(true);
        }

        for (int i = 0; i < tmp.count(); i++) {
            connect(tmp.at(i), &QAction::triggered, this, [=]() {
                m_top_window->setCurrentSortColumn(i);
            });
        }

        sortTypeAction->setMenu(sortTypeMenu);

        //fix bug#82685
        if(m_is_computer){
            sortTypeAction->setEnabled(false);
        }

        //sort order
        auto sortOrderAction = addAction(tr("Sort Order"));
        sortOrderAction->setObjectName(SORT_ORDER_ACTION);
        l<<sortOrderAction;
        QMenu *sortOrderMenu = new QMenu(this);
        tmp.clear();
        //fix bug#97408,change indicator meanings
        //箭头向上为升序，向下为降序，与通常的理解对应
        tmp<<sortOrderMenu->addAction(tr("Descending Order"));
        tmp<<sortOrderMenu->addAction(tr("Ascending Order"));
        int sortOrder = m_view->getSortOrder();
        tmp.at(sortOrder)->setCheckable(true);
        tmp.at(sortOrder)->setChecked(true);

        for (int i = 0; i < tmp.count(); i++) {
            connect(tmp.at(i), &QAction::triggered, this, [=]() {
                m_top_window->setCurrentSortOrder(Qt::SortOrder(i));
            });
        }

        sortOrderAction->setMenu(sortOrderMenu);

        auto sortPreferencesAction = addAction(tr("Sort Preferences"));
        sortPreferencesAction->setObjectName(SORT_PREFERENCES_ACTION);
        l<<sortPreferencesAction;

        auto sortPreferencesMenu = new QMenu(this);
        auto folderFirst = sortPreferencesMenu->addAction(tr("Folder First"));
        folderFirst->setCheckable(true);
        folderFirst->setChecked(m_top_window->getWindowSortFolderFirst());
        connect(folderFirst, &QAction::triggered, this, [=](bool checked) {
            m_top_window->setSortFolderFirst(checked);
            //folderFirst->setChecked(checked);
        });

        if (QLocale::system().name().contains("zh")) {
            auto useDefaultNameSortOrder = sortPreferencesMenu->addAction(tr("Chinese First"));
            useDefaultNameSortOrder->setCheckable(true);
            useDefaultNameSortOrder->setChecked(m_top_window->getWindowUseDefaultNameSortOrder());
            connect(useDefaultNameSortOrder, &QAction::triggered, this, [=](bool checked) {
                m_top_window->setUseDefaultNameSortOrder(checked);
                //useDefaultNameSortOrder->setChecked(checked);
            });
        }

        auto showHidden = sortPreferencesMenu->addAction(tr("Show Hidden"));
        showHidden->setCheckable(true);
        showHidden->setChecked(m_top_window->getWindowShowHidden());
        connect(showHidden, &QAction::triggered, this, [=](bool checked) {
            m_top_window->setShowHidden(checked);
            //showHidden->setChecked(m_top_window->getWindowShowHidden());
        });

        sortPreferencesAction->setMenu(sortPreferencesMenu);
    }

    return l;
}

const QList<QAction *> DirectoryViewMenu::constructFileOpActions()
{
    QList<QAction *> l;

    if (!m_is_trash && !m_is_computer) {
        QString homeUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        bool hasStandardPath = FileUtils::containsStandardPath(m_selections);
        //qDebug() << "constructFileOpActions hasStandardPath:" <<hasStandardPath;
        if (!m_selections.isEmpty() && !m_selections.contains(homeUri) && !m_is_recent && !m_is_filesafe) {
            if (!m_is_favorite) {
                //code from lixiang for kydroid menu
                if (m_is_kydroid) {
                    bool hasDir = false;
                    for (auto uri : m_selections) {
                        auto info = FileInfo::fromUri(uri);
                        if (info->isDir()) {
                            hasDir = true;
                            break;
                        }
                    }
                    if (!hasDir) {
                        l<<addAction(QIcon::fromTheme("edit-copy-symbolic"), tr("Copy"));
                        l.last()->setObjectName(COPY_ACTION);
                        connect(l.last(), &QAction::triggered, this, [=]() {
                            ClipboardUtils::setClipboardFiles(m_selections, false);
                        });
                    }
                }
                else {
                    l<<addAction(QIcon::fromTheme("edit-copy-symbolic"), tr("Copy"));
                    l.last()->setObjectName(COPY_ACTION);
                    connect(l.last(), &QAction::triggered, this, [=]() {
                        ClipboardUtils::setClipboardFiles(m_selections, false);
                    });
                }
            }

            //kydroid has no cut,delete,rename option
            if (m_is_kydroid)
                return l;

            if (!hasStandardPath && !m_is_recent && !m_is_favorite && !m_is_filesafe && !m_is_label_model)
            {
                /* 经过讨论，对于搜索路径，如果当前操作目录可写时保留现有逻辑，不可写时和删除共用逻辑（即判断选中的第一个文件或文件夹的权限），这样改动影响比较小 */
                bool canCut = true;
                QString actualDir =  m_directory;
                if(actualDir.startsWith("search://")){
                    actualDir =  FileUtils::getActualDirFromSearchUri(m_directory);
                }
                auto info = FileInfo::fromUri(actualDir);
                if (!info->canWrite()) {
                    canCut = false;
                    if(m_is_search && !m_selections.isEmpty()){
                        auto selectInfo = FileInfo::fromUri(m_selections.first());
                        if(selectInfo->canWrite())
                            canCut = true;
                    }//end
                }
                if (canCut) {
                    l<<addAction(QIcon::fromTheme("edit-cut-symbolic"), tr("Cut"));
                    l.last()->setObjectName(CUT_ACTION);
                    connect(l.last(), &QAction::triggered, this, [=]() {
                        ClipboardUtils::setClipboardFiles(m_selections, true, m_is_search);
                        m_view->repaintView();
                    });
                }
            }

            bool hasDeleteForever = false;
            if (!m_is_recent && !m_is_favorite && !hasStandardPath && !m_is_filesafe && !m_is_label_model) {
                bool canTrash = true;
                bool canDelete = true;

                /* 同一目录下文件夹和文件的info->cantrash()和info->canDelete()通常一样，采用m_selections.first()判断 */
                auto info = FileInfo::fromUri(m_selections.first());
                if (! info->canTrash())
                    canTrash = false;
                if (! info->canDelete() && (!m_selections.first().startsWith("ftp://")))/* 由于gio info 的can_delete=false,hotfix bug#98208【用例 100365】匿名访问ftp服务器，右键没有删除选项 */
                    canDelete = false;
                if(FileUtils::isLongNameFileOfNotDel2Trash(m_selections.first())){/* 在家目录/下载/扩展目录下存放的长文件名文件使用永久删除，link bug#188864 */
                    canTrash = false;
                }//end

                //fix unencrypted box file can delete to trash issue, link to bug#72948
                if (canTrash && ! m_is_filebox_file && !m_is_mobile_file)
                {
                    l<<addAction(QIcon::fromTheme("edit-delete-symbolic"), tr("Delete to trash"));
                    l.last()->setObjectName(TRASH_ACTION);
                    connect(l.last(), &QAction::triggered, this, [=]() {
                        FileOperationUtils::trash(m_selections, true, m_is_search);
                    });
                }
                else if(m_can_delete && canDelete)
                {
                    hasDeleteForever = true;

                    //fix the bug 77131;
                    if(m_is_filebox_file){
                        l<<addAction(QIcon::fromTheme("edit-clear-symbolic"), tr("Delete"));
                        l.last()->setObjectName(DELETE_ACTION);
                    }else if(!m_is_filebox_file || m_is_mobile_file){
                        l<<addAction(QIcon::fromTheme("edit-clear-symbolic"), tr("Delete forever"));
                        l.last()->setObjectName(DELETE_ACTION);
                    }

                    connect(l.last(), &QAction::triggered, this, [=]() {
                        FileOperationUtils::executeRemoveActionWithDialog(m_selections);
                    });
                }
            }

            if ((m_is_favorite || m_is_label_model) && m_can_delete && !m_is_filesafe && !hasDeleteForever ) {
                l<<addAction(QIcon::fromTheme("edit-clear-symbolic"), tr("Delete forever"));
                l.last()->setObjectName(DELETE_ACTION);
                connect(l.last(), &QAction::triggered, this, [=]() {
                    FileOperationUtils::executeRemoveActionWithDialog(m_selections);
                });
            }

            if (m_is_smb_file && m_can_delete && !hasDeleteForever) {
                l<<addAction(QIcon::fromTheme("edit-clear-symbolic"), tr("Delete forever"));
                l.last()->setObjectName(DELETE_ACTION);
                connect(l.last(), &QAction::triggered, this, [=]() {
                    FileOperationUtils::executeRemoveActionWithDialog(m_selections);
                });
            }

            //fix ftp, sftp file can not delete issue, task#188197
            if (m_is_ftp && m_can_delete && !hasDeleteForever) {
                l<<addAction(QIcon::fromTheme("edit-clear-symbolic"), tr("Delete forever"));
                connect(l.last(), &QAction::triggered, [=]() {
                    FileOperationUtils::executeRemoveActionWithDialog(m_selections);
                });
            }

            if (m_selections.count() > 0 && ! hasStandardPath && !m_is_recent && !m_is_favorite && !m_is_filesafe && !m_is_label_model) {
                l<<addAction(QIcon::fromTheme("document-edit-symbolic"), tr("Rename"));
                l.last()->setObjectName(RENAME_ACTION);
                if(m_is_search && m_selections.count() > 1){/* 搜索页面暂时不支持批量重命名,菜单置灰 */
                    l.last()->setEnabled(false);
                }
                connect(l.last(), &QAction::triggered, this, [=]() {
                    if (m_selections.count() == 1) {
                        m_view->editUri(m_selections.first());
                    } else if (m_selections.count() > 1) {
                        m_view->editUris(m_selections);
                    }
                });
            }

        } else {
            if (!m_is_search && !m_is_recent && !m_is_favorite && !m_is_kydroid && !m_is_filesafe && !m_is_cd && !m_is_label_model)
            {
                auto pasteAction = addAction(QIcon::fromTheme("edit-paste-symbolic"), tr("Paste"));
                l<<pasteAction;
                l.last()->setObjectName(PASTE_ACTION);

                //fix bug#183268, not allow paste in mtp, gphoto2 path or can not write path
                auto info = FileInfo::fromUri(m_directory);
                bool isDirectoryCanWrite = true;
                if (!info->isEmptyInfo()) {
                    isDirectoryCanWrite = info->canWrite();
                    if (!isDirectoryCanWrite) {
                        QString fileSystem = info.get()->fileSystemType();
                        if (fileSystem.isEmpty()) {
                            fileSystem = FileUtils::getFsTypeFromFile(info.get()->uri());
                            qDebug() << "file system :" << fileSystem;
                        }
                        if (fileSystem.contains("udf")) {
                            qDebug() << "file system contains:" << fileSystem;
                            isDirectoryCanWrite = true;
                        }
                    }
                }
                //comment to fix bug#191108, huawei phone can paste file success
//                if (m_directory.startsWith("mtp://") || m_directory.startsWith("gphoto2://")){
//                    isDirectoryCanWrite = false;
//                }

                pasteAction->setEnabled(ClipboardUtils::isClipboardHasFiles() && isDirectoryCanWrite);
                connect(l.last(), &QAction::triggered, this, [=]() {
                    auto op = ClipboardUtils::pasteClipboardFiles(m_directory);
                    if (op) {
                        auto window = dynamic_cast<QWidget *>(m_top_window);
                        auto iface = m_top_window;
                        connect(op, &Peony::FileOperation::operationFinished, window, [=](){
                            auto opInfo = op->getOperationInfo();
                            auto targetUirs = opInfo->dests();
                            //fix bug#196528, selection files icon not update issue
                            QTimer::singleShot(300, window, [=](){
                                iface->setCurrentSelectionUris(targetUirs);
                            });
                        }, Qt::BlockingQueuedConnection);
                    }
                    else{
                        //fix paste file in old path not update issue, link to bug#71627
                        m_top_window->getCurrentPage()->getView()->repaintView();
                    }
                });
            }
            else if (m_is_recent && m_selections.count() >0)
            {
                //fix recent files can not be deleted issue
                l<<addAction(QIcon::fromTheme("edit-clear-symbolic"), tr("Delete"));
                l.last()->setObjectName(DELETE_ACTION);
                connect(l.last(), &QAction::triggered, this, [=]() {
                    FileOperationUtils::remove(m_selections);
                });

                l<<addAction(QIcon::fromTheme("edit-copy-symbolic"), tr("Copy"));
                l.last()->setObjectName(COPY_ACTION);
                connect(l.last(), &QAction::triggered, this, [=]() {
                    QStringList selections;
                    for(auto uri:m_selections)
                    {
                        uri = FileUtils::getTargetUri(uri);
                        selections << uri;
                    }
                    ClipboardUtils::setClipboardFiles(selections, false);
                });
            }

            l<<addAction(QIcon::fromTheme("view-refresh-symbolic"), tr("Refresh"));
            l.last()->setObjectName(REFRESH_ACTION);
            connect(l.last(), &QAction::triggered, this, [=]() {
                m_top_window->refresh();
            });
        }
    }

    //select all and reverse select
    if (m_selections.isEmpty())
    {
        l<<addAction(tr("Select All"));
        l.last()->setObjectName(SELECT_ALL_ACTION);
        connect(l.last(), &QAction::triggered, this, [=]() {
            //qDebug() << "select all";
            m_view->invertSelections();
        });
    }
    else if(! m_is_kydroid)
    {
        l<<addAction(tr("Reverse Select"));
        l.last()->setObjectName(REVERSE_SELECT_ACTION);
        connect(l.last(), &QAction::triggered, this, [=]() {
            //qDebug() << "Reverse select";
            m_view->invertSelections();
        });
    }

    return l;
}

const QList<QAction *> DirectoryViewMenu::constructFilePropertiesActions()
{
    QList<QAction *> l;

    //fix select mutiple file in trash path show empty issue
    if (m_selections.count() > 1 && (m_is_trash || m_is_recent))
        return l;

    //favorite is should not show property
    if (m_selections.isEmpty() && m_directory == "favorite:///")
        return l;

    if (! m_is_search) {
        //包含network的情况下，不显示属性选项
        if (m_is_network) {
            return l;
        }

        for (QString uri : m_selections) {
            if (uri.startsWith("network://")) {
                return l;
            }
        }

        l<<addAction(QIcon::fromTheme("preview-file"), tr("Properties"));
        l.last()->setObjectName(PROPERTIES_ACTION);
        connect(l.last(), &QAction::triggered, this, [=]() {
            //FIXME:
            if (m_selections.isEmpty()) {
                QStringList uris;
                uris<<m_directory;
                QMainWindow *p = PropertiesWindowFactoryPluginManager::getInstance()->create(uris);
                //PropertiesWindow *p = new PropertiesWindow(uris);
                if(this->parentWidget() && this->parentWidget()->isModal()){
                    p->setParent(this->parentWidget());
                }
                p->setAttribute(Qt::WA_DeleteOnClose);
                p->show();
            } else {
                QStringList selectUriList;
                if (m_selections.first().contains("favorite:///")) {
                    for (auto uriIndex = 0; uriIndex < m_selections.count(); ++uriIndex) {
                        if (m_selections.at(uriIndex) == "favorite:///?schema=trash"
                        || m_selections.at(uriIndex) == "favorite:///?schema=recent") {
                            QStringList urisList;
                            urisList << FileUtils::getTargetUri(m_selections.at(uriIndex));
                            //PropertiesWindow *p = new PropertiesWindow(urisList);
                            QMainWindow *p = PropertiesWindowFactoryPluginManager::getInstance()->create(urisList);
                            if(this->parentWidget() && this->parentWidget()->isModal()){
                                p->setParent(this->parentWidget());
                            }
                            p->setAttribute(Qt::WA_DeleteOnClose);
                            p->show();
                        } else {
                            selectUriList<< m_selections.at(uriIndex);
                        }
                    }
                }else if(m_selections.first().startsWith("label:///") && m_version != "lingmo3.0"){
                    for(auto &labelUri : m_selections){/* 标记模式页面为不同目录下的文件（夹），所以每个都需要一个属性对话框 */
                        QStringList urisList;
                        urisList.append(labelUri);
                        QMainWindow *p = PropertiesWindowFactoryPluginManager::getInstance()->create(urisList);
                        //PropertiesWindow *p = new PropertiesWindow(urisList);
                        if(this->parentWidget() && this->parentWidget()->isModal()){
                            p->setParent(this->parentWidget());
                        }
                        p->setAttribute(Qt::WA_DeleteOnClose);
                        p->show();
                    }
                }else {
                    selectUriList = m_selections;
                }

                if (selectUriList.count() > 0) {
                    QMainWindow *p = PropertiesWindowFactoryPluginManager::getInstance()->create(selectUriList);
                    //PropertiesWindow *p = new PropertiesWindow(selectUriList);
                    if(this->parentWidget() && this->parentWidget()->isModal()){
                        p->setParent(this->parentWidget());
                    }
                    p->setAttribute(Qt::WA_DeleteOnClose);
                    p->show();
                }
            }
        });
    } else if (m_selections.count() == 1) {
        l<<addAction(QIcon::fromTheme("preview-file"), tr("Properties"));
        l.last()->setObjectName(PROPERTIES_ACTION);
        connect(l.last(), &QAction::triggered, this, [=]() {
            QMainWindow *p = PropertiesWindowFactoryPluginManager::getInstance()->create(m_selections);
            //PropertiesWindow *p = new PropertiesWindow(m_selections);
            if(this->parentWidget() && this->parentWidget()->isModal()){
                p->setParent(this->parentWidget());
            }
            p->setAttribute(Qt::WA_DeleteOnClose);
            p->show();
        });
    }

    return l;
}

const QList<QAction *> DirectoryViewMenu::constructComputerActions()
{
    QList<QAction *> l;
    if (m_is_computer && m_selections.count() == 1) {
        QString uri = m_selections.first();

        auto info = FileInfo::fromUri(uri);
        if (info->displayName().isEmpty() || info->targetUri().isEmpty()) {
            FileInfoJob j(info);
            j.querySync();
        }

        //not allow format data block, fix bug#66471，66479
        QString targetUri = FileUtils::getTargetUri(uri);
        if (uri == "file:///data" || targetUri == "file:///data"
            || (uri.startsWith("file:///media") && uri.endsWith("/data"))
            || (targetUri.startsWith("file:///media") && targetUri.endsWith("/data")))
            return l;

        auto mount = VolumeManager::getMountFromUri(info->targetUri());
        //fix bug#52491, CDROM and DVD can format issue
        if (nullptr != mount) {
            QString unixDevice = FileUtils::getUnixDevice(info->uri());
            if (! unixDevice.isNull() && ! unixDevice.contains("/dev/sr")
                && info->isVolume() && info->canUnmount()) {
                l<<addAction(QIcon::fromTheme("preview-file"), tr("format"), this, [=] () {
                    // FIXME:// refactory Format_Dialog
                    Format_Dialog* fd  = new Format_Dialog(info->uri(), nullptr, m_view);
                    fd->show();
//                    Format_Dialog *fd = format_dlg_create_delegate::getInstance()->createDlg(info->uri(), nullptr);
//                    fd->show();
                });
                l.last()->setObjectName(FORMAT_ACTION);

                if (!mount) {
                    l.last()->setEnabled(false);
                }
            }
        }
    }

    return l;
}

const QList<QAction *> DirectoryViewMenu::constructTrashActions()
{
    QList<QAction *> l;

    if (m_is_trash) {
        bool isTrashEmpty = m_top_window->getCurrentAllFileUris().isEmpty();

        if (m_selections.isEmpty()) {
            l<<addAction(QIcon::fromTheme("edit-clear-symbolic"), tr("&Clean the Trash"));
            l.last()->setObjectName(CLEAN_THE_TRASH_ACTION);
            l.last()->setEnabled(!isTrashEmpty);
            connect(l.last(), &QAction::triggered, this, [=]() {
                auto uris = m_top_window->getCurrentAllFileUris();
//                auto removeop = Peony::FileOperationUtils::clearRecycleBinWithDialog(uris, this->topLevelWidget());
                // fix #161877 【文件管理器】回收站内右键空白处选择情况回收站，确认弹窗不居中
                auto removeop = Peony::FileOperationUtils::clearRecycleBinWithDialog(uris, dynamic_cast<QWidget *>(m_top_window));

                qApp->setProperty("clearTrash",true);
//                    if(removeop){
//                        removeop->connect(removeop,&Peony::FileDeleteOperation::operationFinished, [=](){
//                            Peony::SoundEffect::getInstance()->recycleBinClearMusic();
//                        });
//                    }

//                AudioPlayManager::getInstance()->playWarningAudio();
//                auto result = QMessageBox::question(nullptr, tr("Delete Permanently"), tr("Are you sure that you want to delete these files? "
//                                                                                          "Once you start a deletion, the files deleting will never be "
//                                                                                          "restored again."));
//                if (result == QMessageBox::Yes) {
////                    SoundEffect::getInstance()->recycleBinClearMusic();
//                    auto uris = m_top_window->getCurrentAllFileUris();
//                    FileOperationUtils::remove(uris);
//                }
            });
        } else {
            l<<addAction(tr("Restore"));
            l.last()->setObjectName(RESTORE_ACTION);
            connect(l.last(), &QAction::triggered, this, [=]() {
                if (m_selections.count() == 1) {
                    auto untrashop = FileOperationUtils::restore(m_selections.first());
                    if(untrashop){
                        auto window = dynamic_cast<QWidget *>(m_top_window);
                        untrashop->connect(untrashop,&Peony::FileUntrashOperation::operationFinished,window, [=](){
                            //Peony::SoundEffect::getInstance()->copyOrMoveSucceedMusic();
                            //Task#152997, use sdk play sound
#ifdef LINGMO_SDK_SOUND_EFFECTS
                            kdk::KSoundEffects::playSound(SoundType::OPERATION_FILE);
#endif
                        });
                    }
                } else {
                    auto untrashop = FileOperationUtils::restore(m_selections);
                    if(untrashop){
                        auto window = dynamic_cast<QWidget *>(m_top_window);
                        untrashop->connect(untrashop,&Peony::FileUntrashOperation::operationFinished,window, [=](){
                            //Peony::SoundEffect::getInstance()->copyOrMoveSucceedMusic();
                            //Task#152997, use sdk play sound
#ifdef LINGMO_SDK_SOUND_EFFECTS
                            kdk::KSoundEffects::playSound(SoundType::OPERATION_FILE);
#endif
                        });
                    }
                }
//                qApp->setProperty("restoreFile",true);
            });
            l<<addAction(QIcon::fromTheme("edit-clear-symbolic"), tr("Delete"));
            l.last()->setObjectName(DELETE_ACTION);
            connect(l.last(), &QAction::triggered, this, [=]() {
                AudioPlayManager::getInstance()->playWarningAudio();
                int result = 0;
                QString message = QObject::tr("Are you sure you want to permanently delete this file?"
                                      " Once deletion begins, "
                                      "the file will not be recoverable.");
                if (m_selections.count() > 1) {
                    message = QObject::tr("Are you sure you want to permanently delete these %1 files?"
                                          " Once deletion begins, "
                                          "these file will not be recoverable.").arg(m_selections.count());
                }

                result = QMessageBox::question(nullptr, "", message, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                if (result == QMessageBox::Yes) {
//                    SoundEffect::getInstance()->recycleBinClearMusic();
                    FileOperationUtils::remove(m_selections);
                }
            });
            l<<addAction(QIcon::fromTheme("edit-copy-symbolic"),tr("Copy"));
            l.last()->setObjectName(COPY_ACTION);
            connect(l.last(), &QAction::triggered, this, [=]() {
                ClipboardUtils::setClipboardFiles(m_selections, false);
            });
            l<<addAction(QIcon::fromTheme("edit-cut-symbolic"),tr("Cut"));
            l.last()->setObjectName(CUT_ACTION);
            connect(l.last(), &QAction::triggered, this, [=]() {
                ClipboardUtils::setClipboardFiles(m_selections, true, m_is_search);
                m_view->repaintView();
            });
        }
    } else if (m_is_recent && m_selections.isEmpty()) {
        l<<addAction(tr("Clean All"));
        l.last()->setObjectName(CLEAN_THE_RECENT_ACTION);
        connect(l.last(), &QAction::triggered, this, [=]() {
            RecentVFSManager::getInstance()->clearAll();
        });
    }

    return l;
}

const QList<QAction *> DirectoryViewMenu::constructSearchActions()
{
    QList<QAction *> l;
    if (m_is_search || m_is_recent) {
        if (m_selections.isEmpty())
            return l;

        l<<addAction(QIcon::fromTheme("new-window-symbolc"), tr("Open Parent Folder in New Window"));
        l.last()->setObjectName(OPEN_IN_NEW_WINDOW_ACTION);
        connect(l.last(), &QAction::triggered, this, [=]() {
            for (auto uri : m_selections) {
                if (m_is_recent)
                    uri = FileUtils::getTargetUri(uri);
                auto parentUri = FileUtils::getParentUri(uri);
                //bool exist = FileUtils::isFileExsit(uri);
                bool exist = true;
                if (exist && ! parentUri.isNull()) {
                    auto *windowIface = m_top_window->create(parentUri);
                    auto newWindow = dynamic_cast<QWidget *>(windowIface);
                    auto selection = m_selections;
#if QT_VERSION > QT_VERSION_CHECK(5, 12, 0)
                    QTimer::singleShot(1000, newWindow, [=]() {
                        if (newWindow)
                            windowIface->setCurrentSelectionUris(selection);
                    });
#else
                    QTimer::singleShot(1000, newWindow, [=]() {
                        if (newWindow)
                            windowIface->setCurrentSelectionUris(selection);
                    });
#endif
                    newWindow->show();
                 }
                else
                {
                    QMessageBox::warning(nullptr,
                                         tr("Error"),
                                         tr("File:\"%1\" is not exist, did you moved or deleted it?").arg(QUrl(uri).path()));
                }
            }
        });
    }
    return l;
}

bool DirectoryViewMenu::isMultFile(std::shared_ptr<FileInfo> info)
{
    if (!info) {
        qDebug() << "file info not valid";
        return false;
    }

    QString uri = info->uri();
    if (uri.startsWith("mult:///") && (!info->isDir())) {
        return true;
    }

    return false;
}

void DirectoryViewMenu::isMobileFile(const QString &uri)
{
    if(uri.startsWith("file:///media/")){
        QString unixDevice = FileInfo::fromUri(uri).get()->unixDeviceFile();
        bool canEject = false;
        bool canStop = false;
        FileEnumerator e;
        e.setEnumerateDirectory("computer:///");
        e.enumerateSync();
        for (auto fileInfo : e.getChildren()) {
            QString uriStr = fileInfo.get()->uri();
            GFile* gFile = g_file_new_for_uri(uriStr.toUtf8().constData());
            GError *err = nullptr;
            QString device;
            bool can_eject = false;
            bool can_stop = false;
            QString target_uri;
            GFileInfo* gFileInfo = g_file_query_info(gFile,
                                           "standard::*," "time::*," "access::*," "mountable::*," "metadata::*," "trash::*," G_FILE_ATTRIBUTE_ID_FILE,
                                           G_FILE_QUERY_INFO_NONE,
                                           g_cancellable_new(),
                                           &err);
            if (err) {
                DEBUG<<err->code<<err->message;
                g_error_free(err);
                g_object_unref(gFile);
                g_object_unref(gFileInfo);
                continue;
            }else{
                if(g_file_info_has_attribute(gFileInfo,G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE_FILE)){
                    device = g_file_info_get_attribute_string(gFileInfo,G_FILE_ATTRIBUTE_MOUNTABLE_UNIX_DEVICE_FILE);
                }
                target_uri = g_file_info_get_attribute_string(gFileInfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
                can_eject = g_file_info_get_attribute_boolean(gFileInfo, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_EJECT);
                can_stop = g_file_info_get_attribute_boolean(gFileInfo, G_FILE_ATTRIBUTE_MOUNTABLE_CAN_STOP);
            }
            g_object_unref(gFile);
            g_object_unref(gFileInfo);
            if((device == unixDevice) || (!target_uri.isEmpty() && uri.startsWith(target_uri))){
                canEject = can_eject;
                canStop = can_stop;
                break;
            }
        }

        if(canEject || canStop){
            m_is_mobile_file = true;
        }
        DEBUG<<"unixDevice:"<<unixDevice<<"canEject:"<<canEject<<"canStop"<<canStop<<"m_is_mobile_file"<<m_is_mobile_file;
    }else{
        auto dev = VolumeManager::getDriveFromUri(uri);
        if(dev != nullptr){
            bool canEject = g_drive_can_eject(dev.get()->getGDrive());
            bool canStop = g_drive_can_stop(dev.get()->getGDrive());
            if(canEject || canStop){
                m_is_mobile_file = true;
            }
            qDebug() << "canEject :" << canEject;
        }
    }
}

const QList<QAction *> DirectoryViewMenu::constructMenuPluginActions()
{
    QList<QAction *> l;
    if (!m_is_favorite && !m_is_label_model) {
        auto pluginIds = MenuPluginManager::getInstance()->getPluginIds();
        //sort plugiins by name, so the menu option orders is relatively fixed
        std::sort(pluginIds.begin(), pluginIds.end());

        for (auto id : pluginIds) {
            auto plugin = MenuPluginManager::getInstance()->getPlugin(id);

            if(m_is_filesafe||m_is_filebox_file) {
                if(plugin->name() == tr("Peony-Qt Filesafe Menu Extension") || plugin->name() == tr("Peony File Labels Menu Extension")) {
                    auto actions = plugin->menuActions(MenuPluginInterface::DirectoryView, m_directory, m_selections);
                    l<<actions;
                    for (auto action : actions) {
                        action->setParent(this);
                        action->setObjectName(plugin->name());
                        addAction(action);
                    }
                }
            } else {
                if(plugin->name() != tr("Peony-Qt Filesafe Menu Extension")) {
                    auto a = Peony::FileOperationManager::getInstance()->isFsynchronizing();
                    if(m_is_mobile_file && Peony::FileOperationManager::getInstance()->isFsynchronizing()){
                        /* 往移动设备中进行文件拷贝fysnc时导致io阻塞，插件暂先屏蔽,待后续改进 */
                        //todo
                        DEBUG<<"mobile file is synchronizing";
                    }else{
                        auto actions = plugin->menuActions(MenuPluginInterface::DirectoryView, m_directory, m_selections);
                        l<<actions;
                        for (auto action : actions) {
                            action->setParent(this);
                            action->setObjectName(plugin->name());
                            addAction(action);
                            qDebug()<< id<<"-==================-";
                            if(id == "Peony File Labels Menu Extension" && m_version != "lingmo3.0"){
                                l<<addSeparator();
                            }
                        }
                    }
                }
            }
        }
    }
    return l;
}

const QList<QAction *> DirectoryViewMenu::constructMultiSelectActions()
{
    QList<QAction *> l;
    if (m_version == "lingmo3.0") {
        return l;
    }
    auto MultiSelectAction = addAction(tr("MultiSelect"));
    l<<MultiSelectAction;
    connect(l.last(), &QAction::triggered, [=]() {
        Peony::DirectoryViewHelper::globalInstance()->getViewIface2ByDirectoryViewWidget(m_view)->doMultiSelect(true);
    });
    return l;

}

void DirectoryViewMenu::keyPressEvent(QKeyEvent *e)
{
    QMenu::keyPressEvent(e);
    if(e->key() == Qt::Key_F10 && e->modifiers() == Qt::ShiftModifier) {
        close();
    }
}
