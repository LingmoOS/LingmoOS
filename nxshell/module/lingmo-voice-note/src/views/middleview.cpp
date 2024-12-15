// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "middleview.h"
#include "vnoteapplication.h"
#include "globaldef.h"
#include "middleviewdelegate.h"
#include "middleviewsortfilter.h"
#include "common/actionmanager.h"
#include "common/standarditemcommon.h"
#include "common/vnoteitem.h"
#include "common/utils.h"
#include "task/exportnoteworker.h"
#include "common/setting.h"
#include "db/vnoteitemoper.h"
#include "moveview.h"
#include "dialog/vnotemessagedialog.h"

#include <DApplication>
#include <DFileDialog>
#include <DLog>
#include <DWindowManagerHelper>

#include <QMouseEvent>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QDrag>
#include <QMimeData>
#include <QStandardPaths>
#include <QThreadPool>

/**
 * @brief MiddleView::MiddleView
 * @param parent
 */
MiddleView::MiddleView(QWidget *parent)
    : DListView(parent)
{
    initModel();
    initDelegate();
    initMenu();
    initUI();
    initConnections();

    setContextMenuPolicy(Qt::NoContextMenu);
    this->setDragEnabled(true);
    this->setDragDropMode(QAbstractItemView::DragOnly);
    this->setAcceptDrops(false);
    //启用多选
    this->setSelectionMode(QAbstractItemView::MultiSelection);
    this->installEventFilter(this);
    this->setResizeMode(ResizeMode::Adjust);
}

/**
 * @brief MiddleView::initModel
 */
void MiddleView::initModel()
{
    m_pDataModel = new QStandardItemModel(this);

    m_pSortViewFilter = new MiddleViewSortFilter(this);
    m_pSortViewFilter->setDynamicSortFilter(false);
    m_pSortViewFilter->setSourceModel(m_pDataModel);

    this->setModel(m_pSortViewFilter);
}

/**
 * @brief MiddleView::initDelegate
 */
void MiddleView::initDelegate()
{
    m_pItemDelegate = new MiddleViewDelegate(this);
    this->setItemDelegate(m_pItemDelegate);
}

/**
 * @brief LeftView::setTouchState 更新触摸屏一指状态
 * @param touchState
 */
void MiddleView::setTouchState(const TouchState &touchState)
{
    m_touchState = touchState;
}

/**
 * @brief MiddleView::initMenu
 */
void MiddleView::initMenu()
{
    m_noteMenu = ActionManager::Instance()->noteContextMenu();
}

/**
 * @brief MiddleView::setSearchKey
 * @param key 搜索关键字
 */
void MiddleView::setSearchKey(const QString &key)
{
    m_searchKey = key;
    m_pItemDelegate->setSearchKey(key);
}

/**
 * @brief MiddleView::setCurrentId
 * @param id
 */
void MiddleView::setCurrentId(qint64 id)
{
    m_currentId = id;
}

/**
 * @brief MiddleView::getCurrentId
 * @return 绑定的id
 */
qint64 MiddleView::getCurrentId()
{
    return m_currentId;
}

/**
 * @brief MiddleView::addRowAtHead
 * @param note
 */
void MiddleView::addRowAtHead(VNoteItem *note)
{
    if (nullptr != note) {
        QStandardItem *item = StandardItemCommon::createStandardItem(note, StandardItemCommon::NOTEITEM);
        m_pDataModel->insertRow(0, item);
        sortView(false);
        QModelIndex index = m_pDataModel->index(item->row(), 0);
        DListView::setCurrentIndex(m_pSortViewFilter->mapFromSource(index));
        this->scrollTo(currentIndex());
        //更新多选判断位置为当前项
        m_currentRow = currentIndex().row();
        //添加note后需要刷新详情页显示
        changeRightView(false);
    }
}

/**
 * @brief MiddleView::appendRow
 * @param note
 */
void MiddleView::appendRow(VNoteItem *note)
{
    if (nullptr != note) {
        QStandardItem *item = StandardItemCommon::createStandardItem(note, StandardItemCommon::NOTEITEM);
        m_pDataModel->appendRow(item);
    }
}

/**
 * @brief MiddleView::clearAll
 */
void MiddleView::clearAll()
{
    m_pDataModel->clear();
    update();
}

/**
 * @brief MiddleView::deleteCurrentRow
 * @return 移除的记事项绑定的数据
 */
QList<VNoteItem *> MiddleView::deleteCurrentRow()
{
    QModelIndexList indexList = selectedIndexes();
    QList<VNoteItem *> noteItemList;
    qSort(indexList);
    for (int i = indexList.size() - 1; i > -1; i--) {
        VNoteItem *noteData = reinterpret_cast<VNoteItem *>(
            StandardItemCommon::getStandardItemData(indexList[i]));
        noteItemList.append(noteData);
        m_pSortViewFilter->removeRow(indexList[i].row());
    }
    return noteItemList;
}

/**
 * @brief MiddleView::getCurrVNotedata
 * @return 当前选中的记事项数据
 */
VNoteItem *MiddleView::getCurrVNotedata() const
{
    VNoteItem *noteData = reinterpret_cast<VNoteItem *>(
        StandardItemCommon::getStandardItemData(currentIndex()));

    return noteData;
}

/**
 * @brief MiddleView::getCurrVNotedataList
 * 获取当前笔记项
 */
QList<VNoteItem *> MiddleView::getCurrVNotedataList() const
{
    QModelIndexList modelList = selectedIndexes();
    QList<VNoteItem *> noteDataList;
    for (auto index : modelList) {
        VNoteItem *noteData = reinterpret_cast<VNoteItem *>(
            StandardItemCommon::getStandardItemData(index));
        noteDataList.append(noteData);
    }

    return noteDataList;
}

/**
 * @brief MiddleView::onNoteChanged
 */
void MiddleView::onNoteChanged()
{
    sortView();
    //初始化位置参数
    initPositionStatus(currentIndex().row());
}

/**
 * @brief MiddleView::rowCount
 * @return 记事项数目
 */
qint32 MiddleView::rowCount() const
{
    return m_pDataModel->rowCount();
}

/**
 * @brief MiddleView::setCurrentIndex
 * @param index
 */
void MiddleView::setCurrentIndex(int index)
{
    initPositionStatus(index);
    DListView::setCurrentIndex(m_pSortViewFilter->index(index, 0));
}

/**
 * @brief MiddleView::editNote
 */
void MiddleView::editNote()
{
    //多选取消重命名
    if (1 == selectedIndexes().count()) {
        edit(currentIndex());
    }
}

/**
 * @brief MiddleView::saveAs
 * 笔记另存为，统一获取文件夹路径
 */
void MiddleView::saveAs(SaveAsType type)
{
    //获取选中的笔记
    QModelIndexList indexList = selectedIndexes();
    QList<VNoteItem *> noteDataList;
    for (auto index : indexList) {
        VNoteItem *noteData = reinterpret_cast<VNoteItem *>(
            StandardItemCommon::getStandardItemData(index));
        noteDataList.append(noteData);
    }

    if (indexList.size() == 0) {
        return;
    }

    //文件筛选类型
    QStringList filterTypes {"TXT(*.txt);;HTML(*.html)", "TXT(*.txt)", "HTML(*.html)", "MP3(*.mp3)"};
    DFileDialog dialog(this);
    dialog.setNameFilter(filterTypes.at(type));
    QString historyDir = "";
    QString defaultName = "";
    //获取默认路径
    if (type == Voice) {
        historyDir = setting::instance()->getOption(VNOTE_EXPORT_VOICE_PATH_KEY).toString();
    } else {
        historyDir = setting::instance()->getOption(VNOTE_EXPORT_TEXT_PATH_KEY).toString();
    }
    if (historyDir.isEmpty()) {
        historyDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    dialog.setDirectory(historyDir);
    //多选或者语音时，选择文件夹
    if (indexList.size() > 1 || type == Voice) {
        dialog.setFileMode(DFileDialog::Directory);
        dialog.setLabelText(DFileDialog::Accept, DApplication::translate("MiddleView", "Save"));
        if (QDialog::Rejected == dialog.exec()) {
            return;
        }
    } else {
        //单选笔记时，保存文件可指定名称
        dialog.setFileMode(DFileDialog::AnyFile);
        dialog.setAcceptMode(DFileDialog::AcceptSave);
        VNoteItem *note = getCurrVNotedata();
        //设置默认名称为笔记名称
        if (note) {
            //获取文件后缀名
            QString fileSuffix = "";
            if (Html == type) {
                fileSuffix = ".html";
            } else if (Text == type) {
                fileSuffix = ".txt";
            } else if (Voice == type) {
                fileSuffix = ".mp3";
            }
            dialog.selectFile(Utils::filteredFileName(note->noteTitle + fileSuffix));
        }
        if (QDialog::Rejected == dialog.exec()) {
            return;
        }
        //获取文件名称
        QList<QUrl> urls = dialog.selectedUrls();
        if (!urls.size()) {
            return;
        }
        defaultName = urls.value(0).fileName();
        if (defaultName.isEmpty()) {
            return;
        }
    }

    //获取选择的文件夹路径
    QString exportDir = dialog.directory().path();
    if (exportDir.isEmpty()) {
        return;
    }

    ExportNoteWorker::ExportType exportType = ExportNoteWorker::ExportNothing;
    QString filter = dialog.selectedNameFilter();
    //选择保存文件的类型并给加上后缀（部分版本文管不会自动加后缀）
    if (filterTypes.at(1) == filter) {
        exportType = ExportNoteWorker::ExportText;
        if (!defaultName.isEmpty() && !defaultName.endsWith(".txt")) {
            defaultName += ".txt";
        }
    } else if (filterTypes.at(2) == filter) {
        exportType = ExportNoteWorker::ExportHtml;
        if (!defaultName.isEmpty() && !defaultName.endsWith(".html")) {
            defaultName += ".html";
        }
    } else if (filterTypes.at(3) == filter) {
        exportType = ExportNoteWorker::ExportVoice;
        if (!defaultName.isEmpty() && !defaultName.endsWith(".mp3")) {
            defaultName += ".mp3";
        }
    }
    ExportNoteWorker *exportWorker = new ExportNoteWorker(
        exportDir, exportType, noteDataList, defaultName);
    exportWorker->setAutoDelete(true);
    connect(exportWorker, &ExportNoteWorker::exportFinished, this, &MiddleView::onExportFinished);
    QThreadPool::globalInstance()->start(exportWorker);
}

/**
 * @brief MiddleView::mousePressEvent
 * @param event
 */
void MiddleView::mousePressEvent(QMouseEvent *event)
{
    this->setFocus();

    if (!m_onlyCurItemMenuEnable) {
        //触控屏手势
        if (event->source() == Qt::MouseEventSynthesizedByQt) {
            //触屏多选
            if (Qt::NoModifier == event->modifiers()) {
                //记录此时触控点的位置，用于move事件中滑动距离与速度
                m_touchPressPoint = event->pos();
                m_touchPressStartMs = QDateTime::currentDateTime().toMSecsSinceEpoch();
                //更新触摸状态
                setTouchState(TouchPressing);
                m_index = indexAt(event->pos());
                m_noteMenu->setPressPoint(QCursor::pos());

                setMouseState(MouseState::pressing);

                //检查是否选中
                m_selectCurrentTimer->start(250);
                //是否弹出右键菜单
                m_popMenuTimer->start(1000);
                return;
            }
        }
        QModelIndex modelIndex = indexAt(event->pos());
        if (!modelIndex.isValid())
            return;
        m_index = modelIndex;
        m_currentRow = m_currentRow == -1 ? 0 : m_currentRow;
        if (Qt::LeftButton == event->button() || Qt::MidButton == event->button()) {
            //ctrl+左/中键
            if (Qt::ControlModifier == event->modifiers()) {
                //当前点击位置为最后一个选择，不做处理
                if (selectedIndexes().count() == 1) {
                    if (selectedIndexes().last().row() == indexAt(event->pos()).row()) {
                        return;
                    }
                }
                //不延续当前状态则清空当前选中
                if (getModifierState() != ModifierState::noModifier && getModifierState() != ModifierState::ctrlModifier) {
                    clearSelection();
                    //普通详情页
                    changeRightView(false);
                    //多选详情页
                    setCurrentIndex(modelIndex.row());
                    selectionModel()->select(m_pSortViewFilter->index(modelIndex.row(), 0), QItemSelectionModel::Select);
                } else {
                    if (selectedIndexes().contains(modelIndex)) {
                        selectionModel()->select(m_pSortViewFilter->index(modelIndex.row(), 0), QItemSelectionModel::Deselect);
                        //刷新详情页
                        if (selectedIndexes().count() == 1) {
                            //普通详情页
                            int row = selectedIndexes().last().row();
                            changeRightView(false);
                            //触发currentChanged信号，刷新详情页
                            setCurrentIndex(row);
                            selectionModel()->select(m_pSortViewFilter->index(row, 0), QItemSelectionModel::Select);
                        } else {
                            //多选详情页
                            changeRightView();
                        }
                    } else {
                        selectionModel()->select(m_pSortViewFilter->index(modelIndex.row(), 0), QItemSelectionModel::Select);
                        //多选详情页
                        changeRightView();
                    }
                }
                //更新当前状态
                setModifierState(ModifierState::ctrlModifier);
                initPositionStatus(modelIndex.row());
                return;
            }
            //shift+左/中键
            else if (Qt::ShiftModifier == event->modifiers()) {
                handleShiftAndPress(modelIndex);
                selectionModel()->select(m_pSortViewFilter->index(modelIndex.row(), 0), QItemSelectionModel::Select);
                //刷新详情页
                if (modelIndex.row() != m_currentRow) {
                    changeRightView();
                } else {
                    //普通详情页
                    changeRightView(false);
                    clearSelection();
                    setCurrentIndex(modelIndex.row());
                }
                return;
            }
            //仅左/中键
            else {
                setTouchState(TouchPressing);
                setModifierState(ModifierState::noModifier);
                setMouseState(MouseState::pressing);
                //普通详情页
                return;
            }
        }
        //右键press
        else {
            if (MenuStatus::ReleaseFromMenu == m_menuState) {
                m_menuState = MenuStatus::Normal;
                return;
            }
            if (Qt::ShiftModifier == event->modifiers() || Qt::CTRL == event->modifiers()) {
                //shift+右键
                if (Qt::ShiftModifier == event->modifiers()) {
                    //首先处理选中
                    if (m_currentRow == modelIndex.row()) {
                        changeRightView(false);
                        clearSelection();
                        setCurrentIndex(modelIndex.row());
                    } else {
                        handleShiftAndPress(modelIndex);
                    }
                }
                //ctrl+右键
                else {
                    if (modelIndex.row() != currentIndex().row()) {
                        selectionModel()->select(m_pSortViewFilter->index(modelIndex.row(), 0), QItemSelectionModel::Select);
                        changeRightView();
                    }
                }
                m_noteMenu->setWindowOpacity(1);
                m_noteMenu->popup(event->globalPos());
            }
            //仅右键
            else if (!m_onlyCurItemMenuEnable || modelIndex == this->currentIndex()) {
                //不在选中范围内
                if (!selectedIndexes().contains(modelIndex)) {
                    clearSelection();
                    //多选时，右击未选中，刷新详情页状态
                    changeRightView(false);
                    DListView::setCurrentIndex(modelIndex);
                    initPositionStatus(modelIndex.row());
                }
                m_noteMenu->setWindowOpacity(1);
                m_noteMenu->popup(event->globalPos());
            }
            event->setModifiers(Qt::NoModifier);
            setTouchState(TouchState::TouchPressing);
        }
    } else {
        //置灰状态下只有当前笔记可操作
        if (currentIndex() == indexAt(event->pos())) {
            //触控屏手势
            if (event->source() == Qt::MouseEventSynthesizedByQt) {
                //记录此时触控点的位置，用于move事件中滑动距离与速度
                m_touchPressPoint = event->pos();
                m_touchPressStartMs = QDateTime::currentDateTime().toMSecsSinceEpoch();
                //更新触摸状态
                setTouchState(TouchPressing);
                m_noteMenu->setPressPoint(QCursor::pos());
                //是否弹出右键菜单
                m_popMenuTimer->start(1000);
                return;
            } else {
                if (Qt::RightButton == event->button()) {
                    m_noteMenu->setWindowOpacity(1);
                    m_noteMenu->popup(event->globalPos());
                }
            }
        }
    }
}

/**
 * @brief MiddleView::mouseReleaseEvent
 * @param event
 */
void MiddleView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_onlyCurItemMenuEnable) {
        return;
    }
    m_isDraging = false;
    //停止计时器
    m_selectCurrentTimer->stop();
    m_popMenuTimer->stop();
    m_menuState = MenuStatus::Normal;

    if (TouchState::TouchMoving == m_touchState) {
        return;
    }
    QModelIndex index = indexAt(event->pos());
    if (MouseState::pressing == m_mouseState && Qt::NoModifier == event->modifiers()) {
        initPositionStatus(indexAt(event->pos()).row());
        if (index.isValid()) {
            changeRightView(false);
            clearSelection();
            setCurrentIndex(index.row());
            //请求刷新详情页为当前笔记
            setTouchState(TouchState::TouchNormal);
            setMouseState(MouseState::normal);
            return;
        }
    }
    //处理拖拽事件，由于与drop操作参数不同，暂未封装
    if (m_touchState == TouchState::TouchDraging) {
        setTouchState(TouchState::TouchNormal);
        return;
    }
    //正常点击状态，选择当前点击选项
    if (index.row() != currentIndex().row() && m_touchState == TouchState::TouchPressing && Qt::NoModifier == event->modifiers()) {
        if (index.isValid()) {
            setCurrentIndex(index.row());
        }
        setTouchState(TouchState::TouchNormal);
        return;
    }
    setTouchState(TouchState::TouchNormal);
    DListView::mouseReleaseEvent(event);
}

/**
 * @brief MiddleView::mouseDoubleClickEvent
 * @param event
 */
void MiddleView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_onlyCurItemMenuEnable) {
        DListView::mouseDoubleClickEvent(event);
    }

    //左键双击事件
    if (event->button() == Qt::LeftButton) {
        if (indexAt(event->pos()) == currentIndex())
            editNote(); //在笔记列表双击左键进入重命名状态
    }
}

/**
 * @brief MiddleView::handleShiftAndPress
 * @param index
 * 处理shift+鼠标press选中操作
 */
void MiddleView::handleShiftAndPress(QModelIndex &index)
{
    if (getModifierState() != ModifierState::noModifier && getModifierState() != ModifierState::shiftAndMouseModifier) {
        clearSelection();
        selectionModel()->select(m_pSortViewFilter->index(m_currentRow, 0), QItemSelectionModel::Select);
    }
    int begin = m_currentRow;
    int end = m_currentRow;

    if (-1 != m_currentRow) {
        if (m_currentRow < index.row()) {
            end = index.row();
        } else if (m_currentRow > index.row()) {
            begin = index.row();
        }
    } else {
        begin = 0;
        end = index.row();
    }

    if (begin < 0)
        return;

    selectionModel()->clear();
    //选中起始位置至点击位置范围内的index
    for (int i = begin; i <= end; i++) {
        selectionModel()->select(m_pSortViewFilter->index(i, 0), QItemSelectionModel::Select);
    }
    //选中大于1请求刷新详情页为多选操作页面
    if (end > begin) {
        changeRightView();
    }

    setModifierState(ModifierState::shiftAndMouseModifier);
}

/**
 * @brief MiddleView::setModifierState
 * @param modifierState
 * 更新辅助键状态
 */
void MiddleView::setModifierState(const ModifierState &modifierState)
{
    m_modifierState = modifierState;
}

/**
 * @brief MiddleView::getModifierState
 * @param
 * 获取当前辅助键状态
 */
MiddleView::ModifierState MiddleView::getModifierState() const
{
    return m_modifierState;
}

/**
 * @brief MiddleView::setMouseState
 * @param mouseState
 * 更新鼠标状态
 */
void MiddleView::setMouseState(const MouseState &mouseState)
{
    m_mouseState = mouseState;
}

/**
 * @brief MiddleView::isMultipleSelected
 * @param
 * 当前是否为多选
 */
bool MiddleView::isMultipleSelected()
{
    return (selectedIndexes().count() > 1);
}

/**
 * @brief MiddleView::haveText
 * @param
 * 当前选中笔记是否有文本
 */
bool MiddleView::haveText()
{
    for (auto index : selectedIndexes()) {
        VNoteItem *noteData = reinterpret_cast<VNoteItem *>(
            StandardItemCommon::getStandardItemData(index));
        if (noteData->haveText()) {
            return true;
        }
    }
    return false;
}

/**
 * @brief MiddleView::haveVoice
 * @param
 * 当前选中笔记是否有语音
 */
bool MiddleView::haveVoice()
{
    for (auto index : selectedIndexes()) {
        VNoteItem *noteData = reinterpret_cast<VNoteItem *>(
            StandardItemCommon::getStandardItemData(index));
        if (noteData->haveVoice()) {
            return true;
        }
    }
    return false;
}

/**
 * @brief MiddleView::mouseMoveEvent
 * @param event
 * 获取笔记数量
 */
int MiddleView::getSelectedCount()
{
    return selectedIndexes().count();
}

/**
 * @brief MiddleView::mouseMoveEvent
 * @param event
 */
void MiddleView::mouseMoveEvent(QMouseEvent *event)
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
    } else if ((event->buttons() & Qt::LeftButton) && m_touchState == TouchState::TouchPressing) {
        if (!m_isDraging && indexAt(event->pos()).isValid()) {
            m_shiftSelection = -1;
            if (!selectedIndexes().contains(m_index)) {
                //解决点击多选问题
                if (!selectedIndexes().contains(m_index)) {
                    clearSelection();
                    selectionModel()->select(m_pSortViewFilter->index(m_index.row(), 0), QItemSelectionModel::Select);
                    //请求刷新详情页为当前笔记
                    m_currentRow = m_index.row();
                    changeRightView(false);
                }
            }

            //需判断移动距离
            if (qAbs(event->pos().x() - m_touchPressPoint.x()) > 3
                || qAbs(event->pos().y() - m_touchPressPoint.y()) > 3) {
                handleDragEvent(false);
            }
        }
    } else {
        return;
        //        DListView::mouseMoveEvent(event);
    }
}

/**
 * @brief MiddleView::keyReleaseEvent
 * @param e
 */
void MiddleView::keyReleaseEvent(QKeyEvent *event)
{
    if (Qt::Key_Shift == event->key()) {
        m_shiftSelection = -1;
        setModifierState(ModifierState::noModifier);
    }
    DListView::keyReleaseEvent(event);
}

/**
 * @brief LeftView::handleTouchSlideEvent  处理触摸屏move事件，区分点击、滑动、拖拽、长按功能
 * @param timeParam 时间间隔
 * @param distY 纵向移动距离
 * @param point 当前时间发生位置
 */
void MiddleView::handleTouchSlideEvent(qint64 timeParam, double distY, QPoint point)
{
    //根据移动距离和时间计算滑动速度，设置滚动步长
    double slideSpeed = ((qAbs(distY)) / timeParam) + 0.3;
    verticalScrollBar()->setSingleStep(static_cast<int>(20 * slideSpeed));
    verticalScrollBar()->triggerAction((distY > 0) ? QScrollBar::SliderSingleStepSub : QScrollBar::SliderSingleStepAdd);
    m_touchPressStartMs = QDateTime::currentDateTime().toMSecsSinceEpoch();
    m_touchPressPoint = point;
}

/**
 * @brief MiddleView::eventFilter
 * @param o
 * @param e
 * @return false 不过滤，事件正常处理
 */
bool MiddleView::eventFilter(QObject *o, QEvent *e)
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
        if (e->type() == QEvent::FocusIn) {
            m_pItemDelegate->setEditIsVisible(true);
            this->update(currentIndex());
        } else if (e->type() == QEvent::Destroy) {
            m_pItemDelegate->setEditIsVisible(false);
            this->update(currentIndex());
        }
    }
    return false;
}

/**
 * @brief MiddleView::keyPressEvent
 * @param e
 */
void MiddleView::keyPressEvent(QKeyEvent *e)
{
    if (m_onlyCurItemMenuEnable || e->key() == Qt::Key_PageUp || e->key() == Qt::Key_PageDown) {
        e->ignore();
    } else if (Qt::Key_Up == e->key() || Qt::Key_Down == e->key()) {
        if (Qt::ShiftModifier == e->modifiers() && Qt::Key_Up == e->key()) {
            int nextIndex = m_shiftSelection == -1 ? m_currentRow - 1 : m_shiftSelection - 1;
            if (getModifierState() != ModifierState::shiftAndUpOrDownModifier) {
                clearSelection();
            }
            if (-1 != nextIndex) {
                scrollTo(m_pSortViewFilter->index(nextIndex, 0));
                //多选在下，shift+上
                if (nextIndex >= m_currentRow) {
                    selectionModel()->select(m_pSortViewFilter->index(m_shiftSelection, 0), QItemSelectionModel::Deselect);
                    //普通详情页
                    if (nextIndex == m_currentRow) {
                        int row = selectedIndexes().last().row();
                        changeRightView(false);
                        //触发currentChanged信号，刷新详情页
                        setCurrentIndex(row);
                        selectionModel()->select(m_pSortViewFilter->index(row, 0), QItemSelectionModel::Select);
                    } else {
                        selectionModel()->select(m_pSortViewFilter->index(m_currentRow, 0), QItemSelectionModel::Select);
                        changeRightView();
                    }

                }
                //多选在上，shift+上
                else {
                    selectionModel()->select(m_pSortViewFilter->index(nextIndex, 0), QItemSelectionModel::Select);
                    selectionModel()->select(m_pSortViewFilter->index(m_currentRow, 0), QItemSelectionModel::Select);
                    //多选详情页
                    changeRightView();
                }
                m_shiftSelection = nextIndex;
                setModifierState(ModifierState::shiftAndUpOrDownModifier);
            } else {
                clearSelection();
                //切换详情显示页面
                changeRightView(false);
                setCurrentIndex(m_currentRow);
                scrollTo(m_pSortViewFilter->index(m_currentRow, 0));
            }
        } else if (Qt::ShiftModifier == e->modifiers() && Qt::Key_Down == e->key()) {
            int nextIndex = m_shiftSelection == -1 ? m_currentRow + 1 : m_shiftSelection + 1;
            if (getModifierState() != ModifierState::shiftAndUpOrDownModifier) {
                clearSelection();
            }
            if (count() != nextIndex) {
                scrollTo(m_pSortViewFilter->index(nextIndex, 0));
                //多选在上，shift+下
                if (nextIndex <= m_currentRow) {
                    selectionModel()->select(m_pSortViewFilter->index(m_shiftSelection, 0), QItemSelectionModel::Deselect);
                    //普通详情页
                    if (nextIndex == m_currentRow) {
                        int row = selectedIndexes().last().row();
                        changeRightView(false);
                        //触发currentChanged信号，刷新详情页
                        setCurrentIndex(row);
                        selectionModel()->select(m_pSortViewFilter->index(row, 0), QItemSelectionModel::Select);
                    } else {
                        selectionModel()->select(m_pSortViewFilter->index(m_currentRow, 0), QItemSelectionModel::Select);
                        changeRightView();
                    }
                }
                //多选在下，shift+下
                else {
                    selectionModel()->select(m_pSortViewFilter->index(nextIndex, 0), QItemSelectionModel::Select);
                    //多选详情页
                    selectionModel()->select(m_pSortViewFilter->index(m_currentRow, 0), QItemSelectionModel::Select);
                    changeRightView();
                }
                m_shiftSelection = nextIndex;
                setModifierState(ModifierState::shiftAndUpOrDownModifier);
            } else {
                clearSelection();
                //切换详情显示页面
                changeRightView(false);
                setCurrentIndex(m_currentRow);
                scrollTo(m_pSortViewFilter->index(m_currentRow, 0));
            }
        } else {
            setModifierState(ModifierState::noModifier);
            if (-1 == currentIndex().row()) {
                setCurrentIndex(m_currentRow);
            }
            DListView::keyPressEvent(e);
            //由于启用多选，导致键盘操作不会清空和实现选中效果，在此处实现
            clearSelection();
            initPositionStatus(currentIndex().row());
            selectionModel()->select(m_pSortViewFilter->index(currentIndex().row(), 0), QItemSelectionModel::Select);
            scrollTo(currentIndex());
            //请求刷新详情页为当前笔记
            changeRightView(false);
        }
    }
    //关于shift+home/end的判断
    else if (Qt::Key_Home == e->key() || Qt::Key_End == e->key()) {
        if (Qt::ShiftModifier == e->modifiers()) {
            QModelIndexList indexes = selectedIndexes();
            if (indexes.size() == 1) {
                m_currentRow = indexes.last().row();
            }
        }
        if (Qt::Key_Home == e->key()) {
            scrollTo(m_pSortViewFilter->index(0, 0));
            clearSelection();
            if (Qt::ShiftModifier == e->modifiers()) {
                setCurrentIndex(m_currentRow);
                for (int i = 0; i <= m_currentRow; i++) {
                    selectionModel()->select(m_pSortViewFilter->index(i, 0), QItemSelectionModel::Select);
                }
                setModifierState(ModifierState::shiftAndHomeOrEndKeyModifier);
                //刷新详情页
                changeRightView(selectedIndexes().count() > 1 ? true : false);
            } else {
                changeRightView(false);
                setCurrentIndex(0);
                initPositionStatus(currentIndex().row());
            }
        } else {
            scrollTo(m_pSortViewFilter->index(count() - 1, 0));
            clearSelection();
            if (Qt::ShiftModifier == e->modifiers()) {
                setCurrentIndex(m_currentRow);
                for (int i = m_currentRow; i < count(); i++) {
                    selectionModel()->select(m_pSortViewFilter->index(i, 0), QItemSelectionModel::Select);
                }
                setModifierState(ModifierState::shiftAndHomeOrEndKeyModifier);
                //刷新详情页
                changeRightView(selectedIndexes().count() > 1 ? true : false);
            } else {
                changeRightView(false);
                setCurrentIndex(count() - 1);
                initPositionStatus(currentIndex().row());
            }
        }
    } else if (Qt::CTRL == e->modifiers() && Qt::Key_A == e->key()) {
        //实现笔记全选功能
        setModifierState(ModifierState::noModifier);
        for (int i = 0; i < count(); i++) {
            selectionModel()->select(m_pSortViewFilter->index(i, 0), QItemSelectionModel::Select);
        }
        if (count() > 1)
            changeRightView();
    }
}

/**
 * @brief MiddleView::initPositionStatus
 */
//初始化位置状态
void MiddleView::initPositionStatus(int row)
{
    m_shiftSelection = -1;
    m_currentRow = row;
}

/**
 * @brief MiddleView::initUI
 */
void MiddleView::initUI()
{
    //TODO:
    //    HQ & scaler > 1 have one line at
    //the footer of DListView,so add footer
    //to solve this bug
    QWidget *footer = new QWidget(this);
    footer->setFixedHeight(1);
    addFooterWidget(footer);
    //    this->installEventFilter(this);
    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, &QTimer::timeout, this, &MiddleView::onRefresh);
    m_refreshTimer->start(30 * 1000);
}

/**
 * @brief MiddleView::initConnections
 */
void MiddleView::initConnections()
{
    //右键菜单滑动
    connect(m_noteMenu, &VNoteRightMenu::menuTouchMoved, this, &MiddleView::handleDragEvent);
    //右键菜单释放
    connect(m_noteMenu, &VNoteRightMenu::menuTouchReleased, this, [=] {
        m_touchState = TouchState::TouchNormal;
        m_menuState = MenuStatus::ReleaseFromMenu;
    });
    //定时器用于判断是否选中当前
    m_selectCurrentTimer = new QTimer(this);
    connect(m_selectCurrentTimer, &QTimer::timeout, [=] {
        if (m_touchState == TouchState::TouchPressing && m_index.isValid())
            if (!selectedIndexes().contains(m_index)) {
                //选中当前并刷新详情页
                this->clearSelection();
                m_currentRow = m_index.row();
                changeRightView(false);
                this->setCurrentIndex(m_index.row());
            }

        m_selectCurrentTimer->stop();
    });
    //定时器用于判断是否弹出菜单
    m_popMenuTimer = new QTimer(this);
    connect(m_popMenuTimer, &QTimer::timeout, [=] {
        if (m_touchState == TouchState::TouchPressing && m_index.isValid()) {
            m_noteMenu->setWindowOpacity(1);
            m_noteMenu->popup(QCursor::pos());
        }
        m_popMenuTimer->stop();
    });
}

/**
 * @brief MiddleView::setVisibleEmptySearch
 * @param visible true 显示搜索无结果界面
 */
void MiddleView::setVisibleEmptySearch(bool visible)
{
    if (visible && m_emptySearch == nullptr) {
        m_emptySearch = new DLabel(this);
        m_emptySearch->setText(DApplication::translate("MiddleView", "No search results"));
        m_emptySearch->setAlignment(Qt::AlignCenter);
        DFontSizeManager::instance()->bind(m_emptySearch, DFontSizeManager::T6);
        m_emptySearch->setForegroundRole(DPalette::PlaceholderText);
        m_emptySearch->setVisible(visible);
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_emptySearch);
        this->setLayout(layout);
    } else if (m_emptySearch) {
        m_emptySearch->setVisible(visible);
    }
}

/**
 * @brief MiddleView::setOnlyCurItemMenuEnable
 * @param enable true 只有选中项右键菜单可弹出
 */
void MiddleView::setOnlyCurItemMenuEnable(bool enable)
{
    m_onlyCurItemMenuEnable = enable;
    m_pItemDelegate->setEnableItem(!enable);
    this->update();
}

/**
 * @brief MiddleView::closeEditor
 * @param editor
 * @param hint
 */
void MiddleView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    Q_UNUSED(hint);
    DListView::closeEditor(editor, QAbstractItemDelegate::NoHint);
}

/**
 * @brief MiddleView::closeMenu
 */
void MiddleView::closeMenu()
{
    m_noteMenu->close();
}

/**
 * @brief MiddleView::noteStickOnTop
 */
void MiddleView::noteStickOnTop()
{
    QModelIndex index = this->currentIndex();
    VNoteItem *noteData = reinterpret_cast<VNoteItem *>(
        StandardItemCommon::getStandardItemData(index));
    if (noteData) {
        VNoteItemOper noteOper(noteData);
        if (noteOper.updateTop(!noteData->isTop)) {
            sortView();
            //刷新起始位置
            m_currentRow = currentIndex().row();
        }
    }
}

/**
 * @brief MiddleView::sortView
 * @param adjustCurrentItemBar true 调整当前滚动条
 */
void MiddleView::sortView(bool adjustCurrentItemBar)
{
    m_pSortViewFilter->sortView();
    if (adjustCurrentItemBar) {
        this->scrollTo(currentIndex(), DListView::PositionAtBottom);
    }
}

/**
 * @brief MiddleView::getAllSelectNote
 * @return 选中的笔记列表
 * 返回当前选中的笔记列表
 */
QModelIndexList MiddleView::getAllSelectNote()
{
    QModelIndexList indexList;
    if (selectedIndexes().count()) {
        indexList = selectedIndexes();
        qSort(indexList);
    }
    return indexList;
}

/**
 * @brief MiddleView::deleteModelIndexs
 * @param indexs　需要删除的笔记列表
 * 删除笔记项
 */
void MiddleView::deleteModelIndexs(const QModelIndexList &indexs)
{
    QModelIndexList indexList = indexs;
    qSort(indexList);
    for (int i = indexList.size() - 1; i > -1; i--) {
        m_pSortViewFilter->removeRow(indexList[i].row());
    }
}

/**
 * @brief MiddleView::triggerDragNote
 * 触发拖动操作
 */
void MiddleView::triggerDragNote()
{
    if (!m_index.isValid()) {
        m_isDraging = true;
        return;
    }

    QList<VNoteItem *> noteDataList = getCurrVNotedataList();

    if (noteDataList.size()) {
        if (m_MoveView == nullptr) {
            m_MoveView = new MoveView(this);
        }

        QModelIndexList modelList = selectedIndexes();
        m_MoveView->setNotesNumber(modelList.count());
        m_MoveView->setNoteList(noteDataList);
        //重设视图大小
        if (DWindowManagerHelper::instance()->hasComposite()) {
            m_MoveView->setFixedSize(282, 91);
        } else {
            m_MoveView->setFixedSize(240, 36);
        }
        QPixmap pixmap = m_MoveView->grab();
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setData(NOTES_DRAG_KEY, QByteArray());
        drag->setMimeData(mimeData);
        drag->setPixmap(pixmap);
        //解决高分屏显示与鼠标位置不对应问题，使用固定位置
        drag->setHotSpot(QPoint(18, 25));
        drag->exec(Qt::MoveAction);
        drag->deleteLater();
        //隐藏菜单
        m_noteMenu->hide();
        //拖拽取消后选中
        if (m_dragSuccess) {
            //笔记项移除后选中
            selectAfterRemoved();
        } else {
            //拖拽取消后，如果当前选中单个笔记，刷新单选详情页
            if (1 == modelList.size()) {
                clearSelection();
                setCurrentIndex(modelList.at(0).row());
            } else {
                for (auto index : modelList) {
                    selectionModel()->select(m_pSortViewFilter->index(index.row(), 0), QItemSelectionModel::Select);
                }
            }
        }
        setDragSuccess(false);
    }
}

/**
 * @brief LeftView::doTouchMoveEvent  处理触摸屏move事件，区分点击、滑动、拖拽、长按功能
 * @param event
 */
void MiddleView::doTouchMoveEvent(QMouseEvent *event)
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
void MiddleView::handleDragEvent(bool isTouch)
{
    setNextSelection();
    if (m_onlyCurItemMenuEnable)
        return;
    if (isTouch) {
        setTouchState(TouchState::TouchDraging);
    }
    m_popMenuTimer->stop();
    m_noteMenu->setWindowOpacity(0.0);
    triggerDragNote();
}

/**
 * @brief MiddleView::changeRightView
 * @param isMultipleDetailPage
 * 请求刷新详情页
 */
void MiddleView::changeRightView(bool isMultipleDetailPage)
{
    emit requestChangeRightView(isMultipleDetailPage);
}

/**
 * @brief MiddleView::setDragSuccess
 * @param dragSuccess
 * 记录笔记项移除前位置
 */
void MiddleView::setDragSuccess(bool dragSuccess)
{
    m_dragSuccess = dragSuccess;
}

/**
 * @brief MiddleView::setNextSelection
 * @param
 * 记录笔记项移除前位置
 */
void MiddleView::setNextSelection()
{
    QModelIndexList indexList = selectedIndexes();
    qSort(indexList);
    if (indexList.count()) {
        m_nextSelection = indexList.first().row();
    }
}

/**
 * @brief MiddleView::selectAfterRemoved
 * @param
 * 笔记项移除后选中
 */
void MiddleView::selectAfterRemoved()
{
    clearSelection();
    //详情页切换为当前笔记
    changeRightView(false);
    if (m_pSortViewFilter->index(m_nextSelection, 0).isValid()) {
        setCurrentIndex(m_nextSelection);
        m_currentRow = m_nextSelection;
    } else if (m_pSortViewFilter->index(m_nextSelection - 1, 0).isValid()) {
        setCurrentIndex(m_nextSelection - 1);
        m_currentRow = m_nextSelection - 1;
    }
    //滚动到当前选中
    scrollTo(currentIndex());
    return;
}

/**
 * @brief MiddleView::popupMenu
 */
void MiddleView::popupMenu()
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
        m_noteMenu->setWindowOpacity(1);
        m_noteMenu->exec(mapToGlobal(curRect.center()));
        if (hasFocus()) {
            m_pItemDelegate->setTabFocus(tabFocus);
        }
    }
}

bool MiddleView::searchEmpty()
{
    return m_emptySearch && m_emptySearch->isVisible();
}

/**
 * @brief MiddleView::onRefresh
 */
void MiddleView::onRefresh()
{
    update();
    emit requestRefresh();
}

/**
 * @brief MiddleView::onExportFinished
 * @param err
 */
void MiddleView::onExportFinished(int err)
{
    //保存文件失败
    if (ExportNoteWorker::Savefailed == err) {
        VNoteMessageDialog audioOutLimit(VNoteMessageDialog::SaveFailed);
        audioOutLimit.exec();
    }

    //无权限
    if (ExportNoteWorker::PathDenied == err) {
        VNoteMessageDialog audioOutLimit(VNoteMessageDialog::NoPermission);
        audioOutLimit.exec();
    }
}
