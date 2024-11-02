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

#include "icon-view.h"
#include "file-item.h"

#include "icon-view-delegate.h"
#include "icon-view-style.h"

#include "directory-view-menu.h"
#include "icon-view-index-widget.h"
#include "file-info.h"
#include "file-utils.h"

#include "global-settings.h"
#include "search-vfs-uri-parser.h"
#include "directoryviewhelper.h"

#include <QMouseEvent>

#include <QDragEnterEvent>
#include <QMimeData>
#include <QDragMoveEvent>
#include <QDropEvent>

#include <QPainter>
#include <QPaintEvent>

#include <QApplication>

#include <QVBoxLayout>

#include <QHoverEvent>

#include <QScrollBar>

#include <QMouseEvent>
#include <QApplication>

#include <QStringList>
#include <QStyleHints>
#include <QPoint>

#include <QDebug>
#include <QToolTip>

#include <QStandardPaths>

#include <QDrag>
#include <QWindow>
#include <QMessageBox>
#include <QFontMetrics>

using namespace Peony;
using namespace Peony::DirectoryView;

IconView::IconView(QWidget *parent) : QListView(parent)
{   
    m_touch_active_timer = new QTimer(this);
    m_touch_active_timer->setSingleShot(true);

    //setFrameShape(QFrame::NoFrame);

    setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAttribute(Qt::WA_TranslucentBackground);

    setAutoScroll(true);
    setAutoScrollMargin(100);
    QString version = qApp->property("version").toString();
    if (version == "lingmo3.0"){
        setStyle(IconViewStyle::getStyle());
    } else {
        setFrameShape(QFrame::NoFrame);
    } 

    //setStyle(IconViewStyle::getStyle());
    //FIXME: do not create proxy in view itself.
    IconViewDelegate *delegate = new IconViewDelegate(this);
    setItemDelegate(delegate);
    connect(delegate, &IconViewDelegate::isEditing, this, [=](const bool &editing)
    {
        m_delegate_editing = editing;
    });

    //fix long file name not fully painted issue when drag sliderbar
    QScrollBar *verticalBar = verticalScrollBar();
    connect(verticalBar, &QScrollBar::sliderPressed, [=](){
        m_slider_bar_draging = true;
    });
    connect(verticalBar, &QScrollBar::sliderReleased, [=](){
        m_slider_bar_draging = false;
    });
    connect(verticalBar, &QScrollBar::valueChanged, [=](){
        if (m_slider_bar_draging)
            viewport()->update(viewport()->rect());
    });

    setSelectionMode(QListView::ExtendedSelection);
    setEditTriggers(QListView::NoEditTriggers);
    setViewMode(QListView::IconMode);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Snap);
    //setWordWrap(true);

    setContextMenuPolicy(Qt::CustomContextMenu);
    if (version == "lingmo3.0"){
        setIconSize(QSize(64, 64));
    } else {
        setIconSize(QSize(86, 86));
    } 
    setGridSize(itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex()) + QSize(20, 20));

    m_renameTimer = new QTimer(this);
    m_renameTimer->setInterval(3000);
    m_renameTimer->setSingleShot(true);
    m_editValid = false;

    setMouseTracking(true);//追踪鼠标

    setViewportMargins(0, 0, 0, 0);
}

IconView::~IconView()
{

}

DirectoryViewProxyIface *IconView::getProxy()
{
    return m_proxy;
}

//selection
//FIXME: implement the selection functions.
void IconView::setSelections(const QStringList &uris)
{
    clearSelection();
    QItemSelection selection;
    for (auto uri: uris) {
        const QModelIndex index = m_sort_filter_proxy_model->indexFromUri(uri);
        if (index.isValid()) {
            QItemSelection selectionToBeMerged(index, index);
            selection.merge(selectionToBeMerged, QItemSelectionModel::Select);
        }
    }

    selectionModel()->select(selection, QItemSelectionModel::Select);
}

const QStringList IconView::getSelections()
{
    QStringList uris;
    QModelIndexList selections = selectedIndexes();
    for (auto index : selections) {
        auto item = m_sort_filter_proxy_model->itemFromIndex(index);
        uris<<item->uri();
    }
    uris.removeDuplicates();
    return uris;
}

void IconView::invertSelections()
{
    QItemSelectionModel *selectionModel = this->selectionModel();
    const QItemSelection currentSelection = selectionModel->selection();
    this->selectAll();
    selectionModel->select(currentSelection, QItemSelectionModel::Deselect);
}

void IconView::scrollToSelection(const QString &uri)
{
    auto index = m_sort_filter_proxy_model->indexFromUri(uri);
    scrollTo(index);
}

//clipboard
void IconView::setCutFiles(const QStringList &uris)
{
    //let delegate and model know how to deal with cut files.
}

//location
//FIXME: implement location functions.
void IconView::setDirectoryUri(const QString &uri)
{
    m_current_uri = uri;
    if (m_current_uri.startsWith("search://")) {
        QString nameRegexp = SearchVFSUriParser::getSearchUriNameRegexp(uri);
        setSearchKey(nameRegexp);
    } else {
        setSearchKey("");
    }
}

const QString IconView::getDirectoryUri()
{
    return m_model->getRootUri();
}

void IconView::beginLocationChange()
{
    m_last_index = QModelIndex();
    traverseNode();
    m_editValid = false;
    m_model->setRootUri(m_current_uri);
}

void IconView::stopLocationChange()
{
    m_model->cancelFindChildren();
}

void IconView::traverseNode()
{
    //fix bug 194738, clear model index
    QList<IconViewIndexWidget *> widgets = this->findChildren<IconViewIndexWidget *>("explor_icon_view_index_widget");
    if (!widgets.isEmpty()) {
        for (int i = 0; i < widgets.size(); ++i) {
            auto node = widgets.at(i);
            if (node) {
                node->setUpdatesEnabled(false);
                node->clearModelIndex();
            }
        }
    }
}

//other
void IconView::open(const QStringList &uris, bool newWindow)
{

}

void IconView::closeView()
{
    this->deleteLater();
}

void IconView::dragEnterEvent(QDragEnterEvent *e)
{
    m_editValid = false;
    if (e->keyboardModifiers() & Qt::ControlModifier)
        m_ctrl_key_pressed = true;
    else
        m_ctrl_key_pressed = false;

    auto action = m_ctrl_key_pressed ? Qt::CopyAction : Qt::MoveAction;
    qDebug()<<"dragEnterEvent()" <<action <<m_ctrl_key_pressed;
    if (e->mimeData()->hasUrls()) {
        if (FileUtils::containsStandardPath(e->mimeData()->urls())) {
            e->ignore();
            if (this == e->source()) {
                clearSelection();
            }
            return;
        }
        e->setDropAction(action);
        e->accept();
    }
}

void IconView::dragMoveEvent(QDragMoveEvent *e)
{
    if (e->keyboardModifiers() & Qt::ControlModifier)
        m_ctrl_key_pressed = true;
    else
        m_ctrl_key_pressed = false;

    //fix can not drag in the second time issue
    if (this->isDraggingState())
    {
        if (m_allow_set_index_widget) {
            m_allow_set_index_widget = false;
            for (auto index : selectedIndexes()) {
                setIndexWidget(index, nullptr);
            }
        }
    }

    auto action = m_ctrl_key_pressed ? Qt::CopyAction : Qt::MoveAction;
    //qDebug()<<"dragMoveEvent()" <<action <<m_ctrl_key_pressed;
    auto index = indexAt(e->pos());
    if (index.isValid() && index != m_last_index) {
        QHoverEvent he(QHoverEvent::HoverMove, e->posF(), e->posF());
        viewportEvent(&he);
    } else {
        QHoverEvent he(QHoverEvent::HoverLeave, e->posF(), e->posF());
        viewportEvent(&he);
    }
    if (this == e->source() || !QModelIndex().flags().testFlag(Qt::ItemIsDropEnabled)) {
        return QListView::dragMoveEvent(e);
    }
    e->setDropAction(action);
    e->accept();
}

void IconView::dropEvent(QDropEvent *e)
{
    m_last_index = QModelIndex();
    //m_edit_trigger_timer.stop();
    if (e->keyboardModifiers() & Qt::ControlModifier)
        m_ctrl_key_pressed = true;
    else
        m_ctrl_key_pressed = false;

    auto action = m_ctrl_key_pressed ? Qt::CopyAction : Qt::MoveAction;
    e->setDropAction(action);
    if (e->keyboardModifiers() & Qt::ShiftModifier) {
        action = Qt::TargetMoveAction;
    }

    //Do not allow dragging files to file manager when searching
    //related to bug#107063,118004
    if (m_current_uri.startsWith("search://") || m_current_uri.startsWith("favorite://")) {
        QMessageBox::warning(this, tr("warn"), tr("This operation is not supported."));
        return;
    }

    auto proxy_index = indexAt(e->pos());
    auto index = m_sort_filter_proxy_model->mapToSource(proxy_index);
    qDebug()<<"dropEvent" <<action <<m_ctrl_key_pressed <<indexAt(e->pos()).isValid();

    QString username = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).split("/").last();
    QString boxpath = "file://"+QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.box";
    QString oldboxpath = "file://box/"+username;

    if(m_current_uri == boxpath || m_current_uri == oldboxpath || m_current_uri == "filesafe:///"){
        return;
    }

    //move in current path, do nothing
    if (e->source() == this)
    {
        if (indexAt(e->pos()).isValid())
        {
            auto uri = m_sort_filter_proxy_model->itemFromIndex(proxy_index)->uri();
            if(!e->mimeData()->urls().contains(uri))
                m_model->dropMimeData(e->mimeData(), action, 0, 0, index);
        } else {
            if (m_ctrl_key_pressed) {
                m_model->dropMimeData(e->mimeData(), Qt::CopyAction, 0, 0, QModelIndex());
            }
        }
        return;
    }

    m_model->dropMimeData(e->mimeData(), action, 0, 0, index);
}

void IconView::mouseMoveEvent(QMouseEvent *e)
{
    QModelIndex itemIndex = indexAt(e->pos());
    if (!itemIndex.isValid()) {
        if (QToolTip::isVisible()) {
            QToolTip::hideText();
        }
    }

    if (m_ignore_mouse_move_event) {
        return;
    }
    QListView::mouseMoveEvent(e);

    // fix #115124, drag selection can not trigger auto scroll in view.
    if (e->buttons() & Qt::LeftButton && !this->viewport()->rect().adjusted(0, autoScrollMargin(), 0, -autoScrollMargin()).contains(e->pos())) {
        doAutoScroll();
    }

    if(getSelections().count()>1)
        multiSelect();
    viewport()->update(viewport()->rect());
}

void IconView::mousePressEvent(QMouseEvent *e)
{
    bool singleClicked = qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick);
    if (singleClicked) {
        if (!m_touch_active_timer->isActive()) {
            m_touch_active_timer->start(1100);
        }
    }

    m_allow_set_index_widget = true;
    if ((e->modifiers() & Qt::ControlModifier || selectionMode() == MultiSelection))
        m_ctrl_key_pressed = true;
    else
        m_ctrl_key_pressed = false;

    QModelIndex itemIndex = indexAt(e->pos());
    if (itemIndex.isValid() && m_multi_select) {
        m_mouse_release_unselect = selectedIndexes().contains(itemIndex);
    } else {
        m_mouse_release_unselect = false;
    }

    auto index = indexAt(e->pos());
    if (e->button() == Qt::LeftButton && (e->modifiers() & Qt::ControlModifier || selectionMode() == MultiSelection) && selectedIndexes().contains(index)) {
        m_noSelectOnPress = true;
    } else {
        m_noSelectOnPress = false;
    }

    QListView::mousePressEvent(e);

    if (e->button() != Qt::LeftButton) {
        return;
    }

    if (true == m_mouse_release_unselect) {
        //selectionModel()->setCurrentIndex(itemIndex, QItemSelectionModel::Select|QItemSelectionModel::Rows);
    }

    if(getSelections().count()>1)
        multiSelect();

    viewport()->update(viewport()->rect());

    if (!itemIndex.isValid()) {
        disableMultiSelect();
    }
    //FIXME: Modify the icon style, only click on the text to respond, click on the icon to not respond
    QRect rect = visualRect(indexAt(e->pos()));
    QSize iconExpectedSize = iconSize();

    QRect testRect(rect.x(), rect.y()+ iconExpectedSize.height(),rect.width(),rect.height()-iconExpectedSize.height());
    if(!testRect.contains(e->pos()))
    {
        m_editValid = false;
        m_renameTimer->start();
        return;
    }
    //m_renameTimer
    if(!m_renameTimer->isActive())
    {
        m_renameTimer->start();
        m_editValid = false;
    }
    else
    {
        //优化文件点击策略，提升用户体验，关联bug#125368
        //在双击时间间隔内，如果未触发双击事件，但是点击的是同一个有效图标，触发双击事件
        //系统默认双击间隔为400ms, 策略为[0,400]，触发双击，(400,3000)触发重命名
        if(m_renameTimer->remainingTime() >0 && m_renameTimer->remainingTime() < 3000 - qApp->styleHints()->mouseDoubleClickInterval()
                && indexAt(e->pos()) == m_last_index && m_last_index.isValid() && m_editValid == true)
        {
            slotRename();
        } else
        {
            m_editValid = false;
        }
    }
}

void IconView::mouseReleaseEvent(QMouseEvent *e)
{
    QListView::mouseReleaseEvent(e);

    m_noSelectOnPress = false;

    if (e->button() != Qt::LeftButton) {
        return;
    }
}

void IconView::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_editValid = false;
    QListView::mouseDoubleClickEvent(event);
}

void IconView::keyPressEvent(QKeyEvent *e)
{
    QListView::keyPressEvent(e);

    if(e->key() == Qt::Key_Space){
        Q_EMIT QListView::activated(currentIndex());/* 与按下enter键效果一样 */
    }

    if (e->key() == Qt::Key_Control)
        m_ctrl_key_pressed = true;

    if(e->key() == Qt::Key_F10 && e->modifiers() == Qt::ShiftModifier) {
        if (getSelections().count() == 1 ) {
            auto currentIndex = selectionModel()->selection().indexes();
            QPoint menuPos = this->visualRect(currentIndex.first()).center();
            Q_EMIT customContextMenuRequested(menuPos);
            return;
        }
    }
}

void IconView::keyReleaseEvent(QKeyEvent *e)
{
    QListView::keyReleaseEvent(e);
    if (e->key() == Qt::Key_Control)
        m_ctrl_key_pressed = false;
}

void IconView::paintEvent(QPaintEvent *e)
{
    QPainter p(this->viewport());
    p.fillRect(this->geometry(), this->palette().base());
    if (m_repaint_timer.isActive()) {
        m_repaint_timer.stop();
        QTimer::singleShot(100, this, [this]() {
            this->repaint();
        });
    }
    QListView::paintEvent(e);
}

void IconView::resizeEvent(QResizeEvent *e)
{
    //FIXME: first resize is disfluency.
    //but I have to reset the index widget in view's resize.
    QListView::resizeEvent(e);
    if (m_delegate_editing && m_increase) {
        m_increase = false;
        return;
    }
    setIndexWidget(m_last_index, nullptr);
}

void IconView::wheelEvent(QWheelEvent *e)
{
    if ((e->modifiers() & Qt::ControlModifier)) {
        if (e->delta() > 0) {
            zoomLevelChangedRequest(true);
        } else {
            zoomLevelChangedRequest(false);
        }
        e->accept();
        return;
    }

    QListView::wheelEvent(e);
    if (e->buttons() == Qt::LeftButton)
        this->viewport()->update();
}

void IconView::updateGeometries()
{
#if QT_VERSION_CHECK(5, 15, 0)
    //try fix #55341
    int verticalValue = verticalOffset();

    QListView::updateGeometries();

    if (!model() || model()->columnCount() == 0 || model()->rowCount() == 0) {
        return;
    }

    if (m_delegate_editing) {
        int characterHeight = qApp->fontMetrics().height();
        int totalHeight = characterHeight * 255/4;

        m_scrollMax = verticalScrollBar()->maximum();
        int maxHeight = viewport()->height() - visualRect(m_last_index).y() - characterHeight - 15 - totalHeight;
        if (maxHeight + m_scrollMax < 0 && m_scrollMax >= 0) {
            verticalScrollBar()->setRange(0, -maxHeight);
        }
        return;
    }

    int itemRowCount = model()->rowCount();
    auto lastIndex = model()->index(itemRowCount - 1, 0);
    QRegion itemRegion = visualRect(lastIndex);
    int lastItemBottom = itemRegion.boundingRect().bottom();

    if ((lastItemBottom + gridSize().height()) < viewport()->height()) {
        verticalScrollBar()->setRange(0, 0);
    } else {
        int vertiacalMax = verticalScrollBar()->maximum();
        verticalScrollBar()->setMaximum(vertiacalMax + gridSize().height());
        verticalScrollBar()->setValue(verticalValue);
    }
#else
    horizontalScrollBar()->setRange(0, 0);
    if (!model()) {
        verticalScrollBar()->setRange(0, 0);
        return;
    }

    if (model()->columnCount() == 0 || model()->rowCount() == 0) {
        verticalScrollBar()->setRange(0, 0);
        return;
    }

    int itemRowCount = model()->rowCount();
    auto lastIndex = model()->index(itemRowCount - 1, 0);
    QRegion itemRegion = visualRect(lastIndex);
    int lastItemBottom = itemRegion.boundingRect().bottom();

    if ((lastItemBottom + gridSize().height()) < viewport()->height()) {
        verticalScrollBar()->setRange(0, 0);
    } else {
        verticalScrollBar()->setSingleStep(gridSize().height()/2);
        verticalScrollBar()->setPageStep(viewport()->height());
        verticalScrollBar()->setRange(0, lastItemBottom - viewport()->height() + gridSize().height());
    }
#endif
}

void IconView::focusInEvent(QFocusEvent *e)
{
    QListView::focusInEvent(e);
    if (e->reason() == Qt::TabFocusReason) {
        if (selectedIndexes().isEmpty()) {
            selectionModel()->select(model()->index(0, 0), QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
        } else {
            scrollTo(selectedIndexes().first(), QListView::PositionAtCenter);
            auto selections = getSelections();
            clearSelection();
            //added for tab key to focus button issue
            //use uri rather than index, to fix crash bug#68788, 96145
            QTimer::singleShot(100, this, [=](){
               setSelections(selections);
            });
        }
    }
}

void IconView::startDrag(Qt::DropActions flags)
{
    auto indexes = selectedIndexes();
    if (indexes.count() > 0) {
        auto pos = mapFromGlobal(QCursor::pos());
        qreal scale = 1.0;
        QWidget *window = this->window();
        if (window) {
            auto windowHandle = window->windowHandle();
            if (windowHandle) {
                scale = windowHandle->devicePixelRatio();
            }
        }

        auto drag = new QDrag(this);
        if(m_current_uri.startsWith("search://")){
            QMimeData* data = model()->mimeData(indexes);
            QVariant isSearchData = QVariant(true);
            data->setData("explor-qt/is-search", isSearchData.toByteArray());
            drag->setMimeData(data);
        }else{
            drag->setMimeData(model()->mimeData(indexes));
        }

        int num = indexes.count();
        if (num > 100) {
            QRect pixmapRect = QRect(100, 100, 400, 400);
            QPixmap pixmap(pixmapRect.size() * scale);
            pixmap.fill(Qt::transparent);
            pixmap.setDevicePixelRatio(scale);
            QPainter painter(&pixmap);
            quint64 count = 0;
            painter.save();
            QRect iconRect = pixmapRect;
            iconRect.setSize(QSize(139, 139));
            for (auto index : indexes) {
               if (count > 10) {
                   break;
               }
               count++;
               iconRect.moveTo(iconRect.x()+3, iconRect.y()+3);
               QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
               if (!icon.isNull()) {
                   painter.save();
                   painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                   painter.drawPixmap(QPoint(100 + count*3, 100 + count*3), icon.pixmap(139, 139));
                   painter.restore();
              }
            }
            QFont font = qApp->font();
            font.setPointSize(10);
            QFontMetrics metrics(font);
            QString text = num > 999 ? "..." : QString::number(num);
            int height = metrics.width(text);
            int width = metrics.height();

            int diameter = std::max(height, width);
            int radius = diameter / 2;
            QRectF textRect = QRectF(iconRect.topRight().x() - diameter - 10, iconRect.topRight().y(), diameter + 10, diameter + 10);
            painter.setBrush(Qt::red);
            painter.setPen(Qt::red);
            painter.drawEllipse(textRect);
            painter.setPen(Qt::white);
            painter.drawText(textRect, Qt::AlignCenter, text);
            painter.restore();


            drag->setPixmap(pixmap);
            drag->setHotSpot(QPoint(200,200));
            drag->setDragCursor(QPixmap(), m_ctrl_key_pressed? Qt::CopyAction: Qt::MoveAction);
            drag->exec(m_ctrl_key_pressed? Qt::CopyAction: Qt::MoveAction);
        } else {
            QRegion rect;
            QHash<QModelIndex, QRect> indexRectHash;
            for (auto index : indexes) {
                rect += (visualRect(index));
                indexRectHash.insert(index, visualRect(index));
            }
            QRect realRect = rect.boundingRect();
            QPixmap pixmap(realRect.size() * scale);
            pixmap.fill(Qt::transparent);
            pixmap.setDevicePixelRatio(scale);
            QPainter painter(&pixmap);

            for (auto index : indexes) {
                painter.save();
                QStyleOptionViewItem opt = viewOptions();
                auto viewItemDelegate = static_cast<IconViewDelegate *>(itemDelegate());
                viewItemDelegate->initIndexOption(&opt, index);

                opt.state |= QStyle::State_Selected;
                opt.rect.setSize(visualRect(index).size());
                painter.translate(indexRectHash.value(index).topLeft() - rect.boundingRect().topLeft());

                viewItemDelegate->setStartDrag(true);
                itemDelegate()->paint(&painter, opt, index);
                viewItemDelegate->setStartDrag(false);
                painter.restore();
            }
            drag->setPixmap(pixmap);
            drag->setHotSpot(pos - rect.boundingRect().topLeft());
            drag->setDragCursor(QPixmap(), m_ctrl_key_pressed? Qt::CopyAction: Qt::MoveAction);
            drag->exec(m_ctrl_key_pressed? Qt::CopyAction: Qt::MoveAction);
        }
    }
}

void IconView::slotRename()
{
    //special path like trash path not allow rename
    if (getDirectoryUri().startsWith("trash://")
        || getDirectoryUri().startsWith("recent://")
        || getDirectoryUri().startsWith("favorite://")
//        || getDirectoryUri().startsWith("search://")   //comment fix in search result can not click to rename issue
        || getDirectoryUri().startsWith("network://")
        || getDirectoryUri().startsWith("label://"))
        return;

    //standardPaths not allow rename
    auto currentSelections = getSelections();
    bool hasStandardPath = FileUtils::containsStandardPath(currentSelections);
    if (hasStandardPath)
        return;

    //delay edit action to avoid doubleClick or dragEvent
    qDebug()<<"slotRename"<<m_editValid;
    QTimer::singleShot(300, m_renameTimer, [&]() {
        qDebug()<<"singleshot"<<m_editValid;
        //fix bug#98951, click edit box boarder will reenter edit issue
        if(m_editValid && ! m_delegate_editing) {
            m_renameTimer->stop();
            setIndexWidget(m_last_index, nullptr);
            edit(m_last_index);
            m_editValid = false;
        }
    });

}

bool IconView::getIgnore_mouse_move_event() const
{
    return m_ignore_mouse_move_event;
}

void IconView::setIgnore_mouse_move_event(bool ignore_mouse_move_event)
{
    m_ignore_mouse_move_event = ignore_mouse_move_event;
}

void IconView::bindModel(FileItemModel *sourceModel, FileItemProxyFilterSortModel *proxyModel)
{
    m_model = sourceModel;
    m_sort_filter_proxy_model = proxyModel;

    auto proxyModelSelectionModelHint = proxyModel->getSelectionModeHint();
    if (proxyModelSelectionModelHint != NoSelection) {
        setSelectionMode(proxyModelSelectionModelHint);
    }

    connect(proxyModel, &FileItemProxyFilterSortModel::setSelectionModeChanged, this, [=]{
        auto proxyModelSelectionModelHint = proxyModel->getSelectionModeHint();
        if (proxyModelSelectionModelHint != NoSelection) {
            setSelectionMode(proxyModelSelectionModelHint);
        }
    });

    setModel(m_sort_filter_proxy_model);

    //edit trigger
    connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selection, const QItemSelection &deselection) {
        //qDebug()<<"selection changed";
        auto currentSelections = selection.indexes();

        for (auto index : deselection.indexes()) {
            this->setIndexWidget(index, nullptr);
        }

        if (state() == QListView::DragSelectingState)
            m_allow_set_index_widget = false;

        //Q_EMIT m_proxy->viewSelectionChanged();
        if (currentSelections.count() == 1) {
            qDebug()<<"m_last_index  "<<(m_last_index == currentSelections.first())<<currentSelections.first();
            if(m_last_index != currentSelections.first())
            {
                m_editValid = false;
            }
            m_last_index = currentSelections.first();
            //qDebug()<<"IconView::bindModel:"<<"selection changed: "<<"resetEditTriggerTimer";
            //this->resetEditTriggerTimer();
        } else {
            m_last_index = QModelIndex();
            m_editValid = false;
        }


        //qDebug()<<"selection changed2"<<m_editValid;
    });
}

void IconView::setProxy(DirectoryViewProxyIface *proxy)
{
    if (!proxy)
        return;

    m_proxy = proxy;
    if (!m_proxy) {
        return;
    }

    //connect(m_model, &FileItemModel::dataChanged, this, &IconView::clearIndexWidget);
    //connect(m_model, &FileItemModel::updated, this, &IconView::resort);

    connect(m_model, &FileItemModel::findChildrenFinished,
            this, &IconView::reportViewDirectoryChanged);

    connect(this, &IconView::activated, [=](const QModelIndex &index) {
        qDebug()<<"double click"<<index.data(FileItemModel::UriRole);
        //when selections is more than 1, let mainwindow to process
        if (getSelections().count() != 1)
            return;
        auto uri = getSelections().first();
        //process open symbolic link lllll
        //auto info = FileInfo::fromUri(uri);
        //if (info->isSymbolLink() && uri.startsWith("file://") && info->isValid())
          //  uri = "file://" + FileUtils::getSymbolicTarget(uri);
        if(!m_multi_select)
            Q_EMIT m_proxy->viewDoubleClicked(uri);
    });



    //menu
//    connect(this, &IconView::customContextMenuRequested, [=](const QPoint &pos){
//        if (!indexAt(pos).isValid())
//            this->clearSelection();

//        //NOTE: we have to ensure that we have cleared the
//        //selection if menu request at blank pos.
//        QTimer::singleShot(1, [=](){
//            Q_EMIT this->getProxy()->menuRequest(QCursor::pos());
//        });
//    });
}

// NOTE: When icon view was resorted,
// index widget would deviated from its normal position by somehow.
// So, do not set any index widget when the resorting.
void IconView::resort()
{
    //fix uncompress selected file will cover file before it issue
    clearIndexWidget();
    if (m_last_index.isValid()) {
        this->setIndexWidget(m_last_index, nullptr);
    }

    if (m_sort_filter_proxy_model)
        m_sort_filter_proxy_model->sort(getSortType(), Qt::SortOrder(getSortOrder()));
}

void IconView::reportViewDirectoryChanged()
{
    if (m_proxy)
        Q_EMIT m_proxy->viewDirectoryChanged();
}

QRect IconView::visualRect(const QModelIndex &index) const
{
    auto rect = QListView::visualRect(index);
    rect.setX(rect.x() + 10);
    rect.setY(rect.y() + 15);
    auto size = itemDelegate()->sizeHint(QStyleOptionViewItem(), index);
    rect.setSize(size);
    return rect;
}

bool IconView::getDelegateEditFlag()
{
    return m_delegate_editing;
}

int IconView::getSortType()
{
    int type = m_sort_filter_proxy_model->expectedSortType();
    return type<0? 0: type;
}

void IconView::setSortType(int sortType)
{
    m_sort_filter_proxy_model->sort(sortType, Qt::SortOrder(getSortOrder()));
}

int IconView::getSortOrder()
{
    return m_sort_filter_proxy_model->expectedSortOrder();
}

void IconView::setSortOrder(int sortOrder)
{
    m_sort_filter_proxy_model->sort(getSortType(), Qt::SortOrder(sortOrder));
}

const QStringList IconView::getAllFileUris()
{
    return m_sort_filter_proxy_model->getAllFileUris();
}

const int IconView::getAllDisplayFileCount()
{
    return m_sort_filter_proxy_model->rowCount();
}

void IconView::editUri(const QString &uri)
{
    setState(QListView::NoState);
    auto origin = FileUtils::getOriginalUri(uri);
    if(uri.startsWith("mtp://"))/* Fixbug#82649:在手机内部存储里新建文件/文件夹时，名称不是可编辑状态,都是默认文件名/文件夹名 */
        origin = uri;
    QModelIndex index = m_sort_filter_proxy_model->indexFromUri(origin);
    setIndexWidget(index, nullptr);
    qDebug() <<"editUri:" <<uri <<origin;
    QListView::scrollTo(index);
    edit(index);
//    if (! m_delegate_editing)
//        edit(m_sort_filter_proxy_model->indexFromUri(origin));
}

void IconView::editUris(const QStringList uris)
{
    //FIXME:
    //implement batch rename.
    setState(QListView::NoState);
    auto origin = FileUtils::getOriginalUri(uris.first());
    if(uris.first().startsWith("mtp://"))/* Fixbug#82649:在手机内部存储里新建文件/文件夹时，名称不是可编辑状态,都是默认文件名/文件夹名 */
        origin = uris.first();
    QModelIndex index = m_sort_filter_proxy_model->indexFromUri(origin);
    setIndexWidget(index, nullptr);
    qDebug() << "editUris:" << uris << origin;
    QListView::scrollTo(index);
    edit(index);
}

void IconView::selectAll()
{
    // fix: #62397
//    for (int i = 0; i < model()->rowCount(); i++) {
//        selectionModel()->select(model()->index(i, 0), QItemSelectionModel::Select);
//    }
    // optimize selectAll(). do not trigger selection changed signal to many times.
    QItemSelection selection(model()->index(0, 0), model()->index(model()->rowCount() - 1, 0));
    selectionModel()->select(selection, QItemSelectionModel::Select);
}

void IconView::clearIndexWidget()
{
    for (int i = 0; i < m_sort_filter_proxy_model->rowCount(); i++) {
        auto index = m_sort_filter_proxy_model->index(i, 0);
        setIndexWidget(index, nullptr);
        closePersistentEditor(index);
    }
}

void IconView::multiSelect()
{
    if (selectionMode() == MultiSelection) {
        return;
    }
    if (GlobalSettings::getInstance()->getValue(MULTI_SELECT).toBool()) {
        m_multi_select = true;
    }
    setSelectionMode(MultiSelection);
    viewport()->update(viewport()->rect());
    Q_EMIT updateSelectStatus(m_multi_select);
}

void IconView::disableMultiSelect()
{
    if (selectionMode() == ExtendedSelection) {
        return;
    }
    m_multi_select = false;
    setSelectionMode(ExtendedSelection);
    viewport()->update(viewport()->rect());
    Q_EMIT updateSelectStatus(m_multi_select);
}

void IconView::setSearchKey(const QString &key)
{
    auto viewItemDelegate = static_cast<IconViewDelegate *>(itemDelegate());
    viewItemDelegate->setSearchKeyword(key);
}

void IconView::edit(const QModelIndex &index)
{
    QListView::edit(index);
}

bool IconView::edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event)
{
    if (trigger == QAbstractItemView::AllEditTriggers) {
        //按照255个字节的高度设置
        int characterHeight = qApp->fontMetrics().height();
        int totalHeight = characterHeight * 255/4;

        m_scrollMax = verticalScrollBar()->maximum();
        int maxHeight = viewport()->height() - visualRect(index).y() - characterHeight - 15 - totalHeight;
        if (maxHeight + m_scrollMax < 0 && m_scrollMax >= 0) {
            m_increase = true;
            verticalScrollBar()->setRange(0, -maxHeight);
        }
    }
    return  QListView::edit(index, trigger, event);
}

QItemSelectionModel::SelectionFlags IconView::selectionCommand(const QModelIndex &index, const QEvent *event) const
{
    if (!event)
        return QListView::selectionCommand(index, event);

    if (event->type() == QEvent::MouseButtonPress) {
        auto e = static_cast<const QMouseEvent *>(event);
        if (e->button() == Qt::LeftButton && (e->modifiers() & Qt::ControlModifier || selectionMode() == MultiSelection) && selectedIndexes().contains(index)) {
            return QItemSelectionModel::NoUpdate;
        }
    } else if (event->type() == QEvent::MouseButtonRelease) {
        auto e = static_cast<const QMouseEvent *>(event);
        if (e->button() == Qt::LeftButton && (e->modifiers() & Qt::ControlModifier || selectionMode() == MultiSelection)) {
            QItemSelectionModel::SelectionFlags flags;
            if (m_noSelectOnPress) {
                flags = QItemSelectionModel::Deselect;
            }
            return flags;
        }
    }
    return QListView::selectionCommand(index, event);
}

void IconView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    QListView::closeEditor(editor,hint);
    verticalScrollBar()->setRange(0, m_scrollMax);
}

void IconView::doMultiSelect(bool isMultiSlelect)
{
    if (isMultiSlelect) {
        multiSelect();
    } else {
        disableMultiSelect();
    }
}

void IconView::setItemsVisible(bool visible)
{
    viewport()->setVisible(visible);
}

bool IconView::isEnableMultiSelect()
{
    return m_multi_select;
}

void IconView::releaseUnselect(bool select)
{
    m_mouse_release_unselect = select;
}

//Icon View 2
IconView2::IconView2(QWidget *parent) : DirectoryViewWidget(parent)
{
    auto layout = new QVBoxLayout(this);
    layout->setObjectName("_iconview2_layout");
    layout->setMargin(0);
    layout->setSpacing(0);
    m_view = new IconView(this);

    QString version = qApp->property("version").toString();
    if (version == "lingmo3.0") {
        m_zoom_level = 25;
    }

    DirectoryViewHelper * viewHelper = DirectoryViewHelper::globalInstance();
    viewHelper->addIconViewWithDirectoryViewWidget(m_view, this);
    connect(m_view, &IconView::updateSelectStatus, viewHelper, &DirectoryViewHelper::updateSelectStatus);

    int defaultZoomLevel = GlobalSettings::getInstance()->getValue(DEFAULT_VIEW_ZOOM_LEVEL).toInt();
    if (defaultZoomLevel >= minimumZoomLevel() && defaultZoomLevel <= maximumZoomLevel())
        m_zoom_level = defaultZoomLevel;

    connect(m_view, &IconView::zoomLevelChangedRequest, this, &IconView2::zoomRequest);

    layout->addWidget(m_view);

    setLayout(layout);
}

IconView2::~IconView2()
{

}

void IconView2::bindModel(FileItemModel *model, FileItemProxyFilterSortModel *proxyModel)
{
    auto layout = findChild<QVBoxLayout *>("_iconview2_layout");
    bool ok = false;
    if (parentWidget()) {
        int statusBarHeight = parentWidget()->property("statusBarHeight").toInt(&ok);
        if (ok) {
            layout->setContentsMargins(0, 0, 0, statusBarHeight);
        }
    }

    disconnect(m_model);
    disconnect(m_proxy_model);
    m_model = model;
    m_proxy_model = proxyModel;

    m_view->bindModel(model, proxyModel);
    connect(m_model, &FileItemModel::selectRequest, this, &DirectoryViewWidget::updateWindowSelectionRequest);
    connect(m_model,&FileItemModel::signal_itemAdded, this, [=](const QString& uri){
        Q_EMIT this->signal_itemAdded(uri);
    });
    connect(model, &FileItemModel::findChildrenFinished, this, &DirectoryViewWidget::viewDirectoryChanged);
    //connect(m_model, &FileItemModel::dataChanged, m_view, &IconView::clearIndexWidget);
    //connect(m_model, &FileItemModel::updated, m_view, &IconView::resort);
    connect(m_model, &FileItemModel::updated, m_view->viewport(), [=]{
        if (this->cursor().shape() == Qt::BusyCursor || this->cursor().shape() == Qt::WaitCursor) {
            return;
        }
        repaintView();
    });

    connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=]() {
        Q_EMIT viewSelectionChanged();
    });

    connect(m_view, &IconView::activated, this, [=](const QModelIndex &index) {
        //平板模式下，长按打开文件处理
        if (m_view->m_touch_active_timer->isActive()) {
            auto costTime = m_view->m_touch_active_timer->interval() - m_view->m_touch_active_timer->remainingTime();
            if (costTime > qApp->doubleClickInterval()) {
                m_view->m_touch_active_timer->stop();
                return;
            }
        }

        //when selections is more than 1, let mainwindow to process
        if (getSelections().count() != 1)
            return;
        auto uri = getSelections().first();
        //process open symbolic link
       // auto info = FileInfo::fromUri(uri);
       // if (info->isSymbolLink() && uri.startsWith("file://") && info->isValid())
        //    uri = "file://" +  FileUtils::getSymbolicTarget(uri);
        if(!m_view->isEnableMultiSelect())
            Q_EMIT this->viewDoubleClicked(uri);

        m_view->m_touch_active_timer->stop();
    });

    connect(m_view, &IconView::customContextMenuRequested, this, [=](const QPoint &pos) {
        if (m_view->m_delegate_editing) {
            return;
        }

        m_menuRequesting = true;

        // we should clear the dirty rubber band due to call context menu.
        bool isDragSelecting = m_view->isDraggingState();
        if (isDragSelecting) {
            // send a fake mouse release event for clear the rubber band.
            // note that we should pass mouse move event durring delaying requesting
            // directory view menu, otherwize both dnd and menu action will be triggered.
            m_view->setIgnore_mouse_move_event(true);
            QMouseEvent fakeEvent(QMouseEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            qApp->sendEvent(m_view, &fakeEvent);
            m_view->viewport()->repaint();
        }

        bool indexWidgetContainsPos = false;
        auto indexWidget = m_view->indexWidget(m_view->currentIndex());
        if (indexWidget) {
            indexWidgetContainsPos = indexWidget->geometry().contains(pos);
        }
        if (!m_view->indexAt(pos).isValid() && !indexWidgetContainsPos)
            m_view->clearSelection();

        //NOTE: we have to ensure that we have cleared the
        //selection if menu request at blank pos.
        QTimer::singleShot(isDragSelecting? 300: 1, this, [=]() {
            m_view->setIgnore_mouse_move_event(false);
            m_view->m_touch_active_timer->stop();
            Q_EMIT this->menuRequest(mapToGlobal(pos));
            m_menuRequesting = false;
        });
    });

    connect(m_proxy_model, &FileItemProxyFilterSortModel::layoutChanged, this, [=]() {
        Q_EMIT this->sortOrderChanged(Qt::SortOrder(getSortOrder()));
    });
    connect(m_proxy_model, &FileItemProxyFilterSortModel::layoutChanged, this, [=]() {
        Q_EMIT this->sortTypeChanged(getSortType());
    });
}

void IconView2::repaintView()
{
    if (m_menuRequesting)
        return;
    m_view->update();
    m_view->viewport()->update();
}

void IconView2::setCurrentZoomLevel(int zoomLevel)
{
    if (zoomLevel <= maximumZoomLevel() && zoomLevel >= minimumZoomLevel()) {
        m_zoom_level = zoomLevel;
        //FIXME: implement zoom
        int base = 16; //50
        QString version = qApp->property("version").toString();
        if (version == "lingmo3.0") {
             base = 64 - 25;
        }
        int adjusted = base + zoomLevel;
        m_view->setIconSize(QSize(adjusted, adjusted));
        m_view->setGridSize(m_view->itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex()) + QSize(20, 20));
    }
}

void IconView2::clearIndexWidget()
{
    for (auto index : m_proxy_model->getAllFileIndexes()) {
        m_view->closePersistentEditor(index);
        m_view->setIndexWidget(index, nullptr);
    }
}


