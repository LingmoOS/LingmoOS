// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "leftview.h"
#include "leftviewdelegate.h"
#include "leftviewsortfilter.h"
#include "globaldef.h"
#include "moveview.h"

#include "dialog/folderselectdialog.h"

#include "common/actionmanager.h"
#include "common/standarditemcommon.h"
#include "common/vnoteforlder.h"
#include "common/vnoteitem.h"
#include "common/setting.h"
#include "widgets/vnoterightmenu.h"
#include "db/vnoteitemoper.h"

#include <DApplication>
#include <DWindowManagerHelper>
#include <QMouseEvent>
#include <QDebug>
#include <QDrag>
#include <QMimeData>
#include <QDebug>
#include <QTimer>
#include <QScrollBar>

/**
 * @brief LeftView::LeftView
 * @param parent
 */
LeftView::LeftView(QWidget *parent)
    : DTreeView(parent)
{
    initModel();
    initDelegate();
    initMenu();
    initConnections();
    setContextMenuPolicy(Qt::NoContextMenu);
    this->setDragEnabled(true);
    this->setDragDropMode(QAbstractItemView::DragDrop);
    this->setDropIndicatorShown(true);
    this->setAcceptDrops(true);
    viewport()->installEventFilter(this);
    this->installEventFilter(this);
}

/**
 * @brief LeftView::initModel
 */
void LeftView::initModel()
{
    m_pDataModel = new QStandardItemModel(this);
    m_pSortViewFilter = new LeftViewSortFilter(this);
    m_pSortViewFilter->setDynamicSortFilter(false);
    m_pSortViewFilter->setSourceModel(m_pDataModel);
    this->setModel(m_pSortViewFilter);
}

/**
 * @brief LeftView::initDelegate
 */
void LeftView::initDelegate()
{
    m_pItemDelegate = new LeftViewDelegate(this);
    this->setItemDelegate(m_pItemDelegate);
}

/**
 * @brief LeftView::getNotepadRoot
 * @return 记事本项父节点
 */
QStandardItem *LeftView::getNotepadRoot()
{
    QStandardItem *pItem = m_pDataModel->item(0);
    if (pItem == nullptr) {
        pItem = StandardItemCommon::createStandardItem(nullptr, StandardItemCommon::NOTEPADROOT);
        m_pDataModel->insertRow(0, pItem);
    }
    return pItem;
}

/**
 * @brief LeftView::getNotepadRootIndex
 * @return 记事本父节点索引
 */
QModelIndex LeftView::getNotepadRootIndex()
{
    return m_pSortViewFilter->mapFromSource(getNotepadRoot()->index());
}

/**
 * @brief LeftView::mousePressEvent
 * @param event
 */
void LeftView::mousePressEvent(QMouseEvent *event)
{
    this->setFocus();
    if (!m_onlyCurItemMenuEnable) {
        //触控屏手势
        if (event->source() == Qt::MouseEventSynthesizedByQt) {
            //记录触控起始位置与时间点
            m_touchPressPoint = event->pos();
            m_touchPressStartMs = QDateTime::currentDateTime().toMSecsSinceEpoch();
            //更新触摸状态
            setTouchState(TouchPressing);
            m_index = indexAt(event->pos());
            m_notepadMenu->setPressPoint(QCursor::pos());
            //检查是否选中
            m_selectCurrentTimer->start(250);
            //是否弹出右键菜单
            m_popMenuTimer->start(1000);
            return;
        }
        event->setModifiers(Qt::NoModifier);
        setTouchState(TouchState::TouchPressing);
        //不使用自动判断
        //        DTreeView::mousePressEvent(event);
        if (event->button() == Qt::RightButton) {
            if (MenuStatus::ReleaseFromMenu == m_menuState) {
                m_menuState = MenuStatus::Normal;
                return;
            }
            QModelIndex index = this->indexAt(event->pos());
            if (StandardItemCommon::getStandardItemType(index) == StandardItemCommon::NOTEPADITEM
                && (!m_onlyCurItemMenuEnable || index == this->currentIndex())) {
                this->setCurrentIndex(index);
                m_notepadMenu->popup(event->globalPos());
                //通过此方法隐藏菜单，在处理拖拽事件结束后hide
                m_notepadMenu->setWindowOpacity(1);
            }
        }
    } else {
        //置灰状态下只有当前记事本可操作
        if (currentIndex() == indexAt(event->pos())) {
            if (event->source() == Qt::MouseEventSynthesizedByQt) {
                m_touchPressPoint = event->pos();
                m_touchPressStartMs = QDateTime::currentDateTime().toMSecsSinceEpoch();
                //更新触摸状态
                setTouchState(TouchPressing);
                m_notepadMenu->setPressPoint(QCursor::pos());
                m_popMenuTimer->start(1000);
                return;
            } else {
                if (Qt::RightButton == event->button()) {
                    m_notepadMenu->setWindowOpacity(1);
                    m_notepadMenu->popup(event->globalPos());
                }
            }
        }
    }
}

/**
 * @brief LeftView::mouseReleaseEvent
 * @param event
 */
void LeftView::mouseReleaseEvent(QMouseEvent *event)
{
    m_isDraging = false;
    //停止计时器
    m_selectCurrentTimer->stop();
    m_popMenuTimer->stop();
    m_menuState = MenuStatus::Normal;
    //处理拖拽事件，由于与drop操作参数不同，暂未封装
    if (m_touchState == TouchState::TouchDraging) {
        setTouchState(TouchState::TouchNormal);
        return;
    }
    if (m_onlyCurItemMenuEnable) {
        return;
    }
    //正常点击状态，选择当前点击选项
    QModelIndex index = indexAt(event->pos());
    if (index.row() != currentIndex().row() && m_touchState == TouchState::TouchPressing) {
        if (index.isValid()) {
            setCurrentIndex(index);
        }
        setTouchState(TouchState::TouchNormal);
        return;
    }
    setTouchState(TouchState::TouchNormal);
    DTreeView::mouseReleaseEvent(event);
}

/**
 * @brief LeftView::setTouchState 更新触摸屏一指状态
 * @param touchState
 */
void LeftView::setTouchState(const TouchState &touchState)
{
    m_touchState = touchState;
}

/**
 * @brief LeftView::mouseDoubleClickEvent
 * @param event
 */
void LeftView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_onlyCurItemMenuEnable) {
        DTreeView::mouseDoubleClickEvent(event);
    }

    //左键双击事件
    if (event->button() == Qt::LeftButton) {
        if (indexAt(event->pos()) == currentIndex())
            editFolder(); //在记事本列表双击左键进入重命名状态
    }
}

/**
 * @brief LeftView::mouseMoveEvent
 * @param event
 */
void LeftView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_onlyCurItemMenuEnable) {
        return;
    }
    //处理触摸屏一指操作
    if ((event->source() == Qt::MouseEventSynthesizedByQt && event->buttons() & Qt::LeftButton)) {
        if (TouchState::TouchOutVisibleRegion != m_touchState) {
            doTouchMoveEvent(event);
        }
        return;
    }
    //处理鼠标拖拽操作
    else if ((event->buttons() & Qt::LeftButton) && m_touchState == TouchState::TouchPressing) {
        if (!m_isDraging && indexAt(event->pos()).isValid()) {
            setCurrentIndex(indexAt(event->pos()));
            //需判断移动距离
            if (qAbs(event->pos().x() - m_touchPressPoint.x()) > 3
                || qAbs(event->pos().y() - m_touchPressPoint.y()) > 3) {
                handleDragEvent(false);
            }
        }
    } else {
        DTreeView::mouseMoveEvent(event);
    }
}

/**
 * @brief LeftView::doTouchMoveEvent  处理触摸屏move事件，区分点击、滑动、拖拽、长按功能
 * @param event
 */
void LeftView::doTouchMoveEvent(QMouseEvent *event)
{
    //处理触摸屏单指move事件，区分滑动、拖拽事件
    //    m_pItemDelegate->setDraging(false);
    double distX = event->pos().x() - m_touchPressPoint.x();
    double distY = event->pos().y() - m_touchPressPoint.y();
    //获取时间间隔
    QDateTime current = QDateTime::currentDateTime();
    qint64 timeParam = current.toMSecsSinceEpoch() - m_touchPressStartMs;

    switch (m_touchState) {
    //首次判断
    case TouchState::TouchPressing:
        //250ms-1000ms之间移动超过10px，判断为拖拽
        if ((timeParam > 250 && timeParam < 1000) && (qAbs(distY) > 10 || qAbs(distX) > 10)) {
            if (!m_isDraging)
                handleDragEvent();
        }
        //250ms之内，上下移动距离超过10px或左右移动距离超过5px，判断为滑动
        else if (timeParam <= 250 && (qAbs(distY) > 10 || qAbs(distX) > 5)) {
            setTouchState(TouchState::TouchMoving);
            handleTouchSlideEvent(timeParam, distY, event->pos());
        }
        break;
    //如果正在拖拽
    case TouchState::TouchDraging:
        if (!m_isDraging)
            handleDragEvent();
        break;
    //如果正在滑动
    case TouchState::TouchMoving:
        if (!viewport()->visibleRegion().contains(event->pos())) {
            setTouchState(TouchState::TouchOutVisibleRegion);
        } else if (qAbs(distY) > 5) {
            handleTouchSlideEvent(timeParam, distY, event->pos());
        }
        break;
    default:
        break;
    }
}

/**
 * @brief LeftView::handleDragEvent
 * @param isTouch 是否触屏
 */
void LeftView::handleDragEvent(bool isTouch)
{
    if (m_onlyCurItemMenuEnable)
        return;
    if (isTouch) {
        setTouchState(TouchState::TouchDraging);
    }
    m_popMenuTimer->stop();
    m_notepadMenu->setWindowOpacity(0.0);
    triggerDragFolder();
}

/**
 * @brief LeftView::keyPressEvent
 * @param e
 */
void LeftView::keyPressEvent(QKeyEvent *e)
{
    if (m_onlyCurItemMenuEnable || e->key() == Qt::Key_PageUp || e->key() == Qt::Key_PageDown) {
        e->ignore();
    } else {
        if (0 == this->currentIndex().row() && e->key() == Qt::Key_Up) {
            e->ignore();
        } else if (e->key() == Qt::Key_Home) {
            //如果自动处理键盘home事件，会导致首个index与根节点重合，导致实际选项为空，此处手动设置选中
            this->setCurrentIndex(m_pSortViewFilter->index(0, 0, getNotepadRootIndex()));
        } else {
            DTreeView::keyPressEvent(e);
        }
    }
}

/**
 * @brief LeftView::restoreNotepadItem
 * @return 当前选中项
 */
QModelIndex LeftView::restoreNotepadItem()
{
    QModelIndex index = this->currentIndex();
    QItemSelectionModel *model = this->selectionModel();

    if (index.isValid() && StandardItemCommon::getStandardItemType(index) == StandardItemCommon::NOTEPADITEM) {
        if (!model->isSelected(index)) {
            this->setCurrentIndex(index);
        }
    } else {
        index = setDefaultNotepadItem();
    }

    return index;
}

/**
 * @brief LeftView::setDefaultNotepadItem
 * @return 当前选中项
 */
QModelIndex LeftView::setDefaultNotepadItem()
{
    QModelIndex index = m_pSortViewFilter->index(0, 0, getNotepadRootIndex());
    this->setCurrentIndex(index);
    return index;
}

/**
 * @brief LeftView::addFolder
 * @param folder
 */
void LeftView::addFolder(VNoteFolder *folder)
{
    if (nullptr != folder) {
        QStandardItem *pItem = StandardItemCommon::createStandardItem(
            folder, StandardItemCommon::NOTEPADITEM);

        QStandardItem *root = getNotepadRoot();
        root->appendRow(pItem);
        QModelIndex index = m_pSortViewFilter->mapFromSource(pItem->index());
        setCurrentIndex(index);
    }
    this->scrollToTop();
}

/**
 * @brief LeftView::eventFilter
 * @param o
 * @param e
 * @return false 不过滤，事件正常处理
 */
bool LeftView::eventFilter(QObject *o, QEvent *e)
{
    if (o == this) {
        if (e->type() == QEvent::FocusIn) {
            QFocusEvent *event = dynamic_cast<QFocusEvent *>(e);
            m_pItemDelegate->setTabFocus(event->reason() == Qt::TabFocusReason);
            if (m_pItemDelegate->isTabFocus()) {
                scrollTo(currentIndex());
            }
        } else if (e->type() == QEvent::FocusOut) {
            m_pItemDelegate->setTabFocus(false);
        }
    } else {
        if (e->type() == QEvent::DragLeave) {
            m_pItemDelegate->setDrawHover(false);
            update();
        } else if (e->type() == QEvent::DragEnter) {
            m_pItemDelegate->setDrawHover(true);
            update();
        }
    }
    return false;
}

/**
 * @brief LeftView::appendFolder
 * @param folder
 */
void LeftView::appendFolder(VNoteFolder *folder)
{
    if (nullptr != folder) {
        QStandardItem *pItem = StandardItemCommon::createStandardItem(
            folder, StandardItemCommon::NOTEPADITEM);

        QStandardItem *root = getNotepadRoot();

        if (nullptr != root) {
            root->appendRow(pItem);
        }
    }
}

/**
 * @brief LeftView::editFolder
 */
void LeftView::editFolder()
{
    edit(currentIndex());
}

/**
 * @brief LeftView::removeFolder
 * @return 删除项绑定的数据
 */
VNoteFolder *LeftView::removeFolder()
{
    QModelIndex index = currentIndex();

    if (StandardItemCommon::getStandardItemType(index) != StandardItemCommon::NOTEPADITEM) {
        return nullptr;
    }

    VNoteFolder *data = reinterpret_cast<VNoteFolder *>(
        StandardItemCommon::getStandardItemData(index));

    m_pSortViewFilter->removeRow(index.row(), index.parent());

    return data;
}

/**
 * @brief LeftView::folderCount
 * @return 记事本个数
 */
int LeftView::folderCount()
{
    int count = 0;

    QStandardItem *root = getNotepadRoot();

    if (nullptr != root) {
        count = root->rowCount();
    }

    return count;
}

/**
 * @brief LeftView::initMenu
 */
void LeftView::initMenu()
{
    m_notepadMenu = ActionManager::Instance()->notebookContextMenu();
}

/**
 * @brief LeftView::initConnections
 */
void LeftView::initConnections()
{
    //右键菜单滑动
    connect(m_notepadMenu, &VNoteRightMenu::menuTouchMoved, this, &LeftView::handleDragEvent);
    //右键菜单释放
    connect(m_notepadMenu, &VNoteRightMenu::menuTouchReleased, this, [=] {
        m_touchState = TouchState::TouchNormal;
        m_menuState = MenuStatus::ReleaseFromMenu;
    });
    //定时器用于判断是否选中当前
    m_selectCurrentTimer = new QTimer(this);
    connect(m_selectCurrentTimer, &QTimer::timeout, [=] {
        if (m_touchState == TouchState::TouchPressing && m_index.isValid())
            this->setCurrentIndex(m_index);
        m_selectCurrentTimer->stop();
    });
    //定时器用于判断是否弹出菜单
    m_popMenuTimer = new QTimer(this);
    connect(m_popMenuTimer, &QTimer::timeout, [=] {
        if (m_touchState == TouchState::TouchPressing && m_index.isValid()) {
            m_notepadMenu->setWindowOpacity(1);
            m_notepadMenu->exec(QCursor::pos());
        }
        m_popMenuTimer->stop();
    });
}

/**
 * @brief LeftView::setOnlyCurItemMenuEnable
 * @param enable
 */
void LeftView::setOnlyCurItemMenuEnable(bool enable)
{
    m_onlyCurItemMenuEnable = enable;
    m_pItemDelegate->setEnableItem(!enable);
    this->update();
}

/**
 * @brief LeftView::sort
 */
void LeftView::sort()
{
    return m_pSortViewFilter->sort(0, Qt::DescendingOrder);
}

/**
 * @brief LeftView::closeEditor
 * @param editor
 * @param hint
 */
void LeftView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    Q_UNUSED(hint);
    DTreeView::closeEditor(editor, QAbstractItemDelegate::NoHint);
}

/**
 * @brief LeftView::closeMenu
 */
void LeftView::closeMenu()
{
    m_notepadMenu->close();
}

/**
 * @brief LeftView::doNoteMove
 * @param src
 * @param dst
 * @return
 */
bool LeftView::doNoteMove(const QModelIndexList &src, const QModelIndex &dst)
{
    if (src.size() && StandardItemCommon::getStandardItemType(dst) == StandardItemCommon::NOTEPADITEM) {
        VNoteFolder *selectFolder = static_cast<VNoteFolder *>(StandardItemCommon::getStandardItemData(dst));
        VNoteItem *tmpData = static_cast<VNoteItem *>(StandardItemCommon::getStandardItemData(src[0]));
        if (selectFolder && tmpData->folderId != selectFolder->id) {
            VNoteItemOper noteOper;
            VNOTE_ITEMS_MAP *srcNotes = noteOper.getFolderNotes(tmpData->folderId);
            VNOTE_ITEMS_MAP *destNotes = noteOper.getFolderNotes(selectFolder->id);
            for (auto it : src) {
                tmpData = static_cast<VNoteItem *>(StandardItemCommon::getStandardItemData(it));
                //更新内存数据
                srcNotes->lock.lockForWrite();
                srcNotes->folderNotes.remove(tmpData->noteId);
                srcNotes->lock.unlock();

                destNotes->lock.lockForWrite();
                tmpData->folderId = selectFolder->id;
                destNotes->folderNotes.insert(tmpData->noteId, tmpData);
                destNotes->lock.unlock();
                //更新数据库
                noteOper.updateFolderId(tmpData);
            }

            //全部移除后重置当前记事本maxid
            if (src.count() == m_notesNumberOfCurrentFolder) {
                VNoteFolder *folder = reinterpret_cast<VNoteFolder *>(StandardItemCommon::getStandardItemData(currentIndex()));
                folder->maxNoteIdRef() = 0;
            } else {
                selectFolder->maxNoteIdRef() += src.size();
            }
            return true;
        }
    }
    return false;
}

/**
 * @brief LeftView::selectMoveFolder
 * @param src
 * @return
 */
QModelIndex LeftView::selectMoveFolder(const QModelIndexList &src)
{
    QModelIndex index;
    if (src.size()) {
        VNoteItem *data = static_cast<VNoteItem *>(StandardItemCommon::getStandardItemData(src[0]));
        QString elideText = data->noteTitle;
        if (m_folderSelectDialog == nullptr) {
            m_folderSelectDialog = new FolderSelectDialog(m_pDataModel, this);
            m_folderSelectDialog->resize(VNOTE_SELECTDIALOG_W, 372);
        }
        m_folderSelectDialog->setFocus();
        QList<VNoteFolder *> folders;
        folders.push_back(static_cast<VNoteFolder *>(StandardItemCommon::getStandardItemData(currentIndex())));
        m_folderSelectDialog->setFolderBlack(folders);
        m_folderSelectDialog->setNoteContextInfo(elideText, src.size());
        m_folderSelectDialog->clearSelection();

        m_folderSelectDialog->exec();
        if (m_folderSelectDialog->result() == FolderSelectDialog::Accepted) {
            index = m_folderSelectDialog->getSelectIndex();
        }
    }
    return index;
}

/**
 * @brief LeftView::getFolderSort
 * @return 当前所有记事本的排序编号字符串
 * 获取记事本顺序
 */
QString LeftView::getFolderSort()
{
    QString tmpQstr = "";
    QModelIndex rootIndex = getNotepadRootIndex();
    QModelIndex currentIndex;
    for (int i = 0; i < folderCount(); i++) {
        currentIndex = m_pSortViewFilter->index(i, 0, rootIndex);
        if (!currentIndex.isValid()) {
            break;
        }
        VNoteFolder *data = reinterpret_cast<VNoteFolder *>(
            StandardItemCommon::getStandardItemData(currentIndex));
        if (tmpQstr.isEmpty()) {
            tmpQstr = QString::number(data->id);
        } else {
            tmpQstr = tmpQstr + "," + QString::number(data->id);
        }
    }
    return tmpQstr;
}

/**
 * @brief LeftView::setFolderSort
 * @return true 排序成功，false 排序失败
 * 设置记事本默认顺序
 */
bool LeftView::setFolderSort()
{
    bool sortResult = false;
    QModelIndex rootIndex = getNotepadRootIndex();
    QModelIndex currentIndex;
    int rowCount = folderCount();
    for (int i = 0; i < rowCount; i++) {
        currentIndex = m_pSortViewFilter->index(i, 0, rootIndex);
        if (!currentIndex.isValid()) {
            break;
        }
        VNoteFolder *data = reinterpret_cast<VNoteFolder *>(
            StandardItemCommon::getStandardItemData(currentIndex));
        if (nullptr != data) {
            reinterpret_cast<VNoteFolder *>(
                StandardItemCommon::getStandardItemData(currentIndex))
                ->sortNumber = rowCount - i;
        }
        sortResult = true;
    }
    return sortResult;
}

/**
 * @brief LeftView::getFirstFolder
 * @return 第一个记事本
 */
VNoteFolder *LeftView::getFirstFolder()
{
    QModelIndex rootIndex = getNotepadRootIndex();
    QModelIndex child = m_pSortViewFilter->index(0, 0, rootIndex);
    if (child.isValid()) {
        VNoteFolder *vnotefolder = reinterpret_cast<VNoteFolder *>(
            StandardItemCommon::getStandardItemData(child));
        return vnotefolder;
    }
    return nullptr;
}

/**
 * @brief LeftView::handleTouchSlideEvent  处理触摸屏move事件，区分点击、滑动、拖拽、长按功能
 * @param timeParam 时间间隔
 * @param distY 纵向移动距离
 * @param point 当前时间发生位置
 */
void LeftView::handleTouchSlideEvent(qint64 timeParam, double distY, QPoint point)
{
    //根据移动距离与时间计算滑动速度，用于设置滚动步长
    double param = ((qAbs(distY)) / timeParam) + 0.3;
    verticalScrollBar()->setSingleStep(static_cast<int>(20 * param));
    verticalScrollBar()->triggerAction((distY > 0) ? QScrollBar::SliderSingleStepSub : QScrollBar::SliderSingleStepAdd);
    m_touchPressStartMs = QDateTime::currentDateTime().toMSecsSinceEpoch();
    m_touchPressPoint = point;
}

/**
 * @brief LeftView::dragEnterEvent
 * @param event
 * 拖拽进入视图事件
 */
void LeftView::dragEnterEvent(QDragEnterEvent *event)
{
    // 判断拖拽进入视图事件触发类型（笔记：NOTES_DRAG_KEY；记事本：NOTEPAD_DRAG_KEY）
    if (!event->mimeData()->hasFormat(NOTES_DRAG_KEY)
        && !event->mimeData()->hasFormat(NOTEPAD_DRAG_KEY)) {
        event->ignore();
        return DTreeView::dragEnterEvent(event);
    }

    if (m_folderDraing) {
        m_pItemDelegate->setDragState(true);
        this->update();
    }
    DTreeView::dragEnterEvent(event);
    setState(DraggingState);
    event->accept();
}

/**
 * @brief LeftView::dragMoveEvent
 * @param event
 * 拖拽移动事件
 */
void LeftView::dragMoveEvent(QDragMoveEvent *event)
{
    DTreeView::dragMoveEvent(event);
    this->update();
    event->accept();
}

/**
 * @brief LeftView::dragLeaveEvent
 * @param event
 * 拖拽离开视图事件
 */
void LeftView::dragLeaveEvent(QDragLeaveEvent *event)
{
    if (m_folderDraing) {
        m_pItemDelegate->setDragState(false);
        m_pItemDelegate->setDrawHover(false);
        this->update();
    }
    event->accept();
}

/**
 * @brief LeftView::doDragMove
 * @param src
 * @param dst
 * 拖拽移动
 */
void LeftView::doDragMove(const QModelIndex &src, const QModelIndex &dst)
{
    if (src.isValid() && dst.isValid() && src != dst) {
        VNoteFolder *tmpFolder = reinterpret_cast<VNoteFolder *>(
            StandardItemCommon::getStandardItemData(dst));
        if (nullptr == tmpFolder) {
            return;
        }

        // 判断当前是否需要进行重新排序
        VNoteFolder *firstFolder = getFirstFolder();
        if (firstFolder && -1 == firstFolder->sortNumber) {
            setFolderSort();
        }

        int tmpRow = qAbs(src.row() - dst.row());
        int dstNum = tmpFolder->sortNumber;
        QModelIndex tmpIndex;
        QModelIndex rootIndex = getNotepadRootIndex();

        // 根据拖拽放下的位置，给相应的记事本重新赋值排序编号
        for (int i = 0; i < tmpRow; i++) {
            // 目标位置所在的行数比被拖拽的记事本的行数大，则将目标位置记事本和被拖拽记事本之间的记事本的排序编号依次加1，反之依次减1
            if (dst.row() > src.row()) {
                tmpIndex = m_pSortViewFilter->index(dst.row() - i, 0, rootIndex);
                if (!tmpIndex.isValid()) {
                    break;
                }
                tmpFolder = reinterpret_cast<VNoteFolder *>(
                    StandardItemCommon::getStandardItemData(tmpIndex));
                tmpFolder->sortNumber += 1;

            } else {
                tmpIndex = m_pSortViewFilter->index(dst.row() + i, 0, rootIndex);
                if (!tmpIndex.isValid()) {
                    break;
                }
                tmpFolder = reinterpret_cast<VNoteFolder *>(
                    StandardItemCommon::getStandardItemData(tmpIndex));
                tmpFolder->sortNumber -= 1;
            }
        }

        tmpFolder = reinterpret_cast<VNoteFolder *>(
            StandardItemCommon::getStandardItemData(src));
        tmpFolder->sortNumber = dstNum;

        sort();

        // 获取重新排序后每个记事本的排序编号，写入配置文件中
        QString folderSortData = getFolderSort();
        setting::instance()->setOption(VNOTE_FOLDER_SORT, folderSortData);
    }
}

void LeftView::setNumberOfNotes(int numberOfNotes)
{
    m_notesNumberOfCurrentFolder = numberOfNotes;
}

/**
 * @brief LeftView::triggerDragFolder
 * 触发拖动操作
 */
void LeftView::triggerDragFolder()
{
    QModelIndex dragIndex = this->indexAt(mapFromGlobal(QCursor::pos()));
    if (!dragIndex.isValid()) {
        m_isDraging = true;
        return;
    }

    VNoteFolder *folder = reinterpret_cast<VNoteFolder *>(StandardItemCommon::getStandardItemData(currentIndex()));
    // 判断当前拖拽的记事本是否可用，如果可用，则初始化拖拽操作的数据
    if (folder) {
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setData(NOTEPAD_DRAG_KEY, QByteArray());

        if (nullptr == m_MoveView) {
            m_MoveView = new MoveView(this);
        }
        if (DWindowManagerHelper::instance()->hasComposite()) {
            m_MoveView->setFixedSize(224, 91);
        } else {
            m_MoveView->setFixedSize(180, 36);
        }
        m_MoveView->setFolder(folder);
        drag->setPixmap(m_MoveView->grab());
        drag->setMimeData(mimeData);
        m_folderDraing = true;
        //解决高分屏显示与鼠标位置不对应问题，使用固定位置
        drag->setHotSpot(QPoint(21, 25));
        drag->exec(Qt::MoveAction);
        drag->deleteLater();
        m_folderDraing = false;
        m_pItemDelegate->setDragState(false);
        m_pItemDelegate->setDrawHover(true);
        //隐藏菜单
        m_notepadMenu->hide();
    }
}

/**
 * @brief LeftView::dropEvent
 * @param event
 * 拖拽放下事件
 */
void LeftView::dropEvent(QDropEvent *event)
{
    // 判断拖拽放下事件触发类型（笔记：NOTES_DRAG_KEY；记事本：NOTEPAD_DRAG_KEY）
    if (event->mimeData()->hasFormat(NOTES_DRAG_KEY)) {
        //拖拽到当前记事本不取消选中
        QModelIndex index = indexAt(event->pos());
        //如果释放位置为当前笔记或空白区域，取消拖拽
        bool isDragCancelled = currentIndex().row() == index.row() || !index.isValid() ? true : false;
        //拖拽取消后选中
        emit dropNotesEnd(isDragCancelled);
    } else if (event->mimeData()->hasFormat(NOTEPAD_DRAG_KEY)) {
        doDragMove(currentIndex(), indexAt(mapFromGlobal(QCursor::pos())));
    }
}

/**
 * @brief LeftView::popupMenu
 */
void LeftView::popupMenu()
{
    QModelIndexList selectIndexes = selectedIndexes();
    if (selectIndexes.count()) {
        QRect curRect = visualRect(selectIndexes.first());
        //判断选中是否可见
        if (!viewport()->visibleRegion().contains(curRect.center())) {
            scrollTo(selectIndexes.first());
            curRect = visualRect(selectIndexes.first());
        }
        bool tabFocus = m_pItemDelegate->isTabFocus();
        m_notepadMenu->setWindowOpacity(1);
        m_notepadMenu->exec(mapToGlobal(curRect.center()));
        if (hasFocus()) {
            m_pItemDelegate->setTabFocus(tabFocus);
        }
    }
}

void LeftView::renameVNote(QString text)
{
    emit renameVNoteFolder(text);
}
