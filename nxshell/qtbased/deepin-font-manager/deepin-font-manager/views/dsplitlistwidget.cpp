// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dsplitlistwidget.h"
#include "globaldef.h"
#include "utils.h"

#include <DApplication>
#include <DStyleHelper>
#include <DApplicationHelper>
#include <DLog>

#include <QToolTip>
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>
#include <QTimer>

#define FTM_SPLIT_TOP_SPACE_TAG "_space_"
#define FTM_SPLIT_TOP_SPLIT_TAG "_split_"
#define FTM_SPLIT_LINE_INDEX    5
#define TITLE_VISIBLE_WIDTH    96
#define IS_NEED_ELLIPSIS  30 //是否需要省略号

DNoFocusDelegate::DNoFocusDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
    , m_parentView(qobject_cast<DSplitListWidget *>(parent))
{

}

//用于去除选中项的边框
/*************************************************************************
 <Function>      paint
 <Description>   绘制函数
 <Author>        null
 <Input>
    <param1>     painter            Description:绘制类的指针
    <param2>     option             Description:当前项
    <param3>     index              Description:当前项索引
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DNoFocusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();
        painter->setFont(DApplication::font());
        painter->setRenderHint(QPainter::Antialiasing, true);

        QVariant varDisplay = index.data(Qt::DisplayRole);
        QString iniTitle = varDisplay.value<QString>();
        QFont nameFont = painter->font();
        QString strTitle = adjustLength(iniTitle, nameFont);

        QStyleOptionViewItem viewOption(option);  //用来在视图中画一个item

        DPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                                  ? DPalette::Normal : DPalette::Disabled;
        if (cg == DPalette::Normal && !(option.state & QStyle::State_Active)) {
            cg = DPalette::Inactive;
        }

        if (strTitle.startsWith(FTM_SPLIT_TOP_SPACE_TAG)) {
            //用于ListView顶部空白
        } else if (strTitle.startsWith(FTM_SPLIT_TOP_SPLIT_TAG)) {

            QRect lineRect;
            lineRect.setX(option.rect.x() + 10);
            lineRect.setY(option.rect.y() + option.rect.height() - 2);
            lineRect.setWidth(option.rect.width() - 20);
            lineRect.setHeight(2);
            //绘制分割线
            DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::ItemBackground);
            painter->fillRect(lineRect, fillColor);
        } else {

            QRect rect;
            rect.setX(option.rect.x());
            rect.setY(option.rect.y());
            rect.setWidth(option.rect.width());
            rect.setHeight(option.rect.height());

            QRect backgroundRect = QRect(rect.left() + 10, rect.top(), rect.width() - 20, rect.height());

            paintBackground(painter, option, backgroundRect, cg);

            //绘制标题
            /* bug#20266 UT000591 */ /*bug 21075 ut000442*/
            QRect fontNameRect;
            if (strTitle == "System") {
                fontNameRect = QRect(backgroundRect.left() + 10, backgroundRect.top() + 1, backgroundRect.width() - 20, backgroundRect.height() - 7);
            } else {
                fontNameRect = QRect(backgroundRect.left() + 10, backgroundRect.top() + 2, backgroundRect.width() - 20, backgroundRect.height() - 7);
            }

            nameFont.setWeight(QFont::Medium);
            nameFont.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6));
            painter->setFont(nameFont);

            if (option.state & QStyle::State_Selected) {
                painter->setPen(QPen(option.palette.color(DPalette::HighlightedText)));
                painter->drawText(fontNameRect, Qt::AlignLeft | Qt::AlignVCenter, strTitle);
            } else {
                painter->setPen(QPen(option.palette.color(DPalette::Text)));
                painter->drawText(fontNameRect, Qt::AlignLeft | Qt::AlignVCenter, strTitle);
            }
        }
        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

/*************************************************************************
 <Function>      sizeHint
 <Description>   获取一项的size
 <Author>        null
 <Input>
    <param1>     option            Description:listview中的一行成员
    <param2>     index             Description:当前项的索引
 <Return>        QSize             Description:返回的尺寸大小
 <Note>          null
*************************************************************************/
QSize DNoFocusDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    Q_UNUSED(index)

    int rowIndex = index.row();

#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
        if (FTM_SPLIT_LINE_INDEX == rowIndex) {
            return QSize(option.rect.width(), 20);
        } else {
            return QSize(option.rect.width(), 24);
        }
    }
#endif

    if (FTM_SPLIT_LINE_INDEX == rowIndex) {
        return QSize(option.rect.width(), 24);
    } else {
        return QSize(option.rect.width(), 36);
    }
}


/*************************************************************************
 <Function>      paintBackground
 <Description>   绘制背景
 <Author>        null
 <Input>
    <param1>     painter                   Description:绘制类指针
    <param2>     option                    Description:listview中的一行成员
    <param3>     backgroundRect            Description:绘制背景区域
    <param4>     cg                        Description:ColorGroup
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DNoFocusDelegate::paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QRect &backgroundRect, const QPalette::ColorGroup cg) const
{
    //绘制左侧列表外部高亮区域的路径
    QPainterPath pathFirst;
    const int radius = 8;
    setPaintPath(backgroundRect, pathFirst, 0, 0, radius);
    bool isTabFocus = m_parentView->IsTabFocus();
    if (option.state & QStyle::State_Selected) {
        //判断是否为通过tab获得的焦点
        if (isTabFocus) {
            //如果为hover状态，颜色亮度需要加亮
            if (option.state & QStyle::State_MouseOver) {
                paintTabBackground(painter, option, backgroundRect, cg, true);
            } else {
                paintTabBackground(painter, option, backgroundRect, cg, false);
            }
        } else if (!isTabFocus) {
            QColor fillColor = option.palette.color(cg, DPalette::Highlight);
            //如果为hover状态，颜色亮度需要加亮
            if (option.state & QStyle::State_MouseOver) {
                fillColor = fillColor.light(120);
            }
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(pathFirst, painter->brush());
        }
    } else if (option.state & QStyle::State_MouseOver) {
        //未选中状态下的hover，需要有淡灰色背景
        DStyleHelper styleHelper;
        QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::ToolTipText);
        fillColor.setAlphaF(0.1);
        painter->setBrush(QBrush(fillColor));
        painter->fillPath(pathFirst, painter->brush());
    }
}

/*************************************************************************
 <Function>      paintTabBackground
 <Description>   绘制tab选中后的背景
 <Author>        null
 <Input>
    <param1>     painter                   Description:绘制类指针
    <param2>     option                    Description:listview中的一行成员
    <param3>     backgroundRect            Description:绘制背景区域
    <param4>     cg                        Description:ColorGroup
    <param4>     isHover                   Description:是否为hover状态
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DNoFocusDelegate::paintTabBackground(QPainter *painter, const QStyleOptionViewItem &option,
                                          const QRect &backgroundRect, const QPalette::ColorGroup cg, const bool isHover) const
{
    //绘制左侧列表外部高亮区域的路径
    QPainterPath pathFirst;
    const int radius = 8;
    setPaintPath(backgroundRect, pathFirst, 0, 0, radius);

    //绘制左侧列表窗口色区域的路径
    QPainterPath pathSecond;
    setPaintPath(backgroundRect, pathSecond, 2, 2, 6);

    //绘制左侧列表内部高亮区域的路径
    QPainterPath pathThird;
    setPaintPath(backgroundRect, pathThird, 3, 3, 6);

    QColor fillColor = option.palette.color(cg, DPalette::Highlight);

    //如果为hover状态，颜色亮度需要加亮
    if (isHover) {
        fillColor = fillColor.light(120);
    }
    painter->setBrush(QBrush(fillColor));
    painter->fillPath(pathFirst, painter->brush());

    QColor fillColor2 = option.palette.color(cg, DPalette::Window);
    painter->setBrush(QBrush(fillColor2));
    painter->fillPath(pathSecond, painter->brush());

    painter->setBrush(QBrush(fillColor));
    painter->fillPath(pathThird, painter->brush());

}

/*************************************************************************
 <Function>      setPaintPath
 <Description>   设置需要绘制区域的路径
 <Author>        null
 <Input>
    <param1>     bgRect            Description:listview一项的区域范围
    <param2>     path              Description:需要绘制区域的路径
    <param3>     xDifference       Description:x方向需要变化的数值
    <param4>     yDifference       Description:y方向需要变化的数值
    <param5>     radius            Description:圆弧半径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DNoFocusDelegate::setPaintPath(const QRect &bgRect, QPainterPath &path, const int xDifference, const int yDifference, const int radius) const
{
    QPoint path_bottomRight(bgRect.bottomRight().x() - xDifference, bgRect.bottomRight().y() - yDifference);
    QPoint path_topRight(bgRect.topRight().x() - xDifference, bgRect.topRight().y() + yDifference);
    QPoint path_topLeft(bgRect.topLeft().x() + xDifference, bgRect.topLeft().y() + yDifference);
    QPoint path_bottomLeft(bgRect.bottomLeft().x() + xDifference, bgRect.bottomLeft().y() - yDifference);
    path.moveTo(path_bottomRight - QPoint(0, 10));
    path.lineTo(path_topRight + QPoint(0, 10));
    path.arcTo(QRect(QPoint(path_topRight - QPoint(radius * 2, 0)), QSize(radius * 2, radius * 2)), 0, 90);
    path.lineTo(path_topLeft + QPoint(10, 0));
    path.arcTo(QRect(QPoint(path_topLeft), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(path_bottomLeft - QPoint(0, 10));
    path.arcTo(QRect(QPoint(path_bottomLeft - QPoint(0, radius * 2)), QSize(radius * 2, radius * 2)), 180, 90);
    path.lineTo(path_bottomRight - QPoint(10, 0));
    path.arcTo(QRect(QPoint(path_bottomRight - QPoint(radius * 2, radius * 2)), QSize(radius * 2, radius * 2)), 270, 90);
}



/*************************************************************************
 <Function>      adjustLength
 <Description>   自适应长度
 <Author>        null
 <Input>
    <param1>     titleName       Description:名字
    <param2>     font            Description:当前字体
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
QString DNoFocusDelegate::adjustLength(QString &titleName, QFont &font) const
{
    QFontMetrics fontMetric(font);

    QString finalTitle = "";
    QString m_curTitle = "";

    for (auto str : titleName) {
        m_curTitle += str;
        if (fontMetric.width(m_curTitle) > TITLE_VISIBLE_WIDTH) {
            if (m_curTitle == titleName) {
                finalTitle = m_curTitle;
                break;
            } else {
                if (fontMetric.width("...") > IS_NEED_ELLIPSIS) {
                    finalTitle = m_curTitle;
                } else {
                    finalTitle =   m_curTitle.append("...");
                }
                break;
            }
        } else {
            finalTitle = titleName;
        }
    }

    return finalTitle;
}

DSplitListWidget::DSplitListWidget(QWidget *parent)
    : DListView(parent)
{
    //去除选中项的边框
    this->setItemDelegate(new DNoFocusDelegate(this));
    this->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
    this->setAutoScroll(false);
    this->setMouseTracking(true);
    this->installEventFilter(this);
    initListData();
    connect(m_signalManager, &SignalManager::setSpliteWidgetScrollEnable, this, &DSplitListWidget::setRefreshFinished);
    connect(m_signalManager, &SignalManager::setLostFocusState, this, [ = ](bool isTrue) {
        m_isFocusFromFontListView = isTrue;
    });
}

DSplitListWidget::~DSplitListWidget() {}

void DSplitListWidget::initListData()
{
    m_titleStringList << DApplication::translate("Category", "All Fonts")
                      << DApplication::translate("Category", "System")
                      << DApplication::translate("Category", "User")
                      << DApplication::translate("Category", "Favorites")
                      << DApplication::translate("Category", "Active")
                      << DApplication::translate("Category", "Chinese")
                      << DApplication::translate("Category", "Monospaced");
    for (int i = 0; i < m_titleStringList.size(); i++) {
        QString titleString = m_titleStringList.at(i);
        m_titleStringIndexMap.insert(titleString, i);
    }

    m_categoryItemModell = new QStandardItemModel(this);

    int iTitleIndex = 0;
    for (int i = 0; i < m_titleStringList.size() + 1; i++) {
        QStandardItem *item = new QStandardItem;
        if (FTM_SPLIT_LINE_INDEX == i) {
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
            item->setData(QVariant::fromValue(QString(FTM_SPLIT_TOP_SPLIT_TAG)), Qt::DisplayRole);
        } else {
            QString titleString = m_titleStringList.at(iTitleIndex++);
            item->setData(QVariant::fromValue(titleString), Qt::DisplayRole);
        }
        m_categoryItemModell->appendRow(item);
    }

    this->setModel(m_categoryItemModell);

    //设置默认选中
    QModelIndex index = m_categoryItemModell->index(AllFont, 0);
    selectionModel()->select(index, QItemSelectionModel::Select);
}

/*************************************************************************
 <Function>      setIsHalfWayFocus
 <Description>   设置是否为其它原因获取的焦点
 <Author>        null
 <Input>
    <param1>     IsHalfWayFocus            Description:是否为其它原因获取的焦点
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DSplitListWidget::setIsHalfWayFocus(bool IsHalfWayFocus)
{
    m_IsHalfWayFocus = IsHalfWayFocus;
}

/*************************************************************************
 <Function>      IsTabFocus
 <Description>   判断是否tab选中
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        bool            Description:是否tab选中
 <Note>          null
*************************************************************************/
bool DSplitListWidget::IsTabFocus() const
{
    return m_IsTabFocus;
}

/*************************************************************************
 <Function>      getStatus
 <Description>   获取当前SplitListview的状态
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        FocusStatus            Description:当前SplitListview的状态
 <Note>          null
*************************************************************************/
FocusStatus &DSplitListWidget::getStatus()
{
    m_currentStatus.m_IsFirstFocus = this->m_IsFirstFocus;
    m_currentStatus.m_IsHalfWayFocus = this->m_IsHalfWayFocus;
    m_currentStatus.m_IsMouseClicked = this->m_IsMouseClicked;

    return m_currentStatus;
}

/*************************************************************************
 <Function>      setCurrentStatus
 <Description>   设置SplitListView的状态
 <Author>        null
 <Input>
    <param1>     currentStatus            Description:SplitListView的状态
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DSplitListWidget::setCurrentStatus(const FocusStatus &currentStatus)
{
    m_currentStatus = currentStatus;
}

/*************************************************************************
 <Function>      mouseMoveEvent
 <Description>   鼠标移动事件
 <Author>        null
 <Input>
    <param1>     event            Description:鼠标移动事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DSplitListWidget::mouseMoveEvent(QMouseEvent *event)
{
    //正在安装过程中，禁止鼠标事件
    if (m_isIstalling) {
        return;
    }

    //更新用于判断是否弹出提示信息的鼠标状态标志位
    m_isMouseMoved = true;
    if (QToolTip::isVisible()) {
        QToolTip::hideText();
    }

    QPoint difference_pos = event->pos() - lastTouchBeginPos;

    //通过亮的点的y值差距获得移动的趋势
    if (difference_pos.y() > 0) {
        m_IsPositive = true;
    } else {
        m_IsPositive = false;
    }

    QPoint clickPoint = event->pos();
    QModelIndex modelIndex = indexAt(clickPoint);

    if (modelIndex.row() == 5)
        return;
    DListView::mouseMoveEvent(event);
}

/*************************************************************************
 <Function>      setRefreshFinished
 <Description>   设置安装是否结束
 <Author>        null
 <Input>
    <param1>     isInstalling            Description:安装是否结束
 <Return>        null                    Description:null
 <Note>          null
*************************************************************************/
void DSplitListWidget::setRefreshFinished(bool isInstalling)
{
    if (isInstalling) {
        m_refreshFinished = false;
    } else {
        m_refreshFinished = true;
    }
}

/*************************************************************************
 <Function>      wheelEvent
 <Description>   滚轮事件
 <Author>        null
 <Input>
    <param1>     event            Description:滚轮事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DSplitListWidget::wheelEvent(QWheelEvent *event)
{
    if (!m_refreshFinished)
        return;
    int now = -1;
    //bug47986处理方案中或出现选项在分隔符位置的场景，此时菜单选项为空，之前不需考虑为空的情况，所以需要增加以下判断
    //选项为空则当前位置为分隔符
    if (selectedIndexes().count() == 0)
        now = FTM_SPLIT_LINE_INDEX;
    else
        now = selectedIndexes().last().row();
    int next = now;
    if (event->delta() > 0) {
        if (now > 0) {
            if (now == 6) {
                next = now - 2;
            } else {
                next = now - 1;
            }
        } else {
            return;
        }
    } else {
        if (now < this->count() - 1) {
            if (now == 4) {
                next = now + 2;
            } else {
                next = now + 1;
            }
        } else {
            return;
        }
    }
    if (next == now) {
        return;
    } else {
        this->setModel(m_categoryItemModell);
        QStandardItem *item = m_categoryItemModell->item(next);
        QModelIndex modelIndex = m_categoryItemModell->indexFromItem(item);
        setCurrentIndex(modelIndex);
        Q_EMIT m_signalManager->changeView();
        if (next > 5) {
            emit  onListWidgetItemClicked(next - 1);
        } else {
            emit  onListWidgetItemClicked(next);
        }
    }
}

/*************************************************************************
 <Function>      setCurrentPage
 <Description>   进行操作后切换到当前的页面
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DSplitListWidget::setCurrentPage()
{
    QModelIndex current = currentIndex();

    int row = current.row();
    qDebug() << __FUNCTION__ << row << m_LastPageNumber;
    if (row < 0 || FTM_SPLIT_LINE_INDEX == row || row == m_LastPageNumber) {
        return;
    }

    m_LastPageNumber = row;
    QStandardItem *item = m_categoryItemModell->item(row);
    QVariant varUserData = item->data(Qt::DisplayRole).value<QVariant>();
    int realIndex = m_titleStringIndexMap.value(varUserData.toString());

    emit onListWidgetItemClicked(realIndex);
    emit m_signalManager->changeView();
}

/*************************************************************************
 <Function>      setLastPageNumber
 <Description>   记录当前界面的上一个界面
 <Author>        null
 <Input>
    <param1>     LastPageNumber            Description:当前界面的上一个界面
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DSplitListWidget::setLastPageNumber(int LastPageNumber)
{
    m_LastPageNumber = LastPageNumber;
}

void DSplitListWidget::setIsIstalling(bool isIstalling)
{
    m_isIstalling = isIstalling;
}

/*************************************************************************
 <Function>      mouseReleaseEvent
 <Description>   鼠标释放事件
 <Author>        null
 <Input>
    <param1>     event           Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DSplitListWidget::mouseReleaseEvent(QMouseEvent *event)
{
    //TODO
    //正在安装过程中，禁止鼠标事件
    if (m_isIstalling)
        return;
    QPoint clickPoint = event->pos();
    QModelIndex modelIndex = indexAt(clickPoint);
    //  根据移动趋势进行移动到分割线时的处理
    if (modelIndex.row() == FTM_SPLIT_LINE_INDEX) {
        //
        if (m_IsPositive) {
            modelIndex = m_categoryItemModell->index(6, 0);
            setCurrentIndex(modelIndex);
        } else {
            modelIndex = m_categoryItemModell->index(4, 0);
            setCurrentIndex(modelIndex);
        }
    }
    setCurrentPage();

    //如果鼠标释放时，位置在有效菜单项，则延时弹出提示信息
    if (viewport()->visibleRegion().contains(event->pos())) {
        //初始化鼠标状态标志位
        m_isMouseMoved = false;

        QModelIndex curIndex = indexAt(event->pos());
        QPoint showPoint = event->globalPos();
        const QString tooltip = curIndex.data(Qt::DisplayRole).toString();
        QTimer::singleShot(500, [ = ] {
            //如果悬停位置为空、位置为分割线或中途鼠标移动,不弹出提示信息
            if (tooltip.isEmpty() || tooltip == "_split_" || m_isMouseMoved == true)
                return;
            QToolTip::showText(showPoint, tooltip, this);
        });
    }

    DListView::mouseReleaseEvent(event);
}

/*************************************************************************
 <Function>      keyPressEvent
 <Description>   鼠标点击事件
 <Author>        null
 <Input>
    <param1>     event            Description:鼠标点击事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DSplitListWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up) {
        if (currentIndex().row() == 0) {
            QModelIndex modelIndex = m_categoryItemModell->index(7, 0);
            setCurrentIndex(modelIndex);
        } else {
            DListView::keyPressEvent(event);
        }
        setCurrentPage();
    } else if (event->key() == Qt::Key_Down) {
        if (currentIndex().row() == 7) {
            QModelIndex modelIndex = m_categoryItemModell->index(0, 0);
            setCurrentIndex(modelIndex);
        } else {
            DListView::keyPressEvent(event);
        }
        setCurrentPage();
    } else if (event->key() == Qt::Key_End || event->key() == Qt::Key_Home) {
        event->accept();
    } else {
        DListView::keyPressEvent(event);
    }
}

/*************************************************************************
 <Function>      eventFilter
 <Description>   事件过滤器
 <Author>        null
 <Input>
    <param1>     obj              Description:UNUSED
    <param2>     event            Description:事件
 <Return>        bool             Description:是否处理
 <Note>          null
*************************************************************************/
bool DSplitListWidget::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)

//失去焦点的时候对这几个标志位进行复位
    if (event->type() == QEvent::FocusOut) {
        m_IsMouseClicked = false;
        m_IsTabFocus = false;
        m_IsLeftFocus = false;
        m_IsHalfWayFocus = false;
    }

//获取焦点时只要不是通过鼠标点击获取焦点以及不是打开软件自动设置的焦点以及不是其他过程中途设置的焦点，就是判断为通过tab获取到的焦点
    if (event->type() == QEvent::FocusIn) {
        qDebug() << m_isFocusFromFontListView;
        if (!m_IsMouseClicked && !m_IsFirstFocus && !m_IsHalfWayFocus && !m_isFocusFromFontListView) {
            m_IsTabFocus = true;
        }
//        m_IsFristTimeFocus = false;
        m_IsFirstFocus = false;
    }
    return false;
}


/*************************************************************************
 <Function>      helpEvent
 <Description>   左侧列表hover现实提示信息.
 <Author>        null
 <Input>
    <param1>     event             Description:QHelpEvent
    <param2>     view              Description:listview
    <param3>     option            Description:当前项
    <param4>     index             Description:当前项的索引
 <Return>        bool              Description:委托可以处理该事件，则返回true;否则返回false
 <Note>          null
*************************************************************************/
bool DNoFocusDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view
                                 , const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QToolTip::hideText();
    if (event->type() == QEvent::ToolTip) {
        const QString tooltip = index.data(Qt::DisplayRole).toString();
        qDebug() << __FUNCTION__ << "__now Hover is :__" << tooltip;
        if (tooltip.isEmpty() || tooltip == "_split_") {
            hideTooltipImmediately();
        } else {
            int tooltipsize = tooltip.size();
            const int nlong = 32;
            int lines = tooltipsize / nlong + 1;
            QString strtooltip;
            for (int i = 0; i < lines; ++i) {
                strtooltip.append(tooltip.mid(i * nlong, nlong));
                strtooltip.append("\n");
            }
            strtooltip.chop(1);
//            QTimer::singleShot(1000, [ = ]() {
            QToolTip::showText(event->globalPos(), strtooltip, view);

//            QTimer::singleShot(500, []() {
//                QToolTip::hideText();
//            });
//            });
        }
        return false;
    }
    return DNoFocusDelegate::helpEvent(event, view, option, index);
}

/*************************************************************************
 <Function>      hideTooltipImmediately
 <Description>   隐藏提示信息
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DNoFocusDelegate::hideTooltipImmediately()
{
    QWidgetList qwl = QApplication::topLevelWidgets();
    for (QWidget *qw : qwl) {
        if (QStringLiteral("QTipLabel") == qw->metaObject()->className()) {
            qw->close();
        }
    }
}

/*************************************************************************
 <Function>      mousePressEvent
 <Description>   鼠标点击事件
 <Author>        null
 <Input>
    <param1>     event            Description:鼠标点击事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DSplitListWidget::mousePressEvent(QMouseEvent *event)
{
    QPoint clickPoint = event->pos();
    QModelIndex modelIndex = indexAt(clickPoint);

    m_IsMouseClicked = true;
    //正在安装过程中，禁止鼠标事件
    if (Qt::RightButton == event->button() || Qt::MiddleButton == event->button() || m_isIstalling) {
        return;
    }

    //鼠标按下时记录点击的点，用作后面获得移动的趋势
    lastTouchBeginPos = event->pos();

    //应该设置焦点，否则鼠标在其他区域release会导致缺失焦点。
    //    setFocus(Qt::MouseFocusReason);
    if (modelIndex.row() == currentIndex().row())
        return;
    //ut000465根据产品要求左侧列表点击不做focus效果处理，因此即使左侧列表通过tab获取焦点，点击后去除tabfocus状态，不绘制focus选中边框
    if (hasFocus() && m_IsTabFocus) {
        m_IsTabFocus = false;
    }
    DListView::mousePressEvent(event);
}
