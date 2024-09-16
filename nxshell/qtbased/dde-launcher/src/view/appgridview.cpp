// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appgridview.h"
#include "constants.h"
#include "calculate_util.h"
#include "util.h"
#include "appslistmodel.h"
#include "fullscreenframe.h"
#include "windowedframe.h"

#include <DGuiApplicationHelper>

#include <QDebug>
#include <QWheelEvent>
#include <QTimer>
#include <QDrag>
#include <QMimeData>
#include <QtGlobal>
#include <QDrag>
#include <QPropertyAnimation>
#include <QLabel>
#include <QPainter>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>

DGUI_USE_NAMESPACE

AppGridView::AppGridView(const ViewType viewType, QWidget *parent)
    : QListView(parent)
    , m_dropThresholdTimer(new QTimer(this))
    , m_gestureInter(new Gesture("org.deepin.dde.Gesture1"
                                 , "/org/deepin/dde/Gesture1"
                                 , QDBusConnection::systemBus()
                                 , nullptr))
    , m_pDelegate(nullptr)
    , m_appManager(AppsManager::instance())
    , m_calcUtil(CalculateUtil::instance())
    , m_longPressed(false)
    , m_pixLabel(nullptr)
    , m_calendarWidget(nullptr)
    , m_vlayout(nullptr)
    , m_monthLabel(nullptr)
    , m_dayLabel(nullptr)
    , m_weekLabel(nullptr)
    , m_viewType(viewType)
{
    initUi();
    initConnection();
}

const QModelIndex AppGridView::indexAt(const int index) const
{
    return model()->index(index, 0, QModelIndex());
}

/**
 * @brief AppGridView::indexYOffset
 * @param index item index
 * @return item Y offset of current view
 */
int AppGridView::indexYOffset(const QModelIndex &index) const
{
    return indexRect(index).y();
}

void AppGridView::setContainerBox(const QWidget *container)
{
    m_containerBox = container;
}

void AppGridView::setDelegate(DragPageDelegate *pDelegate)
{
    m_pDelegate = pDelegate;
}

DragPageDelegate *AppGridView::getDelegate()
{
    return m_pDelegate;
}

void AppGridView::dropEvent(QDropEvent *e)
{
    e->accept();
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    // 收藏列表允许从其他视图拖拽进入
    if (listModel->category() == AppsListModel::Favorite) {
        const ItemInfo_v1 &info = m_appManager->getItemInfo(e->mimeData()->data("DesktopPath"));
        const QModelIndex index = indexAt(e->pos());
        if (index.isValid()) {
            listModel->insertItem(info, index.row());
        } else {
            listModel->insertItem(info, listModel->rowCount());
        }

        listModel->clearDraggingIndex();
        return;
    }

    if (m_dropThresholdTimer->isActive())
        m_dropThresholdTimer->stop();

    AppItemDelegate *itemDelegate = qobject_cast<AppItemDelegate *>(this->itemDelegate());
    if (!itemDelegate)
        return;

    if (m_calcUtil->fullscreen() && getViewType() == MainView) {
        QModelIndex dropIndex = indexAt(e->pos());
        QModelIndex dragIndex = indexAt(m_dragStartPos);
        if (dropIndex.isValid() && dragIndex.isValid() && dragIndex != dropIndex && (m_viewType != PopupView)) {
            itemDelegate->setDirModelIndex(QModelIndex(), QModelIndex());
            listModel->updateModelData(dragIndex, dropIndex);
        }
    }

    // 解决快速拖动应用，没有动画效果的问题，当拖拽小于200毫秒时，禁止拖动
    if (m_dragLastTime.elapsed() < 200)
        return;

    m_enableDropInside = true;

    // set the correct hover item.
    emit entered(QListView::indexAt(e->pos()));
}

void AppGridView::mousePressEvent(QMouseEvent *e)
{
    const QModelIndex &clickedIndex = QListView::indexAt(e->pos());
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel) {
        // 搜索模式时, model类对象 为 SortFilterProxyModel
        QSortFilterProxyModel *filterModel = qobject_cast<QSortFilterProxyModel *>(model());
        if (!filterModel) {
            qWarning() << "null sortFilterModel !!!";
            return;
        }

        listModel = qobject_cast<AppsListModel *>(filterModel->sourceModel());
        if (!listModel) {
            qWarning() << "null appListModel !!!";
            return;
        }
    }

    if (e->button() == Qt::RightButton) {
        if (clickedIndex.isValid() && !getViewMoveState()) {
            QPoint rightClickPoint = QCursor::pos();
            // 触控屏右键
            if (e->source() == Qt::MouseEventSynthesizedByQt) {
                QPoint indexPoint = mapToGlobal(indexRect(clickedIndex).center());

                if (listModel->category() == AppsListModel::FullscreenAll || listModel->category() == AppsListModel::WindowedAll) {
                    rightClickPoint.setX(indexPoint.x() + indexRect(clickedIndex).width() - 58 * m_calcUtil->getScreenScaleX());
                } else if (listModel->category() == AppsListModel::Search) {
                    rightClickPoint.setX(indexPoint.x() + indexRect(clickedIndex).width()  - 90 * m_calcUtil->getScreenScaleX());
                } else {
                    rightClickPoint.setX(indexPoint.x() + indexRect(clickedIndex).width() - 3 * m_calcUtil->getScreenScaleX());
                }
            }

            // 触发右键菜单
            emit popupMenuRequested(rightClickPoint, clickedIndex);
        }
    }

    if (e->buttons() == Qt::LeftButton && !m_lastFakeAni) {
        m_dragStartPos = e->pos();

        // TODO: topLeft --> center();
        // 记录动画的终点位置
        setDropAndLastPos(appIconRect(indexAt(e->pos())).topLeft());

        // 记录点击文件夹时的初始行数
        if (clickedIndex.isValid() && clickedIndex.data(AppsListModel::ItemIsDirRole).toBool()) {
            m_appManager->setDirAppRow(clickedIndex.row());
            m_appManager->setDirAppPageIndex(listModel->getPageIndex());
        }
    }

    if (m_pDelegate)
        m_pDelegate->mousePress(e);

    QListView::mousePressEvent(e);
}

void AppGridView::dragEnterEvent(QDragEnterEvent *e)
{
    m_appManager->setReleasePos(indexAt(e->pos()).row());
    m_dragLastTime.start();

    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    const QModelIndex dragIndex = m_appManager->dragModelIndex();
    const ItemInfo_v1 &info = m_appManager->getItemInfo(e->mimeData()->data("DesktopPath"));
    // 从其他视图列表中拖入重复的数据直接返回，避免总有一个item正在拖拽中的状态导致没有绘制的问题
    if (listModel && listModel->category() == AppsListModel::Favorite && dragIndex.model() && dragIndex.model() != listModel) {
        if (m_appManager->contains(m_appManager->appsInfoList(AppsListModel::Favorite), info) ) {
            qDebug() << "repeated data...";
            return;
        }
    }

    const QModelIndex index = indexAt(e->pos());

    if (model()->canDropMimeData(e->mimeData(), e->dropAction(), index.row(), index.column(), QModelIndex())) {
        emit entered(QModelIndex());
        return e->accept();
    }
}

void AppGridView::dragMoveEvent(QDragMoveEvent *e)
{
    m_dropThresholdTimer->stop();

    if (m_lastFakeAni)
        return;

    const QPoint pos = e->pos();
    const QRect containerRect = this->rect();

    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    const QModelIndex dropIndex = QListView::indexAt(pos);
    QModelIndex dragIndex = indexAt(m_dragStartPos);

    if (dropIndex.isValid()) {
        m_dropToPos = dropIndex.row();
    } else if (containerRect.contains(pos)) {
        int lastRow = listModel->rowCount() - 1;
        QModelIndex lastIndex = listModel->index(lastRow);
        QPoint lastPos = indexRect(lastIndex).center();
        if (pos.x() > lastPos.x() && pos.y() > lastPos.y())
            m_dropToPos = lastIndex.row();
    }

    // 当文件夹展开窗口隐藏，当前显示的视图类型为 MainView 时，由于展开窗口的QDrag::exec() 其实未执行完毕，所以这里做了特殊处理，
    // 以便实现两个视图页面进行应用删除和插入的同时，实现动画的路径的计算处理。
    // 做法：保存从文件夹中的拖拽的应用信息、文件夹展开窗口隐藏后当前显示的视图列表所在页、列表以及模型对象、列表当前页对应的模型类型
    ItemInfo_v1 dragInfo = e->mimeData()->imageData().value<ItemInfo_v1>();
    m_appManager->setDragItem(dragInfo);
    m_appManager->setCategory(listModel->category());
    m_appManager->setPageIndex(listModel->getPageIndex());
    m_appManager->setReleasePos(m_dropToPos);
    m_appManager->setListModel(listModel);
    m_appManager->setListView(this);

#ifdef QT_DEBUG
    qDebug() << "list view:" << this;
#endif

    // 分页切换后隐藏label过渡效果
    emit requestScrollStop();

    AppItemDelegate *itemDelegate = qobject_cast<AppItemDelegate *>(this->itemDelegate());
    if (!itemDelegate)
        return;

    QPoint moveNext = e->pos() - m_dragStartPos;
    const QRect curRect = indexRect(dropIndex);
    bool dragIsDir = dragIndex.data(AppsListModel::ItemIsDirRole).toBool();
    bool dropIsDir = dropIndex.data(AppsListModel::ItemIsDirRole).toBool();
    bool dragDropIsDir = (dragIsDir && dropIsDir);

    // 拖拽的对象和释放处的对象不同 && 不能同时为文件夹 && 拖拽的对象不能为文件夹
    // 即只有拖拽对象为普通应用时, 才允许合并入(或者创建)文件夹.
    bool isDiff = ((dragIndex != dropIndex) && !dragDropIsDir && !dragIsDir);
    int eventXPos = e->pos().x();
    int eventYPos = e->pos().y();

    int rectCenterXPos = curRect.center().x();
    int rectCenterYPos = curRect.center().y();
    if (m_calcUtil->fullscreen() && (moveNext.x() > 0 || moveNext.y() < 0)) {
        // 向右拖动
        if ((eventXPos >= rectCenterXPos && (eventXPos <= curRect.right())) && isDiff) {
            // 触发文件夹特效
            itemDelegate->setDirModelIndex(dragIndex, dropIndex);
        } else if (((eventYPos >= curRect.top()) && (eventYPos <= rectCenterYPos)) && isDiff) {
            itemDelegate->setDirModelIndex(dragIndex, dropIndex);
        } else {
            // 释放前执行app交换动画
            if (m_enableAnimation)
                m_dropThresholdTimer->start();
        }
    } else if (m_calcUtil->fullscreen() && (moveNext.x() < 0 || moveNext.y() > 0)) {
        // 向左拖动
        if ((eventXPos >= curRect.left()) && (eventXPos <= rectCenterXPos) && isDiff) {
            // 触发文件夹特效
            itemDelegate->setDirModelIndex(dragIndex, dropIndex);
        } else if ((eventYPos >= rectCenterYPos) && (eventYPos <= curRect.bottom()) && isDiff) {
            itemDelegate->setDirModelIndex(dragIndex, dropIndex);
        }
        else {
            // 释放前执行app交换动画
            if (m_enableAnimation)
                m_dropThresholdTimer->start();
        }
    } else {
        itemDelegate->setDirModelIndex(QModelIndex(), QModelIndex());
        // 释放前执行app交换动画
        if (m_enableAnimation)
            m_dropThresholdTimer->start();
    }
}

void AppGridView::dragOut(int pos)
{
    m_enableAnimation = false;
    m_dropToPos = pos;

    prepareDropSwap();
    dropSwap();
}

void AppGridView::dragIn(const QModelIndex &index, bool enableAnimation)
{
    m_enableAnimation = enableAnimation;
    m_dragStartPos = indexRect(index).center();
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    listModel->setDraggingIndex(index);
}

void AppGridView::setDropAndLastPos(const QPoint& itemPos)
{
    m_dropPoint = itemPos;
}

void AppGridView::flashDrag()
{
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    int dragDropRow = listModel->dragDropIndex().row();

    // 刷新的时候不执行drag，因为实际上并没有拖动的动作。在wayland环境中阻塞会在Drag::exec()函数中。
    startDrag(indexAt(dragDropRow), false);
}

/**
 * @brief AppGridView::dragLeaveEvent 离开listview时触发分页
 * @param e 拖动离开事件指针对象
 */
void AppGridView::dragLeaveEvent(QDragLeaveEvent *e)
{
    m_dropThresholdTimer->stop();

    // 获取光标在listview中的坐标
    QPoint pos = mapFromGlobal(QCursor::pos());
    int padding = m_calcUtil->appMarginLeft();
    QMargins margin(padding, DLauncher::APP_DRAG_SCROLL_THRESHOLD,
                    padding, DLauncher::APP_DRAG_SCROLL_THRESHOLD);
    QRect containerRect = this->contentsRect().marginsRemoved(margin);

    const QModelIndex dropStart = QListView::indexAt(m_dragStartPos);

    if (pos.x() > containerRect.right())
        emit requestScrollRight(dropStart);
    else if (pos.x() < containerRect.left())
        emit requestScrollLeft(dropStart);

    QListView ::dragLeaveEvent(e);
}

void AppGridView::mouseMoveEvent(QMouseEvent *e)
{
    e->accept();

    setState(NoState);

    const QModelIndex &idx = indexAt(e->pos());

    // 鼠标在app上划过时实现选中效果
    if (idx.isValid())
        emit entered(idx);

    if (e->buttons() != Qt::LeftButton)
        return;

    // 当点击的位置在图标上就不把消息往下传(listview 滚动效果)
    if (!idx.isValid() && m_pDelegate && !m_longPressed)
        m_pDelegate->mouseMove(e);

    // 如果是触屏事件转换而来并且没有收到后端的延时触屏消息，不进行拖拽
    if (e->source() == Qt::MouseEventSynthesizedByQt && !m_longPressed)
        return;

    if (qAbs(e->x() - m_dragStartPos.x()) > DLauncher::DRAG_THRESHOLD ||
            qAbs(e->y() - m_dragStartPos.y()) > DLauncher::DRAG_THRESHOLD) {
        setViewMoveState(true);

        // 开始拖拽后,导致fullscreenframe只收到mousePress事件,收不到mouseRelease事件,需要处理一下异常
        if (idx.isValid())
            emit requestMouseRelease();

        return startDrag(QListView::indexAt(m_dragStartPos));
    }
}

void AppGridView::mouseReleaseEvent(QMouseEvent *e)
{
    // request main frame hide when click invalid area
    if (e->button() != Qt::LeftButton)
        return;

    if (m_pDelegate)
        m_pDelegate->mouseRelease(e);

    int diff_x = qAbs(e->pos().x() - m_dragStartPos.x());
    int diff_y = qAbs(e->pos().y() - m_dragStartPos.y());

    // 小范围位置变化，当作没有变化，针对触摸屏
    if (diff_x < DLauncher::TOUCH_DIFF_THRESH && diff_y < DLauncher::TOUCH_DIFF_THRESH)
        QListView::mouseReleaseEvent(e);

    // 点击列表空白位置且列表没有移动时，手动触发点击信号
    if (!indexAt(e->pos()).isValid() && !getViewMoveState())
        emit QListView::clicked(QModelIndex());
}

QPixmap AppGridView::creatSrcPix(const QModelIndex &index)
{
    QPixmap srcPix;

    const QString &desktop = index.data(AppsListModel::AppDesktopRole).toString();
    if (desktop.contains("/dde-calendar.desktop")) {
#ifdef QT_DEBUG
        qInfo() << "category: " << static_cast<AppsListModel *>(model())->category();
#endif
        const auto itemSize = m_calcUtil->appIconSize(static_cast<AppsListModel *>(model())->category());
        const double iconZoom =  itemSize.width() / 64.0;
        QStringList calIconList = m_calcUtil->calendarSelectIcon();

        m_calendarWidget->setFixedSize(itemSize);
        m_calendarWidget->setAutoFillBackground(true);
        QPalette palette = m_calendarWidget->palette();
        palette.setBrush(QPalette::Window,
                         QBrush(QPixmap(calIconList.at(0)).scaled(
                                    m_calendarWidget->size(),
                                    Qt::IgnoreAspectRatio,
                                    Qt::SmoothTransformation)));
        m_calendarWidget->setPalette(palette);

        m_vlayout->setSpacing(0);
        auto monthPix = loadSvg(calIconList.at(1), QSize(20, 10) * iconZoom);
        m_monthLabel->setPixmap(monthPix.scaled(monthPix.width(), monthPix.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_monthLabel->setFixedHeight(monthPix.height());
        m_monthLabel->setAlignment(Qt::AlignCenter);
        m_monthLabel->setFixedWidth(itemSize.width() - 5 * iconZoom);
        m_monthLabel->show();
        m_vlayout->addWidget(m_monthLabel, Qt::AlignVCenter);

        auto dayPix = loadSvg(calIconList.at(2), QSize(28, 26) * iconZoom);
        m_dayLabel->setPixmap(dayPix.scaled(dayPix.width(), dayPix.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_dayLabel->setAlignment(Qt::AlignCenter);
        m_dayLabel->setFixedHeight(m_dayLabel->pixmap()->height());
        m_dayLabel->raise();
        m_dayLabel->show();
        m_vlayout->addWidget(m_dayLabel, Qt::AlignVCenter);

        auto weekPix = loadSvg(calIconList.at(3), QSize(14, 6) * iconZoom);
        m_weekLabel->setPixmap(weekPix.scaled(weekPix.width(), weekPix.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        m_weekLabel->setFixedHeight(m_weekLabel->pixmap()->height());
        m_weekLabel->setAlignment(Qt::AlignCenter);
        m_weekLabel->setFixedWidth(itemSize.width() + 5 * iconZoom);
        m_weekLabel->show();

        m_vlayout->addWidget(m_weekLabel, Qt::AlignVCenter);
        m_vlayout->setSpacing(0);
        m_vlayout->setContentsMargins(0, 10 * iconZoom, 0, 10 * iconZoom);
        m_calendarWidget->setLayout(m_vlayout);
        m_calendarWidget->show();

        srcPix = m_calendarWidget->grab(m_calendarWidget->rect());
        m_calendarWidget->hide();
        m_calendarWidget->setLayout(nullptr);
    } else {
        return index.data(AppsListModel::AppDragIconRole).value<QPixmap>();
    }

    return srcPix;
}

/**
 * @brief AppGridView::appIconRect 计算应用图标的矩形位置，该接口中的浮点数据
 * 是从视图代理paint接口中摘过来的matlab模拟的数据
 * @param index 应用图标所在的模型索引
 * @return 应用图标对应的矩形大小
 */
QRect AppGridView::appIconRect(const QModelIndex &index)
{
    qreal ratio = qApp->devicePixelRatio();
    const QSize iconSize = index.data(AppsListModel::AppIconSizeRole).toSize() * ratio;

    const static double x1 = 0.26418192;
    const static double x2 = -0.38890932;

    const int w = indexRect(index).width();
    const int h = indexRect(index).height();
    const int sub = qAbs((w - h) / 2);

    QRect ibr;
    if (w == h)
         ibr = indexRect(index);
    else if (w > h)
          ibr = indexRect(index) - QMargins(sub, 0, sub, 0);
    else
          ibr = indexRect(index) - QMargins(0, 0, 0, sub * 2);

    const double result = x1 * ibr.width() + x2 * iconSize.width();
    int margin = result > 0 ? result * 0.71 : 1;
    QRect br = ibr.marginsRemoved(QMargins(margin, 1, margin, margin * 2));

    int iconTopMargin = 6;

    const int iconLeftMargins = (br.width() - iconSize.width()) / 2;
    return QRect(br.topLeft() + QPoint(iconLeftMargins, iconTopMargin - 2), iconSize);
}

/**
 * @brief AppGridView::startDrag 处理listview中app的拖动操作
 * @param index 被拖动app的对应的模型索引
 * @param execDrag 如果实际执行了drag操作设置为true，否则传入false
 */
void AppGridView::startDrag(const QModelIndex &index, bool execDrag)
{
    if (!index.isValid()) {
        qWarning() << "invalid index ...";
        return;
    }

    m_appManager->setDragModelIndex(index);
    setViewMoveState();

    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel) {
        // 搜索模式时, model类对象 为 SortFilterProxyModel
        QSortFilterProxyModel *filterModel = qobject_cast<QSortFilterProxyModel *>(model());
        if (!filterModel)
            return;

        listModel = qobject_cast<AppsListModel *>(filterModel->sourceModel());
        if (!listModel)
            return;
    }

    QRect grabRect = appIconRect(index);
    if (m_calcUtil->fullscreen()) {
        int topMargin = m_calcUtil->appMarginTop();
        int leftMargin = m_calcUtil->appMarginLeft();
        grabRect = QRect(grabRect.topLeft() + QPoint(0, topMargin) + QPoint(leftMargin, 0), grabRect.size());
    }

    QPixmap srcPix;
    const bool itemIsDir = index.data(AppsListModel::ItemIsDirRole).toBool();
    if (itemIsDir)
        srcPix = grab(grabRect);
    else
        srcPix = creatSrcPix(index);

    if (!qobject_cast<AppsListModel *>(model())) {
        qDebug() << "appgridview's model is null";
        return;
    }

    const qreal ratio = qApp->devicePixelRatio();
    AppsListModel::AppCategory category = qobject_cast<AppsListModel *>(model())->category();

    srcPix = srcPix.scaled(m_calcUtil->appIconSize(category) * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    srcPix.setDevicePixelRatio(ratio);

    // 创建拖拽释放时的应用图标
    m_pixLabel->setPixmap(srcPix);
    m_pixLabel->setFixedSize(srcPix.size() / ratio);
    m_pixLabel->move(srcPix.rect().center() / ratio);

    QPropertyAnimation *posAni = new QPropertyAnimation(m_pixLabel.data(), "pos", m_pixLabel.data());
    connect(posAni, &QPropertyAnimation::finished, [&, listModel] () {
        m_pixLabel->hide();
        bool dropItemIsDir = indexAt(m_dropToPos).data(AppsListModel::ItemIsDirRole).toBool();
        if (!m_lastFakeAni) {
            // 增加文件夹展开视图列表的判断逻辑
            if (m_enableDropInside && !dropItemIsDir && (getViewType() != PopupView))
                listModel->dropSwap(m_dropToPos);

            // 删除被拖拽的item，并将被拖拽的应用插入到新的位置
            if (!dropItemIsDir && getViewType() != PopupView)
                listModel->dropSwap(indexAt(m_dragStartPos).row());

            // TODO: 搜索模式下的会新增一个控件，因此这里暂时删除针对搜索模式的业务，不影响主体功能
            int releasePos = AppsManager::instance()->releasePos();

#ifdef QT_DEBUG
            qDebug() << "releasePos: " << releasePos << ", dragMode: " << m_appManager->dragMode();
#endif
            if (m_appManager->listModel() && m_appManager->dragMode() == AppsManager::DirOut) {
                m_appManager->listModel()->insertItem(releasePos);
                m_appManager->listModel()->clearDraggingIndex();
            }

            listModel->clearDraggingIndex();
        } else {
            connect(m_lastFakeAni, &QPropertyAnimation::finished, listModel, &AppsListModel::clearDraggingIndex);
        }

        setDropAndLastPos(QPoint(0, 0));
        m_enableDropInside = false;
        m_lastFakeAni = nullptr;
    });

    m_dropToPos = index.row();
    listModel->setDraggingIndex(index);

    int old_page = 0;
    if (m_containerBox)
        old_page = m_containerBox->property("curPage").toInt();

    if (execDrag) {
        QDrag *drag = new QDrag(this);
        drag->setMimeData(model()->mimeData(QModelIndexList() << index));
        drag->setPixmap(srcPix);
        drag->setHotSpot(srcPix.rect().center() / ratio);
        setState(DraggingState);
        drag->exec(Qt::MoveAction);
    }

    // 拖拽操作完成后暂停app移动动画
    m_dropThresholdTimer->stop();

    // 未触发分页则直接返回,触发分页则执行分页后操作
    emit dragEnd();

    int cur_page = 0;
    if (m_containerBox)
        cur_page = m_containerBox->property("curPage").toInt();

    // 当拖动应用出现触发分页的情况，关闭上一个动画， 直接处理当前当前页的动画
    if (cur_page != old_page) {
        posAni->stop();
        return;
    }

    QRect rectIcon(QPoint(), m_pixLabel->size());

    m_pixLabel->move((QCursor::pos() - rectIcon.center()));
    m_pixLabel->show();

    posAni->setEasingCurve(QEasingCurve::Linear);
    posAni->setDuration(DLauncher::APP_DRAG_MININUM_TIME);
    posAni->setStartValue((QCursor::pos() - rectIcon.center()));

    if (getViewType() != PopupView && m_calcUtil->fullscreen()) {
        // 全屏模式下，主界面视图中应用被拖拽释放后...
        posAni->setEndValue(mapToGlobal(m_dropPoint) + QPoint(m_calcUtil->appMarginLeft(), 0));
    } else if (getViewType() == PopupView && m_calcUtil->fullscreen()) {
        // 全屏模式下，文件夹展开窗口中的应用被拖拽释放后...
        int releasePos = AppsManager::instance()->releasePos();

        // 这里进行坐标计算时，不需要对页码进行计算，因为动画仅仅在当前页面进行展示，所以不需要进行考虑页码对动画起点和终点的影响
        // releasePos += m_appManager->getPageIndex() * m_calcUtil->appPageItemCount(m_appManager->getCategory());
        if (!m_appManager->listView()) {
            qDebug() << "destination listview ptr is null...";
            return;
        }

        QModelIndex itemIndex = m_appManager->listView()->indexAt(releasePos);
        QRect itemRect = m_appManager->listView()->indexRect(itemIndex);
        QPoint endPos = m_appManager->listView()->mapToGlobal(itemRect.topLeft());
        posAni->setEndValue(endPos + QPoint(m_calcUtil->appMarginLeft(), 0));
#ifdef QT_DEBUG
        qDebug() << "releasePos:" << releasePos << ", model index valid status:" << itemIndex.isValid() << ", itemRect:" << itemRect << ", mapToGlobal:" << m_appManager->listView()->mapToGlobal(itemRect.center());
#endif
    } else {
        m_pixLabel->hide();
        posAni->setStartValue(mapToGlobal(QCursor::pos()));
        posAni->setEndValue(mapToGlobal(indexRect(QListView::indexAt(m_dragStartPos)).center()));
    }

    //不开启特效则不展示动画
    if (!DGuiApplicationHelper::isSpecialEffectsEnvironment()) {
        posAni->setStartValue(posAni->endValue());
        posAni->setDuration(0);
    }

    posAni->start(QPropertyAnimation::DeleteWhenStopped);
}

/**
 * @brief AppGridView::fitToContent
 * change view size to fit viewport content
 */
void AppGridView::fitToContent()
{
    const QSize size { contentsRect().width(), contentsSize().height() };
    if (size == rect().size())
        return;

    setFixedSize(size);
}

/**
 * @brief AppGridView::prepareDropSwap 创建移动item的动画
 */
void AppGridView::prepareDropSwap()
{
    if (m_lastFakeAni || m_dropThresholdTimer->isActive() || !m_enableAnimation)
        return;

    const QModelIndex dropIndex = indexAt(m_dropToPos);
    if (!dropIndex.isValid())
        return;

    const QModelIndex dragStartIndex = indexAt(m_dragStartPos);
    if (dragStartIndex == dropIndex)
        return;

    // startPos may be in the space between two icons.
    if (!dragStartIndex.isValid()) {
        m_dragStartPos = indexRect(dropIndex).center();
        return;
    }

    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    listModel->clearDraggingIndex();
    listModel->setDraggingIndex(dragStartIndex);
    listModel->setDragDropIndex(dropIndex);

    const int startIndex = dragStartIndex.row();
    const bool moveToNext = startIndex <= m_dropToPos;
    const int start = moveToNext ? startIndex : m_dropToPos;
    const int end = !moveToNext ? startIndex : m_dropToPos;

    if (start == end)
        return;

    int index = 0;
    for (int i = (start + moveToNext); i != (end - !moveToNext); ++i)
        createFakeAnimation(i, moveToNext, index++);

    createFakeAnimation(end - !moveToNext, moveToNext, index++, true);

    // item最后回归的位置
    setDropAndLastPos(appIconRect(dropIndex).topLeft());

    m_dragStartPos = indexRect(dropIndex).center();
}

/**
 * @brief AppGridView::createFakeAnimation 创建列表中item移动的动画效果
 * @param pos 需要移动的item当前所在的行数
 * @param moveNext item是否移动的标识
 * @param isLastAni 是最后的那个item移动的动画标识
 */
void AppGridView::createFakeAnimation(const int pos, const bool moveNext, const int rIndex, const bool isLastAni)
{
    if (rIndex >= m_floatLabels.size())
        return;

    // listview n行1列,肉眼所及的都是app自动换行后的效果
    const QModelIndex index(indexAt(pos));

    QLabel *floatLabel = m_floatLabels[rIndex];
    QPropertyAnimation *ani = new QPropertyAnimation(floatLabel, "pos", floatLabel);

    const auto ratio = devicePixelRatioF();
    const QSize rectSize = index.data(AppsListModel::ItemSizeHintRole).toSize();

    QPixmap pixmap(rectSize * ratio);
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(ratio);

    QStyleOptionViewItem item;
    item.rect = QRect(QPoint(0, 0), rectSize);
    item.features |= QStyleOptionViewItem::HasDisplay;

    QPainter painter(&pixmap);
    itemDelegate()->paint(&painter, item, index);

    floatLabel->setFixedSize(rectSize);
    floatLabel->setPixmap(pixmap);
    floatLabel->show();

    if (m_calcUtil->fullscreen()) {
        int topMargin = m_calcUtil->appMarginTop();
        int leftMargin = m_calcUtil->appMarginLeft();
        ani->setStartValue(indexRect(index).topLeft() - QPoint(0, -topMargin) + QPoint(leftMargin, 0));
        ani->setEndValue(indexRect(indexAt(moveNext ? pos - 1 : pos + 1)).topLeft() - QPoint(0, -topMargin) + QPoint(leftMargin, 0));
    } else {
        ani->setStartValue(indexRect(index).topLeft());
        ani->setEndValue(indexRect(indexAt(moveNext ? pos - 1 : pos + 1)).topLeft());
    }


    // InOutQuad 描述起点矩形到终点矩形的速度曲线
    ani->setEasingCurve(QEasingCurve::Linear);
    ani->setDuration(DLauncher::APP_DRAG_MININUM_TIME);

    if (!DGuiApplicationHelper::isSpecialEffectsEnvironment()) {
        ani->setStartValue(ani->endValue());
        ani->setDuration(0);
    }

    connect(ani, &QPropertyAnimation::finished, floatLabel, &QLabel::hide);
    if (isLastAni) {
        m_lastFakeAni = ani;
        connect(ani, &QPropertyAnimation::finished, this, &AppGridView::dropSwap);
        connect(ani, &QPropertyAnimation::valueChanged, m_dropThresholdTimer, &QTimer::stop);
    }

    ani->start(QPropertyAnimation::DeleteWhenStopped);
}

/**
 * @brief AppGridView::dropSwap
 * 删除拖动前item所在位置的item，插入拖拽的item到新位置
 */
void AppGridView::dropSwap()
{
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    listModel->dropSwap(m_dropToPos);
    m_lastFakeAni = nullptr;

    setState(NoState);
}

const QRect AppGridView::indexRect(const QModelIndex &index) const
{
    return rectForIndex(index);
}

void AppGridView::setViewType(AppGridView::ViewType type)
{
    m_viewType = type;
}

AppGridView::ViewType AppGridView::getViewType() const
{
    return m_viewType;
}

void AppGridView::setViewMoveState(bool moving)
{
    m_moveGridView = moving;
}

bool AppGridView::getViewMoveState() const
{
    return m_moveGridView;
}

/** 提前创建好拖拽过程中需要用到的label
 * （修复在龙芯下运行时创建对象会崩溃的问题）
 * @brief AppGridView::createMovingLabel
 */
void AppGridView::createMovingComponent()
{
    // 拖拽释放鼠标时显示的应用图标
    if (!m_pixLabel) {
        m_pixLabel.reset(new QLabel(topLevelWidget()));
        m_pixLabel->hide();
    }

    // 拖拽过程中位置交换时显示的应用图标
    if (!m_floatLabels.size()) {
        // 单页最多28个应用
        for (int i = 0; i < m_calcUtil->appPageItemCount(AppsListModel::FullscreenAll); i++) {
            QLabel *moveLabel = new QLabel(this);
            moveLabel->hide();
            m_floatLabels << moveLabel;
        }
    }

    if (!m_calendarWidget) {
        m_calendarWidget = new QWidget(this);
        m_calendarWidget->hide();
    }

    if (!m_vlayout) {
        m_vlayout = new QVBoxLayout(this);
    }

    if (!m_monthLabel) {
        m_monthLabel = new QLabel(this);
        m_monthLabel->hide();
    }

    if (!m_dayLabel) {
        m_dayLabel = new QLabel(this);
        m_dayLabel->hide();
    }

    if (!m_weekLabel) {
        m_weekLabel = new QLabel(this);
        m_weekLabel->hide();
    }
}

void AppGridView::initConnection()
{
#ifndef DISABLE_DRAG_ANIMATION
    connect(m_dropThresholdTimer, &QTimer::timeout, this, &AppGridView::prepareDropSwap);
#else
    connect(m_dropThresholdTimer, &QTimer::timeout, this, &AppGridView::dropSwap);
#endif

    // 根据后端延迟触屏信号控制是否可进行图标拖动，收到延迟触屏信号可拖动，没有收到延迟触屏信号、点击松开就不可拖动
    connect(m_gestureInter, &Gesture::TouchSinglePressTimeout, this, &AppGridView::onTouchSinglePresse, Qt::UniqueConnection);
    connect(m_gestureInter, &Gesture::TouchUpOrCancel, this, &AppGridView::onTouchUpOrDown, Qt::UniqueConnection);
    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, &AppGridView::onLayoutChanged);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &AppGridView::onThemeChanged);
}

void AppGridView::initUi()
{
#ifdef QT_DEBUG
    setStyleSheet("QListView{border: 1px solid red;}");
#endif

    m_dropThresholdTimer->setInterval(DLauncher::APP_DRAG_SWAP_THRESHOLD);
    m_dropThresholdTimer->setSingleShot(true);

    viewport()->installEventFilter(this);
    viewport()->setAcceptDrops(true);
    createMovingComponent();

    setUniformItemSizes(true);
    setMouseTracking(true);
    setAcceptDrops(true);
    setDragEnabled(true);
    setWrapping(true);
    setFocusPolicy(Qt::NoFocus);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(false);// 解决拖拽释放前有小黑点出现的问题
    setMovement(QListView::Free);
    setWrapping(true);
    setLayoutMode(QListView::Batched);
    setResizeMode(QListView::Adjust);
    setViewMode(QListView::IconMode);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    setFrameStyle(QFrame::NoFrame);
    setViewportMargins(0, 0, 0, 0);
    viewport()->setAutoFillBackground(false);

    onThemeChanged(DGuiApplicationHelper::instance()->themeType());
    onLayoutChanged();
}

void AppGridView::onLayoutChanged()
{
#define ITEM_SPACING 15
    if (getViewType() == PopupView) {
#ifdef QT_DEBUG
        int leftMargin = m_calcUtil->appMarginLeft();
        int topMargin = m_calcUtil->appMarginTop();
        int itemSpacing = m_calcUtil->appItemSpacing();
        qDebug() << "set PopView margin" << ":topMargin:" << topMargin << ", leftMargin:" << leftMargin << ", itemSpacing:" << itemSpacing;
#endif
        setViewportMargins(0, 0, 0, 0);
        setSpacing(ITEM_SPACING);
    } else {
        int leftMargin = m_calcUtil->appMarginLeft();
        int topMargin = m_calcUtil->appMarginTop();
        int itemSpacing = m_calcUtil->appItemSpacing();
#ifdef QT_DEBUG
        qDebug() << "set MainView margin" << ":topMargin:" << topMargin << ", leftMargin:" << leftMargin << ", itemSpacing:" << itemSpacing;
#endif
        setSpacing(itemSpacing);
        setViewportMargins(leftMargin, topMargin, leftMargin, 0);
    }
}

void AppGridView::onTouchSinglePresse(int time, double scalex, double scaley)
{
    Q_UNUSED(time);
    Q_UNUSED(scalex);
    Q_UNUSED(scaley);

    m_longPressed = true;
}

void AppGridView::onTouchUpOrDown(double scalex, double scaley)
{
    Q_UNUSED(scalex);
    Q_UNUSED(scaley);

    m_longPressed = false;
}

void AppGridView::onThemeChanged(DGuiApplicationHelper::ColorType)
{
    update();
}
