/*
 * KWin Style LINGMO
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#include "debug.h"
#include "kyfiledialog.h"
#include <QDebug>
#include "uikyfiledialog.h"
#include "kyfiledialogprivate.h"
#include "sidebar.h"
#include "pathbar.h"
#include "lingmostylehelper/lingmostylehelper.h"
#include <explorer-qt/controls/directory-view/directory-view-container.h>
#include <explorer-qt/controls/menu/directory-view-menu/directory-view-menu.h>
#include <explorer-qt/controls/directory-view/directory-view-widget.h>
#include <explorer-qt/controls/directory-view/directory-view-factory/directory-view-factory-manager.h>
#include <explorer-qt/file-operation-utils.h>
#include <QDesktopServices>
#include <QUrl>
#include <PeonyFileInfo>
#include <PeonyFileItemModel>
#include <QDebug>
#include <QMimeDatabase>
#include <QMimeType>
#include <KWindowEffects>
#include <QPainter>
#include <QRect>
#include <QVector4D>
#include <QRegExp>
#include <QMessageBox>
#include <QScrollBar>
#include <QCoreApplication>
#include <QTranslator>
#include <qwindow.h>
#include <QScreen>
#include <QDesktopWidget>
#include <QPainterPath>
#include <QDBusInterface>
#include <QDBusMessage>
#include <qpa/qplatformdialoghelper.h>
#include "../qt5-lingmo-platformtheme/xatom-helper.h"
#include <explorer-qt/file-utils.h>
#include <explorer-qt/create-template-operation.h>
#include <explorer-qt/global-settings.h>
#include <explorer-qt/search-vfs-uri-parser.h>
#include <explorer-qt/file-meta-info.h>
#include <explorer-qt/clipboard-utils.h>
#include "debug.h"
#include "settings/lingmo-style-settings.h"

using namespace LINGMOFileDialog;

KyNativeFileDialog::KyNativeFileDialog(QWidget *parent)
    : QDialog(parent),
      d_ptr(new KyNativeFileDialogPrivate)
{
    QString locale = QLocale::system().name();

    if(qApp->property("Language").isValid())
    {
        QString language = qApp->property("Language").toString();
        QFile file("/usr/share/qt5-lingmo-platformtheme/qt5-lingmo-filedialog_" + language + ".qm");
        if(file.exists())
            locale = language;
    }
    installTranslate(locale);

    mKyFileDialogUi = new Ui_KyFileDialog(this);

//    setStyle(nullptr);

//    m_fileSystemModel = new QFileSystemModel();

    connect(d_ptr.get()->m_timer, &QTimer::timeout, this, [&](){
//        pDebug << "timeout isActive:..........." << d_ptr.get()->m_timer->isActive();
        this->show();
        if(d_ptr.get()->m_timer->isActive()){
            pDebug << "timer stop....";
            d_ptr.get()->m_timer->stop();
        }
    });

    mKyFileDialogUi->setupUi(this);
    d_ptr.get()->m_container = mKyFileDialogUi->m_container;

//    QString path = "file://" + QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
//    pDebug << "initialDirPath:" << path << selectedUrls();
//    goToUri(path, false);
    getCurrentPage()->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mKyFileDialogUi->m_sider, &FileDialogSideBar::goToUriRequest, this, [this](const QString &uri, bool addToHistory, bool forceUpdate){
        pDebug << "goToUri0000";
        if (uri.startsWith("favorite://")) {
            QString u = Peony::FileUtils::getTargetUri(uri);
            goToUri(u, addToHistory, forceUpdate);

        }
        else
            goToUri(uri, addToHistory, forceUpdate);
    });

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::directoryChanged, this, &KyNativeFileDialog::updateWindowState);

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::directoryChanged, this, [=](){
        pDebug << "directoryChanged........" << getCurrentUri();

        Q_EMIT directoryEntered(QUrl(getCurrentUri()));
        pDebug << "locationChangeEnd....";
        QCursor c;
        c.setShape(Qt::ArrowCursor);
        this->setCursor(c);
        mKyFileDialogUi->m_sider->setCursor(c);

        if(m_fileDialogHelper->isViewInitialFinished())
            intiContainerSort();

//        mKyFileDialogUi->m_fileNameEdit->setText("");
        setShortCuts();
    });

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::directoryChanged, this, [=](){
        updateSearchProgressBar();
    });
    connect(getCurrentPage(), &Peony::DirectoryViewContainer::viewDoubleClicked, this, [=](const QString &uri){
            auto info = Peony::FileInfo::fromUri(uri);
            pDebug << "viewDoubleClicked....." << uri << info->isDir() << info->isVolume();
            if (info->isDir() || info->isVolume()) {
                pDebug << "goToUrixxxxxxx....";
                goToUri(uri);
            } else {
                onAcceptButtonClicked();
            }
        });

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::selectionChanged, this, &KyNativeFileDialog::updateStatusBar);

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::menuRequest, this, &KyNativeFileDialog::containerMenuRequest);

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::signal_itemAdded, this, [=](const QString& uri){
        pDebug << "signal_itemAdded....." << m_uris_to_edit << uri;
        if(m_uris_to_edit.isEmpty())
            return;
        QString editUri = Peony::FileUtils::urlDecode(m_uris_to_edit.first());
        QString infoUri = Peony::FileUtils::urlDecode(uri);
        if (editUri == infoUri ) {
            getCurrentPage()->getView()->scrollToSelection(uri);
            getCurrentPage()->getView()->editUri(uri);
        }
        m_uris_to_edit.clear();
    });

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::updateWindowLocationRequest, this, [=](const QString &uri, bool addToHistory, bool forceUpdate){
        pDebug << "page updateWindowLocationRequest.....uri:" << uri << getCurrentUri() << forceUpdate;
        if(uri == "")
            return;
        QString s = uri;
        QString s1 = s.endsWith("/") ? s.remove(s.length() - 1, 1) : s;
        QString s2 = getCurrentUri();
        QString s3 = s2.endsWith("/") ? s2.remove(s2.length() - 1, 1) : s2;
        if(s1 != s3 && m_fileDialogHelper->isViewInitialFinished())
        {
            pDebug << "s1:" << s1 << "s3:" << s3;
            goToUri(uri, addToHistory);
        }
    });

    connect(mKyFileDialogUi->m_pathbar, &Peony::AdvancedLocationBar::updateWindowLocationRequest,
            this, [this](const QString &uri, bool addHistory, bool forceUpdate){
        pDebug << "goToUrijjjjj....";
        goToUri(uri, addHistory, forceUpdate);
    });

    connect(mKyFileDialogUi->m_pathbar, &Peony::AdvancedLocationBar::refreshRequest, this, [](){

    });
    connect(mKyFileDialogUi->m_pathbar, &Peony::AdvancedLocationBar::searchRequest, this, [=](const QString &path, const QString &key){
        pDebug << "path.....:" << path << "key:" << key << m_last_no_SearchPath;
        if(key == "")
        {
            forceStopLoading();
            pDebug << "key is null m_last_no_SearchPath:" << m_last_no_SearchPath;
            m_isClearSearchKey = true;
//            m_needSearch = false;
            goToUri(m_last_no_SearchPath, true);

//            QCursor c;
//            c.setShape(Qt::ArrowCursor);
//            this->setCursor(c);
//            getCurrentPage()->getView()->setCursor(c);
//            mKyFileDialogUi->m_sider->setCursor(c);

        }
        else
        {
            auto targetUri = Peony::SearchVFSUriParser::parseSearchKey(path, key, true, false, "", true);
            pDebug << "updateSearch targetUri:" <<targetUri;
            pDebug << "updateSearch path:" <<path;
//            goToUri(targetUri, true);
            bool isSearchEngine = true;
            const QByteArray id(LINGMO_SEARCH_SCHEMAS);
            if (QGSettings::isSchemaInstalled(id)) {
                QGSettings *searchSettings = new QGSettings(id, QByteArray(), this);
                if (!searchSettings || !searchSettings->keys().contains(SEARCH_METHOD_KEY)) {
                    isSearchEngine = false;
                }
            } else {
                isSearchEngine = false;
            }

//            auto targetUri = Peony::SearchVFSUriParser::parseSearchKey(path, key, true, false, "", true);
            targetUri = Peony::SearchVFSUriParser::addSearchKey(targetUri, isSearchEngine);
            pDebug << "updateSearch targetUri:" <<targetUri;
            pDebug << "updateSearch path:" <<path;
            m_needSearch = true;
            goToUri(targetUri, true);

            QCursor c;
            c.setShape(Qt::BusyCursor);
            this->setCursor(c);
            if(getCurrentPage() && getCurrentPage()->getView())
                getCurrentPage()->getView()->setCursor(c);
            mKyFileDialogUi->m_sider->setCursor(c);
            mKyFileDialogUi->setCursor(c);

        }
    });

    mKyFileDialogUi->m_backButton->setToolTip(tr("Go Back"));
    mKyFileDialogUi->m_backButton->setIcon(QIcon::fromTheme("go-previous-symbolic"));
    mKyFileDialogUi->m_backButton->setAutoRaise(true);
    mKyFileDialogUi->m_backButton->setEnabled(false);
    connect(mKyFileDialogUi->m_backButton, &QToolButton::clicked, this, &KyNativeFileDialog::goBack);


    mKyFileDialogUi->m_forwardButton->setToolTip(tr("Go Forward"));
    mKyFileDialogUi->m_forwardButton->setIcon(QIcon::fromTheme("go-next-symbolic"));
    mKyFileDialogUi->m_forwardButton->setAutoRaise(true);
    mKyFileDialogUi->m_forwardButton->setEnabled(false);
    connect(mKyFileDialogUi->m_forwardButton, &QToolButton::clicked, this, &KyNativeFileDialog::goForward);

    mKyFileDialogUi->m_toParentButton->setToolTip(tr("Cd Up"));
    mKyFileDialogUi->m_toParentButton->setIcon(QIcon::fromTheme("go-up-symbolic"));
    mKyFileDialogUi->m_toParentButton->setAutoRaise(true);
    mKyFileDialogUi->m_toParentButton->setEnabled(false);
    connect(mKyFileDialogUi->m_toParentButton, &QToolButton::clicked, this,  &KyNativeFileDialog::goToParent);

    mKyFileDialogUi->m_searchBtn->setIcon(QIcon::fromTheme("edit-find-symbolic"));
    mKyFileDialogUi->m_searchBtn->setToolTip(tr("Search"));
    mKyFileDialogUi->m_searchBtn->setIconSize(QSize(16, 16));
    mKyFileDialogUi->m_searchBtn->setAutoRaise(true);

    mKyFileDialogUi->m_searchBtn->setProperty("useIconHighlightEffect", true);
    mKyFileDialogUi->m_searchBtn->setProperty("iconHighlightEffectMode", 1);
    connect(mKyFileDialogUi->m_searchBtn, &QToolButton::clicked, this, &KyNativeFileDialog::searchButtonClicked);

    mKyFileDialogUi->m_modeButton->setToolTip(tr("View Type"));
    mKyFileDialogUi->m_modeButton->setIcon(QIcon::fromTheme("view-grid-symbolic"));
    mKyFileDialogUi->m_modeButton->setProperty("isWindowButton", 1);
    mKyFileDialogUi->m_modeButton->setProperty("useIconHighlightEffect", 0x2);
    mKyFileDialogUi->m_modeButton->setAutoRaise(true);

    mKyFileDialogUi->m_useGlobalSortAction->setChecked(Peony::GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool());
    connect(mKyFileDialogUi->m_useGlobalSortAction, &QAction::triggered, this, [=](bool checked){
        Peony::GlobalSettings::getInstance()->setValue(USE_GLOBAL_DEFAULT_SORTING, checked);
    });

    mKyFileDialogUi->m_sortButton->setToolTip(tr("Sort Type"));
    mKyFileDialogUi->m_sortButton->setProperty("isWindowButton", 1);
    mKyFileDialogUi->m_sortButton->setProperty("useIconHighlightEffect", 0x2);
    mKyFileDialogUi->m_sortButton->setAutoRaise(true);

    mKyFileDialogUi->m_maximizeAndRestore->setToolTip(tr("Maximize"));
    mKyFileDialogUi->m_maximizeAndRestore->setIcon(QIcon::fromTheme("window-close-symbolic"));
    mKyFileDialogUi->m_maximizeAndRestore->setAutoRaise(true);
    mKyFileDialogUi->m_maximizeAndRestore->setProperty("isWindowButton", 1);
    mKyFileDialogUi->m_maximizeAndRestore->setProperty("useIconHighlightEffect", 0x2);
    connect(mKyFileDialogUi->m_maximizeAndRestore, &QToolButton::clicked, this, [=]() {
        if (!this->isMaximized()) {
            this->showMaximized();
        } else {
            this->showNormal();
        }
        updateMaximizeState();
    });

    mKyFileDialogUi->m_closeButton->setToolTip(tr("Close"));
    mKyFileDialogUi->m_closeButton->setIcon(QIcon::fromTheme("window-close-symbolic"));
    mKyFileDialogUi->m_closeButton->setAutoRaise(true);
    mKyFileDialogUi->m_closeButton->setProperty("isWindowButton", 0x2);

    connect(mKyFileDialogUi->m_modeMenu, &QMenu::triggered, this, [=](QAction *action) {
        if (action == mKyFileDialogUi->m_listModeAction) {
            this->getCurrentPage()->switchViewType("List View");
        } else {
            this->getCurrentPage()->switchViewType("Icon View");
        }
    });

    connect(mKyFileDialogUi->m_acceptButton, &QPushButton::clicked, this, &KyNativeFileDialog::onAcceptButtonClicked);

    connect(mKyFileDialogUi->m_rejectButton, &QPushButton::clicked, this, &KyNativeFileDialog::onRejectButtonClicked);

    connect(mKyFileDialogUi->m_fileTypeCombo,
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),
            this, [this](const QString s){
        pDebug << "activated..." << s;
        selectNameFilter(s);});

    connect(mKyFileDialogUi->m_fileTypeCombo,
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::activated),
            this, &KyNativeFileDialog::selectedNameFilterChanged);

    connect(mKyFileDialogUi->m_fileTypeCombo,
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, [this](const QString s){
        pDebug << "currentIndexChanged..." << s;
        selectNameFilter(s);});

    connect(mKyFileDialogUi->m_closeButton, &QToolButton::clicked, this, &KyNativeFileDialog::onRejectButtonClicked);

    connect(mKyFileDialogUi->m_fileNameEdit, &QLineEdit::textChanged, this,  &KyNativeFileDialog::lineEditTextChange);

    connect(getCurrentPage(), &Peony::DirectoryViewContainer::viewTypeChanged, this, &KyNativeFileDialog::onSwitchView);

    connect(mKyFileDialogUi->m_sortOrderGroup, &QActionGroup::triggered, this, [=](QAction *action) {
        int index = mKyFileDialogUi->m_sortOrderGroup->actions().indexOf(action);
        getCurrentPage()->setSortOrder(Qt::SortOrder(index));
        setSortType();
    });

    connect(mKyFileDialogUi->m_sortTypeGroup, &QActionGroup::triggered, this, [=](QAction *action) {
        int index = mKyFileDialogUi->m_sortTypeGroup->actions().indexOf(action);
        getCurrentPage()->setSortType(Peony::FileItemModel::ColumnType(index));
    });

    connect(mKyFileDialogUi->m_sortMenu, &QMenu::aboutToShow, this, [=]() {
        bool originPathVisible = getCurrentUri()== "trash:///";
        mKyFileDialogUi->m_originalPath->setVisible(originPathVisible);
        mKyFileDialogUi->m_sortTypeGroup->actions().at(getCurrentSortColumn())->setChecked(true);
        mKyFileDialogUi->m_sortOrderGroup->actions().at(getCurrentSortOrder())->setChecked(true);
    });

    foreach (QAction *action, mKyFileDialogUi->m_sortTypeGroup->actions())
    {
        bool checked = (int(getCurrentPage()->getSortType()) ==
                        mKyFileDialogUi->m_sortTypeGroup->actions().indexOf(action) ? true : false);
        action->setChecked(checked);
    }

    refreshContainerSort();
    updateMaximizeState();
    onSwitchView();
    isTableModel();

    m_model = new QStringListModel(this);
    m_completer = new QCompleter(mKyFileDialogUi->m_fileNameEdit);
    m_completer->setModel(m_model);
    m_completer->setMaxVisibleItems(10);
    m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
//    m_completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    mKyFileDialogUi->m_fileNameEdit->setCompleter(m_completer);

    int number = QApplication::desktop()->screenNumber(QCursor::pos());
    if(number<0){
        number=0;
    }
    QSize size = QGuiApplication::screens().at(number)->geometry().size();
    pDebug << "setmaxsize:" << size;
    this->setMaximumSize(size);
}

KyNativeFileDialog::~KyNativeFileDialog()
{
    pDebug << "~~~~~~~~KyNativeFileDialog";

}

void KyNativeFileDialog::updateMaximizeState()
{
    bool maximized = this->isMaximized();
    if (maximized) {
        mKyFileDialogUi->m_maximizeAndRestore->setToolTip(tr("Restore"));
        mKyFileDialogUi->m_maximizeAndRestore->setIcon(QIcon::fromTheme("window-restore-symbolic"));
    } else {
        mKyFileDialogUi->m_maximizeAndRestore->setToolTip(tr("Maximize"));
        mKyFileDialogUi->m_maximizeAndRestore->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    }
}

Peony::FMWindowIface *KyNativeFileDialog::create(const QString &uri)
{
    auto dialog = new KyNativeFileDialog();

    return dialog;
}

Peony::FMWindowIface *KyNativeFileDialog::create(const QStringList &uris)
{
    auto dialog = new KyNativeFileDialog();

    return dialog;
}

const QStringList KyNativeFileDialog::getCurrentSelections()
{
    if(containerView())
        return containerView()->getSelections();
    else
        return QStringList();
}

const QStringList KyNativeFileDialog::getCurrentSelectionsList()
{
    QStringList list;
    foreach (QString str, getCurrentSelections()) {
        list.append(Peony::FileUtils::urlDecode(str));
    }
    return list;
}

const QStringList KyNativeFileDialog::getCurrentAllFileUris()
{
    if(containerView())
        return containerView()->getAllFileUris();
    return QStringList();
}
const QList<std::shared_ptr<Peony::FileInfo>> KyNativeFileDialog::getCurrentSelectionFileInfos()
{
    const QStringList uris = getCurrentSelections();
    QList<std::shared_ptr<Peony::FileInfo>> infos;
    for(auto uri : uris) {
        auto info = Peony::FileInfo::fromUri(uri);
        infos<<info;
    }
    return infos;
}

void KyNativeFileDialog::setCurrentSelections(QStringList selections)
{
    QStringList list;
    foreach (QString str, selections) {
        list.append(Peony::FileUtils::urlEncode(str));
    }
    if(containerView()){
        containerView()->setSelections(list);
        pDebug << "get setCurrentSelections....:" << containerView()->getSelections();
    }
}

Qt::SortOrder KyNativeFileDialog::getCurrentSortOrder()
{
    return getCurrentPage()->getSortOrder();
}

int KyNativeFileDialog::getCurrentSortColumn()
{
    return getCurrentPage()->getSortType();
}

bool KyNativeFileDialog::getWindowShowHidden()
{
    auto settings = Peony::GlobalSettings::getInstance();
    if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        return settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool();
    } else {
        auto uri = getCurrentUri();
        auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            return metaInfo->getMetaInfoVariant(SHOW_HIDDEN_PREFERENCE).isValid()? metaInfo->getMetaInfoVariant(SHOW_HIDDEN_PREFERENCE).toBool(): (settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool());
        } else {
            pDebug<<"can not get file meta info"<<uri;
            return settings->getValue(SHOW_HIDDEN_PREFERENCE).toBool();
        }
    }
}
bool KyNativeFileDialog::getWindowUseDefaultNameSortOrder()
{
    auto settings = Peony::GlobalSettings::getInstance();
    if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        return settings->getValue(SORT_CHINESE_FIRST).isValid()? settings->getValue(SORT_CHINESE_FIRST).toBool(): true;
    } else {
        auto uri = getCurrentUri();
        auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            return metaInfo->getMetaInfoVariant(SORT_CHINESE_FIRST).isValid()? metaInfo->getMetaInfoVariant(SORT_CHINESE_FIRST).toBool(): (settings->getValue(SORT_CHINESE_FIRST).isValid()? settings->getValue(SORT_CHINESE_FIRST).toBool(): true);
        } else {
            pDebug<<"can not get file meta info"<<uri;
            return settings->getValue(SORT_CHINESE_FIRST).isValid()? settings->getValue(SORT_CHINESE_FIRST).toBool(): true;
        }
    }
}
bool KyNativeFileDialog::getWindowSortFolderFirst()
{
    auto settings = Peony::GlobalSettings::getInstance();
    if (settings->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
        return settings->getValue(SORT_FOLDER_FIRST).isValid()? settings->getValue(SORT_FOLDER_FIRST).toBool(): true;
    } else {
        auto uri = getCurrentUri();
        auto metaInfo = Peony::FileMetaInfo::fromUri(uri);
        if (metaInfo) {
            return metaInfo->getMetaInfoVariant(SORT_FOLDER_FIRST).isValid()? metaInfo->getMetaInfoVariant(SORT_FOLDER_FIRST).toBool(): (settings->getValue(SORT_FOLDER_FIRST).isValid()? settings->getValue(SORT_FOLDER_FIRST).toBool(): true);
        } else {
            pDebug<<"can not get file meta info"<<uri;
            return settings->getValue(SORT_FOLDER_FIRST).isValid()? settings->getValue(SORT_FOLDER_FIRST).toBool(): true;
        }
    }
}

void KyNativeFileDialog::refresh()
{
    getCurrentPage()->refresh();
}

void KyNativeFileDialog::forceStopLoading()
{
    getCurrentPage()->stopLoading();

    //Key_escape also use as cancel
    if (Peony::ClipboardUtils::isClipboardHasFiles())
    {
        Peony::ClipboardUtils::clearClipboard();
        if(containerView())
            containerView()->repaintView();
    }
}

void KyNativeFileDialog::setShowHidden(bool showHidden)
{
    getCurrentPage()->setShowHidden(showHidden);
}

void KyNativeFileDialog::setUseDefaultNameSortOrder(bool use)
{
    if (!getCurrentPage()) {
        return;
    }
    getCurrentPage()->setUseDefaultNameSortOrder(use);
}

void KyNativeFileDialog::setSortFolderFirst(bool set)
{
    if (!getCurrentPage()) {
        return;
    }
    getCurrentPage()->setSortFolderFirst(set);
}

void KyNativeFileDialog::setCurrentSelectionUris(const QStringList &uris)
{
    getCurrentPage()->stopLoading();

    //Key_escape also use as cancel
    if (Peony::ClipboardUtils::isClipboardHasFiles())
    {
        Peony::ClipboardUtils::clearClipboard();
        if(containerView())
            containerView()->repaintView();
    }
}

void KyNativeFileDialog::setCurrentSortOrder (Qt::SortOrder order)
{
    getCurrentPage()->setSortOrder(order);
}

void KyNativeFileDialog::setCurrentSortColumn (int sortColumn)
{
    getCurrentPage()->setSortType(Peony::FileItemModel::ColumnType(sortColumn));
}

void KyNativeFileDialog::editUri(const QString &uri)
{
    if(containerView())
        containerView()->editUri(uri);
}

void KyNativeFileDialog::editUris(const QStringList &uris)
{
    if(containerView())
        containerView()->editUris(uris);
}

bool KyNativeFileDialog::getFilterWorking()
{
    return false;
}

void KyNativeFileDialog::beginSwitchView(const QString &viewId)
{
    if (getCurrentUri() == "computer:///")
        return;
    auto selection = getCurrentSelections();
    if(!getCurrentPage()||!(getCurrentPage()->getView()))
        return;

    if (getCurrentPage()->getView()->viewId() == viewId)
        return;

    getCurrentPage()->switchViewType(viewId);

    // change default view id
    auto factoryManager = Peony::DirectoryViewFactoryManager2::getInstance();
    auto internalViews = factoryManager->internalViews();
    if (internalViews.contains(viewId))
        Peony::GlobalSettings::getInstance()->setValue(DEFAULT_VIEW_ID, viewId);

    setCurrentSelections(selection);
    if (selection.count() >0)
        getCurrentPage()->getView()->scrollToSelection(selection.first());

}


/**
 * @brief 当前目录
 */
const QString KyNativeFileDialog::getCurrentUri()
{
    return Peony::FileUtils::urlDecode(getCurrentPage()->getCurrentUri());
}

/**
 * @param directory 路径
 * @brief 设置路径为directory
 */
void KyNativeFileDialog::setDirectory(const QString &directory)
{
    pDebug << "setDirectoryoooo...:" << directory;
    setDirectoryUrl(QUrl(directory));
}

void KyNativeFileDialog::setDirectory(const QDir &directory)
{
    setDirectoryUrl(QUrl::fromLocalFile(directory.absolutePath()));
}

QDir KyNativeFileDialog::directory() const
{
    return QDir(directoryUrl().toLocalFile());
}

void KyNativeFileDialog::setDirectoryUrl(const QUrl &directory)
{
    pDebug << "setDirectoryUrl.....:" << directory;
    if (!getCurrentPage()) {
        return;
    }
    goToUri(directory.toString(), true);
}

QUrl KyNativeFileDialog::directoryUrl() const
{
    if (!getCurrentPage()) {
        return QUrl(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    } else {
        return QUrl(Peony::FileUtils::urlDecode(getCurrentPage()->getCurrentUri()));
    }
}

void KyNativeFileDialog::goToUri(const QString &uri, bool addToHistory, bool forceUpdate)
{
    pDebug << "goToUri....,,,,:" << uri << getCurrentUri();

    if(containerView() && m_fileDialogHelper){
        pDebug << "connect............";
        connect(containerView(), &Peony::DirectoryViewWidget::viewDirectoryChanged,
                m_fileDialogHelper, &KyFileDialogHelper::viewInitialFinished, Qt::UniqueConnection);
    }

    if(uri.isEmpty() || uri == getCurrentUri())
        return;
    QString nUri = uri, nCurrentUri = getCurrentUri();
    pDebug << "nUri000..." << nUri << "nCurrentUri...." << nCurrentUri;
    if(nUri.endsWith("/")){
        if((nUri.length() >= 2 && nUri[nUri.length() - 2] != "/") || nUri.length() <2)
            nUri = nUri.remove(nUri.length() - 1, 1);
    }
    if(nCurrentUri.endsWith("/")){
        if((nCurrentUri.length() >= 2 && nCurrentUri[nCurrentUri.length() - 2] != "/") || nCurrentUri.length() <2)
            nCurrentUri = nCurrentUri.remove(nCurrentUri.length() - 1, 1);
    }
    pDebug << "nUri..." << nUri << "nCurrentUri...." << nCurrentUri;
    if(nUri == nCurrentUri)   {
        return;
    }
    pDebug << "getCurrentUri....,,,,:" << getCurrentUri();
    pDebug << "isShow......:" << (m_fileDialogHelper == nullptr) <<  m_fileDialogHelper->isShow() << isInitialGoToUriNum;
    if(true)//(isInitialGoToUriNum || (m_fileDialogHelper != nullptr && m_fileDialogHelper->isShow()))
    {
        isInitialGoToUriNum = false;
        getCurrentPage()->stopLoading();
        if(!m_fileDialogHelper->isShow())
            addToHistory = false;
        pDebug << "getCurrentUri....,,,,:" << m_fileDialogHelper->isShow() << getCurrentUri();
        pDebug << "gotouri123123:" << uri << addToHistory << forceUpdate;
        if(uri.startsWith("search:///search_uris="))
            getCurrentPage()->goToUri(uri, addToHistory, forceUpdate);
        else
        getCurrentPage()->goToUri(Peony::FileUtils::urlEncode(uri), addToHistory, forceUpdate);
    }
}

/**
 * @param filename文件名
 * @brief 选择指定文件
 */
void KyNativeFileDialog::selectFile(const QString &filename)
{
    QUrl url = getCurrentUri();
    pDebug << "selectFileoooo..........:" << filename;
//    if(!QFile::exists(filename.remove(0,7)))
//    {
//        pDebug << "selectFile not exist:" << filename.remove(0,7);
//        return;
//    }
    QString path = filename;

    if(url.toString() != path)
    {
        if(path.startsWith("/"))
            path = ("file://" + path);

        pDebug << "select filename path:" << path;

        QUrl pathUrl(path);
        pDebug << "select pathUrl:" << pathUrl.path();

        QDir dir(pathUrl.path());

        if(!path.endsWith("/"))
            dir.cdUp();
        setDirectoryUrl(QUrl("file://" + dir.path()));
        pDebug << "url2222:" << dir.path();
    }
    pDebug << "select url....";
    QList<QUrl> urls;
    if(path.startsWith("/"))
        path = "file://" + path;
    pDebug << "filename:...." << path;
    urls.append(QUrl(path));
    pDebug << "setInitiallySelectedFiles......" << path;
    m_fileDialogHelper->options()->setInitiallySelectedFiles(urls);
//    QDir dir(url.path());
//    url.setPath(dir.absoluteFilePath(filename));
//    pDebug << "selectFile url:" << url << dir.absoluteFilePath(filename) << filename;
    selectUrl(QUrl(path));
}

QStringList KyNativeFileDialog::selectedFiles() const
{
    QStringList list;
    for (const QUrl &url : selectedUrls()) {
        QUrl fileUrl(url);
        pDebug << "selectedFiles fileUrl.....:" << fileUrl.toString() << fileUrl.path();
//        if(!fileUrl.toString().startsWith("recent://") && !fileUrl.toString().startsWith("trash://"))
//            list << fileUrl.toLocalFile();
//        else{
//            list << Peony::FileUtils::getEncodedUri(fileUrl.toString());
//        }
        list << Peony::FileUtils::getEncodedUri(fileUrl.toString());
    }
    pDebug << "selectedFiles...." << list;
    return list;
}

void KyNativeFileDialog::selectUrl(const QUrl &url)
{
    return;
    Q_D(KyNativeFileDialog);
    pDebug << "selectUrlkkkkk........url.path:" << url.path() << isDir(url.path());
    if(d->fileMode != QFileDialog::DirectoryOnly && d->fileMode != QFileDialog::Directory && isDir(url.path()))
        setCurrentInputName(QFileInfo(url.path()).fileName());
}

QList<QUrl> KyNativeFileDialog::selectedUrls() const
{
    pDebug << "selectedUrls........:";
    if (!getCurrentPage()) {
        return QList<QUrl>();
    }
    pDebug << "selectedUrls........:" << getCurrentPage()->getCurrentSelections();
    QStringList urlStrList = getCurrentPage()->getCurrentSelections();
    QList<QUrl> urls;
    for (auto uri : urlStrList) {
        pDebug << "selectedUrls urlDecode....:" << Peony::FileUtils::urlDecode(uri) << QUrl(Peony::FileUtils::urlDecode(uri)) << Peony::FileUtils::getTargetUri(uri);
        if (uri.startsWith("trash://") || uri.startsWith("recent://")
                || uri.startsWith("computer://") || uri.startsWith("favorite://")
                || uri.startsWith("filesafe://"))
                    urls << Peony::FileUtils::getTargetUri(uri);
        else{
//            pDebug << "1123456789...." << Peony::FileUtils::getUriBaseName(uri) << Peony::FileUtils::getFileDisplayName(uri) <<
//                      Peony::FileUtils::toDisplayUris(QStringList() << uri) << Peony::FileUtils::getParentUri(uri) << Peony::FileUtils::getOriginalUri(uri);
//            pDebug << "4567545678...." << QUrl(Peony::FileUtils::urlDecode(uri)).path() << QUrl(Peony::FileUtils::urlDecode(uri)).toDisplayString() <<
//                       QUrl(Peony::FileUtils::urlDecode(uri)).toLocalFile();
//            pDebug << "4567545678...." << QUrl(uri).path() << QUrl(uri).toDisplayString() <<
//                       QUrl(uri).toLocalFile();
            uri = uri.remove(0, 7);
            pDebug << "target uri...." << Peony::FileUtils::getTargetUri(uri) << Peony::FileUtils::urlDecode(uri);
            urls << QUrl::fromLocalFile(Peony::FileUtils::urlDecode(uri));//Peony::FileUtils::urlDecode(uri);
        }
        pDebug << "selectedUrls uri test......." << uri << urls;
    }
    pDebug << "selectedUrls..." << urls;
    return urls;
}

QStringList qt_strip_filters(const QStringList &filters)
{
    QStringList strippedFilters;
    QRegExp r(QString::fromLatin1("^(.*)\\(([^()]*)\\)$"));
    const int numFilters = filters.count();
    strippedFilters.reserve(numFilters);
    for (int i = 0; i < numFilters; ++i) {
        QString filterName;
        int index = r.indexIn(filters[i]);
        if (index >= 0)
            filterName = r.cap(1);
        strippedFilters.append(filterName.simplified());
    }
    return strippedFilters;
}

void KyNativeFileDialog::setComBoxItems(const QStringList &filters)
{
    mKyFileDialogUi->m_fileTypeCombo->clear();
    mKyFileDialogUi->m_fileTypeCombo->addItems(filters);
}

void KyNativeFileDialog::setNameFilters(const QStringList &filters)
{
    Q_D(KyNativeFileDialog);
    d->nameFilters = filters;
    pDebug << "setNameFilters filters..... :" << filters;
    m_nameFilterMap.clear();

    QStringList l;
    if (testOption(QFileDialog::HideNameFilterDetails) && (d->fileMode != QFileDialog::DirectoryOnly && d->fileMode != QFileDialog::Directory)) {
        l = qt_strip_filters(filters);
        pDebug << "HideNameFilterDetails..." << l;
        setComBoxItems(l);
    } else {
        l = filters;
        setComBoxItems(filters);
    }
    for(int i = 0; i < filters.length(); i++)
        m_nameFilterMap.insert(i, l[i]);

    int index = mKyFileDialogUi->m_fileTypeCombo->currentIndex();
    pDebug << "setNameFilters index.....:" << index;
    selectNameFilterCurrentIndex(index);
}

QStringList KyNativeFileDialog::nameFilters() const
{
    Q_D( const KyNativeFileDialog);
    return d->nameFilters;
}

void KyNativeFileDialog::selectNameFilter(const QString &filter)
{
    Q_D( const KyNativeFileDialog);

    pDebug << "selectNameFilter.,,,,,....." << filter;
    QString key = filter;
    int index = 0;
    if(!mKyFileDialogUi->m_fileTypeCombo->findText(key)){
        if (testOption(QFileDialog::HideNameFilterDetails) && (d->fileMode != QFileDialog::DirectoryOnly && d->fileMode != QFileDialog::Directory)) {
            QStringList l = qt_strip_filters(QStringList(filter));
            pDebug << "llllllllllll" << l << l.length();
            if(l.length() > 0)
                key = l.first();
        } else {
            key = filter;
        }
    }
    pDebug << "key:" << key;
    if(!key.isEmpty())
        index = mKyFileDialogUi->m_fileTypeCombo->findText(key);
    if(index < 0)
        return;
    pDebug << "index:" << index;

    selectNameFilterByIndex(index);
    selectNameFilterCurrentIndex(index);
    Q_EMIT filterSelected(filter);
}


QString KyNativeFileDialog::selectedNameFilter() const
{
    Q_D(const KyNativeFileDialog);
    const QComboBox *box = mKyFileDialogUi->m_fileTypeCombo;
    return box ? d->nameFilters.value(box->currentIndex()) : QString();
}

void KyNativeFileDialog::selectNameFilterByIndex(int index)
{
    Q_D(KyNativeFileDialog);
    if (index < 0 || index >= d->nameFilters.length() || !getCurrentPage()) {
        return;
    }
    mKyFileDialogUi->m_fileTypeCombo->setCurrentIndex(index);
    QStringList nameFilters = d->nameFilters;
    if (index == nameFilters.size()) {
        nameFilters.append(d->nameFilters[d->nameFilters.length() - 1]);
        setNameFilters(nameFilters);
    }
    QString nameFilter = nameFilters.at(index);
    QStringList newNameFilters = QPlatformFileDialogHelper::cleanFilterList(nameFilter);
    pDebug << "selectNameFilterByIndex00000000000";
    if (/*d->acceptMode == QFileDialog::AcceptSave &&*/ !newNameFilters.isEmpty() && (d->fileMode != QFileDialog::DirectoryOnly && d->fileMode != QFileDialog::Directory)) {
        QMimeDatabase db;
        QString text = copyEditText();//mKyFileDialogUi->m_fileNameEdit->text();
        pDebug << "selectNameFilterByIndex text...." << text;
        pDebug << "selectNameFilterByIndex newNameFilters...." << newNameFilters;
        QStringList list = text.split(".");
        pDebug << "selectNameFilterByIndex11111111 list" << list;
        if(list.length() > 1)
        {
            if("." + list[list.length() - 1] != newNameFilters[0])
            {
                int m = list[list.length() - 1].length();
                int n = text.length() - m;
                pDebug << "mmmmmmmmmm..." << m << "   nnnnnnnn.... " << n;
                QString s = text.remove(n, m);
                pDebug << "s000000....." << s;
                QStringList filters = newNameFilters[0].split(".");
                s = s + filters[filters.length() - 1];
                pDebug << "s11111111....." << s;
                mKyFileDialogUi->m_fileNameEdit->setText(s);
            }
        }else{

        }
    }
    if ((d->fileMode == QFileDialog::DirectoryOnly || d->fileMode == QFileDialog::Directory) && QStringList("/") != newNameFilters) {
         newNameFilters = QStringList("/");
    }
}

void KyNativeFileDialog::selectNameFilterCurrentIndex(int index)
{
    Q_D(KyNativeFileDialog);

    if(index < 0 || index >= d->nameFilters.length())
        return;
    QString nameFilter = d->nameFilters[index];
    pDebug << "selectNameFilterCurrentIndex nameFilter..." << nameFilter;

    if(nameFilter.isEmpty())
        return;
    QStringList filterList = QPlatformFileDialogHelper::cleanFilterList(nameFilter);

    pDebug << "selectNameFilterCurrentIndex filterList:" << filterList;
    QStringList mimeTypeFilterList;

    pDebug << "selectNameFilterCurrentIndex filemode.....:" << fileMode();
    if(fileMode() == QFileDialog::Directory || fileMode() == QFileDialog::DirectoryOnly)
    {
        getCurrentPage()->addFileDialogFiltersCondition(mimeTypeFilterList, filterList, QDir::Dirs, Qt::CaseInsensitive);
    }
    else
        getCurrentPage()->addFileDialogFiltersCondition(mimeTypeFilterList, filterList, filter(), Qt::CaseInsensitive);
    if(m_fileDialogHelper->isViewInitialFinished()){
        refreshContainerSort();
        refreshCompleter();
    }
}

int KyNativeFileDialog::selectNameFilterIndex() const
{
    const QComboBox *box = mKyFileDialogUi->m_fileTypeCombo;
    return box ? box->currentIndex() : -1;
}

QDir::Filters KyNativeFileDialog::filter()
{
    Q_D(KyNativeFileDialog);
    return d->filters;
}

void KyNativeFileDialog::setFilter(QDir::Filters filters)
{
    Q_D(KyNativeFileDialog);
    d->filters = filters;
}

void KyNativeFileDialog::setAcceptMode(QFileDialog::AcceptMode mode)
{
    Q_D(KyNativeFileDialog);
    d->acceptMode = mode;
    updateAcceptButtonState();
    if (mode == QFileDialog::AcceptOpen) {
        mKyFileDialogUi->m_fileNameLabel->setText(tr("Name"));//文件名
        mKyFileDialogUi->m_fileNameLabel->setAlignment(Qt::AlignCenter);
        mKyFileDialogUi->m_newFolderButton->hide();
        mKyFileDialogUi->m_acceptButton->setText(tr("Open"));
        mKyFileDialogUi->m_rejectButton->setText(tr("Cancel"));
        connect(mKyFileDialogUi->m_fileNameEdit , &QLineEdit::textChanged, this, &KyNativeFileDialog::onCurrentInputNameChanged);

    } else {
        mKyFileDialogUi->m_fileNameLabel->setText(tr("Save as"));
        mKyFileDialogUi->m_fileNameLabel->setAlignment(Qt::AlignCenter);
        mKyFileDialogUi->m_newFolderButton->setText(tr("New Folder"));
        mKyFileDialogUi->m_newFolderButton->show();
        mKyFileDialogUi->m_acceptButton->setText(tr("Save"));
        mKyFileDialogUi->m_rejectButton->setText(tr("Cancel"));
        mKyFileDialogUi->m_acceptButton->setDefault(true);
        connect(mKyFileDialogUi->m_fileNameEdit, &QLineEdit::textChanged, this, &KyNativeFileDialog::onCurrentInputNameChanged);
        connect(mKyFileDialogUi->m_newFolderButton, &QPushButton::clicked, this, &KyNativeFileDialog::onNewFolder);
    }
}

QFileDialog::AcceptMode KyNativeFileDialog::acceptMode() const
{
    Q_D(const KyNativeFileDialog);
    return d->acceptMode;
}

void KyNativeFileDialog::setFileMode(QFileDialog::FileMode mode)
{
    Q_D(KyNativeFileDialog);
//    if (d->fileMode == QFileDialog::DirectoryOnly || d->fileMode == QFileDialog::Directory) {
//        mKyFileDialog->setNameFilters(QStringList());
//    }
    if(mode == QFileDialog::ExistingFile || mode == QFileDialog::AnyFile ||
       mode == QFileDialog::DirectoryOnly)
    {
        pDebug << "setFileMode000................" << mode;

        getCurrentPage()->setSelectionMode(QAbstractItemView::SingleSelection);
    }
    else
    {
        pDebug << "setFileMode00011111................" << mode;

        getCurrentPage()->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }

    if (mode == QFileDialog::DirectoryOnly || mode == QFileDialog::Directory) {
        pDebug << "m_fileDialogHelper options setNameFilters....";
        m_fileDialogHelper->options()->setNameFilters(QStringList() << tr("Directories"));
        setNameFilters(QStringList() << tr("Directories"));
        mKyFileDialogUi->m_fileTypeCombo->clear();
        mKyFileDialogUi->m_fileTypeCombo->addItem(tr("Directories"));
        mKyFileDialogUi->m_fileTypeCombo->setEnabled(false);
    }

    d->fileMode = mode;
    updateAcceptButtonState();
}

QFileDialog::FileMode KyNativeFileDialog::fileMode()
{
    Q_D(KyNativeFileDialog);
    return d->fileMode;
}

void KyNativeFileDialog::setViewMode(ViewMode mode)
{
    if(containerView()){
        pDebug << "setViewMode...." << mode << containerView()->viewId();
        if(mode == ViewMode::List && containerView()->viewId() != "List View")
        {
            this->getCurrentPage()->switchViewType("List View");
        }
        else if(mode == ViewMode::Icon && containerView()->viewId() != "Icon View")
            this->getCurrentPage()->switchViewType("Icon View");
        onSwitchView();
    }
}

KyNativeFileDialog::ViewMode KyNativeFileDialog::viewMode() const
{
//    this->getCurrentPage()->switchViewType("List View");

    if(containerView() && containerView()->viewId() == "List View")
        return ViewMode::List;
    return  ViewMode::Icon;
}

void KyNativeFileDialog::setLabelText(QFileDialog::DialogLabel label, const QString &text)
{
    switch (static_cast<int>(label)) {
    case QFileDialog::Accept:
        mKyFileDialogUi->m_acceptButton->setText(text);
        break;
    case QFileDialog::Reject:
        mKyFileDialogUi->m_rejectButton->setText(text);
    default:
        break;
    }
}

QString KyNativeFileDialog::labelText(QFileDialog::DialogLabel label) const
{
    switch (static_cast<int>(label)) {
    case QFileDialog::Accept:
        return mKyFileDialogUi->m_acceptButton->text();
    case QFileDialog::Reject:
        return mKyFileDialogUi->m_rejectButton->text();
    default:
        break;
    }

    return QString();
}

void KyNativeFileDialog::setOptions(QFileDialog::Options options)
{
    Q_D(KyNativeFileDialog);
    d->options = options;
    if (options.testFlag(QFileDialog::ShowDirsOnly)) {
        d->m_container->setFilterLabelConditions(selectedNameFilter());
    }
}

void KyNativeFileDialog::setOption(QFileDialog::Option option, bool on)
{
    Q_D(KyNativeFileDialog);
    QFileDialog::Options options = d->options;
    if (on) {
        options |= option;
    } else {
        options &= ~option;
    }
    setOptions(options);
}

bool KyNativeFileDialog::testOption(QFileDialog::Option option) const
{
    Q_D(const KyNativeFileDialog);
    return m_fileDialogHelper->options()->testOption(static_cast<QFileDialogOptions::FileDialogOption>(option));
}

QFileDialog::Options KyNativeFileDialog::options() const
{
    Q_D(const KyNativeFileDialog);
    return d->options;
}

void KyNativeFileDialog::setCurrentInputName(const QString &name)
{
    pDebug << "setCurrentInputName.........:" << name;
    m_copyEditText = name;
//    mKyFileDialogUi->m_fileNameEdit->setText(name);
    QMimeDatabase db;
    const QString &suffix = db.suffixForFileName(name);
    if (suffix.isEmpty()) {
        mKyFileDialogUi->m_fileNameEdit->selectAll();
    } else {
        mKyFileDialogUi->m_fileNameEdit->setSelection(0, name.length() - suffix.length() - 1);
    }
}

void KyNativeFileDialog::onAcceptButtonClicked()
{
    Q_D(KyNativeFileDialog);
    if (!getCurrentPage()) {
        return;
    }
    pDebug << "onAcceptButtonClicked:" << d->acceptMode << d->fileMode;
    pDebug << "onAcceptButtonClicked000 sfiles..........:" << getCurrentSelectionsList();
    pDebug << "onAcceptButtonClicked111 sfiles..........:" << selectedFiles();

    if(d->fileMode == QFileDialog::ExistingFile || d->fileMode == QFileDialog::ExistingFiles || d->fileMode == QFileDialog::AnyFile)
    {
        QList<QString> sFiles = selectedFiles();
        for (int i = 0; i < sFiles.length(); ++i) {
            if(isDir(sFiles[i]))
            {
                pDebug << "gotouri666666" << "file://" + sFiles[i];
                goToUri(sFiles[i], true);
                return;
            }
        }
    }

    if(copyEditText() != "")
    {
        lineEditTextChange(copyEditText());
    }
    pDebug << "onAcceptButtonClicked directoryUrl:" << directoryUrl() << directory();
    QList<QString> sFiles = selectedFiles();
    pDebug << "onAcceptButtonClicked sfiles..........:" << sFiles;
//        pDebug << "getSelections........:" << containerView()->getSelections();
//        pDebug << "uri:" << Peony::FileUtils::getTargetUri(getCurrentUri());
//        pDebug << "uri1111111111:" << Peony::FileUtils::getParentUri(getCurrentUri());
    if(!m_searchMode)
    {
        pDebug << "onAcceptButtonClicked getCurrentUri........:" << getCurrentUri();
        if(!Peony::FileUtils::isFileExsit(getCurrentUri())){
            pDebug << directory().path() << "getCurrentUri directory not exists" << getCurrentUri();
        return;
        }
    }
    else
    {
        foreach (QString str, sFiles) {
                if(!Peony::FileUtils::isFileExsit(str))//
                {
                    pDebug << "FileUtils path,,,:" << str << "not exists!";
                    return;
                }
        }
    }

    if (d->acceptMode == QFileDialog::AcceptSave)
    {
        if(!doSave(sFiles))
        {
            pDebug << "da save faile!";
            return;
        }
        else
            pDebug << "da save success!";

    }
    else if(d->acceptMode == QFileDialog::AcceptOpen)
    {
        if(!doOpen(sFiles))
        {
            pDebug << "da open faile!";
            return;
        }
        else
            pDebug << "da open success!";
    }
    Q_EMIT m_fileDialogHelper->accept();
}

bool KyNativeFileDialog::saveMessageBox(QString name)
{
    if(QMessageBox::warning(this, tr("Warning"), name + " " + tr("exist, are you sure replace?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        return true;
    }
    return false;
}

bool KyNativeFileDialog::checkSaveFileExsits(QString path)
{
    bool exists = false;
    if(fileMode() == QFileDialog::Directory || fileMode() == QFileDialog::DirectoryOnly)
    {
        QDir dir(path);
        exists = dir.exists();
    }
    else{
        QFile file(path);
        exists = file.exists();
    }
    pDebug << "checkSaveFileExsits path...." << path << exists;
    return exists;
}

bool KyNativeFileDialog::doSave(QStringList sFiles)
{
    Q_D(KyNativeFileDialog);

    QString file_name = copyEditText();
    bool suffixCheck = false;
    QStringList nameFilters = d->nameFilters;
    for (QString nameFilterList : nameFilters) {
        for (QString nameFilter : QPlatformFileDialogHelper::cleanFilterList(nameFilterList)) {
            pDebug << "nameFilter:" << nameFilter << "nameFilterList:" << nameFilterList;
            if(nameFilter == "*")
                break;
            QRegExp re(nameFilter, Qt::CaseInsensitive, QRegExp::Wildcard);
            if (re.exactMatch(file_name)) {
                pDebug << "file_name:" << file_name;
                suffixCheck = true;
            };
        }
        if (suffixCheck) {
            break;
        }
    }
    pDebug << "suffixCheck:" << suffixCheck << "nameFilters:" << nameFilters;
    if (!suffixCheck && !nameFilters.isEmpty()) {
        QMimeDatabase mdb;
        if(mKyFileDialogUi->m_fileTypeCombo->currentIndex() < 0 ||
                mKyFileDialogUi->m_fileTypeCombo->currentIndex() >= nameFilters.length())
            return false;
        QString nameFilter = nameFilters[mKyFileDialogUi->m_fileTypeCombo->currentIndex()];
        QStringList newNameFilters = QPlatformFileDialogHelper::cleanFilterList(nameFilter);
        if (!newNameFilters.isEmpty()) {
            if(m_fileDialogHelper->options()->defaultSuffix() == ""){

                for (const QString &filter : newNameFilters) {
                    QString suffix = mdb.suffixForFileName(filter);
                    pDebug << "suffix....." << suffix;
                    if (suffix.isEmpty()) {
                        QRegExp  regExp(filter.mid(2), Qt::CaseInsensitive, QRegExp::Wildcard);
                        mdb.allMimeTypes().first().suffixes().first();
                        for (QMimeType m : mdb.allMimeTypes()) {
                            for (QString suffixe : m.suffixes()) {
                                if (regExp.exactMatch(suffixe)) {
                                    suffix = suffixe;
                                    break;
                                }
                            }
                            if (!suffix.isEmpty()) {
                                break;
                            }
                        }

                    }

                    pDebug << "suffix:" << suffix;
                    if (!suffix.isEmpty()) {
                        QString oldName = file_name;
                        oldName.append('.' + suffix);
                        setCurrentInputName(oldName);
                        pDebug << "path///:" << directory().path() + "/" + oldName;
                        if(checkSaveFileExsits(directory().path() + "/" + oldName)){
                            if(!saveMessageBox(oldName))
                                return false;
                            break;
                        }
                        if(filter == newNameFilters[newNameFilters.length() - 1])
                        {
                            QString oldName = file_name;
                            oldName.append('.' +  mdb.suffixForFileName(newNameFilters[0]));
                            if(checkSaveFileExsits(directory().path() + "/" + oldName)){
                                if(!saveMessageBox(oldName))
                                    return false;
                                break;
                            }
                            setCurrentInputName(oldName);
                            return true;
                        }
                    }
                }
            }
            else{
                file_name.append('.' + m_fileDialogHelper->options()->defaultSuffix());
                if(checkSaveFileExsits(directory().path() + "/" + file_name)){
                    if(!saveMessageBox(file_name))
                        return false;
                }
                setCurrentInputName(file_name);
                pDebug << "path///:" << directory().path() + "/" + file_name;
            }
        }
    }
    else if(suffixCheck)
    {
        pDebug << "suffixCheck.....:" << suffixCheck << "path///:" << directory().path() + "/" + file_name;
        if(checkSaveFileExsits(directory().path() + "/" + file_name)){
            if(!saveMessageBox(file_name))
                return false;
        }
    }
    return true;
}

bool KyNativeFileDialog::doOpen(QStringList sFiles)
{
    Q_D(KyNativeFileDialog);

//    if(d->fileMode != QFileDialog::AnyFile && sFiles.length() == 0)
//        return false;
//    if(d->fileMode == QFileDialog::DirectoryOnly && sFiles.length() > 1)
//        return false;
    pDebug << "doOpen fileMode:" << d->fileMode;
    if(d->fileMode == QFileDialog::ExistingFile || d->fileMode == QFileDialog::ExistingFiles)
    {
        int isfiles = 0;
        int isDirctory = 0;
        foreach (QString files, sFiles)
        {
            if(isDir(files))
            {
                isDirctory++;
            }
            else
                isfiles++;
        }
        pDebug << "isfiles:" << isfiles << "isDirctory:" << isDirctory;
        if((isfiles > 0 && isDirctory > 0) || (isDirctory > 1) || (isfiles <= 0 && isDirctory <= 0))
            return false;
        else{
            pDebug << "open setSelected...";
            Q_EMIT filesSelected(selectedUrls());
        }
    }
    else if(d->fileMode == QFileDialog::AnyFile)
    {
        if(sFiles.length() > 0)
        {
            Q_EMIT filesSelected(selectedUrls());
        }
        else if(copyEditText() != "")
        {
            QList<QUrl> urlList;
            pDebug << "directory path:" << directory().path() << directoryUrl().path();
            urlList.append(QUrl(directory().path() + "/" + copyEditText()));
            Q_EMIT filesSelected(urlList);
        }
        else
            return false;
    }
    else if(d->fileMode == QFileDialog::DirectoryOnly || d->fileMode == QFileDialog::Directory)
    {
        if(copyEditText() != "")
        {
            if(sFiles.isEmpty() || (!sFiles.isEmpty() && !isDir(sFiles[0])))
            {
                return false;
            }
            else
                Q_EMIT filesSelected(selectedUrls());
        }
        else
        {
            if (!getCurrentPage()) {
                return false;
            }
            QList<QUrl> urls;
            QString uri = getCurrentPage()->getCurrentUri();
            pDebug << "current uri........:" << uri;

            if (uri.startsWith("trash://") || uri.startsWith("recent://")
                    || uri.startsWith("computer://") || uri.startsWith("favorite://")
                    || uri.startsWith("filesafe://"))
            {
                urls << Peony::FileUtils::getTargetUri(uri);
            }
            else
            {
                urls << QUrl::fromLocalFile(Peony::FileUtils::urlDecode(uri));
            }
            if(containerView())
                pDebug << "select uri....:" << containerView()->getSelections();

            Q_EMIT filesSelected(urls);
        }
    }
    pDebug << "onAcceptButtonClicked selectedfiles:" << sFiles;

    return true;
}


void KyNativeFileDialog::onRejectButtonClicked()
{
   Q_EMIT m_fileDialogHelper->reject();
}

bool KyNativeFileDialog::isDir(QString path)
{   
    auto fileSymLinkInfo = Peony::FileInfo::fromUri(path);
    if(fileSymLinkInfo->isSymbolLink()) {
        path = fileSymLinkInfo->symlinkTarget();
    }
    else
        path = Peony::FileUtils::urlEncode(path);

    QFile file(path);
    QFileInfo fileInfo(file);
    pDebug << "isDir path1111:" << path << fileInfo.isDir() << Peony::FileUtils::isFileDirectory(path) <<  Peony::FileUtils::getFileIsFolder(path);;
    if(fileInfo.isDir())
        return true;

    pDebug << "isDir path2222222..:" << path << Peony::FileUtils::isFileDirectory(path) <<  Peony::FileUtils::getFileIsFolder(path);
    return Peony::FileUtils::isFileDirectory(path);;
}

void KyNativeFileDialog::goBack()
{
    pDebug << "goback...." << getCurrentPage()->canGoBack();
    getCurrentPage()->goBack();
}

void KyNativeFileDialog::goForward()
{
    pDebug << "goForward...." << getCurrentPage()->canGoForward();
    getCurrentPage()->goForward();
}

void KyNativeFileDialog::goToParent()
{
    pDebug << "goToParent...." << getCurrentPage()->canCdUp();
    getCurrentPage()->cdUp();

}

void KyNativeFileDialog::setHelper(KyFileDialogHelper* helepr)
{
    if(helepr != nullptr)
        m_fileDialogHelper = helepr;
}

void KyNativeFileDialog::onSwitchView()
{
    pDebug << "onSwitchView containerView:" << (containerView() == nullptr);
    if(containerView() != nullptr)
    {
        QString viewId = containerView()->viewId();
        pDebug << "onSwitchView:" << viewId;
        if (viewId == "List View") {
            mKyFileDialogUi->m_modeButton->setIcon(QIcon::fromTheme("view-list-symbolic"));
            this->mKyFileDialogUi->m_listModeAction->setChecked(true);
            this->mKyFileDialogUi->m_iconModeAction->setChecked(false);
        } else {
            mKyFileDialogUi->m_modeButton->setIcon(QIcon::fromTheme("view-grid-symbolic"));
            this->mKyFileDialogUi->m_listModeAction->setChecked(false);
            this->mKyFileDialogUi->m_iconModeAction->setChecked(true);
        }
    }
}

void KyNativeFileDialog::updateWindowState()
{
    Q_D(KyNativeFileDialog);
    pDebug << "updateWindowState.............." << getCurrentUri() << directory() << directoryUrl() << mKyFileDialogUi->m_fileNameEdit->text() << copyEditText();
    //mKyFileDialogUi->m_fileNameEdit->setText("");
    pDebug << "updateWindowState m_searchMode.............." <<m_searchMode << getCurrentUri() << m_isClearSearchKey;

//    if(m_searchMode && !getCurrentUri().startsWith("search:///") && !m_isClearSearchKey)
//    {
//        m_isClearSearchKey = false;
//        searchButtonClicked();
//    }

    refreshCompleter();
//    const QStringList list = getCurrentPage()->getAllFileUris();
//    pDebug << "getAllFileUris:" << list;
//    pDebug << "getselectUris:" << getCurrentSelectionsList();

//    m_CurrentPathAllFiles.clear();

//    foreach (QString str, list) {
//        str = Peony::FileUtils::urlDecode(str);
//        m_CurrentPathAllFiles.append(str.split("/").last());
//    }

//    pDebug << "updateWindowState 1111111111111";
//    QStringListModel *model = (QStringListModel *)(m_completer->model());
//    model->setStringList(m_CurrentPathAllFiles);
//    pDebug << "updateWindowState 2222222222222";

    auto uri = getCurrentUri();
    pDebug << "updateWindowState uri updatePath....." << uri << getCurrentPage()->canGoBack();
    mKyFileDialogUi->m_pathbarWidget->updatePath(uri);
    mKyFileDialogUi->m_backButton->setEnabled(getCurrentPage()->canGoBack());
    mKyFileDialogUi->m_forwardButton->setEnabled(getCurrentPage()->canGoForward());
    mKyFileDialogUi->m_toParentButton->setEnabled(getCurrentPage()->canCdUp());
    updateStatusBar();
    pDebug << "updateWindowState 44444";
    lineEditTextChange(copyEditText());
    pDebug << "updateWindowState 55555555555555555555" << uri;

}

QString KyNativeFileDialog::selectName()
{
    QStringList selectList = getCurrentPage()->getCurrentSelections();
    QString nameStr = "";
    for(int i = 0; i < selectList.length(); i++)
    {
        pDebug << "selectName ....i:" << i << Peony::FileUtils::getFileDisplayName(selectList[i]) << isDir(selectList[i]);
        if(acceptMode() == QFileDialog::AcceptSave && isDir(selectList[i])){
            continue;
        }
        if(acceptMode() == QFileDialog::AcceptOpen && (fileMode() == QFileDialog::ExistingFile ||
           fileMode() == QFileDialog::ExistingFiles || fileMode() == QFileDialog::AnyFile) && isDir(selectList[i])){
            continue;
        }
        QString str = Peony::FileUtils::getFileDisplayName(selectList[i]);//Peony::FileUtils::urlDecode(selectList[i]);
        QString name = "";
        if(selectList.length() == 1)
            name += str.split("/").last();
        else if(i < (selectList.length() -1))
            name += "\"" + str.split("/").last() + "\" ";
        else
            name += "\"" + str.split("/").last() + "\"";
        nameStr += name;
    }

    if(acceptMode() == QFileDialog::AcceptSave){
        for(int i = 0; i < selectList.length(); i++) {
            if(isDir(selectList[i])) {
                mKyFileDialogUi->m_acceptButton->setText(tr("Open"));
                return nameStr;
            }
        }
        mKyFileDialogUi->m_acceptButton->setText(tr("Save"));
    }
    return nameStr;
}

void KyNativeFileDialog::updateStatusBar()
{
    if(getCurrentSelectionsList().length() > 0)
    {
        foreach (QString str, getCurrentSelections()) {
            pDebug << "updateStatusBar str" << str;
            Q_EMIT currentChanged(QUrl(QUrl(str).path()));
        }
    }
    updateAcceptButtonState();
    QString nameStr = selectName();
    if(nameStr != "" && !mKyFileDialogUi->m_fileNameEdit->hasFocus())
        mKyFileDialogUi->m_fileNameEdit->setText(nameStr);
    pDebug << "updateStatusBar............" << mKyFileDialogUi->m_fileNameEdit->hasFocus() << getCurrentUri();
}

void KyNativeFileDialog::containerMenuRequest(const QPoint &pos)
{
    pDebug << "menuRequest....00000" << pos << QCursor::pos() << (this->getCurrentPage() == nullptr);
    if(this->getCurrentPage())
        pDebug << "menuRequest....1111111" << (this->getCurrentPage()->getView() == nullptr);
    Peony::DirectoryViewMenu menu(this);
    QStringList hideActionsName;
    hideActionsName.append(OPEN_IN_NEW_WINDOW_ACTION);
    hideActionsName.append(OPEN_IN_NEW_TAB_ACTION);
    pDebug << "hideActionsName...." << hideActionsName;
    menu.setHiddenActionsByObjectName(hideActionsName);
    pDebug << "menuRequest....11111";
    menu.exec(QCursor::pos());
    m_uris_to_edit = menu.urisToEdit();
}

void KyNativeFileDialog::delayShow()
{
    Q_D(const KyNativeFileDialog);
    //QTBUG48248 借鉴kde plasma-integration里用的QTimer
    pDebug << "delayShow start..........." << getCurrentUri();
    d->m_timer->start();
}

void KyNativeFileDialog::discardDelayedShow()
{
    Q_D(const KyNativeFileDialog);
    pDebug << "discardDelayedShow stop...........";
    d->m_timer->stop();
}


void KyNativeFileDialog::resizeEvent(QResizeEvent *e)
{
    pDebug << "resizeEvent....." << this->width() << mKyFileDialogUi->m_siderWidget->width() << mKyFileDialogUi->m_container->width();
    QDialog::resizeEvent(e);
}

void KyNativeFileDialog::paintEvent(QPaintEvent *e)
{
    KWindowEffects::enableBlurBehind(this->winId(), true);

    QColor color = this->palette().base().color();
    QColor colorBase = QColor(this->palette().base().color());

    int R1 = color.red();
    int G1 = color.green();
    int B1 = color.blue();
    qreal a1 = 0.3;

    int R2 = colorBase.red();
    int G2 = colorBase.green();
    int B2 = colorBase.blue();
    qreal a2 = 1;

    qreal a = 1 - (1 - a1)*(1 - a2);

    qreal R = (a1*R1 + (1 - a1)*a2*R2) / a;
    qreal G = (a1*G1 + (1 - a1)*a2*G2) / a;
    qreal B = (a1*B1 + (1 - a1)*a2*B2) / a;

    colorBase.setRed(R);
    colorBase.setGreen(G);
    colorBase.setBlue(B);
    auto sidebarOpacity = Peony::GlobalSettings::getInstance()->getValue(SIDEBAR_BG_OPACITY).toInt();

    if(sidebarOpacity != 100)
        mKyFileDialogUi->m_sider->viewport()->setAttribute(Qt::WA_TranslucentBackground);

    colorBase.setAlphaF(sidebarOpacity/100.0);

    QPainterPath sidebarPath;
    //sidebarPath.setFillRule(Qt::FillRule::WindingFill);
    QRect sideBarRect = QRect(mKyFileDialogUi->m_sider->mapTo(this, QPoint()), mKyFileDialogUi->m_sider->size());
    auto adjustedRect = sideBarRect.adjusted(0, 0, 0, 0);
    int radius = 6;
    if(this && this->property("normalRadius").isValid())
        radius = this->property("normalRadius").toInt();
    sidebarPath.addRoundedRect(adjustedRect, radius, radius);

//    sidebarPath.addRect(adjustedRect.adjusted(0, 0, 0, -6));
//    sidebarPath.addRect(adjustedRect.adjusted(6, 0, 0, 0));
    QPainter p(this);
    QPainterPath contentPath;


    if (LINGMOStyleSettings::isSchemaInstalled("org.lingmo.style")) {
        auto settings = LINGMOStyleSettings::globalInstance();
        if(settings->keys().contains("windowRadius"))
            radius = settings->get("windowRadius").toInt();
        else if(this && this->property("maxRadius").isValid())
            radius = this->property("maxRadius").toInt();
    }
    else if(this && this->property("maxRadius").isValid())
        radius = this->property("maxRadius").toInt();

//    qDebug() << "KyNativeFileDialog radius........" << radius;
    contentPath.addRoundedRect(this->rect(), radius, radius);
    auto bgPath = contentPath - sidebarPath;
    p.fillPath(bgPath, color);
    p.fillPath(sidebarPath, colorBase);

    QPalette palette = mKyFileDialogUi->m_sider->viewport()->palette();

    palette.setColor(QPalette::Base, colorBase);//window AlternateBase Base NoRole
//    pDebug << "viewport color:" << palette.color(QPalette::AlternateBase) <<
//              palette.color(QPalette::Window) << palette.color(QPalette::Base)
//               << palette.color(QPalette::NoRole) << palette.color(QPalette::Shadow);

    mKyFileDialogUi->m_sider->viewport()->setPalette(palette);

    QPalette sPalette = mKyFileDialogUi->m_sider->verticalScrollBar()->palette();
    //sPalette.setColor(QPalette::Button, Qt::red);//window AlternateBase Base NoRole
    sPalette.setColor(QPalette::Base, colorBase);//window AlternateBase Base NoRole

//    pDebug << "scrollbar color:" << sPalette.color(QPalette::AlternateBase) <<
//              sPalette.color(QPalette::Window) << sPalette.color(QPalette::Base)
//               << sPalette.color(QPalette::NoRole) << sPalette.color(QPalette::Shadow);

    mKyFileDialogUi->m_sider->verticalScrollBar()->setPalette(sPalette);


//    mKyFileDialogUi->m_sider->viewport()->setAttribute(Qt::WA_StyleSheet);
//    mKyFileDialogUi->m_sider->viewport()->setObjectName("viewport");
//    mKyFileDialogUi->m_sider->viewport()->setStyleSheet("QWidget#viewport{background-color:red}");
//    mKyFileDialogUi->m_sider->viewport()->setAttribute(Qt::WA_TranslucentBackground);

    //    mKyFileDialogUi->m_sider->verticalScrollBar()->setObjectName("scrollbar");
    //    mKyFileDialogUi->m_sider->verticalScrollBar()->setAttribute(Qt::WA_StyleSheet);
    //    mKyFileDialogUi->m_sider->verticalScrollBar()->setStyleSheet("QWidget#scrollbar{background-color:red}");
//    m_effect->setTransParentPath(sidebarPath);
//    m_effect->setTransParentAreaBg(colorBase);

//    //color.setAlphaF(0.5);
//    m_effect->setWindowBackground(color);

//    m_effect->drawWindowShadowManually(&p, this->rect(), false);
    QDialog::paintEvent(e);
}

void KyNativeFileDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
    {
        auto uri = Peony::FileUtils::getParentUri(getCurrentUri());
        //qDebug() << "goUp Action" << getCurrentUri() << uri;
        if (uri.isNull())
            return;
        goToUri(uri, true, true);
    }

    return QDialog::keyPressEvent(e);
}


Peony::DirectoryViewContainer *KyNativeFileDialog::getCurrentPage()
{
    Q_D(const KyNativeFileDialog);
    return d->m_container;
}

Peony::DirectoryViewContainer *KyNativeFileDialog::getCurrentPage() const
{
    Q_D(const KyNativeFileDialog);
    return d->m_container;
}

Peony::DirectoryViewWidget *KyNativeFileDialog::containerView() const
{
    return getCurrentPage()->getView();
}

void KyNativeFileDialog::updateAcceptButtonState()
{
    Q_D(const KyNativeFileDialog);

    pDebug << "updateAcceptButtonState......" << getCurrentUri() << d->acceptMode << d->fileMode;
    if(d->acceptMode == QFileDialog::AcceptOpen)
    {
        if(d->fileMode != QFileDialog::Directory && d->fileMode != QFileDialog::DirectoryOnly)
        {
            mKyFileDialogUi->m_acceptButton->setEnabled(!(getCurrentSelectionsList().length() == 0));
            return;
            bool isSelectFile = false;
            if(copyEditText() == "")
                mKyFileDialogUi->m_acceptButton->setEnabled(!(getCurrentSelectionsList().length() == 0));

            if(copyEditText() != "")
            {
                QString text = copyEditText();
                QStringList list = text.split("\"");
                pDebug << "list234565432............" << list;
                for (int i = list.length() - 1; i >= 0; i--) {
                    if(list[i] == "" || list[i] == " ")
                        list.removeAt(i);
                }
                pDebug << "list testtttttt..............:" << list;

                foreach (QString str, list) {
                    if(str.length() > 0 && str[0] == "\"")
                        str = str.remove(0, 1);
                    if(str.length() > 0 && str[str.length() - 1] == "\"")
                        str = str.remove(str.length() - 1, 1);
                    QString parentPath = getCurrentUri();

                    if(parentPath.endsWith("/"))
                        parentPath.remove(parentPath.length() - 1, 1);
                    pDebug << "updateAcceptButtonState str0000:" << (parentPath + "/" + str);//parentPath.startsWith("recent://")

                    if ((parentPath.startsWith("trash://") || parentPath.startsWith("recent://")
                         || parentPath.startsWith("computer://") || parentPath.startsWith("favorite://")
                         || parentPath.startsWith("filesafe://")) && getCurrentSelections().length() > 0)
                    {
                        QString targetUri = Peony::FileUtils::getTargetUri(getCurrentSelections()[0]);
                        QString targetPath = Peony::FileUtils::urlDecode(targetUri);
                        pDebug << "getTargetUri uri....." << targetUri;
                        pDebug << "uriiiiii...." << targetPath;
                        if(Peony::FileUtils::isFileExsit(targetPath))
                        {
                            isSelectFile = true;
                            pDebug << "isSelectFileeeeeeee11111...." << isSelectFile;
                            break;
                        }
                    }
                    else
                    {
                        QString targetPath = Peony::FileUtils::urlDecode(parentPath + "/" + str);
                        pDebug << "targetPath11111 uri....." << targetPath << Peony::FileUtils::isFileExsit(targetPath);

                        QString targetPath1 = Peony::FileUtils::urlEncode(parentPath + "/" + str);
                        pDebug << "targetPath123432 uri....." << targetPath1 << Peony::FileUtils::isFileExsit(targetPath1);

                        if(Peony::FileUtils::isFileExsit(parentPath + "/" + str))
                            isSelectFile = true;
                        pDebug << "isSelectFileeeeeee22222...." << isSelectFile;
                        break;
                    }
                }
                mKyFileDialogUi->m_acceptButton->setEnabled(isSelectFile);
                return;
            }
        }
        else
        {
            bool isSelectFile = true;
            if(copyEditText() != "")
            {
                QString text = copyEditText();
                QStringList list = text.split("\"");
                pDebug << "list0000............" << list;
                for (int i = list.length() - 1; i >= 0; i--) {
                    if(list[i] == "" || list[i] == " ")
                        list.removeAt(i);
                }
                pDebug << "list1111..............:" << list;

                foreach (QString str, list) {
                    pDebug << "strrrrrrrrr....." << str;
                    if(str.length() > 0 && str[0] == "\"")
                        str = str.remove(0, 1);
                    if(str.length() > 0 && str[str.length() - 1] == "\"")
                        str = str.remove(str.length() - 1, 1);
                    QString parentPath = getCurrentUri();

                    pDebug << "parentPath......" << parentPath;
                    if(parentPath.endsWith("/"))
                        parentPath.remove(parentPath.length() - 1, 1);
                    pDebug << "updateAcceptButtonState str,,,,,,:" <<  (parentPath + "/" + str) << Peony::FileUtils::isFileExsit(parentPath + "/" + str);

                    if(isDir(parentPath + "/" + str))
                    {
                        pDebug << "isSelectFile000000...:" << isSelectFile << (parentPath + "/" + str);
                        isSelectFile = true;
                        break;
                    }
                    else
                    {
                        isSelectFile = false;
                    }
                }
            }
            pDebug << "isSelectFile...." << isSelectFile;
            mKyFileDialogUi->m_acceptButton->setEnabled(isSelectFile);
        }
    }
    else
    {
        if(getCurrentSelectionsList().length() != 0){
            mKyFileDialogUi->m_acceptButton->setEnabled(true);
            return;
        }
        if(copyEditText() == "")
            mKyFileDialogUi->m_acceptButton->setEnabled(false);
        else
            mKyFileDialogUi->m_acceptButton->setEnabled(true);
        pDebug << "isenabled......:" << mKyFileDialogUi->m_acceptButton->isEnabled();
    }
    pDebug << "acceptButton isEnabled:" << mKyFileDialogUi->m_acceptButton->isEnabled();
}

void KyNativeFileDialog::onCurrentInputNameChanged()
{
    Q_D(KyNativeFileDialog);
    d->currentInputName = copyEditText();
    updateAcceptButtonState();
}

void KyNativeFileDialog::onNewFolder()
{
    Peony::CreateTemplateOperation op(getCurrentUri(), Peony::CreateTemplateOperation::EmptyFolder);
    op.run();
    return;
    QDir dir = directory();
    pDebug << "onNewFolder getCurrentUri:" << getCurrentUri() << directory() << directoryUrl();
    QString path = "";
    if(directory().path() != "/")
        path = directory().path();
    path += "/" + tr("NewFolder");
    if(!dir.exists(path))
    {
        bool newFolder = dir.mkdir(path);
        pDebug << "newFolder:" << newFolder;
    }
}

void KyNativeFileDialog::setSortType()
{
    if(getCurrentPage()->getSortOrder() ==  Qt::AscendingOrder)
    {
        mKyFileDialogUi->m_sortButton->setIcon(QIcon::fromTheme("view-sort-descending-symbolic"));
        mKyFileDialogUi->m_descending->setChecked(true);
        mKyFileDialogUi->m_ascending->setChecked(false);
    }
    else
    {
        mKyFileDialogUi->m_sortButton->setIcon(QIcon::fromTheme("view-sort-ascending-symbolic"));
        mKyFileDialogUi->m_ascending->setChecked(true);
        mKyFileDialogUi->m_descending->setChecked(false);
    }
}

void KyNativeFileDialog::intiContainerSort()
{
    auto settings = Peony::GlobalSettings::getInstance();
    auto sortType = settings->isExist(SORT_COLUMN)? settings->getValue(SORT_COLUMN).toInt() : 0;
    auto sortOrder = settings->isExist(SORT_ORDER)? settings->getValue(SORT_ORDER).toInt() : 0;
    pDebug << "sortType:" << sortType << sortOrder;
    getCurrentPage()->setSortType(Peony::FileItemModel::ColumnType(sortType));
    getCurrentPage()->setSortOrder(Qt::SortOrder(sortOrder));
    setSortType();
    setShowHidden(getWindowShowHidden());
}

void KyNativeFileDialog::refreshContainerSort()
{
    intiContainerSort();
    setUseDefaultNameSortOrder(getWindowUseDefaultNameSortOrder());
    setSortFolderFirst(getWindowSortFolderFirst());
}

void KyNativeFileDialog::refreshCompleter()
{
    const QStringList list = getCurrentPage()->getAllFileUris();
    pDebug << "getAllFileUris:" << list;
    pDebug << "getselectUris:" << getCurrentSelectionsList();

    m_CurrentPathAllFiles.clear();

    foreach (QString str, list) {
        str = Peony::FileUtils::urlDecode(str);
        m_CurrentPathAllFiles.append(str.split("/").last());
    }

    pDebug << "updateWindowState 1111111111111";
    QStringListModel *model = (QStringListModel *)(m_completer->model());
    model->setStringList(m_CurrentPathAllFiles);
    pDebug << "updateWindowState 2222222222222";
}

void KyNativeFileDialog::searchButtonClicked()
{
    m_searchMode = ! m_searchMode;
    if(m_searchMode)
    {
        mKyFileDialogUi->m_fileNameEdit->setText("");
        mKyFileDialogUi->m_fileNameEdit->setReadOnly(true);
        setCurrentSelections(QStringList());
    }
    else
    {
        mKyFileDialogUi->m_fileNameEdit->setReadOnly(false);
    }
    pDebug << "searchButtonClicked searchButtonClicked" <<m_searchMode;

    if (! getCurrentUri().startsWith("search://"))
        m_last_no_SearchPath = getCurrentUri();

    pDebug << "m_last_no_SearchPath....." << m_last_no_SearchPath << getCurrentUri() << getCurrentPage()->getCurrentUri();
    //Q_EMIT this->updateSearchRequest(m_search_mode);
    setSearchMode(m_searchMode);
}

void KyNativeFileDialog::setSearchMode(bool mode)
{
    mKyFileDialogUi->m_searchBtn->setCheckable(mode);
    mKyFileDialogUi->m_searchBtn->setChecked(mode);
//    mKyFileDialogUi->m_searchBtn->setDown(mode);
    mKyFileDialogUi->m_pathbar->switchEditMode(mode);
}

void KyNativeFileDialog::lineEditTextChange(QString text)
{
    m_copyEditText = text;
    if(m_searchMode)
        return;
    QString url;
    if(directory().path() != "/")
        url = QUrl(getCurrentUri()).path();//directoryUrl().toString();//getCurrentUri()
    pDebug << "mKyFileDialogUi->m_fileNameEdit......:" << getCurrentUri() << url << text;
    pDebug << "listview55555555555555.........." << m_CurrentPathAllFiles.length();
    QString parentPath = getCurrentUri();

    if(parentPath.endsWith("/"))
        parentPath.remove(parentPath.length() - 1, 1);
    if(url.endsWith("/"))
        url.remove(url.length() - 1, 1);

    pDebug << "parentPath:" << parentPath;
    pDebug << "getCurrentSelections......:" << getCurrentSelectionsList() << text;

    if(!text.contains("\""))//输入框中只有1个文件
    {   
        if ((parentPath.startsWith("trash://") || parentPath.startsWith("recent://")
             || parentPath.startsWith("computer://") || parentPath.startsWith("favorite://")
             || parentPath.startsWith("filesafe://")) && getCurrentSelections().length() > 0)
        {
            for(int i = 0; i < getCurrentSelections().length(); i++)
            {
                QString targetUri = Peony::FileUtils::getTargetUri(getCurrentSelections()[i]);
                Peony::FileInfo fileInfo(targetUri);
                pDebug << "text:" << text << "displayName:" << fileInfo.displayName();
                if(fileInfo.displayName() == text){
                    updateAcceptButtonState();
                    return;
                }
            }
            QString targetUri = Peony::FileUtils::getTargetUri(getCurrentSelections()[0]);
            QString targetPath = Peony::FileUtils::urlDecode(targetUri);
            pDebug << "getTargetUri uri....." << targetUri;
            pDebug << "uriiiiii...." << targetPath;
            if(Peony::FileUtils::isFileExsit(targetPath))
            {
                QStringList selectList;
                selectList.append(getCurrentSelections()[0]);
                pDebug << "selectList...000000" << selectList;
                setCurrentSelections(selectList);
                updateAcceptButtonState();
                return;
            }
        }
        else
        {
            pDebug << "Peony::FileUtils::isFileExsit123:" << (parentPath + "/" + text) << Peony::FileUtils::isFileExsit(parentPath + "/" + text);
            //        pDebug << "Peony::FileUtils::isFileExsit:" << (parentPath + "/" + text) << Peony::FileUtils::isFileExsit(parentPath + "/" + text);
            for(int i = 0; i < getCurrentSelections().length(); i++)
            {
                if(Peony::FileUtils::getFileDisplayName(getCurrentSelections()[i]) == text){
                    updateAcceptButtonState();
                    return;
                }
            }

            if(Peony::FileUtils::isFileExsit(parentPath + "/" + text)){
                pDebug << "file exists....";
                //m_container选择指定文件
                QStringList selectList;
                selectList.append(parentPath + "/" + text);
                pDebug << "selectList...1111111" << selectList;
                setCurrentSelections(selectList);
                if(containerView())
                    pDebug << "selectList...22222222222" << containerView()->getSelections();
                updateAcceptButtonState();
                return;
            }
            else
            {
                QString path = parentPath + "/" + text;
                pDebug << path << "not exists!";
                setCurrentSelections(QStringList());
                updateAcceptButtonState();
                return;
            }
        }
    }
    else
    {
        if ((parentPath.startsWith("trash://") || parentPath.startsWith("recent://")
             || parentPath.startsWith("computer://") || parentPath.startsWith("favorite://")
             || parentPath.startsWith("filesafe://")))
        {
            QStringList list = text.split("\"");
            pDebug << "listttt1111............" << list;
            for (int i = list.length() - 1; i >= 0; i--) {
                if(list[i] == "" || list[i] == " ")
                    list.removeAt(i);
            }
            QStringList displayNameList;
            for(int i = 0; i < getCurrentSelections().length(); i++)
            {
                QString targetUri = Peony::FileUtils::getTargetUri(getCurrentSelections()[i]);
                Peony::FileInfo fileInfo(targetUri);
                pDebug << "text:" << text << "displayName:" << fileInfo.displayName();
                displayNameList.append(fileInfo.displayName());
            }
            pDebug << "list:" << list;
            pDebug << "displayNameList:" << displayNameList;
            for(int i = 0; i < list.length(); i++){
                if(!displayNameList.contains(list[i]))
                    break;
                else if(i == list.length() - 1){
                    updateAcceptButtonState();
                    return;
                }
            }

            QStringList selectList;
            foreach (QString str, getCurrentSelections())
            {
                QString targetUri = Peony::FileUtils::getTargetUri(str);
                QString targetPath = Peony::FileUtils::urlDecode(targetUri);
                pDebug << "getTargetUri uri0000....." << targetUri;
                pDebug << "uriiiiii00000...." << targetPath;
                if(Peony::FileUtils::isFileExsit(targetPath))
                {
                    selectList.append(str);
                }
            }
            pDebug << "selectList...222222222" << selectList;
            setCurrentSelections(selectList);
            updateAcceptButtonState();
            return;
        }
        else{
            QStringList selectList;
            QStringList list = text.split("\"");
            pDebug << "listttt1111............" << list;
            for (int i = list.length() - 1; i >= 0; i--) {
                if(list[i] == "" || list[i] == " ")
                    list.removeAt(i);
            }
            pDebug << "listtttt22222222..............:" << list;

            QStringList displayNameList;
            for(int i = 0; i < getCurrentSelections().length(); i++)
            {
                QString displayName = Peony::FileUtils::getFileDisplayName(getCurrentSelections()[i]);
                displayNameList.append(displayName);
            }
            pDebug << "list:" << list;
            pDebug << "displayNameList:" << displayNameList;
            for(int i = 0; i < list.length(); i++){
                if(!displayNameList.contains(list[i]))
                    break;
                else if(i == list.length() - 1){
                    updateAcceptButtonState();
                    return;
                }
            }

            pDebug << "listtttttt3333333.........................";
            foreach (QString str, list) {
                if(str.length() > 0 &&  str[0] == "\"")
                    str = str.remove(0, 1);
                if(str.length() > 0 && str[str.length() - 1] == "\"")
                    str = str.remove(str.length() - 1, 1);
                pDebug << "str:" << str;
                QFile file(url + "/" + str);

                if(Peony::FileUtils::isFileExsit(parentPath + "/" + str))//(file.exists())
                    selectList.append(parentPath + "/" + str);
            }
            pDebug << "selectList...333333" << selectList;
            setCurrentSelections(selectList);
            updateAcceptButtonState();
        }
        return;
    }
}

void KyNativeFileDialog::setShortCuts()
{
    if (!m_shortcutsSet) {
        //stop loading action
        QAction *stopLoadingAction = new QAction(this);
        stopLoadingAction->setShortcut(QKeySequence(Qt::Key_Escape));
        addAction(stopLoadingAction);
        connect(stopLoadingAction, &QAction::triggered, this, &KyNativeFileDialog::forceStopLoading);

        //show hidden action
        QAction *showHiddenAction = new QAction(this);
        showHiddenAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
        showHiddenAction->setCheckable(true);
        addAction(showHiddenAction);
        connect(showHiddenAction, &QAction::triggered, this, [=]() {
            //qDebug() << "show hidden";
            this->setShowHidden(!getWindowShowHidden());
        });

        auto undoAction = new QAction(QIcon::fromTheme("edit-undo-symbolic"), tr("Undo"), this);
        undoAction->setShortcut(QKeySequence::Undo);
        addAction(undoAction);
        connect(undoAction, &QAction::triggered, [=]() {
            Peony::FileOperationManager::getInstance()->undo();
        });

        auto redoAction = new QAction(QIcon::fromTheme("edit-redo-symbolic"), tr("Redo"), this);
        redoAction->setShortcut(QKeySequence::Redo);
        addAction(redoAction);
        connect(redoAction, &QAction::triggered, [=]() {
            Peony::FileOperationManager::getInstance()->redo();
        });

        //add CTRL+D for delete operation
        auto trashAction = new QAction(this);
        trashAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_Delete<<QKeySequence(Qt::CTRL + Qt::Key_D));
        connect(trashAction, &QAction::triggered, [=]() {
            auto currentUri = getCurrentUri();
            if (currentUri.startsWith("search://")
                    || currentUri.startsWith("favorite://") || currentUri == "filesafe:///"
                    || currentUri.startsWith("kmre://") || currentUri.startsWith("kydroid://"))
                return;

            auto uris = this->getCurrentSelections();

            QString desktopPath = "file://" +  QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            QString desktopUri = Peony::FileUtils::getEncodedUri(desktopPath);
            QString homeUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            QString documentPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
            QString musicPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
            QString moviesPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
            QString picturespPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
            QString downloadPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
            if (!uris.isEmpty() && !uris.contains(desktopUri) && !uris.contains(homeUri) && !uris.contains(documentPath) && !uris.contains(musicPath)
                    && !uris.contains(moviesPath) && !uris.contains(picturespPath) && !uris.contains(downloadPath)) {
                bool isTrash = this->getCurrentUri() == "trash:///";
                if (!isTrash) {
                    Peony::FileOperationUtils::trash(uris, true);
                } else {
                    Peony::FileOperationUtils::executeRemoveActionWithDialog(uris);
                }
            }
        });
        addAction(trashAction);

        auto deleteAction = new QAction(this);
        deleteAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::SHIFT + Qt::Key_Delete));
        addAction(deleteAction);
        connect(deleteAction, &QAction::triggered, [=]() {
            auto currentUri = getCurrentUri();
            if (currentUri.startsWith("search://") || currentUri == "filesafe:///"
                    || currentUri.startsWith("kmre://") || currentUri.startsWith("kydroid://"))
                return;

            auto uris = this->getCurrentSelections();

            QString desktopPath = "file://" +  QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
            QString documentPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
            QString musicPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
            QString moviesPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
            QString picturespPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
            QString downloadPath = Peony::FileUtils::getEncodedUri("file://" +  QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
            QString desktopUri = Peony::FileUtils::getEncodedUri(desktopPath);
            QString homeUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            if (! uris.contains(desktopUri) && !uris.contains(homeUri) && !uris.contains(documentPath) && !uris.contains(musicPath)
                    && !uris.contains(moviesPath) && !uris.contains(picturespPath) && !uris.contains(downloadPath)) {
                Peony::FileOperationUtils::executeRemoveActionWithDialog(uris);
            }
        });

        auto searchAction = new QAction(this);
        searchAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::CTRL + Qt::Key_F));
        connect(searchAction, &QAction::triggered, this, [=]() {
            searchButtonClicked();
        });
        addAction(searchAction);

        //F4 or Alt+D, change to address
        auto locationAction = new QAction(this);
        locationAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_F4<<QKeySequence(Qt::ALT + Qt::Key_D));
        connect(locationAction, &QAction::triggered, this, [=]() {
            mKyFileDialogUi->m_pathbar->startEdit();
        });
        addAction(locationAction);

        auto closeWindowAction = new QAction(this);
        closeWindowAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::ALT + Qt::Key_F4));
        connect(closeWindowAction, &QAction::triggered, this, [=]() {
            this->close();
        });
        addAction(closeWindowAction);

        auto maxAction = new QAction(this);
        maxAction->setShortcut(QKeySequence(Qt::Key_F11));
        connect(maxAction, &QAction::triggered, this, [=]() {
            if(!m_istableModel){
                //showFullScreen has some issue, change to showMaximized, fix #20043
                mKyFileDialogUi->m_pathbar->cancelEdit();
                if (!this->isMaximized()) {
                    this->showMaximized();
                } else {
                    this->showNormal();
                }
                updateMaximizeState();
            }
        });
        addAction(maxAction);

        auto refreshWindowAction = new QAction(this);
        refreshWindowAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
        connect(refreshWindowAction, &QAction::triggered, this, [=]() {
            this->refresh();
        });
        addAction(refreshWindowAction);

        auto listToIconViewAction = new QAction(this);
        listToIconViewAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
        connect(listToIconViewAction, &QAction::triggered, this, [=]() {
            this->beginSwitchView(QString("Icon View"));
        });
        addAction(listToIconViewAction);

        auto iconToListViewAction = new QAction(this);
        iconToListViewAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_2));
        connect(iconToListViewAction, &QAction::triggered, this, [=]() {
            this->beginSwitchView(QString("List View"));
        });
        addAction(iconToListViewAction);

        auto reverseSelectAction = new QAction(this);
        reverseSelectAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L));
        connect(reverseSelectAction, &QAction::triggered, this, [=]() {
            this->getCurrentPage()->getView()->invertSelections();
        });
        addAction(reverseSelectAction);

        auto refreshAction = new QAction(this);
        refreshAction->setShortcut(Qt::Key_F5);
        connect(refreshAction, &QAction::triggered, this, [=]() {
            this->refresh();
        });
        addAction(refreshAction);

        //select all files in view, fix bug#115442
        auto *selectAllAction = new QAction(this);
        selectAllAction->setShortcut(QKeySequence::SelectAll);
        connect(selectAllAction, &QAction::triggered, this, [=]() {
            if (this->getCurrentPage()->getView())
            {
                /// note: 通过getAllFileUris设置的全选效率过低，如果增加接口则会导致二进制兼容性问题
                /// 所以这里使用现有的反选接口实现高效的全选，这个方法在mainwindow中也有用到
                //auto allFiles = this->getCurrentPage()->getView()->getAllFileUris();
                //this->getCurrentPage()->getView()->setSelections(allFiles);
                setCurrentSelections(QStringList());
                this->getCurrentPage()->getView()->invertSelections();
            }
        });
        addAction(selectAllAction);

        //file operations
        auto *copyAction = new QAction(this);
        copyAction->setShortcut(QKeySequence::Copy);
        connect(copyAction, &QAction::triggered, [=]() {
            bool is_recent = false;
            if (!this->getCurrentSelections().isEmpty())
            {
                //                if (this->getCurrentSelections().first().startsWith("trash://", Qt::CaseInsensitive)) {
                //                    return ;
                //                }
                if (this->getCurrentSelections().first().startsWith("recent://", Qt::CaseInsensitive)) {
                    is_recent = true;
                }
                if (this->getCurrentSelections().first().startsWith("favorite://", Qt::CaseInsensitive)) {
                    return ;
                }
            }
            else
                return;

            QStringList selections;
            if (is_recent)
            {
                for(auto uri:this->getCurrentSelections())
                {
                    uri = Peony::FileUtils::getTargetUri(uri);
                    selections << uri;
                }
            }
            else{
                selections = this->getCurrentSelections();
            }

            Peony::ClipboardUtils::setClipboardFiles(selections, false);
        });
        addAction(copyAction);

        auto *pasteAction = new QAction(this);
        pasteAction->setShortcut(QKeySequence::Paste);
        connect(pasteAction, &QAction::triggered, [=]() {
            auto currentUri = getCurrentUri();
            if (currentUri.startsWith("trash://") || currentUri.startsWith("recent://")
                    || currentUri.startsWith("computer://") || currentUri.startsWith("favorite://")
                    || currentUri.startsWith("search://") || currentUri == "filesafe:///")
            {
                /* Add hint information,link to bug#107640. */
                QMessageBox::warning(this, tr("warn"), tr("This operation is not supported."));
                return;
            }
            if (Peony::ClipboardUtils::isClipboardHasFiles()) {
                //FIXME: how about duplicated copy?
                //FIXME: how to deal with a failed move?
                auto op = Peony::ClipboardUtils::pasteClipboardFiles(this->getCurrentUri());
                if (op) {
                    connect(op, &Peony::FileOperation::operationFinished, this, [=](){
                        auto opInfo = op->getOperationInfo();
                        auto targetUirs = opInfo->dests();
                        setCurrentSelectionUris(targetUirs);
                    }, Qt::BlockingQueuedConnection);
                }
                else{
                    //fix paste file in old path not update issue, link to bug#71627
                    this->getCurrentPage()->getView()->repaintView();
                }
            }
        });
        addAction(pasteAction);

        auto *cutAction = new QAction(this);
        cutAction->setShortcut(QKeySequence::Cut);
        connect(cutAction, &QAction::triggered, [=]() {
            if (!this->getCurrentSelections().isEmpty()) {
                //                if (this->getCurrentSelections().first().startsWith("trash://", Qt::CaseInsensitive)) {
                //                    return ;
                //                }
                if (this->getCurrentSelections().first().startsWith("recent://", Qt::CaseInsensitive)) {
                    return ;
                }
                if (this->getCurrentSelections().first().startsWith("favorite://", Qt::CaseInsensitive)) {
                    return ;
                }

                auto currentUri = getCurrentUri();
                if (currentUri.startsWith("search://"))
                    return;

                QString desktopPath = "file://" +  QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                QString desktopUri = Peony::FileUtils::getEncodedUri(desktopPath);
                QString homeUri = "file://" +  QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
                if (! this->getCurrentSelections().contains(desktopUri) && ! this->getCurrentSelections().contains(homeUri))
                {
                    Peony::ClipboardUtils::setClipboardFiles(this->getCurrentSelections(), true);
                    this->getCurrentPage()->getView()->repaintView();
                }
            }
        });
        addAction(cutAction);

        m_shortcutsSet = true;
    }
}

void KyNativeFileDialog::initialViewId()
{
    if(!containerView())
        return;
    int zoomLevel = containerView()->currentZoomLevel();

    auto viewId = Peony::DirectoryViewFactoryManager2::getInstance()->getDefaultViewId(zoomLevel, getCurrentUri());
    pDebug << "initialViewId viewId:" << viewId;
    beginSwitchView(viewId);
}

QString KyNativeFileDialog::copyEditText()
{
    return m_copyEditText;
}

void KyNativeFileDialog::isTableModel()
{
    QDBusInterface *interFace = new QDBusInterface(SERVICE, PATH, INTERFACE, QDBusConnection::sessionBus());
    if(interFace->isValid()){
        connect(interFace, SIGNAL(mode_change_signal(bool)), this, SLOT(updateTableModel(bool)));
    }
    QDBusMessage message = QDBusMessage::createMethodCall(SERVICE, PATH, INTERFACE, "get_current_tabletmode");
    QDBusMessage ret = QDBusConnection::sessionBus().call(message);
    if (ret.type() != QDBusMessage::ReplyMessage)
    {
        //从返回参数获取返回值
        pDebug << "complex type failed!";
        updateTableModel(false);
    }
    else
    {
        updateTableModel(ret.arguments()[0].value<bool>());
    }
}

void KyNativeFileDialog::updateTableModel(bool tableModel)
{
    m_istableModel = tableModel;
    mKyFileDialogUi->m_maximizeAndRestore->setVisible(!tableModel);
}

void KyNativeFileDialog::updateSearchProgressBar()
{
    if(m_needSearch){
        pDebug << "updateSearchProgressBar...." << m_searchMode << m_needSearch;
        Q_EMIT mKyFileDialogUi->m_pathbar->updateSearchProgress(false);
        QCursor c;
        c.setShape(Qt::ArrowCursor);
        this->setCursor(c);
        getCurrentPage()->getView()->setCursor(c);
        mKyFileDialogUi->m_sider->setCursor(c);
    }
}

void KyNativeFileDialog::installTranslate(QString locale)
{
    m_translator = new QTranslator(this);
    pDebug << "local: " << locale;
    if (m_translator->load("/usr/share/qt5-lingmo-platformtheme/qt5-lingmo-filedialog_" + locale)) {
        pDebug << "Load translations file success!";
        QApplication::installTranslator(m_translator);
    } else {
        pDebug << "Load translations file failed!";
    }

    m_translator0 = new QTranslator(this);
    pDebug << "local: " << locale;
    if (m_translator0->load("/usr/share/qt5-lingmo-platformtheme/qt5-lingmo-platformtheme_" + locale)) {
        pDebug << "Load translations file success!";
        QApplication::installTranslator(m_translator0);
    } else {
        pDebug << "Load translations file failed!";
    }

    m_translator1 = new QTranslator(this);
    pDebug << "QTranslator t load" << m_translator1->load("/usr/share/libexplorer-qt/libexplorer-qt_" + locale);
    QApplication::installTranslator(m_translator1);
    m_translator2 = new QTranslator(this);
    pDebug << "QTranslator t2 load" << m_translator2->load("/usr/share/explorer-qt-desktop/explorer-qt-desktop_" + locale);
    QApplication::installTranslator(m_translator2);
    m_translator3 = new QTranslator(this);
    pDebug << "QTranslator t3 load" << m_translator3->load("/usr/share/qt5/translations/qt_" + locale);
    QApplication::installTranslator(m_translator3);
}

KyFileDialogHelper::KyFileDialogHelper() : QPlatformFileDialogHelper(), mKyFileDialog(new KyNativeFileDialog)
{
    mKyFileDialog->setHelper(this);

    pDebug << "KyFileDialogHelper........." << (options() == nullptr);
    connect(mKyFileDialog, &KyNativeFileDialog::currentChanged, this, &KyFileDialogHelper::currentChanged);
    connect(mKyFileDialog, &KyNativeFileDialog::directoryEntered, this, &KyFileDialogHelper::directoryEntered);
    //QFileDialog::accept()里_q_emitUrlSelected会发送fileSelected信号 这里会导致发两次fileSelected的信号
    //    connect(mKyFileDialog, &KyNativeFileDialog::fileSelected, this, &KyFileDialogHelper::fileSelected);
    //QFileDialog::accept()里_q_emitUrlsSelected会发送filesSelected信号 这里会导致发两次filesSelected的信号
    //    connect(mKyFileDialog, &KyNativeFileDialog::filesSelected, this, &KyFileDialogHelper::filesSelected);
    connect(mKyFileDialog, &KyNativeFileDialog::filterSelected, this, &KyFileDialogHelper::filterSelected);
    connect(mKyFileDialog, &QDialog::accepted, this, &KyFileDialogHelper::accept);
    connect(mKyFileDialog, &QDialog::rejected, this, &KyFileDialogHelper::reject);

}

KyFileDialogHelper::~KyFileDialogHelper()
{
    pDebug << "~~~~~~~~KyFileDialogHelper";
    ///不可以析构  普通函数里QFileDialog::show不显示和QFiledialog->show()会显示 析构的话BT下载工具打开不显示界面
//    if(mKyFileDialog){
//        mKyFileDialog->deleteLater();
//        mKyFileDialog = nullptr;
//    }

}

void KyFileDialogHelper::exec()
{
    mKyFileDialog->discardDelayedShow();
    pDebug << "KyFileDialogHelper::exec..............";
    mKyFileDialog->exec();
}

void KyFileDialogHelper::hide()
{
    mKyFileDialog->discardDelayedShow();
    mKyFileDialog->hide();
}

bool KyFileDialogHelper::show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent)
{
    if(m_isShow){
        mKyFileDialog->delayShow();
        return true;
    }
    pDebug << "show........." << (options() == nullptr);
    pDebug << "option initialselectfiles..." << options()->initiallySelectedFiles();
    pDebug << "option initialDirectory..." << options()->initialDirectory();

    m_initialDirectory = options()->initialDirectory();
    m_initialSelectFiles = options()->initiallySelectedFiles();

    if(parent != nullptr)
        pDebug << "window....:" << parent->x() << parent->y() << parent->width() << parent->height();
    else
        pDebug << "window is nullptr....";
    initDialog();

    mKyFileDialog->setAttribute(Qt::WA_NativeWindow, true);
    mKyFileDialog->setWindowFlags(windowFlags);
    mKyFileDialog->setWindowModality(windowModality);

    if(parent){
            if(QWidget *p = mKyFileDialog->find(parent->winId())){
                for(QFileDialog *fd : p->findChildren<QFileDialog *>()){
                    if(options()->windowTitle() == fd->windowTitle()){
                        pDebug << "filedoalog set parent...." << fd->geometry();
                        if(mKyFileDialog->parentWidget() != fd){
                            ///设置parent是QFiledialog 之前设置的是p 会有问题 showevent里有时候会找不到对应name的
                            mKyFileDialog->setParent(fd, windowFlags);
                        }
                        pDebug << "filediaog directory000...." << fd->directory();
                        pDebug << "filedialog select000....." << fd->selectedFiles();
                        pDebug << "filedialog selectUrls000....." << fd->selectedUrls();
                        pDebug << "filedialog directoryUrl000....." << fd->directoryUrl();
                        if(m_initialSelectFiles.length() > 0 && QFile::exists(m_initialSelectFiles.value(0).path())){
                            QDir dir(m_initialSelectFiles.value(0).path());
                            dir.cdUp();
                            pDebug <<"dirrrrr..." << dir.path();
                            mKyFileDialog->mKyFileDialogUi->m_pathbarWidget->updatePath(QUrl::fromLocalFile(dir.absolutePath()).toString());
                             mKyFileDialog->setDirectory(dir);
                        } else if(fd->directory().exists()){
                            mKyFileDialog->mKyFileDialogUi->m_pathbarWidget->updatePath(QUrl::fromLocalFile(fd->directory().absolutePath()).toString());

                            mKyFileDialog->setDirectory(fd->directory());
                        }
//                        if(fd->directoryUrl().)

                        //                for(QPushButton *btn : mKyFileDialog->findChildren<QPushButton*>())
                        //                {
                        //                    if(btn->objectName() == "acceptButton")
                        //                    {
                        //                        pDebug << "mydialog find object acceptButton";
                        //                    }
                        //                }
                        break;
                    }
                }
            }
    }
    else{
        for(QWidget *widget : qApp->allWidgets()){
            if(QFileDialog *fd = qobject_cast<QFileDialog *>(widget)){
                if(options()->windowTitle() == fd->windowTitle()){
                    pDebug << "parent us null filedoalog set parent...." << fd->geometry() << options()->windowTitle() << fd->objectName();
                    if(mKyFileDialog->parentWidget() != fd){
                        mKyFileDialog->setParent(fd, windowFlags);
                    }
                    pDebug << "parent us null filedoalog directory...." << fd->directory();
                    pDebug << "parent us null filedoalog selectedFiles...." << m_initialSelectFiles.value(0).path() << QFile::exists(m_initialSelectFiles.value(0).path());
                    if(m_initialSelectFiles.length() > 0 && QFile::exists(m_initialSelectFiles.value(0).path())){
                        QDir dir(m_initialSelectFiles.value(0).path());
                        dir.cdUp();
                        pDebug <<"dirrrrr..." << dir.path();
                        mKyFileDialog->mKyFileDialogUi->m_pathbarWidget->updatePath(QUrl::fromLocalFile(dir.absolutePath()).toString());

                         mKyFileDialog->setDirectory(dir);
                    }
                    else if(fd->directory().exists()){
                        mKyFileDialog->mKyFileDialogUi->m_pathbarWidget->updatePath(QUrl::fromLocalFile(fd->directory().absolutePath()).toString());

                        mKyFileDialog->setDirectory(fd->directory());
                    }
                    break;
                }
            }
        }
    }

    if(options()->viewMode() == QFileDialog::List && mKyFileDialog->viewMode() != KyNativeFileDialog::Icon)
        mKyFileDialog->setViewMode(KyNativeFileDialog::Icon);
    else if(options()->viewMode() == QFileDialog::Detail && mKyFileDialog->viewMode() != KyNativeFileDialog::List)
        mKyFileDialog->setViewMode(KyNativeFileDialog::List);

    //remove windows header
//    QString platform = QGuiApplication::platformName();
//    if(platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive))
//    {
//        kdk::LingmoUIStyleHelper::self()->removeHeader(mKyFileDialog);
//    } else {
//        MotifWmHints hints;
//        hints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
//        hints.functions = MWM_FUNC_ALL;
//        hints.decorations = MWM_DECOR_BORDER;
//        XAtomHelper::getInstance()->setWindowMotifHint(mKyFileDialog->winId(), hints);
//    }
    kdk::LingmoUIStyleHelper::self()->removeHeader(mKyFileDialog);


    mKyFileDialog->delayShow();
    viewInitialFinished();
    return true;
}

void KyFileDialogHelper::initDialog()
{
    m_isShow = true;

    pDebug << "initDialog filter...." << options()->filter() << options()->nameFilters() << options()->mimeTypeFilters() << options()->viewMode();
    if (options()->windowTitle().isEmpty()) {
        mKyFileDialog->setWindowTitle(options()->acceptMode() == QFileDialogOptions::AcceptOpen ? tr("Open File")
                                                                                          : tr("Save File"));
    } else {
        mKyFileDialog->setWindowTitle(options()->windowTitle());
    }
    QStringList nameFilters;
    foreach (QString nameFilter, options()->nameFilters()) {
        nameFilters.append(QPlatformFileDialogHelper::cleanFilterList(nameFilter));
    }
    pDebug << "nameFilters....:" << nameFilters;
    if(nameFilters.length() < 1)
    {
        pDebug << "contains *....";
        QStringList list = options()->nameFilters();
        list.append(tr("All Files (*)"));
        options()->setNameFilters(list);
    }
    pDebug << "option NameFilters:" << options()->nameFilters();
    pDebug << "option mode:" << options()->acceptMode() << options()->fileMode() << options()->viewMode();
    mKyFileDialog->setFilter(options()->filter());
    mKyFileDialog->setFileMode(QFileDialog::FileMode(options()->fileMode()));
    mKyFileDialog->setAcceptMode(QFileDialog::AcceptMode(options()->acceptMode()));
    pDebug << "initDialog filters:" << options()->nameFilters();
    mKyFileDialog->setNameFilters(options()->nameFilters());
    pDebug << "view mode:" << options()->viewMode() << mKyFileDialog->viewMode();

    for(int i = 0; i < QFileDialogOptions::DialogLabelCount; ++i) {
        auto label = static_cast<QFileDialogOptions::DialogLabel>(i);
        if(options()->isLabelExplicitlySet(label)) {
            mKyFileDialog->setLabelText(static_cast<QFileDialog::DialogLabel>(label), options()->labelText(label));
        }
    }
//    pDebug << "initiallySelectedMimeTypeFilter,,,,:" << options()->initiallySelectedMimeTypeFilter() << options()->initiallySelectedNameFilter();
    auto filter = options()->initiallySelectedMimeTypeFilter();
    pDebug << "filter.........." << filter;
     if(!filter.isEmpty()) {
         selectMimeTypeFilter(filter);
     }
     else {
         filter = options()->initiallySelectedNameFilter();
         if(!filter.isEmpty()) {
             pDebug << "initiallySelectedNameFilter...." << filter;
             selectNameFilter(filter);
         }
     }

     connect(mKyFileDialog->containerView(), &Peony::DirectoryViewWidget::viewDirectoryChanged,
             this, &KyFileDialogHelper::viewInitialFinished, Qt::UniqueConnection);
}

void KyFileDialogHelper::viewInitialFinished()
{
    if(m_viewInitialFinished)
        return;
    pDebug << "viewInitialFinished....";
    QTimer::singleShot(100, this, [this](){
        pDebug << "viewport update1111111.......";
        mKyFileDialog->mKyFileDialogUi->m_sider->viewport()->update();
    });

    QTimer::singleShot(500, this, [this](){

        auto selectirectory = m_initialDirectory;// options()->initialDirectory();
        auto selectedFiles = m_initialSelectFiles; //options()->initiallySelectedFiles();
        QStringList strList;

        pDebug << "m_viewInitialFinished:" << m_viewInitialFinished;
        pDebug << "viewInitialFinished selectirectory:" << selectirectory;
        pDebug << "viewInitialFinished selectedFiles:" << selectedFiles;

        if(m_viewInitialFinished)
            return;
        if(selectedFiles.length() > 0)
        {
            pDebug << "initiallySelectedFiles:" << selectedFiles;

            for(const auto& selectedFile: selectedFiles) {
                auto info = Peony::FileInfo::fromUri(selectedFile.path());
//                if (!info->isDir() && !info->isVolume())
//                {
                    selectFile(selectedFile);
                    strList.append(selectedFile.toString());
//                }
            }
            pDebug << "strList.......:" << strList;
            pDebug << "getCurrentUri,,.......:" << mKyFileDialog->getCurrentUri();

            if(strList.length() > 0){
                QString path;
                QUrl url(strList[0]);
                if(mKyFileDialog->isDir(url.path()) && options()->fileMode() != QFileDialog::Directory &&
                        options()->fileMode() != QFileDialog::DirectoryOnly){
                    path = url.path();
                }
                else{
                    QDir dir(url.path());
                    dir.cdUp();
                    path = dir.path();
                }
                pDebug << "initial path:" << path << url;
                if(mKyFileDialog->getCurrentUri() != ("file://" + path))
                {
                    pDebug << "initial change directory..." << ("file://" + path);
                    mKyFileDialog->setDirectoryUrl(QUrl("file://" + path));
                }

                pDebug << "setSelections...." << strList << (mKyFileDialog->containerView() == nullptr);

                mKyFileDialog->setCurrentSelections(strList);
                pDebug << "initial getselections:" << mKyFileDialog->getCurrentSelectionsList();
                //mKyFileDialog->containerView()->scrollToSelection(strList[0]);
                QString selectName = mKyFileDialog->selectName();
                if(selectName == "" && strList.length() > 0)
                {
                    QString str0 = strList[0];
                    selectName = str0.split("/").last();
                }
                pDebug << "initial selectName:" << selectName;
                mKyFileDialog->mKyFileDialogUi->m_fileNameEdit->setText(selectName);
            }
        }

        pDebug << "initially selectirectory:" << selectirectory.toString() << selectirectory.path() << QFile::exists(selectirectory.path()) << Peony::FileUtils::isFileExsit(selectirectory.toString());
        pDebug << "current path....." << mKyFileDialog->getCurrentUri();
        if(strList.length() <= 0 && Peony::FileUtils::isFileExsit(selectirectory.toString()))//QFile::exists(selectirectory.path()))
        {
            QString cStr = mKyFileDialog->getCurrentUri();
            QString sStr = selectirectory.toString();

            if(cStr.endsWith("/"))
                cStr = cStr.remove(cStr.length() - 1, 1);
            if(sStr.endsWith("/"))
                sStr = sStr.remove(sStr.length() - 1, 1);
            pDebug << "cStr...." << cStr << "sStr...." << sStr;
            if(cStr != sStr)
                mKyFileDialog->setDirectoryUrl(selectirectory);
        }

        mKyFileDialog->intiContainerSort();
        mKyFileDialog->initialViewId();
        pDebug << "viewport update222222.......";
        mKyFileDialog->mKyFileDialogUi->m_sider->viewport()->update();
//        mKyFileDialog->refresh();
        m_viewInitialFinished = true;
    });
}

bool KyFileDialogHelper::isViewInitialFinished()
{
    return m_viewInitialFinished;
}

bool KyFileDialogHelper::isShow()
{
    return m_isShow;
}

bool KyFileDialogHelper::defaultNameFilterDisables() const
{
    return false;
}

void KyFileDialogHelper::setDirectory(const QUrl &directory)
{
    QString path = directory.path();
    if(path.startsWith("/"))
       path = "file://" + path;
    pDebug << "helper setDirectory...." << directory.toString() << directory.path() << path;
    mKyFileDialog->setDirectory(path);
//    pDebug << "setInitialDirectory.....";
//    options()->setInitialDirectory(directory);
}

QUrl KyFileDialogHelper::directory() const
{
    return QUrl(mKyFileDialog->directory().absolutePath());
}

void KyFileDialogHelper::selectFile(const QUrl &filename)
{
    pDebug << "KyFileDialogHelper selectFile.............:" << filename;
    m_selectedFiles = filename;
    mKyFileDialog->selectFile(filename.toString());
}

QList<QUrl> KyFileDialogHelper::selectedFiles() const
{
    pDebug << "selectedFiles0000..........:" << mKyFileDialog->directoryUrl() << mKyFileDialog->directory() << mKyFileDialog->getCurrentUri() << mKyFileDialog->acceptMode();
    if(options()->acceptMode() == QFileDialogOptions::AcceptOpen)
    {
        QList<QUrl> urlList = mKyFileDialog->selectedUrls();
        if((options()->fileMode() == QFileDialogOptions::Directory || options()->fileMode() == QFileDialogOptions::DirectoryOnly) &&
                 mKyFileDialog->copyEditText() == "" && m_viewInitialFinished)
        {
            if (!mKyFileDialog->getCurrentPage()) {
                return QList<QUrl>();
            }
            QList<QUrl> urls;
            QString uri = mKyFileDialog->getCurrentPage()->getCurrentUri();
            pDebug << "KyFileDialogHelper selectedFiles current uri........:" << uri;

            if (uri.startsWith("trash://") || uri.startsWith("recent://")
                    || uri.startsWith("computer://") || uri.startsWith("favorite://")
                    || uri.startsWith("filesafe://"))
            {
                urls << Peony::FileUtils::getTargetUri(uri);
            }
            else
            {
                if(uri.startsWith("file:///"))
                    uri = uri.remove(0,7);
                pDebug << "open selectedFiles00000..........:" << uri;
                urls << QUrl::fromLocalFile(Peony::FileUtils::urlDecode(uri));
            }

           return urls;
        }
        if(options()->fileMode() != QFileDialogOptions::AnyFile || urlList.length() > 0)
            return urlList;
        else if(urlList.length() == 0)
        {
            QString path = mKyFileDialog->getCurrentUri();
            if(path.isEmpty())
                return urlList;
            if(!path.endsWith("/"))
                path += "/";
            pDebug << "selected filesss...:" << path + mKyFileDialog->copyEditText();
            urlList.append(QUrl(path + mKyFileDialog->copyEditText()));
            return urlList;
        }
    }
    else
    {
        QList<QUrl> urls;
        QString path = mKyFileDialog->getCurrentUri();
        if(path.isEmpty())
            return urls;
        if(!path.endsWith("/"))
            path += "/";
        path += mKyFileDialog->copyEditText();
        if(path.startsWith("file:///"))
            path = path.remove(0,7);
        pDebug << "save selectedFiles00000..........:" << path;
        urls << QUrl::fromLocalFile(Peony::FileUtils::urlDecode(path));
        pDebug << "save selectedFiles2222..........:" << urls;
        return urls;
    }
}

void KyFileDialogHelper::setFilter()
{
}

void KyFileDialogHelper::selectNameFilter(const QString &filter)
{
    pDebug << "helper selectNameFilter......" << filter;
    mKyFileDialog->selectNameFilter(filter);
}

void KyFileDialogHelper::selectMimeTypeFilter(const QString &filter)
{
    pDebug << "selectMimeTypeFilter0000001111....." << filter;
    QMimeDatabase db;
    QMimeType mimetype = db.mimeTypeForName(filter);
    if(!mimetype.isValid())
        return;
    mKyFileDialog->selectNameFilter(mimetype.filterString());
}

QString KyFileDialogHelper::selectedNameFilter() const
{
    return mKyFileDialog->selectedNameFilter();
}

QString KyFileDialogHelper::selectedMimeTypeFilter() const
{
    pDebug << "selectMimeTypeFilter0000002222222......";
    return QString();
}

bool KyFileDialogHelper::isSupportedUrl(const QUrl &url) const
{
    pDebug << "options initialdirectory...." << options()->initialDirectory();
    pDebug << "options selectfiles..." << options()->initiallySelectedFiles();
    pDebug << "isSupportedUrl..." << url << url.isLocalFile();
    pDebug << "options windowTitle" << options()->windowTitle();
    if(!mKyFileDialog->isVisible()){
        return false;
    }
//    return false;
    return url.isLocalFile();

}
