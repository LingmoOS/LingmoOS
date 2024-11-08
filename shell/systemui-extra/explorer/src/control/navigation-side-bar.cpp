/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#include "navigation-side-bar.h"
#include "side-bar-model.h"
#include "side-bar-proxy-filter-sort-model.h"
#include "side-bar-abstract-item.h"
#include "volume-manager.h"
#include "volumeManager.h"
#include "side-bar-menu.h"
#include "side-bar-abstract-item.h"
#include "bookmark-manager.h"
#include "file-info.h"
#include "file-info-job.h"
#include "main-window.h"

#include "global-settings.h"

#include "file-enumerator.h"
#include "gerror-wrapper.h"

#include "file-utils.h"
#include "plugin-manager.h"
#include "vfs-plugin-manager.h"

#include "x11-window-manager.h"
#include "tag-management.h"

#include <QHeaderView>
#include <QPushButton>

#include <QVBoxLayout>

#include <QEvent>

#include <QPainter>

#include <QScrollBar>

#include <QKeyEvent>

#include <QUrl>
#include <QDropEvent>
#include <QMimeData>

#include <QTimer>
#include <QMessageBox>

#include <QPainterPath>
#include <QLabel>

#include <QDebug>
#include <QToolTip>
#include <QtConcurrent>
#include <QStandardPaths>
#include <QApplication>

#include <QTextLayout>
#include <QTextLine>

#ifdef LINGMO_SDK_DATACOLLECT
#include <lingmosdk/diagnosetest/libkydatacollect.h>
#endif

#define NAVIGATION_SIDEBAR_ITEM_BORDER_RADIUS 4
#define MINIMUM_COLUMN_SIZE 2

using namespace Peony;

NavigationSideBar::NavigationSideBar(QWidget *parent) : QTreeView(parent)
{
    static NavigationSideBarStyle *global_style = new NavigationSideBarStyle;

    header()->setMinimumSectionSize(30);

    setSortingEnabled(true);

    setIconSize(QSize(16, 16));

    setProperty("useIconHighlightEffect", true);
    //both default and highlight.
    setProperty("iconHighlightEffectMode", 1);

    this->verticalScrollBar()->setProperty("drawScrollBarGroove", false);

    setDragDropMode(QTreeView::DropOnly);

    setProperty("highlightMode", true);
    setProperty("doNotBlur", true);
    viewport()->setProperty("doNotBlur", true);

    //auto delegate = new NavigationSideBarItemDelegate(this);
    //setItemDelegate(delegate);

    installEventFilter(this);

    setStyleSheet(".NavigationSideBar"
                  "{"
                  "border: 0px solid transparent"
                  "}");

    //setStyle(global_style);

    setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAttribute(Qt::WA_TranslucentBackground);
    header()->setSectionResizeMode(QHeaderView::Fixed);
    header()->setStretchLastSection(true);
    header()->setMinimumSectionSize(MINIMUM_COLUMN_SIZE);
    header()->hide();

    setContextMenuPolicy(Qt::CustomContextMenu);

    setExpandsOnDoubleClick(false);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_model = new Peony::SideBarModel(this);
    m_proxy_model = new Peony::SideBarProxyFilterSortModel(this);
    m_proxy_model->setSourceModel(m_model);

    this->setModel(m_proxy_model);

    setMouseTracking(true);//追踪鼠标
    setAutoScrollMargin(0);

    VolumeManager *volumeManager = VolumeManager::getInstance();
    connect(volumeManager,&Peony::VolumeManager::volumeAdded,this,[=](const std::shared_ptr<Peony::Volume> &volume){
        m_proxy_model->invalidate();//display DVD device in real time.
    });
    connect(volumeManager,&Peony::VolumeManager::volumeRemoved,this,[=](const std::shared_ptr<Peony::Volume> &volume){
        m_proxy_model->invalidate();//The drive does not display when the DVD device is removed.
        //qDebug() << "volumeRemoved:" <<QToolTip::text();
        //fix abnormal pull out usb device tips not hide issue, link to bug#81190
        if (QToolTip::isVisible()) {
            QToolTip::hideText();
        }
    });
    connect(volumeManager,&Peony::VolumeManager::driveDisconnected,this,[=](const std::shared_ptr<Peony::Drive> &drive){
        m_proxy_model->invalidate();//Multiple udisk eject display problem
    });
        connect(volumeManager,&Peony::VolumeManager::mountAdded,this,[=](const std::shared_ptr<Peony::Mount> &mount){
        m_proxy_model->invalidate();//display udisk in real time after format it.
    });

    connect(this, &QTreeView::expanded, [=](const QModelIndex &index) {
        auto item = m_proxy_model->itemFromIndex(index);
        qDebug()<<item->uri();
        /*!
          \bug can not expanded? enumerator can not get prepared signal, why?
          */
        bool isShowNetwork = Peony::GlobalSettings::getInstance()->isExist(SHOW_NETWORK) ?
                    Peony::GlobalSettings::getInstance()->getValue(SHOW_NETWORK).toBool() : true;
        if (item->type() == SideBarAbstractItem::NetWorkItem && !isShowNetwork) {
            this->setRowHidden(index.row(), index.parent(), true);
            return;
        }

        item->findChildrenAsync();
    });

    connect(this, &QTreeView::collapsed, [=](const QModelIndex &index) {
        auto item = m_proxy_model->itemFromIndex(index);
        item->clearChildren();
    });

    connect(Experimental_Peony::VolumeManager::getInstance(), &Experimental_Peony::VolumeManager::signal_mountFinished,this,[=](){
        //fix open mutiple explorer window, mount in side bar crash issue, link to bug#116201,116589
        if(!m_currSelectedItem)
            return;
        //规避182166的情况
//        JumpDirectory(m_currSelectedItem->uri());
//        qDebug()<<"挂载后跳转路径："<<m_currSelectedItem->uri();
    });

    connect(this, &QTreeView::clicked, [=](const QModelIndex &index) {
        switch (index.column()) {
        case 0: {
            m_currSelectedItem = m_proxy_model->itemFromIndex(index);
            //fix open mutiple explorer window, mount in side bar crash issue, link to bug#116201,116589
            if (! m_currSelectedItem)
                break;
            //continue to fix crash issue, related to bug#116201,116589
            m_currSelectedItem->connect(m_currSelectedItem, &SideBarAbstractItem::destroyed, this, [=]{
                m_currSelectedItem = nullptr;
            });
            if(m_currSelectedItem->isMountable()&&!m_currSelectedItem->isMounted())
                m_currSelectedItem->mount();
            else{
                JumpDirectory(m_currSelectedItem->uri());
#ifdef LINGMO_SDK_DATACOLLECT
                auto parent = index.parent();
                //三级或以下子树跳转才收集数据，分析侧边栏树结构用户使用率
                if (parent.isValid() && parent.parent().isValid()){
                    sendKdkDataAsync();
                }
#endif
            }
            break;

        }
        case 1: {
            auto item = m_proxy_model->itemFromIndex(index);
            if (item->isMounted() || item->isEjectable()||item->isStopable()) {
                if(item->getDevice().contains("/dev/sr") && FileUtils::isBusyDevice(item->getDevice())){/* 光盘在刻录数据、镜像等操作时,若处于busy状态时，不可弹出。link to bug#143293 */
                    QMessageBox::information(this, tr("Tips"), tr("The device is in busy state, please perform this operation later."));
                    break;
                }
                auto leftIndex = m_proxy_model->index(index.row(), 0, index.parent());
                this->collapse(leftIndex);
                item->ejectOrUnmount();
            } else {
                // if item is not unmountable, just be same with first column.
                // fix #39716
                if (!item->uri().isNull()) {
                    Q_EMIT this->updateWindowLocationRequest(item->uri());
                }
            }
            break;
        }
        default:
            break;
        }
    });

    connect(this, &QTreeView::customContextMenuRequested, this, [=](const QPoint &pos) {
        auto index = indexAt(pos);
        auto item = m_proxy_model->itemFromIndex(index);
        if (item) {
            if (item->type() != Peony::SideBarAbstractItem::SeparatorItem) {
                Peony::SideBarMenu menu(item, nullptr, this);
                QList<QAction *> actionList;
                MainWindow *window = dynamic_cast<MainWindow *>(this->topLevelWidget());

                //fix bug#216256, open data block in menu fail issue
                auto curUri = item->uri();
                if (item->uri() == "computer:///lingmo-data-volume") {
                    curUri = "file:///data";
                }

                actionList << menu.addAction(QIcon::fromTheme("window-new-symbolic"), tr("Open In New Window"), [=](){
                    auto enumerator = new Peony::FileEnumerator;
                    enumerator->setEnumerateDirectory(curUri);
                    enumerator->setAutoDelete();

                    enumerator->connect(enumerator, &Peony::FileEnumerator::prepared, this, [=](const std::shared_ptr<Peony::GErrorWrapper> &err = nullptr, const QString &t = nullptr, bool critical = false){
                        auto targetUri = Peony::FileUtils::getTargetUri(curUri);
                        if (!targetUri.isEmpty()) {
                            auto enumerator2 = new Peony::FileEnumerator;
                            enumerator2->setEnumerateDirectory(targetUri);
                            enumerator2->connect(enumerator2, &Peony::FileEnumerator::prepared, this, [=](const std::shared_ptr<Peony::GErrorWrapper> &err = nullptr, const QString &t = nullptr, bool critical = false){
                                if (!critical) {
                                    auto newWindow = window->create(targetUri);
                                    dynamic_cast<QWidget *>(newWindow)->show();
                                } else {
                                    auto info = FileInfo::fromUri(targetUri);
                                    QMessageBox::critical(0, 0, tr("Can not open %1, %2").arg(info.get()->displayName()).arg(err.get()->message()));
                                }
                                enumerator2->deleteLater();
                            });
                            enumerator2->prepare();
                        } else if (!err.get() && !critical) {
                            auto newWindow = window->create(curUri);
                            dynamic_cast<QWidget *>(newWindow)->show();
                        }
                    });

                    enumerator->connect(enumerator, &Peony::FileEnumerator::prepared, [=](){
                        enumerator->deleteLater();
                    });

                    enumerator->prepare();
                });
                if (!qApp->property("tabletMode").toBool()) {
                    actionList << menu.addAction(QIcon::fromTheme("tab-new-symbolic"), tr("Open In New Tab"), [=](){
                        auto enumerator = new Peony::FileEnumerator;
                        enumerator->setEnumerateDirectory(item->uri());
                        enumerator->setAutoDelete();

                        enumerator->connect(enumerator, &Peony::FileEnumerator::prepared, this, [=](const std::shared_ptr<Peony::GErrorWrapper> &err = nullptr, const QString &t = nullptr, bool critical = false){
                            auto targetUri = Peony::FileUtils::getTargetUri(item->uri());
                            if (!targetUri.isEmpty()) {
                                auto enumerator2 = new Peony::FileEnumerator;
                                enumerator2->setEnumerateDirectory(targetUri);
                                enumerator2->connect(enumerator2, &Peony::FileEnumerator::prepared, this, [=](const std::shared_ptr<Peony::GErrorWrapper> &err = nullptr, const QString &t = nullptr, bool critical = false){
                                    if (!critical) {
                                        window->addNewTabs(QStringList()<<targetUri);
                                        dynamic_cast<QWidget *>(window)->show();
                                    } else {
                                        auto info = FileInfo::fromUri(targetUri);
                                        QMessageBox::critical(0, 0, tr("Can not open %1, %2").arg(info.get()->displayName()).arg(err.get()->message()));
                                    }
                                    enumerator2->deleteLater();
                                });
                                enumerator2->prepare();
                            } else if (!err.get() && !critical) {
                                window->addNewTabs(QStringList()<<item->uri());
                                dynamic_cast<QWidget *>(window)->show();
                            }
                        });

                        enumerator->connect(enumerator, &Peony::FileEnumerator::prepared, [=](){
                            enumerator->deleteLater();
                        });

                        enumerator->prepare();
                    });
                }

                if (item->type() == SideBarAbstractItem::FileSystemItem) {
                    if ((0 != QString::compare(item->uri(), "computer:///")) &&
                        (0 != QString::compare(item->uri(), "filesafe:///"))) {
                        for (const auto &actionItem : actionList) {
                            if(item->isVolume()){/* 分区才去需要判断是否已挂载 */
                                actionItem->setEnabled(item->isMounted());
                                if(item->getDevice().contains("/dev/sr")){/* 光盘在刻录数据、镜像等操作时,即若处于busy状态时，该菜单置灰不可用。 */
                                    actionItem->setDisabled(FileUtils::isBusyDevice(item->getDevice()));
                                }
                            }
                        }
                    }
                }

                menu.exec(mapToGlobal(pos));
            }
        }
    });

    connect(m_model, &QAbstractItemModel::dataChanged, this, [=](){
        this->viewport()->update();
        m_proxy_model->invalidate();
    });

    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &QTreeView::doItemsLayout);

    connect(Peony::GlobalSettings::getInstance(), &GlobalSettings::valueChanged, this, [=](const QString& key){
        if (SHOW_NETWORK == key) {
            for (int i = 0; i < m_proxy_model->rowCount(); ++i) {
                auto index = m_proxy_model->index(i, 0);
                auto item = m_proxy_model->itemFromIndex(index);
                bool isShowNetwork = Peony::GlobalSettings::getInstance()->isExist(SHOW_NETWORK) ?
                            Peony::GlobalSettings::getInstance()->getValue(SHOW_NETWORK).toBool() : true;
                if (item->type() == SideBarAbstractItem::NetWorkItem) {
                    this->setRowHidden(index.row(), index.parent(), !isShowNetwork);
                    if (!isShowNetwork) {
                        item->findChildrenAsync();
                    }
                    return;
                }
            }
            this->viewport()->update();
        }
    });

    connect(VFSPluginManager::getInstance(), &VFSPluginManager::updateVFSPlugin, this, [=](VFSPluginIface *vfsPIface, bool enable){
        qDebug() << __func__ << vfsPIface->name() << enable;
        for (int i = 0; i < m_proxy_model->rowCount(); ++i) {
            auto index = m_proxy_model->index(i, 0);
            auto item = m_proxy_model->itemFromIndex(index);
            if (item->type() == SideBarAbstractItem::VFSItem
                    && item->uri().contains(vfsPIface->uriScheme())
                    && !item->uri().contains("computer:///")
                    && vfsPIface->pluginType() == PluginInterface::VFSPlugin) {
                if (m_currSelectedItem) {
                    if (!m_currSelectedItem->uri().startsWith("file://") && enable) {
                        JumpDirectory("computer:///");
                    }
                }
                this->setRowHidden(index.row(), index.parent(), enable);
                return;
            }
            //hide kmre
            if (item->type() == SideBarAbstractItem::FavoriteItem
                    && vfsPIface->uriScheme() == "kmre://"
                    && vfsPIface->pluginType() == PluginInterface::VFSPlugin) {
                if (m_currSelectedItem) {
                    if (!m_currSelectedItem->uri().startsWith("file:///") && enable) {
                        JumpDirectory("computer:///");
                    }
                }

                for (int j = 0; j < m_proxy_model->rowCount(index); ++j) {
                    auto tIndex = m_proxy_model->index(j, 0, index);
                    auto tItem = m_proxy_model->itemFromIndex(tIndex);
                    if (tItem->uri().startsWith("kmre:///")) {
                        this->setRowHidden(tIndex.row(), tIndex.parent(), enable);
                        return;
                    }
                }
            }
        }
    });

    connect(m_model, &SideBarModel::signal_collapsedChildren, this, [=](const QModelIndex &index){
        QModelIndex modelIndex = m_proxy_model->mapFromSource(index);
        collapse(modelIndex);
    });

    //expandToDepth(1);/* 快速访问、计算机、网络 各模块往下展开一层 */
    for(int row =0; row < model()->rowCount();row++)
    {
        auto index = model()->index(row,0);
        auto srcIndex = m_proxy_model->mapToSource(index);
        auto item = m_model->itemFromIndex(srcIndex);
        auto type = item->type();
        if (item->type() != SideBarAbstractItem::VFSItem && item->type() != SideBarAbstractItem::SeparatorItem) {
            expand(index);
        }

        QStringList disExtensions = GlobalSettings::getInstance()->getValue(DISABLED_EXTENSIONS).toStringList();
        for (auto extensions : disExtensions) {
            VFSPluginIface *pIface = dynamic_cast<VFSPluginIface*>(PluginManager::getInstance()->getPluginByFileName(extensions));
            if (pIface && pIface->pluginType() == PluginInterface::VFSPlugin
                    && item->type() == SideBarAbstractItem::VFSItem
                    && !item->uri().contains("computer:///")
                    && item->uri().contains(pIface->uriScheme())) {
                this->setRowHidden(index.row(), index.parent(), true);
            }

            if (pIface && pIface->pluginType() == PluginInterface::VFSPlugin
                    && item->type() == SideBarAbstractItem::FavoriteItem
                    && pIface->uriScheme() == "kmre://"
                    && item->uri().contains(pIface->uriScheme())) {
                this->setRowHidden(index.row(), index.parent(), true);
            }
        }

//        if(item->uri()=="filesafe:///")/* 文件保护箱默认不展开 */
//            continue;
//        expand(index);
    }
    /* 打开文件管理器默认聚焦在家目录上 */
    QString homeUri =  "file://" +  QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QItemSelectionModel *selectionModel = this->selectionModel();
    for (int i = 0; i < m_proxy_model->rowCount(); ++i) {
        auto index = m_proxy_model->index(i, 0);
        auto item = m_proxy_model->itemFromIndex(index);
        if (item->uri() == homeUri) {
            auto index1 = m_proxy_model->index(i, 1);
            this->setCurrentIndex(index);
            /* 通过QItemSelection设置选中项 */
            QItemSelection selection(index, index1);
            selectionModel->select(selection, QItemSelectionModel::Select);
        }
    }//end
}

void NavigationSideBar::sendKdkDataAsync()
{
#ifdef LINGMO_SDK_DATACOLLECT
    QtConcurrent::run([=]() {
        //story 24997, collet second level tree child click event
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP,KEVENT_CLICK);
        //传入事件描述“点击侧边栏展开子树”，上传点击事件，收集使用二级以上子树跳转数据
        kdk_dia_upload_default(node,"explorer","secondLevelChildClick");

        //释放内存
        kdk_dia_data_free(node);
    });
#endif
}

bool NavigationSideBar::eventFilter(QObject *obj, QEvent *e)
{
    return false;
}

bool NavigationSideBar::viewportEvent(QEvent *e)
{
    if (e->type() == QEvent::ToolTip) {
        // 处理 ToolTip 事件的代码
        QHelpEvent *helpEvent = static_cast<QHelpEvent*>(e);
        QModelIndex itemIndex = indexAt(helpEvent->pos());
        if (!itemIndex.isValid()) {
            return QTreeView::viewportEvent(e);
        }

        //获取提示文本和index
        QModelIndex parentIndex = itemIndex.parent();
        QModelIndex firstColumnIndex;
        if (parentIndex.isValid()) {
            firstColumnIndex = model()->index(itemIndex.row(), 0, parentIndex);
        } else {
            firstColumnIndex = model()->index(itemIndex.row(), 0);
        }
        QString text = firstColumnIndex.data(Qt::DisplayRole).toString();

        //获取style绘制文本的宽度，小于文本的宽度 则显示
        NavigationSideBarItemDelegate *delegate = static_cast<NavigationSideBarItemDelegate *>(itemDelegate());
        QStyleOptionViewItem opt = viewOptions();
        delegate->initStyleOption(&opt, firstColumnIndex);
        opt.rect = visualRect(firstColumnIndex);
        QRect rect = QApplication::style()->subElementRect(QStyle::SE_ItemViewItemText, &opt, nullptr);
        const QWidget *widget = opt.widget;
        const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, widget) + 1;
        QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding

        QTextLayout textLayout(text, opt.font);

        QTextOption textOption;
        const bool wrapText = opt.features & QStyleOptionViewItem::WrapText;
        textOption.setWrapMode(wrapText ? QTextOption::WordWrap : QTextOption::ManualWrap);
        textOption.setTextDirection(opt.direction);
        textOption.setAlignment(QStyle::visualAlignment(opt.direction, opt.displayAlignment));

        textLayout.setTextOption(textOption);
        textLayout.beginLayout();

        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            return QTreeView::viewportEvent(e);;

        line.setLineWidth(textRect.width());

        const bool isShow = line.naturalTextWidth() > textRect.width();

        textLayout.endLayout();
        //qDebug() << "line.naturalTextWidth(): "  << line.naturalTextWidth() <<"textRect: " <<textRect <<"text: " << text;
        if (!isShow) {
            QToolTip::hideText();
            e->ignore();
            return true;
        }
    }
    return QTreeView::viewportEvent(e);
}

void NavigationSideBar::updateGeometries()
{
    setViewportMargins(8, 0, 4, 0);
    QTreeView::updateGeometries();
    if(m_notAllowHorizontalMove){
        horizontalScrollBar()->setValue(0);/* hotfix bug#93557 */
    }
    m_notAllowHorizontalMove = false;
}

void NavigationSideBar::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    //skip unmount indicator index
    if (index.isValid()) {
        if (index.column() == 0) {
            QTreeView::scrollTo(index, hint);
        }
    }
}

void NavigationSideBar::paintEvent(QPaintEvent *event)
{
    QPalette pal = qApp->palette();
    pal.setColor(QPalette::Base, QColor(Qt::transparent));
    setPalette(pal);
    viewport()->setPalette(pal);
    QTreeView::paintEvent(event);
}

void NavigationSideBar::resizeEvent(QResizeEvent *e)
{
    QTreeView::resizeEvent(e);
    if (header()->count() > 0) {
        setColumnWidth(0, this->viewport()->width() - MINIMUM_COLUMN_SIZE - viewportMargins().left() - viewportMargins().right() - verticalScrollBar()->width());
        setColumnWidth(1, MINIMUM_COLUMN_SIZE);
    }
}

void NavigationSideBar::dropEvent(QDropEvent *e)
{
    //fix invalid index cause crash issue
    if (! indexAt(e->pos()).isValid())
        return;

    QString destUri = m_proxy_model->itemFromIndex(indexAt(e->pos()))->uri();
    auto data = e->mimeData();
    auto bookmark = Peony::BookMarkManager::getInstance();
    if (bookmark->isLoaded()) {
        for (auto url : data->urls()) {
            if(url.toString().startsWith("filesafe:///")){
                QMessageBox::warning(this, tr("warn"), tr("This operation is not supported."));
                continue;
            }
            if (dropIndicatorPosition() == QAbstractItemView::AboveItem || dropIndicatorPosition() == QAbstractItemView::BelowItem || "favorite:///" == destUri) {
                // add to bookmark
                e->setAccepted(true);

                //FIXME: replace BLOCKING api in ui thread.
                auto info = Peony::FileInfo::fromUri(url.toDisplayString());
                if (info->displayName().isNull()) {
                    Peony::FileInfoJob j(info);
                    j.querySync();
                }
                if (info->isDir()) {
                    bookmark->addBookMark(url.url());
                }
            }
        }
    }

    if (e->keyboardModifiers() == Qt::ControlModifier) {
        m_model->dropMimeData(e->mimeData(), Qt::CopyAction, 0, 0, QModelIndex());
        e->accept();
        return;
    }

    QTreeView::dropEvent(e);
}

QSize NavigationSideBar::sizeHint() const
{
    return QTreeView::sizeHint();
}

void NavigationSideBar::JumpDirectory(const QString &uri)
{
    if((uri=="" || uri.startsWith("computer://")) && m_currSelectedItem && m_currSelectedItem->getDevice().startsWith("/dev/sd"))
    {/* 异常U盘 */
        QMessageBox::information(nullptr, tr("Tips"), tr("This is an abnormal Udisk, please fix it or format it"));
        return;
    }

    auto info = FileInfo::fromUri(uri);

    // try fix #174128, explorer stucked when click sidebar network item sometimes.
    if (!uri.startsWith("network:///")) {
        if (info.get()->isEmptyInfo()) {
            FileInfoJob j(info);
            j.querySync();
        }
    }

    auto targetUri = FileUtils::getTargetUri(uri);
    if (targetUri == "" && uri== "burn://")
    {
        qDebug() << "empty drive"<<uri;
        QMessageBox::information(nullptr, tr("Tips"), tr("This is an empty drive, please insert a Disc."));
        return;
    }

    if (!targetUri.isEmpty()) {
        Q_EMIT this->updateWindowLocationRequest(targetUri);
        return;
    }

    // try fixing #133429.
    if (m_currSelectedItem->getDevice().startsWith("/dev/sr") && uri.startsWith("computer://")) {
        return;
    }

    //some side bar item doesn't have a uri.
    //do not emit signal with a null uri to window.
    if (!uri.isNull())
        Q_EMIT this->updateWindowLocationRequest(uri);
}

void NavigationSideBar::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Left||event->key()==Qt::Key_Right)
    {
        m_notAllowHorizontalMove = true;
    }

    QTreeView::keyPressEvent(event);

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Space) {
        if (!selectedIndexes().isEmpty()) {
            auto index = selectedIndexes().first();
            auto uri = index.data(Qt::UserRole).toString();
            Q_EMIT this->updateWindowLocationRequest(uri, true);
        }
    }

    if(event->key() == Qt::Key_Left||event->key()==Qt::Key_Right)
    {/* 按下左右键不可使侧边栏内容左右平移显示 */
        horizontalScrollBar()->setValue(0);/* hotfix bug#93557 */
    }
}

void NavigationSideBar::focusInEvent(QFocusEvent *event)
{
    QTreeView::focusInEvent(event);
    if (event->reason() == Qt::TabFocus) {
        if (selectedIndexes().isEmpty()) {
            selectionModel()->select(model()->index(0, 0), QItemSelectionModel::Select);
            selectionModel()->select(model()->index(0, 1), QItemSelectionModel::Select);
        } else {
            scrollTo(selectedIndexes().first(), QTreeView::PositionAtCenter);
            auto selections = selectedIndexes();
            clearSelection();
            QTimer::singleShot(100, this, [=](){
                for (auto index : selections) {
                    selectionModel()->select(index, QItemSelectionModel::Select);
                }
            });
        }
    }
    GlobalSettings::getInstance()->setValue("LAST_FOCUS_PEONY_WINID", dynamic_cast<MainWindow *>(this->topLevelWidget())->winId());
}

void NavigationSideBar::wheelEvent(QWheelEvent *event)
{
    QTreeView::wheelEvent(event);
    if (event->orientation()==Qt::Horizontal) {
        /* 触摸板左右滑动不可使侧边栏内容左右平移显示 */
        horizontalScrollBar()->setValue(0);/* hotfix bug#93557 */
    }
}

int NavigationSideBar::sizeHintForColumn(int column) const
{
    if (column == 1)
        return MINIMUM_COLUMN_SIZE;

    if (column == 0)
        return viewport()->width() - MINIMUM_COLUMN_SIZE - viewportMargins().left() - viewportMargins().right() - verticalScrollBar()->width();

    return QTreeView::sizeHintForColumn(column);
}

QStyleOptionViewItem NavigationSideBar::viewOptions() const
{
    auto opt = QTreeView::viewOptions();
    auto hoverColor = opt.palette.color(QPalette::BrightText);
    hoverColor.setAlphaF(0.05);
    opt.palette.setBrush(QPalette::Disabled, QPalette::Midlight, hoverColor);
    return opt;
}

void NavigationSideBar::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        if ((event->dropAction() == Qt::MoveAction) && FileUtils::containsStandardPath(event->mimeData()->urls())) {
            event->ignore();
            return;
        }
    }
    QAbstractItemView::dragEnterEvent(event);
}

NavigationSideBarItemDelegate::NavigationSideBarItemDelegate(QObject *parent)
{

}

QSize NavigationSideBarItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto size = QStyledItemDelegate::sizeHint(option, index);
    //task#106007 【文件管理器】文件管理器应用做平板UI适配，修改侧边栏项的高度--48px
    //size.setHeight(36);
    return size;
}

void NavigationSideBarItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
//    if (index.column() == 1) {
//        QPainterPath rightRoundedRegion;
//        rightRoundedRegion.setFillRule(Qt::WindingFill);
//        auto rect = option.rect;
//        auto view = qobject_cast<const QAbstractItemView *>(option.widget);
//        if (view) {
//            rect.setRight(view->viewport()->rect().right());
//        }
//        rightRoundedRegion.addRoundedRect(rect, NAVIGATION_SIDEBAR_ITEM_BORDER_RADIUS, NAVIGATION_SIDEBAR_ITEM_BORDER_RADIUS);
//        rightRoundedRegion.addRect(rect.adjusted(0, 0, -NAVIGATION_SIDEBAR_ITEM_BORDER_RADIUS, 0));
//        //painter->setClipPath(rightRoundedRegion);
//    }

    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QStyledItemDelegate::paint(painter, option, index);
    painter->restore();
}

NavigationSideBarContainer::NavigationSideBarContainer(QWidget *parent) : Peony::SideBar(parent)
{
    setMinimumWidth(144);  /* 设计要求侧边栏最小宽度为144px */
    setAttribute(Qt::WA_TranslucentBackground);

    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0, 4, 0, 0);
    m_layout->setSpacing(0);

    QWidget *widget = new QWidget;
    m_layout->addWidget(new TitleLabel(this));
    widget->setLayout(m_layout);
    setWidget(widget);

    auto sideBar = new NavigationSideBar(this);
    addSideBar(sideBar);
    connect(sideBar, &NavigationSideBar::updateWindowLocationRequest, this, &NavigationSideBarContainer::updateWindowLocationRequest);
 }

#include "file-label-model.h"
void NavigationSideBarContainer::addSideBar(NavigationSideBar *sidebar)
{
    if (m_sidebar)
        return;

    m_sidebar = sidebar;
    m_layout->addWidget(sidebar);
    //m_layout->addStretch();

    m_labelDialog = new FileLabelBox(this);
    m_labelDialog->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_labelDialog->hide();
    m_layout->addWidget(m_labelDialog);

    QWidget *w = new QWidget(this);
    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(4, 4, 2, 4);

    connect(m_labelDialog->selectionModel(), &QItemSelectionModel::selectionChanged, [=]()
    {
        QModelIndex index = m_labelDialog->currentIndex();
        QString name = index.data(Qt::DisplayRole).toString();
        int id = index.data(Qt::UserRole).toInt();
        if (id)
        {
            //QString uri = "label:///" + QString::number(id);
            QString uri = "label:///" + name;
            Q_EMIT m_sidebar->updateWindowLocationRequest(uri);
        }
    });
    //when clicked in blank, currentChanged may not triggered
    connect(m_labelDialog, &FileLabelBox::leftClickOnBlank, [=]()
    {
        //setLabelNameFilter("");
    });


    LabelButton *labelButton = new LabelButton(this);
    labelButton->setProperty("isWindowButton", 0x1);
    //m_label_button->setCheckable(true);

    labelButton->setFocusPolicy(Qt::FocusPolicy(labelButton->focusPolicy()));
    labelButton->setLastIcon(":/icons/lingmo-down-symbolic");
    labelButton->setFirstIcon(":/icons/sign");
    labelButton->setText(tr("Manager tags..."));

    LabelButton *control = new LabelButton(this);
    control->setText(tr("More tags..."));
    control->setProperty("isWindowButton", 0x1);
    control->hide();

    connect(control, &QPushButton::clicked, this, [=](bool checked){
        Peony::TagManagement::getInstance()->show();
    });

    l->setSpacing(0);

    l->addWidget(control);
    l->addWidget(labelButton);
    connect(labelButton, &QPushButton::clicked, this, [=](){
        bool checked = !labelButton->getShow();
        if (checked) {
            labelButton->setLastIcon(":/icons/lingmo-up-symbolic");
        } else {
            labelButton->setLastIcon(":/icons/lingmo-down-symbolic");
        }
        labelButton->setShow(checked);
        m_labelDialog->setFloatWidgetVisible(checked);
    });

    connect(m_labelDialog, &FileLabelBox::fileLabelVisible, this, [=](bool checked){
        control->setVisible(checked);
        m_labelDialog->setVisible(checked);
    });

    w->setLayout(l);
    m_layout->addWidget(w);
    setLayout(m_layout);

    setTabOrder(m_sidebar, labelButton);
}

QSize NavigationSideBarContainer::sizeHint() const
{
    auto size = QWidget::sizeHint();
    auto width = Peony::GlobalSettings::getInstance()->getValue(DEFAULT_SIDEBAR_WIDTH).toInt();
    qDebug() << "sizeHint set DEFAULT_SIDEBAR_WIDTH:"<<width;
    //fix width value abnormal issue
    if (width <= 0)
        width = 210;
    size.setWidth(width);
    return size;
}

NavigationSideBarStyle::NavigationSideBarStyle()
{

}

void NavigationSideBarStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    painter->save();
    switch (element) {
    case QStyle::PE_IndicatorItemViewItemDrop: {
        /* hotfixbug#99344：拖拽文件到侧边栏，出现黑框 */
        painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);/* 反锯齿 */
        /* 按设计要求，边框颜色为调色板highlight值，圆角为6px */
        QColor color = option->palette.color(QPalette::Highlight);
        painter->setPen(color);
        painter->drawRoundedRect(option->rect, 6, 6);
        painter->restore();
        return;
    }
//    case QStyle::PE_IndicatorBranch: {
//        if (option->rect.x() == 0) {
//            QPainterPath leftRoundedRegion;
//            leftRoundedRegion.setFillRule(Qt::WindingFill);
//            leftRoundedRegion.addRoundedRect(option->rect, NAVIGATION_SIDEBAR_ITEM_BORDER_RADIUS, NAVIGATION_SIDEBAR_ITEM_BORDER_RADIUS);
//            leftRoundedRegion.addRect(option->rect.adjusted(NAVIGATION_SIDEBAR_ITEM_BORDER_RADIUS, 0, 0, 0));
//            painter->setClipPath(leftRoundedRegion);
//        }
//        const QStyleOptionViewItem *tmp = qstyleoption_cast<const QStyleOptionViewItem *>(option);
//        QStyleOptionViewItem opt = *tmp;
//        if (!opt.state.testFlag(QStyle::State_Selected)) {
//            if (opt.state & QStyle::State_Sunken) {
//                opt.palette.setColor(QPalette::Highlight, opt.palette.button().color());
//            }
//            if (opt.state & QStyle::State_MouseOver) {
//                opt.palette.setColor(QPalette::Highlight, opt.palette.mid().color());
//            }
//        }
//        qApp->style()->drawPrimitive(element, &opt, painter, widget);
//        painter->restore();
//        return;
//    }

    case QStyle::PE_PanelItemViewRow: {
        painter->restore();
        return;
        break;
    }
    case QStyle::PE_PanelItemViewItem: {
        break;
    }
    default:
        break;
    }

    qApp->style()->drawPrimitive(element, option, painter, widget);
    painter->restore();
}

void NavigationSideBarStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == QStyle::CE_ItemViewItem) {
        const QStyleOptionViewItem *tmp = qstyleoption_cast<const QStyleOptionViewItem *>(option);
        QStyleOptionViewItem opt = *tmp;
        if (!opt.state.testFlag(QStyle::State_Selected)) {
            if (opt.state & QStyle::State_Sunken) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.button().color());
            }
            if (opt.state & QStyle::State_MouseOver) {
                opt.palette.setColor(QPalette::Highlight, opt.palette.mid().color());
            }
        }
        return qApp->style()->drawControl(element, &opt, painter, widget);
    }
}

TitleLabel::TitleLabel(QWidget *parent):QWidget(parent)
{
    X11WindowManager::getInstance()->registerWidget(this);
    m_pix_label = new QLabel(this);
    //task#106007 【文件管理器】文件管理器应用做平板UI适配，修改应用图标可以跟随主题框架
    m_pix_label->setPixmap(QIcon::fromTheme("system-file-manager").pixmap(32,32));

    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        m_gSettings = new QGSettings("org.lingmo.style", QByteArray(), this);
        connect(m_gSettings, &QGSettings::changed, this, [=](const QString &key) {
            if("iconThemeName" == key){
                m_pix_label->setPixmap(QIcon::fromTheme("system-file-manager").pixmap(32,32));
            }
        });
    }

    m_text_label = new QLabel(tr("Peony"),this);
    QHBoxLayout *l = new QHBoxLayout(this);
    l->setMargin(8); /* 按设计要求间距为8px */
    l->addWidget(m_pix_label);
    l->addSpacing(8);
    l->addWidget(m_text_label);
    l->addStretch();
    this->setFixedHeight(sizeHint().height());
}

LabelButton::LabelButton(QWidget *parent) : QPushButton(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    m_mainLayout = new QHBoxLayout(this);
    m_firstSymbolic = new QLabel(this);
    m_lastSymbolic = new QLabel(this);
    m_text = new QLabel(this);
    m_firstSymbolic->setProperty("useIconHighlightEffect", 0x2);
    m_lastSymbolic->setProperty("useIconHighlightEffect", 0x2);
    m_mainLayout->addWidget(m_firstSymbolic);
    m_mainLayout->addWidget(m_text);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_lastSymbolic);
    setLayout(m_mainLayout);
}

//void LabelButton::mousePressEvent(QMouseEvent *event)
//{
//    if (!m_isPress) {
//        m_isPress = true;
//    }
//    Q_UNUSED(event);
//}

//void LabelButton::mouseReleaseEvent(QMouseEvent *event)
//{
//    if (m_isPress) {
//        m_show = !m_show;
//        Q_EMIT clicked(m_show);
//    }

//    m_isPress = false;

//    Q_UNUSED(event);
//}


void LabelButton::setLastIcon(const QString &symbolic)
{
     m_lastSymbolic->setPixmap(QIcon(symbolic).pixmap(64,64));
}

void LabelButton::setFirstIcon(const QString &symbolic)
{
     m_firstSymbolic->setPixmap(QIcon(symbolic).pixmap(64,64));
}

void LabelButton::setText(QString text)
{
     m_text->setText(text);
}

void LabelButton::setShow(bool isShow)
{
    m_show = isShow;
}

bool LabelButton::getShow()
{
    return m_show;
}
