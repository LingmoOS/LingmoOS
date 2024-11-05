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

#include "intel-navigation-side-bar.h"
#include "intel-side-bar-model.h"
#include "intel-side-bar-proxy-filter-sort-model.h"
#include "intel-side-bar-abstract-item.h"
#include "volume-manager.h"

#include "intel-side-bar-menu.h"
#include "intel-side-bar-abstract-item.h"
#include "bookmark-manager.h"
#include "file-info.h"
#include "file-info-job.h"
#include "main-window.h"

#include "global-settings.h"

#include "file-enumerator.h"
#include "gerror-wrapper.h"

#include "file-utils.h"

#include <QHeaderView>
#include <QPushButton>

#include <QVBoxLayout>

#include <QEvent>

#include <QPainter>

#include <QScrollBar>

#include <QKeyEvent>
#include <QLabel>

#include <QUrl>
#include <QDropEvent>
#include <QMimeData>

#include <QTimer>

#include <QDebug>
#include <QMessageBox>
#include <QApplication>

using namespace Peony;
namespace Intel {

NavigationSideBar::NavigationSideBar(QWidget *parent) : QTreeView(parent)
{
    setIconSize(QSize(16, 16));

    setProperty("useIconHighlightEffect", true);
    //both default and highlight.
    setProperty("iconHighlightEffectMode", 1);

    this->verticalScrollBar()->setProperty("drawScrollBarGroove", false);

    setDragDropMode(QTreeView::DropOnly);

    setProperty("doNotBlur", true);
    viewport()->setProperty("doNotBlur", true);

    auto delegate = new NavigationSideBarItemDelegate(this);
    setItemDelegate(delegate);

    installEventFilter(this);

    setStyleSheet(".Intel--NavigationSideBar{border: 0px solid transparent}");

    setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAttribute(Qt::WA_Disabled, false);
    header()->setSectionResizeMode(QHeaderView::Custom);
    header()->hide();

    setStyle(NavigationSideBarStyle::getStyle());

    setContextMenuPolicy(Qt::CustomContextMenu);

    setExpandsOnDoubleClick(false);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_model = new Peony::Intel::SideBarModel(this);
    m_proxy_model = new Peony::Intel::SideBarProxyFilterSortModel(this);
    m_proxy_model->setSourceModel(m_model);

    this->setModel(m_proxy_model);

    // To hide the expend button of side bar
    this->setRootIsDecorated(false);

    // this->setIndentation(0);
    // this->setRootIsDecorated(false);
    // this->setLayoutDirection(Qt::RightToLeft);
    // this->resetIndentation();
    VolumeManager *volumeManager = VolumeManager::getInstance();
    connect(volumeManager,&Peony::VolumeManager::volumeAdded,this,[=](const std::shared_ptr<Peony::Volume> &volume){
        m_proxy_model->invalidate();//display DVD device in real time.
    });
    connect(volumeManager,&Peony::VolumeManager::volumeRemoved,this,[=](const std::shared_ptr<Peony::Volume> &volume){
        m_proxy_model->invalidate();//The drive does not display when the DVD device is removed.
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
        item->findChildrenAsync();
    });

    connect(this, &QTreeView::collapsed, [=](const QModelIndex &index) {
        auto item = m_proxy_model->itemFromIndex(index);
        item->clearChildren();
    });

//    connect(this, &QTreeView::clicked, [=](const QModelIndex &index) {

        //! Delete the second column to fit the topic
//        switch (index.column()) {
//        case 0: {
//            auto item = m_proxy_model->itemFromIndex(index);
//            //some side bar item doesn't have a uri.
//            //do not emit signal with a null uri to window.
//            if (!item->uri().isNull())
//                Q_EMIT this->updateWindowLocationRequest(item->uri());
//            break;
//        }
//        case 1: {
//            auto item = m_proxy_model->itemFromIndex(index);
//            if (item->isMounted() || item->isEjectable()) {
//                auto leftIndex = m_proxy_model->index(index.row(), 0, index.parent());
//                this->collapse(leftIndex);
//                item->ejectOrUnmount();
//            }
//            break;
//        }
//        default:
//            break;
//        }
//    });

    connect(this, &QTreeView::customContextMenuRequested, this, [=](const QPoint &pos) {
        auto index = indexAt(pos);
        auto item = m_proxy_model->itemFromIndex(index);
        if (item) {
            if (item->type() != Peony::Intel::SideBarAbstractItem::SeparatorItem) {
                Peony::Intel::SideBarMenu menu(item, nullptr, this);
                MainWindow *window = dynamic_cast<MainWindow *>(this->topLevelWidget());
                menu.addAction(QIcon::fromTheme("window-new-symbolic"), tr("Open In &New Window"), [=](){
                    auto enumerator = new Peony::FileEnumerator;
                    enumerator->setEnumerateDirectory(item->uri());
                    enumerator->setAutoDelete();

                    enumerator->connect(enumerator, &Peony::FileEnumerator::prepared, this, [=](const std::shared_ptr<Peony::GErrorWrapper> &err = nullptr, const QString &t = nullptr, bool critical = false){
                        auto targetUri = Peony::FileUtils::getTargetUri(item->uri());
                        if (!targetUri.isEmpty()) {
                            auto newWindow = window->create(targetUri);
                            dynamic_cast<QWidget *>(newWindow)->show();
                        } else if (!err.get() && !critical) {
                            auto newWindow = window->create(item->uri());
                            dynamic_cast<QWidget *>(newWindow)->show();
                        }
                    });

                    enumerator->connect(enumerator, &Peony::FileEnumerator::prepared, [=](){
                        enumerator->deleteLater();
                    });

                    enumerator->prepare();
                });

                if (!qApp->property("tabletMode").toBool()) {
                    menu.addAction(QIcon::fromTheme("tab-new-symbolic"), tr("Open In New &Tab"), [=](){
                        auto enumerator = new Peony::FileEnumerator;
                        enumerator->setEnumerateDirectory(item->uri());
                        enumerator->setAutoDelete();

                        enumerator->connect(enumerator, &Peony::FileEnumerator::prepared, this, [=](const std::shared_ptr<Peony::GErrorWrapper> &err = nullptr, const QString &t = nullptr, bool critical = false){
                            auto targetUri = Peony::FileUtils::getTargetUri(item->uri());
                            if (!targetUri.isEmpty()) {
                                window->addNewTabs(QStringList()<<targetUri);
                                dynamic_cast<QWidget *>(window)->show();
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
                menu.exec(mapToGlobal(pos));
            }
        }
    });

    connect(m_model, &QAbstractItemModel::dataChanged, this, [=](){
        this->viewport()->update();
    });

    //! \bug if annotated it, favorite in side bar will have a empty line, why?
    expandAll();

}

bool NavigationSideBar::eventFilter(QObject *obj, QEvent *e)
{
    return false;
}

void NavigationSideBar::updateGeometries()
{
    setViewportMargins(4, 0, 4, 0);
    QTreeView::updateGeometries();
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
    QTreeView::paintEvent(event);
}

void NavigationSideBar::resizeEvent(QResizeEvent *e)
{
    QTreeView::resizeEvent(e);
    if (header()->count() > 0)
        header()->resizeSection(0, this->viewport()->width() - 30);
}

void NavigationSideBar::dropEvent(QDropEvent *e)
{
    if (dropIndicatorPosition() == QAbstractItemView::AboveItem || dropIndicatorPosition() == QAbstractItemView::BelowItem) {
        // add to bookmark
        e->setAccepted(true);

        auto data = e->mimeData();
        auto bookmark = Peony::BookMarkManager::getInstance();
        if (bookmark->isLoaded()) {
            for (auto url : data->urls()) {
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
    QTreeView::dropEvent(e);
}

QSize NavigationSideBar::sizeHint() const
{
    auto size = QTreeView::sizeHint();
    auto width = Peony::GlobalSettings::getInstance()->getValue(DEFAULT_SIDEBAR_WIDTH).toInt();
    qDebug() << "sizeHint set DEFAULT_SIDEBAR_WIDTH:"<<width;
    //fix width value abnormal issue
    if (width <= 0)
        width = 210;
    size.setWidth(width);
    return size;
}

void NavigationSideBar::keyPressEvent(QKeyEvent *event)
{
    QTreeView::keyPressEvent(event);

    if (event->key() == Qt::Key_Return) {
        if (!selectedIndexes().isEmpty()) {
            auto index = selectedIndexes().first();
            auto uri = index.data(Qt::UserRole).toString();
            Q_EMIT this->updateWindowLocationRequest(uri, true);
        }
    }
}


void NavigationSideBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    QModelIndex index = indexAt(event->pos());

    if (!index.isValid())
        return;

    auto item = m_proxy_model->itemFromIndex(index);
    //bug#116016 收起计算机目录后展开计算机下方无显示
    if (!index.parent().isValid() &&
        event->x() < rect().right() - 25 &&
        event->x() > rect().right() - 80) {
        // if click expend rect
        QPoint point(event->x(), event->y());
        if (!isExpanded(indexAt(point)))
            expand(indexAt(point));
        else
            collapse(indexAt(point));

    }
    else if (event->x() < rect().right() - 15 &&
             event->x() > rect().right() - 45 &&
             (item->isEjectable() || item->isMountable())) {
        // if click umount rect
        auto leftIndex = m_proxy_model->index(index.row(), 0, index.parent());
        this->collapse(leftIndex);
        item->ejectOrUnmount();
    }
    else {
        item = m_proxy_model->itemFromIndex(index);
        QString uri = item->uri();
        auto info = FileInfo::fromUri(uri);
        if (info.get()->isEmptyInfo()) {
            FileInfoJob j(info);
            j.querySync();
        }
        auto targetUri = FileUtils::getTargetUri(uri);
        if (!targetUri.isEmpty()) {
            Q_EMIT this->updateWindowLocationRequest(targetUri);
            QTreeView::mousePressEvent(event);
            return;
        }
        if (!uri.isNull())
            Q_EMIT this->updateWindowLocationRequest(uri);
        QTreeView::mousePressEvent(event);
    }
}

bool NavigationSideBar::isRemoveable(const QModelIndex &index)
{
    auto item = m_proxy_model->itemFromIndex(index);
    if (item != nullptr)
        return item->isRemoveable();
    else
        return false;
}

bool NavigationSideBar::isMounted(const QModelIndex &index)
{
    auto item = m_proxy_model->itemFromIndex(index);
    if (item != nullptr)
        return item->isMounted();
    else
        return false;
}

NavigationSideBarItemDelegate::NavigationSideBarItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

QSize NavigationSideBarItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bool is_intel = (QString::compare("V10SP1-edu", GlobalSettings::getInstance()->getProjectName(), Qt::CaseInsensitive) == 0);
    if (is_intel) {
        auto size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(56);
        return size;
    }
    return QStyledItemDelegate::sizeHint(option, index);
}

void NavigationSideBarItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    if (!index.isValid() || option.state == QStyle::State_None)
        return;

    QStyleOptionViewItem opt = option;

    //! \brief Temporarily replace drawPrimitive
    if (opt.state.testFlag(QStyle::State_MouseOver) && !opt.state.testFlag(QStyle::State_Selected))
        opt.state = QStyle::State_Enabled;

    painter->setRenderHint(QPainter::Antialiasing, true);

    QStyledItemDelegate::paint(painter, opt, index);

    NavigationSideBar* view = qobject_cast<NavigationSideBar*>(this->parent());
    if (view == nullptr)
        return;

    //! \brief print mount icon
    if (view->isRemoveable(index) && view->isMounted(index)) {
        QRect rect = option.rect;
        rect.setY(rect.top() + sizeHint(option, index).height()/3);
        rect.setX(rect.right() - 30);
        rect.setSize(QSize(16, 16));
        painter->drawPixmap(rect, QPixmap(":/img/media-eject"));
    }

    if (!index.model()->hasChildren(index))
        return;

    if (view->isExpanded(index)) {
        QRect rect = option.rect;
        rect.setY(rect.top() + sizeHint(option, index).height()/3);
        rect.setX(rect.right() - 45);
        rect.setSize(QSize(16, 16));
        painter->drawPixmap(rect, QPixmap(":/img/branches2"));
    }
    else {
        QRect rect = option.rect;
        rect.setTop(rect.top() + sizeHint(option, index).height()/3);
        rect.setX(rect.right() - 45);
        rect.setSize(QSize(16, 16));
        painter->drawPixmap(rect, QPixmap(":/img/branches1"));
    }
}

NavigationSideBarContainer::NavigationSideBarContainer(QWidget *parent) : Peony::SideBar(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);

    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0, 4, 0, 0);
    m_layout->setSpacing(0);
    auto sideBar = new NavigationSideBar(this);

    QWidget *widget = new QWidget;
    m_layout->addWidget(new TitleLabel(this));
    m_layout->addWidget(sideBar);
    widget->setLayout(m_layout);

    setWidget(widget);
    //addSideBar(sideBar);

    connect(sideBar, &NavigationSideBar::updateWindowLocationRequest, this, &NavigationSideBarContainer::updateWindowLocationRequest);
}

void NavigationSideBarContainer::addSideBar(NavigationSideBar *sidebar)
{
    if (m_sidebar)
        return;

    m_sidebar = sidebar;
    m_layout->addWidget(sidebar);

//    QWidget *w = new QWidget(this);
//    QVBoxLayout *l = new QVBoxLayout;
//    l->setContentsMargins(4, 4, 2, 4);

//    m_label_button = new QPushButton(QIcon::fromTheme("emblem-important-symbolic"), tr("All tags..."), this);
//    m_label_button->setProperty("useIconHighlightEffect", true);
//    m_label_button->setProperty("iconHighlightEffectMode", 1);
//    m_label_button->setProperty("fillIconSymbolicColor", true);
//    m_label_button->setCheckable(true);

//    l->addWidget(m_label_button);

//    connect(m_label_button, &QPushButton::clicked, m_sidebar, &NavigationSideBar::labelButtonClicked);

//    w->setLayout(l);

//    m_layout->addWidget(w);

    setLayout(m_layout);
}

QSize NavigationSideBarContainer::sizeHint() const
{
    return SideBar::sizeHint();
}

TitleLabel::TitleLabel(QWidget *parent):QWidget(parent)
{
    this->setFixedHeight(50);
    m_pix_label = new QLabel(this);
    m_pix_label->setPixmap(QIcon(":/custom/icons/app-controlsetting").pixmap(32,32));
    m_text_label = new QLabel(tr("Files"),this);
    QHBoxLayout *l = new QHBoxLayout(this);
    l->setMargin(16);
    l->addWidget(m_pix_label);
    l->addSpacing(16);
    l->addWidget(m_text_label);
    l->addStretch();
}

static NavigationSideBarStyle *global_instance = nullptr;

NavigationSideBarStyle::NavigationSideBarStyle(QStyle *style) : QProxyStyle(style) {}

NavigationSideBarStyle* NavigationSideBarStyle::getStyle()
{
    if (!global_instance)
        global_instance = new NavigationSideBarStyle;
    return global_instance;
}

//! \brief replace polish, delete hover state
void NavigationSideBarStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case PE_PanelItemViewItem: {
        //! \bug never in here in qt5-lingmo-platformtheme-tablet-1205
        if (option->state.testFlag(QStyle::State_MouseOver) && !option->state.testFlag(QStyle::State_Selected)) {
            return;
        }
        else {
            QProxyStyle::drawPrimitive(element, option, painter, widget);
            return;
        }
    }
    case PE_PanelItemViewRow: {

        const QStyleOptionViewItem *tmp = qstyleoption_cast<const  QStyleOptionViewItem *>(option);
        QStyleOptionViewItem opt = *tmp;
        opt.rect.adjust(0, 1, 0, 0);

        QProxyStyle::drawPrimitive(element, &opt, painter, widget);

        return;
    }
    case PE_IndicatorBranch: {

        const QStyleOptionViewItem *tmp = qstyleoption_cast<const QStyleOptionViewItem *>(option);
        QStyleOptionViewItem opt = *tmp;
        opt.rect.adjust(0, 1, 0, 0);
        if (option->state.testFlag(QStyle::State_MouseOver) && !option->state.testFlag(QStyle::State_Selected)) {
            return;
        }
        else {
            QProxyStyle::drawPrimitive(element, &opt, painter, widget);
            return;
        }
    }
    case QStyle::PE_Frame:
    case QStyle::PE_FrameWindow: {
        return;
    }
    default: QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
}

void NavigationSideBarStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element) {
    case CE_ItemViewItem: {
        const QStyleOptionViewItem *tmp = qstyleoption_cast<const QStyleOptionViewItem *>(option);
        QStyleOptionViewItem opt = *tmp;
        opt.rect.adjust(0, 1, 0, 0);

        QProxyStyle::drawControl(element, &opt, painter, widget);
        return;
    }
    default: QProxyStyle::drawControl(element, option, painter, widget);
    }
}


}
