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
 * Authors: Meihong He <hemeihong@kylinos.cn>
 *
 */

#include "directory-view-container.h"
#include "directory-view-plugin-iface.h"
#include "directory-view-plugin-iface2.h"
#include "directory-view-widget.h"
#include "directory-view-factory-manager.h"
#include "file-utils.h"
#include "global-settings.h"

#include "file-label-model.h"

#include "directory-view-factory-manager.h"

#include "file-item-proxy-filter-sort-model.h"
#include "global-settings.h"
#include "file-info.h"
#include "file-meta-info.h"

#include <QVBoxLayout>
#include <QAction>

#include <QApplication>

using namespace Peony;

DirectoryViewContainer::DirectoryViewContainer(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);

    m_model = new FileItemModel(this);
    m_proxy_model = new FileItemProxyFilterSortModel(this);
    m_proxy_model->setSourceModel(m_model);

    //m_proxy = new DirectoryView::StandardViewProxy;

    setContentsMargins(0, 0, 0, 0);
    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setLayout(m_layout);

//    connect(m_proxy, &DirectoryViewProxyIface::viewDirectoryChanged,
//            this, &DirectoryViewContainer::directoryChanged);

//    connect(m_proxy, &DirectoryViewProxyIface::viewSelectionChanged,
//            this, &DirectoryViewContainer::selectionChanged);

//    connect(m_proxy, &DirectoryViewProxyIface::menuRequest,
//            this, &DirectoryViewContainer::menuRequest);
    connect(m_model, &FileItemModel::changePathRequest, this, &DirectoryViewContainer::signal_responseUnmounted);

    this->setProperty("statusBarUpdate", false);
    connect(m_model, &FileItemModel::updated, [=](){
        if(this->property("statusBarUpdate").isValid() && this->property("statusBarUpdate").toBool() == false){
            this->setProperty("statusBarUpdate", true);
            QTimer::singleShot(400, this, [=](){
                Q_EMIT this->statusBarChanged();
                this->setProperty("statusBarUpdate", false);
            });
        }
    });

//    connect(FileLabelModel::getGlobalModel(), &FileLabelModel::dataChanged, this, [=](){
//        refresh();
//    });

    if (QGSettings::isSchemaInstalled("org.lingmo.control-center.panel.plugins")) {
        m_control_center_plugin = new QGSettings("org.lingmo.control-center.panel.plugins", QByteArray(), this);
        connect(m_control_center_plugin, &QGSettings::changed, this, [=](const QString &key) {
           qDebug() << "panel settings changed:" <<key;
           if (getView()->viewId() == "List View" && (key == "date" || key == "hoursystem"))
              refresh();
        });
    }

    connect(DirectoryViewFactoryManager2::getInstance(), &DirectoryViewFactoryManager2::updateViewEnable, this, [=](const QString &name, DirectoryViewPluginIface2* factory, const bool enable){
        QString currentUri = getCurrentUri();

        int zoomLevel = -1;

        if (currentUri.isNull())
            return;

        auto viewId = DirectoryViewFactoryManager2::getInstance()->getDefaultViewId(zoomLevel, currentUri);
        switchViewType(viewId);

        //update status bar zoom level
        updateStatusBarSliderStateRequest();
        if (zoomLevel < 0)
            zoomLevel = getView()->currentZoomLevel();

        setZoomLevelRequest(zoomLevel);
        //qDebug() << "setZoomLevelRequest:" <<zoomLevel;
        if (m_view)
            m_view->setCurrentZoomLevel(zoomLevel);
    });
}

DirectoryViewContainer::~DirectoryViewContainer()
{
//    m_proxy->closeProxy();
//    if (m_proxy->getView())
//        m_proxy->getView()->closeView();
}

const QStringList DirectoryViewContainer::getBackList()
{
    QStringList l;
    for (auto uri : m_back_list) {
        l<<uri;
    }
    return l;
}

const QStringList DirectoryViewContainer::getForwardList()
{
    QStringList l;
    for (auto uri : m_forward_list) {
        l<<uri;
    }
    return l;
}

bool DirectoryViewContainer::canGoBack()
{
    return !m_back_list.isEmpty();
}

void DirectoryViewContainer::goBack()
{
    if (!canGoBack())
        return;

    auto uri = m_back_list.takeLast();
    //avoid same uri add twice
    int count = m_forward_list.count();
    if (count <= 0 || m_forward_list.at(0) != getCurrentUri())
        m_forward_list.prepend(getCurrentUri());
    Q_EMIT updateWindowLocationRequest(uri, false);
}

bool DirectoryViewContainer::canGoForward()
{
    return !m_forward_list.isEmpty();
}

void DirectoryViewContainer::goForward()
{
    if (!canGoForward())
        return;
    qDebug() << "m_back_list.append goForward:"<<getCurrentUri();
    auto uri = m_forward_list.takeFirst();
    //avoid same uri add twice
    int count = m_back_list.count();
    if (! getCurrentUri().contains("search://") &&
        (count <= 0 || m_back_list.at(count-1) != getCurrentUri()))
        m_back_list.append(getCurrentUri());

    Q_EMIT updateWindowLocationRequest(uri, false);
}

bool DirectoryViewContainer::canCdUp()
{
    if (!m_view)
        return false;

    if("label:///" == FileUtils::getParentUri(m_view->getDirectoryUri()))/* 全局标记页面‘上一级’菜单置灰 */
        return false;

    return !FileUtils::getParentUri(m_view->getDirectoryUri()).isNull();
}

void DirectoryViewContainer::cdUp()
{
    if (!canCdUp())
        return;

    auto uri = FileUtils::getParentUri(m_view->getDirectoryUri());
    if (uri.isNull())
        return;

    Q_EMIT updateWindowLocationRequest(uri, true);
}

void DirectoryViewContainer::setSortFilter(int FileTypeIndex, int FileMTimeIndex, int FileSizeIndex)
{
    qDebug()<<"setSortFilter:"<<FileTypeIndex<<"MTime:"<<FileMTimeIndex<<"size:"<<FileSizeIndex;
    m_proxy_model->setFilterConditions(FileTypeIndex, FileMTimeIndex, FileSizeIndex);
}

void DirectoryViewContainer::setFilterLabelConditions(QString name)
{
    m_proxy_model->setFilterLabelConditions(name);
}

void DirectoryViewContainer::setShowHidden(bool showHidden)
{
    m_proxy_model->setShowHidden(showHidden);
}

void DirectoryViewContainer::setShowFileExtensions(bool show)
{
    m_model->setShowFileExtensions(show);
}

void DirectoryViewContainer::setUseDefaultNameSortOrder(bool use)
{
    m_proxy_model->setUseDefaultNameSortOrder(use);
}

void DirectoryViewContainer::setSortFolderFirst(bool folderFirst)
{
    m_proxy_model->setFolderFirst(folderFirst);
}

void DirectoryViewContainer::addFilterCondition(int option, int classify, bool updateNow)
{
    m_proxy_model->addFilterCondition(option, classify, updateNow);
}

void DirectoryViewContainer::addFileNameFilter(QString key, bool updateNow)
{
    m_proxy_model->addFileNameFilter(key, updateNow);
}

void DirectoryViewContainer::removeFilterCondition(int option, int classify, bool updateNow)
{
    m_proxy_model->removeFilterCondition(option, classify, updateNow);
}

void DirectoryViewContainer::clearConditions()
{
    m_proxy_model->clearConditions();
}

void DirectoryViewContainer::updateFilter()
{
    m_proxy_model->update();
}

void DirectoryViewContainer::goToUri(const QString &uri, bool addHistory, bool forceUpdate)
{
    int zoomLevel = -1;
    if (m_view)
        zoomLevel = m_view->currentZoomLevel();

    if (forceUpdate)
        goto update;

    if (uri.isNull())
        return;

    if (getCurrentUri() == uri)
        return;

update:
    if (addHistory) {
        m_forward_list.clear();
        QString curUri = getCurrentUri();
        qDebug() << "getCurrentUri():" <<curUri<<uri;
        //fix bug 41094, avoid go back to same path issue
        if (! curUri.startsWith("search://")
            && !FileUtils::isSamePath(curUri, uri)) {
            if(getCurrentUri().startsWith("label:///") && FileUtils::getTargetUri(getCurrentUri()) == uri){
                m_back_list.append(FileUtils::getTargetUri(getCurrentUri()));/* 解决标记路径进入文件夹内，后退不了问题 */
            }else{
                qDebug() << "m_back_list.append first:"<<curUri;
                m_back_list.append(getCurrentUri());
            }
        }else if(curUri.startsWith("search://")){
            //process remeber search record,only remeber the last search history,relate to bug#94229
            if (m_back_list.length() > 0 ){
                QString preHistory = m_back_list.last();
                //如果已经有一条搜索记录，需要先去掉，再加入最近的搜索记录
                if (preHistory.startsWith("search://")){
                    m_back_list.takeLast();
                }
            }
            qDebug() << "m_back_list.append second:"<<curUri;
            m_back_list.append(curUri);
        }
    }

    auto viewId = DirectoryViewFactoryManager2::getInstance()->getDefaultViewId(zoomLevel, uri);
    switchViewType(viewId);

    //update status bar zoom level
    updateStatusBarSliderStateRequest();
    if (zoomLevel < 0)
        zoomLevel = getView()->currentZoomLevel();

    setZoomLevelRequest(zoomLevel);
    //qDebug() << "setZoomLevelRequest:" <<zoomLevel;
    if (m_view)
        m_view->setCurrentZoomLevel(zoomLevel);

    m_current_uri = uri;

    //special uri process
    if (m_current_uri.endsWith("/."))
        m_current_uri = m_current_uri.left(m_current_uri.length()-2);
    if (m_current_uri.endsWith("/.."))
        m_current_uri = m_current_uri.left(m_current_uri.length()-3);

    if (m_view) {
        m_view->setDirectoryUri(m_current_uri);
        m_view->beginLocationChange();
    }

    updatePreviewPageRequest();
    m_proxy_model->checkSortSettings();
}

void DirectoryViewContainer::switchViewType(const QString &viewId)
{
    /*
    if (!m_proxy)
        return;

    if (m_proxy->getView()) {
        if (viewId == m_proxy->getView()->viewId())
            return;
    }
    */

    if (getView()) {
        if (getView()->viewId() == viewId)
            return;
    }

    auto viewManager = DirectoryViewFactoryManager2::getInstance();
    auto factory = viewManager->getFactory(viewId);
    if (!factory)
        return;

//    auto settings = GlobalSettings::getInstance();
//    auto sortType = settings->isExist(SORT_COLUMN)? settings->getValue(SORT_COLUMN).toInt() : 0;
//    auto sortOrder = settings->isExist(SORT_ORDER)? settings->getValue(SORT_ORDER).toInt() : 0;

    auto oldView = m_view;
    QStringList selection;
    if (oldView) {
        //fix switch to computer view and back change to default sort issue, link to bug#92261
        //修复列表视图下改变排序，切换为计算机视图再切回来，排序设置恢复默认方式问题
//        sortType = oldView->getSortType();
//        sortOrder = oldView->getSortOrder();
        selection = oldView->getSelections();
        m_layout->removeWidget(dynamic_cast<QWidget*>(oldView));
        oldView->deleteLater();
    }
    auto view = factory->create();
    m_view = view;
    view->setParent(this);
    //connect the view's signal.
    view->bindModel(m_model, m_proxy_model);
    //view->setProxy(m_proxy);

    //fix go to root path issue after refresh
    view->setDirectoryUri(getCurrentUri());

//    view->setSortType(sortType);
//    view->setSortOrder(sortOrder);

    connect(m_view, &DirectoryViewWidget::menuRequest, this, &DirectoryViewContainer::menuRequest);
    connect(m_view, &DirectoryViewWidget::viewDirectoryChanged, this, [=](){
        if (DirectoryViewFactoryManager2::getInstance()->internalViews().contains(m_view->viewId())) {
            auto dirInfo = FileInfo::fromUri(m_current_uri);
            if (dirInfo.get()->isEmptyInfo() && !dirInfo.get()->uri().startsWith("search://") && !dirInfo.get()->uri().startsWith("label://")) {
                goBack();
                if (!m_forward_list.isEmpty())
                    m_forward_list.takeFirst();
            } else {
                Q_EMIT this->directoryChanged();
            }
        } else {
            Q_EMIT this->directoryChanged();
        }
    });
    connect(m_view, &DirectoryViewWidget::viewDoubleClicked, this, &DirectoryViewContainer::viewDoubleClicked);
    connect(m_view, &DirectoryViewWidget::viewDoubleClicked, this, &DirectoryViewContainer::onViewDoubleClicked);
    connect(m_view, &DirectoryViewWidget::viewSelectionChanged, this, &DirectoryViewContainer::selectionChanged);

    connect(m_view, &DirectoryViewWidget::zoomRequest, this, &DirectoryViewContainer::zoomRequest);

    connect(m_view, &DirectoryViewWidget::updateWindowSelectionRequest, this, &DirectoryViewContainer::updateWindowSelectionRequest);

    //similar to double clicked, but just jump directory only.
    //note that if view use double clicked signal, this signal should
    //not sended again.
    connect(m_view, &DirectoryViewWidget::updateWindowLocationRequest, this, [=](const QString &uri) {
        Q_EMIT this->updateWindowLocationRequest(uri, true);
    });
    connect(m_view, &DirectoryViewWidget::signal_itemAdded, this, [=](const QString& uri) {
        Q_EMIT this->signal_itemAdded(uri);
    });

    //m_proxy->switchView(view);
    m_layout->addWidget(dynamic_cast<QWidget*>(view), Qt::AlignBottom);

    if (this->topLevelWidget()->objectName() == "_explorer_mainwindow") {
        DirectoryViewFactoryManager2::getInstance()->setDefaultViewId(viewId);
    }

    if (!selection.isEmpty()) {
        view->setSelections(selection);
    }

    QAction *cdUpAction = new QAction(m_view);
    cdUpAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::ALT + Qt::Key_Up));
    connect(cdUpAction, &QAction::triggered, this, [=]() {
        this->cdUp();
    });
    this->addAction(cdUpAction);

    QAction *goBackAction = new QAction(m_view);
    goBackAction->setShortcut(QKeySequence::Back);
    connect(goBackAction, &QAction::triggered, this, [=]() {
        this->goBack();
    });
    this->addAction(goBackAction);

    QAction *goForwardAction = new QAction(m_view);
    goForwardAction->setShortcut(QKeySequence::Forward);
    connect(goForwardAction, &QAction::triggered, this, [=]() {
        this->goForward();
    });
    this->addAction(goForwardAction);

    QAction *editAction = new QAction(m_view);
    editAction->setShortcuts(QList<QKeySequence>()<<QKeySequence(Qt::ALT + Qt::Key_E)<<Qt::Key_F2);
    connect(editAction, &QAction::triggered, this, [=]() {
        auto selections = m_view->getSelections();

        bool hasStandardPath = FileUtils::containsStandardPath(selections);
        bool isSearchTab = m_view->getDirectoryUri().startsWith("search:///");
        if (selections.count() == 1 && !hasStandardPath) {
            QString one = selections.first();
            if(one.startsWith("filesafe:///") && one.remove("filesafe:///").indexOf("/") == -1 || one.startsWith("label://")) {
                return ;
            }
            //修复在选中文件不可见时，重命名操作不会跳转显示重命名文件问题，link to bug#160799
            m_view->scrollToSelection(selections.first());
            m_view->editUri(selections.first());
        } else if (selections.count() > 1 && !hasStandardPath && !isSearchTab) {/* 搜索tab页面中暂时不允许执行批量重命名操作 */
            for (auto uri : selections) {
                QString one = uri;
                if(one.startsWith("filesafe:///") && one.remove("filesafe:///").indexOf("/") == -1) {
                    return ;
                }
            }
            m_view->editUris(selections);
        }
    });
    this->addAction(editAction);

    Q_EMIT viewTypeChanged();
}

void DirectoryViewContainer::refresh()
{
    if (!m_view)
        return;
    m_view->beginLocationChange();
    m_proxy_model->checkSortSettings();
}

void DirectoryViewContainer::bindNewProxy(DirectoryViewProxyIface *proxy)
{
    //disconnect old proxy
    //connect new proxy
}

const QStringList DirectoryViewContainer::getCurrentSelections()
{
    if (m_view)
        return m_view->getSelections();
    return QStringList();
}

const int DirectoryViewContainer::getCurrentRowcount()
{
    return m_model->rowCount(QModelIndex());
}

const QString DirectoryViewContainer::getCurrentUri()
{
    if (m_view) {
        return m_view->getDirectoryUri();
    }
    return nullptr;
}

const QStringList DirectoryViewContainer::getAllFileUris()
{
    if (m_view)
        return m_view->getAllFileUris();
    return QStringList();
}

void DirectoryViewContainer::stopLoading()
{
    if (m_view) {
        m_view->stopLocationChange();
        Q_EMIT this->directoryChanged();
    }
}

void DirectoryViewContainer::tryJump(int index)
{
    QStringList l;
    l<<m_back_list<<getCurrentUri()<<m_forward_list;
    if (0 <= index && index < l.count()) {
        auto targetUri = l.at(index);
        m_back_list.clear();
        m_forward_list.clear();
        for (int i = 0; i < l.count(); i++) {
            if (i < index) {
                m_back_list<<l.at(i);
            }
            if (i > index) {
                m_forward_list<<l.at(i);
            }
        }
        Q_EMIT updateWindowLocationRequest(targetUri, false, true);
    }
}

FileItemModel::ColumnType DirectoryViewContainer::getSortType()
{
    if (!m_view)
        return FileItemModel::FileName;
    int type = m_view->getSortType();
    return FileItemModel::ColumnType(type);
}

void DirectoryViewContainer::setSortType(FileItemModel::ColumnType type)
{
    if (!m_view)
        return;

    if (this->topLevelWidget()->objectName() == "_explorer_mainwindow") {
        if (Peony::GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
            Peony::GlobalSettings::getInstance()->setValue(SORT_COLUMN, type);
        } else {
            auto metaInfo = FileMetaInfo::fromUri(getCurrentUri());
            if (metaInfo) {
                metaInfo->setMetaInfoVariant(SORT_COLUMN, type);
            } else {
                qCritical()<<"can not set meta info";
            }
        }
    }

    m_view->setSortType(type);
    //Peony::GlobalSettings::getInstance()->setValue (SORT_TYPE, type);
}

Qt::SortOrder DirectoryViewContainer::getSortOrder()
{
    if (!m_view)
        return Qt::AscendingOrder;
    int order = m_view->getSortOrder();
    return Qt::SortOrder(order);
}

void DirectoryViewContainer::setSortOrder(Qt::SortOrder order)
{
    if (order < 0)
        return;
    if (!m_view)
        return;

    if (this->topLevelWidget()->objectName() == "_explorer_mainwindow") {
        if (Peony::GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool()) {
            Peony::GlobalSettings::getInstance()->setValue(SORT_ORDER, order);
        } else {
            auto metaInfo = FileMetaInfo::fromUri(getCurrentUri());
            if (metaInfo) {
                metaInfo->setMetaInfoVariant(SORT_ORDER, order);
            } else {
                qCritical()<<"can not set meta info";
            }
        }
    }

    m_view->setSortOrder(order);
}

void DirectoryViewContainer::onViewDoubleClicked(const QString& uri)
{

}

void DirectoryViewContainer::setSelectionMode(QAbstractItemView::SelectionMode mode)
{
    m_proxy_model->setSelectionModeHint(mode);
}

void DirectoryViewContainer::updateCurrentFilesThumbnails()
{
    m_model->updateCurrentFilesThumbnails();
}

void DirectoryViewContainer::addFileDialogFiltersCondition(const QStringList &mimeTypeFilters, const QStringList &nameFilters, QDir::Filters dirFilters, Qt::CaseSensitivity caseSensitivity)
{
    if (m_proxy_model) {
        m_proxy_model->setFilterConditions(mimeTypeFilters, nameFilters, dirFilters, caseSensitivity);
    }
}
