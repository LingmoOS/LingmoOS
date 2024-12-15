// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "packagelistview.h"
#include "model/abstract_package_list_model.h"
#include "utils/utils.h"
#include "singleInstallerApplication.h"

#include <QPainter>
#include <QShortcut>
#include <QScroller>

PackagesListView::PackagesListView(QWidget *parent)
    : DListView(parent)
    , m_bLeftMouse(false)
    , m_rightMenu(nullptr)
{
    initUI();                // 初始化界面参数
    initConnections();       // 初始化链接
    initRightContextMenu();  // 初始化右键菜单
    initShortcuts();         // 添加快捷删除键
}

/**
 * @brief PackagesListView::initUI 初始化listView参数
 */
void PackagesListView::initUI()
{
    QScroller::grabGesture(this, QScroller::TouchGesture);  // 添加触控屏触控

    setVerticalScrollMode(ScrollPerPixel);              // 设置垂直滚动的模式
    setSelectionMode(QListView::SingleSelection);       // 只允许单选
    setAutoScroll(true);                                // 允许自动滚动
    setMouseTracking(true);                             // 设置鼠标跟踪
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);  // 滚动条一直存在
}

/**
 * @brief PackagesListView::initConnections 初始化链接
 */
void PackagesListView::initConnections()
{
    // 鼠标右键点击，根据条件判断是否需要弹出右键菜单
    connect(this,
            &PackagesListView::signalShowContextMenu,
            this,
            &PackagesListView::slotListViewShowContextMenu,
            Qt::ConnectionType::QueuedConnection);
}

/**
 * @brief PackagesListView::initShortcuts 添加快捷删除键
 */
void PackagesListView::initShortcuts()
{
    if (SingleInstallerApplication::mode == SingleInstallerApplication::NormalChannel) {
        QShortcut *deleteShortcut = new QShortcut(QKeySequence::Delete, this);  // 初始化快捷键
        deleteShortcut->setContext(Qt::ApplicationShortcut);                    // 设置快捷键的显示提示
        connect(deleteShortcut, &QShortcut::activated, this, &PackagesListView::slotShortcutDeleteAction);  // 链接快捷键
    }
}

/**
 * @brief PackagesListView::mousePressEvent 鼠标按下事件
 * @param event
 * 如果左键按下，则置标志位为true
 * 解决后续右键菜单左键取消的问题
 */
void PackagesListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {  // 当前检测到鼠标左键按下
        m_bLeftMouse = true;
    } else {
        m_bLeftMouse = false;  // 当前按下的不是左键
    }
    DListView::mousePressEvent(event);
}

/**
 * @brief PackagesListView::mouseReleaseEvent 鼠标释放事件
 * @param event
 * 判断当前model是否准备就绪
 */
void PackagesListView::mouseReleaseEvent(QMouseEvent *event)
{
    AbstractPackageListModel *listModel = qobject_cast<AbstractPackageListModel *>(this->model());  // 获取debListModel
    if (!listModel->isWorkerPrepare()) {                                                            // 当前model未就绪
        return;
    }
    emit signalCurrentIndexRow(m_currentIndex);
    DListView::mouseReleaseEvent(event);
}

/**
 * @brief PackagesListView::setSelection 设置选中
 * @param rect
 * @param command
 */
void PackagesListView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    DListView::setSelection(rect, command);  // 转发消息

    QPoint clickPoint(rect.x(), rect.y());
    QModelIndex modelIndex = indexAt(clickPoint);

    m_highlightIndex = modelIndex;
    m_currModelIndex = m_highlightIndex;
    if (!m_bLeftMouse) {
        m_bShortcutDelete = false;               // 不允许删除
        emit signalShowContextMenu(modelIndex);  // 显示右键菜单
    } else {
        m_bShortcutDelete = true;
    }
}

/**
 * @brief PackagesListView::keyPressEvent 键盘按键按下
 * @param event
 */
void PackagesListView::keyPressEvent(QKeyEvent *event)
{
    m_bLeftMouse = true;
    // 添加 右键菜单快捷键
    if ((event->modifiers() == Qt::AltModifier) && (event->key() == Qt::Key_M)) {
        if (this->selectionModel()->currentIndex().row() > -1) {
            m_bShortcutDelete = false;
            // 增加右键菜单的调出判断，当前有焦点且状态为允许右键菜单出现
            if (this->hasFocus() && m_bIsRightMenuShow) {
                // 右键菜单的触发位置为当前item的位置。此前为鼠标的位置。
                m_rightMenu->exec(mapToGlobal(m_rightMenuPos));
            }
        }
    }
    AbstractPackageListModel *listModel = qobject_cast<AbstractPackageListModel *>(this->model());  // 获取debListModel
    if (listModel) {
        if (listModel->isWorkerPrepare()) {  // 当前model未就绪
            if (event->key() == Qt::Key_Down) {  // 默认tab键获取焦点行是第一行，按down键行号+1，但不超过listview的最大行数
                m_currentIndex = qMin(m_currentIndex + 1, this->model()->rowCount() - 1);
                emit signalCurrentIndexRow(m_currentIndex);
            }
            if (event->key() == Qt::Key_Up) {  // 按up键行号-1，但不小于0
                m_currentIndex = qMax(m_currentIndex - 1, 0);
                emit signalCurrentIndexRow(m_currentIndex);
            }
        }
    }
    DListView::keyPressEvent(event);
}

/**
 * @brief PackagesListView::initRightContextMenu 初始化右键菜单
 */
void PackagesListView::initRightContextMenu()
{
    if (nullptr == m_rightMenu &&
        SingleInstallerApplication::mode == SingleInstallerApplication::NormalChannel) {  // 当前右键菜单未初始化
        m_rightMenu = new DMenu(this);                                                    // 初始化右键菜单

        // 给右键菜单添加快捷键Delete
        QAction *deleteAction = new QAction(tr("Delete"), this);

        m_rightMenu->addAction(deleteAction);  // 右键菜单添加action
        connect(deleteAction, &QAction::triggered, this, &PackagesListView::slotRightMenuDeleteAction);  // action 添加链接事件
    }
}

/**
 * @brief PackagesListView::onListViewShowContextMenu  显示右键菜单
 * @param index
 */
void PackagesListView::slotListViewShowContextMenu(QModelIndex index)
{
    Q_UNUSED(index)
    AbstractPackageListModel *listModel = qobject_cast<AbstractPackageListModel *>(this->model());  // 获取debListModel
    if (listModel) {
        if (listModel->isWorkerPrepare())  // 当前model未就绪
            emit signalCurrentIndexRow(index.row());
    }

    m_bShortcutDelete = false;  // 右键菜单显示时不允许使用快捷键删除
    m_currModelIndex = index;
    // 修改右键菜单的调出判断，当前有焦点且状态为允许右键菜单出现
    if (m_bIsRightMenuShow && m_rightMenu != nullptr) {
        // 在当前鼠标位置显示右键菜单
        m_rightMenu->exec(QCursor::pos());
    }
}

/**
 * @brief PackagesListView::onShortcutDeleteAction 快捷键删除
 */
void PackagesListView::slotShortcutDeleteAction()
{
    // fix bug: 42602 添加多个deb包到软件包安装器，选择列表中任一应用，连续多次点击delete崩溃
    if (-1 == m_currModelIndex.row() || m_rightMenu->isVisible() || this->count() == 1) {
        return;
    }

    // fix bug: 42602 添加多个deb包到软件包安装器，选择列表中任一应用，连续多次点击delete崩溃
    // fix bug: 44901 https://pms.uniontech.com/zentao/bug-view-44901.htm
    //  只删除当前选中的项
    if (m_currModelIndex.row() < this->count() && this->selectionModel()->selectedIndexes().contains(m_currModelIndex))
        emit signalRemoveItemClicked(m_currModelIndex);
}

/**
 * @brief PackagesListView::onRightMenuDeleteAction 右键菜单删除选中的项
 */
void PackagesListView::slotRightMenuDeleteAction()
{
    if (-1 == m_currModelIndex.row()) {
        return;
    }

    emit signalRemoveItemClicked(m_currModelIndex);
}

/**
 * @brief PackagesListView::paintEvent 获取当前index
 * @param event
 * 定位右键菜单弹出的位置
 */
void PackagesListView::paintEvent(QPaintEvent *event)
{
    // 获取currentIndex并保存，用于右键菜单的定位。
    m_currentIndex = this->currentIndex().row();
    DListView::paintEvent(event);
}

/**
 * @brief PackagesListView::getPos
 * @param rect 某一个Item的rect
 * @param index Item的index.row()
 *
 * 获取某一个Item的rect,与index。用于保存位置参数触发右键菜单
 */
void PackagesListView::slotGetPos(QRect rect, int index)
{
    if (index == m_currentIndex) {  // 获取当前项右键菜单出现的位置
        m_rightMenuPos.setX(rect.x() + rect.width() - 162);
        m_rightMenuPos.setY(rect.y() + rect.height() / 2);
    }
}

/**
 * @brief PackagesListView::setRightMenuShowStatus 设置是否可以显示右键菜单
 * @param isShow
 */
void PackagesListView::setRightMenuShowStatus(bool isShow)
{
    m_bIsRightMenuShow = isShow;  // 设置右键菜单是否显示的标识位
}

/**
 * @brief PackagesListView::focusInEvent 焦点FocusIn事件
 * @param event
 * 当焦点切换到PackListView时，默认选中第一项。
 */
void PackagesListView::focusInEvent(QFocusEvent *event)
{
    if (event->reason() == Qt::TabFocusReason) {  // tab焦点
        if (this->count() > 0) {
            m_currModelIndex = this->model()->index(0, 0);
            this->setCurrentIndex(m_currModelIndex);  // 存在焦点时，默认选入第一项
            m_currentIndex = 0;
            AbstractPackageListModel *listModel = qobject_cast<AbstractPackageListModel *>(this->model());  // 获取debListModel
            if (listModel) {
                if (listModel->isWorkerPrepare())  // 当前model未就绪
                    emit signalCurrentIndexRow(0);
            }
        }
    }
}

void PackagesListView::focusOutEvent(QFocusEvent *event)
{
    // not change focus index when the right menu pop-up.
    if (event->reason() != Qt::PopupFocusReason) {
        this->clearSelection();
        m_currentIndex = -1;
        m_currModelIndex = this->model()->index(-1, -1);
        emit signalCurrentIndexRow(-1);
    }

    DListView::focusOutEvent(event);
}

/**
 * @brief event 事件
 */
bool PackagesListView::event(QEvent *event)
{
    // 字体变化事件
    if (event->type() == QEvent::FontChange) {
        qInfo() << DFontSizeManager::fontPixelSize(qGuiApp->font());
        if (DFontSizeManager::fontPixelSize(qGuiApp->font()) <= 13) {  // 当前字体大小是否小于13
            emit signalChangeItemHeight(50 - 2 * (13 - DFontSizeManager::fontPixelSize(qGuiApp->font())));
        } else {
            emit signalChangeItemHeight(52 + 2 * (DFontSizeManager::fontPixelSize(qGuiApp->font()) - 13));
        }
    }

    return DListView::event(event);
}
