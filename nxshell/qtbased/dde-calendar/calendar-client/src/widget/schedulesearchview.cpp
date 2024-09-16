// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "schedulesearchview.h"
#include "schedulectrldlg.h"
#include "myscheduleview.h"
#include "scheduledlg.h"
#include "scheduledatamanage.h"
#include "constants.h"
#include "cscheduleoperation.h"
#include "scheduledaterangeinfo.h"
#include "calendarmanage.h"
#include "calendarglobalenv.h"

#include <DHiDPIHelper>
#include <DPalette>
#include <DGuiApplicationHelper>

#include <QAction>
#include <QMenu>
#include <QListWidget>
#include <QPainter>
#include <QHBoxLayout>
#include <QStylePainter>
#include <QRect>
#include <QPainterPath>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QShortcut>

DGUI_USE_NAMESPACE
CScheduleSearchItem::CScheduleSearchItem(QWidget *parent)
    : DLabel(parent)
    , m_rightMenu(new DMenu(this))
    , m_timeFormat(CalendarManager::getInstance()->getTimeFormat())
{
    //设置对象名称和辅助显示名称
    this->setObjectName("CScheduleDataItem");
    this->setAccessibleName("CScheduleDataItem");

    m_editAction = new QAction(tr("Edit"), this);
    m_deleteAction = new QAction(tr("Delete"), this);
    connect(m_editAction, SIGNAL(triggered(bool)), this, SLOT(slotEdit()));
    connect(m_deleteAction, SIGNAL(triggered(bool)), this, SLOT(slotDelete()));
    setTheMe(DGuiApplicationHelper::instance()->themeType());
    QObject::connect(CalendarManager::getInstance(), &CalendarManager::signalTimeFormatChanged,
                     this, &CScheduleSearchItem::slotTimeFormatChanged);
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::paletteTypeChanged,
                     this,
                     &CScheduleSearchItem::setTheMe);
    m_mouseStatus = M_NONE;
    installEventFilter(this);
    //设置焦点类型
    setFocusPolicy(Qt::TabFocus);
}

void CScheduleSearchItem::setBackgroundColor(QColor color1)
{
    m_Backgroundcolor = color1;
}

void CScheduleSearchItem::setText(QColor tColor, QFont font)
{
    m_tTextColor = tColor;
    m_tFont = font;
    //设置时间显示宽度
    setDurationSize(font);
}

void CScheduleSearchItem::setTimeC(QColor tColor, QFont font)
{
    m_timecolor = tColor;
    m_timefont = font;
}

void CScheduleSearchItem::setData(DSchedule::Ptr vScheduleInfo, QDate date)
{
    m_ScheduleInfo = vScheduleInfo;
    m_date = date;
    update();
}

void CScheduleSearchItem::setRoundtype(int rtype)
{
    m_roundtype = rtype;
    update();
}

void CScheduleSearchItem::setTheMe(int type)
{
    if (type == 2) {
        m_presscolor.background = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        m_presscolor.background.setAlphaF(1);
        m_presscolor.timeColor = "#FFFFFF";
        m_presscolor.timeColor.setAlphaF(1);
        m_presscolor.textColor = "#FFFFFF";
        m_presscolor.textColor.setAlphaF(1);

        m_hovercolor.background = "#FFFFFF";
        m_hovercolor.background.setAlphaF(0.2);
        m_hovercolor.timeColor = "#6D7C88";
        m_hovercolor.timeColor.setAlphaF(1);
        m_hovercolor.textColor = "#C0C6D4";
        m_hovercolor.textColor.setAlphaF(1);
    } else {
        m_presscolor.background = CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor();
        m_presscolor.background.setAlphaF(1);
        m_presscolor.timeColor = "#FFFFFF";
        m_presscolor.timeColor.setAlphaF(1);
        m_presscolor.textColor = "#FFFFFF";
        m_presscolor.textColor.setAlphaF(1);

        m_hovercolor.background = "#000000";
        m_hovercolor.background.setAlphaF(0.2);
        m_hovercolor.timeColor = "#526A7F";
        m_hovercolor.timeColor.setAlphaF(1);
        m_hovercolor.textColor = "#414D68";
        m_hovercolor.textColor.setAlphaF(1);
    }
}

/**
 * @brief CScheduleSearchItem::setDurationSize 根据时间字体font设置时间宽度
 */
void CScheduleSearchItem::setDurationSize(QFont font)
{
    QFontMetrics fm1(font);
    QString currentTimeStr = QTime::currentTime().toString("hh:mm");
    //设置时间显示最大宽度
    m_durationSize = fm1.horizontalAdvance(currentTimeStr + "-" + currentTimeStr);
}
void CScheduleSearchItem::slotEdit()
{
    CScheduleDlg dlg(0, this);
    dlg.setData(m_ScheduleInfo);
    dlg.exec();
}

void CScheduleSearchItem::slotDelete()
{
    //删除日程
    CScheduleOperation _scheduleOperation(m_ScheduleInfo->scheduleTypeID(), this);
    bool _isDelete = _scheduleOperation.deleteSchedule(m_ScheduleInfo);
    //删除日程后，将焦点设置给父类
    if (_isDelete) {
        parentWidget()->setFocus(Qt::TabFocusReason);
    } else {
        this->setFocus();
    }
}

/**
 * @brief CScheduleSearchItem::slotTimeFormatChanged 更新时间显示格式
 */
void CScheduleSearchItem::slotTimeFormatChanged(int value)
{
    if (value) {
        m_timeFormat = "hh:mm";
    } else {
        m_timeFormat = "h:mm";
    }
    update();
}

void CScheduleSearchItem::slotSchotCutClicked()
{
    //选中该item时才可以使用快捷键
    if (m_tabFocus) {
        //节日日程不能使用
        if (CScheduleOperation::isFestival(m_ScheduleInfo))
            return;
        m_rightMenu->clear();
        m_rightMenu->addAction(m_editAction);
        m_rightMenu->addAction(m_deleteAction);
        //获取item坐标,并转换为全局坐标
        QPointF itemPos = QPointF(this->rect().x() + this->rect().width() / 2,
                                  this->rect().y() + this->rect().height() / 2);
        QPoint shoutCurPos = mapToGlobal(itemPos.toPoint());
        m_rightMenu->exec(shoutCurPos);
    }
}

void CScheduleSearchItem::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    int labelRightX = width() - 2; //绘制区域x方向右边界坐标点
    int labelBottomY = height();    //绘制区域y方向下边界坐标点
    QPainter painter(this);
    QColor bColor = m_Backgroundcolor;
    QColor textcolor = m_tTextColor;
    QColor timecolor = m_timecolor;

    switch (m_mouseStatus) {
    case M_NONE: {
        bColor = m_Backgroundcolor;
        textcolor = m_tTextColor;
        timecolor = m_timecolor;
    } break;
    case M_HOVER: {
        bColor = m_hovercolor.background;
        textcolor = m_hovercolor.textColor;
        timecolor = m_hovercolor.timeColor;
    } break;
    case M_PRESS: {
        bColor = m_presscolor.background;
        textcolor = m_presscolor.textColor;
        timecolor = m_presscolor.timeColor;
    } break;
    }
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    painter.setBrush(QBrush(bColor));
    painter.setPen(Qt::NoPen);

    QPainterPath painterPath;
    painterPath.moveTo(m_radius, m_borderframewY);

    if (m_roundtype == 1 || m_roundtype == 3) {
        painterPath.arcTo(QRect(m_borderframewX, m_borderframewY, m_radius * 2, m_radius * 2), 90, 90);
    } else {
        painterPath.lineTo(m_borderframewX, m_borderframewY);
        painterPath.lineTo(m_borderframewX, m_radius);
    }
    painterPath.lineTo(m_borderframewX, labelBottomY - m_radius);

    if (m_roundtype == 1 || m_roundtype == 2) {
        painterPath.arcTo(QRect(m_borderframewX, labelBottomY - m_radius * 2, m_radius * 2, m_radius * 2), 180, 90);
    } else {
        painterPath.lineTo(m_borderframewX, labelBottomY);
        painterPath.lineTo(m_radius, labelBottomY);
    }
    painterPath.lineTo(labelRightX - m_radius, labelBottomY);

    if (m_roundtype == 1 || m_roundtype == 2) {
        painterPath.arcTo(QRect(labelRightX - m_radius * 2, labelBottomY - m_radius * 2, m_radius * 2, m_radius * 2), 270, 90);
    } else {
        painterPath.lineTo(labelRightX, labelBottomY);
        painterPath.lineTo(labelRightX, labelBottomY - m_radius);
    }
    painterPath.lineTo(labelRightX, m_radius);

    if (m_roundtype == 1 || m_roundtype == 3) {
        painterPath.arcTo(QRect(labelRightX - m_radius * 2, m_borderframewY, m_radius * 2, m_radius * 2), 0, 90);

    } else {
        painterPath.lineTo(labelRightX, m_borderframewY);
        painterPath.lineTo(labelRightX - m_radius, m_borderframewY);
    }

    painterPath.lineTo(m_radius, m_borderframewY);
    painterPath.closeSubpath();
    painter.drawPath(painterPath);
    painter.restore();

    painter.setFont(m_timefont);
    painter.setPen(timecolor);

    QString datestr;

    datestr = m_ScheduleInfo->dtStart().toString(m_timeFormat)
              + "-" + m_ScheduleInfo->dtEnd().toString(m_timeFormat);

    int flag = Qt::AlignLeft | Qt::AlignVCenter;

    if (m_ScheduleInfo->allDay()) {
        datestr = tr("All Day");
    }
    painter.drawText(QRect(12, 8, m_durationSize, labelBottomY - 16), flag, datestr);

    painter.save();
    DScheduleType::Ptr type =  gScheduleManager->getScheduleTypeByScheduleId(m_ScheduleInfo->scheduleTypeID());
    if (nullptr != type) {
        bColor = type->typeColor().colorCode();
    }
    QPen pen(bColor);
    pen.setWidth(2);
    painter.setPen(pen);
    //由于绘制的矩形大小的改变，所以需要调整起始和截止y坐标
    painter.drawLine(m_durationSize + 17, m_borderframewY, m_durationSize + 17, labelBottomY);
    painter.restore();

    painter.setFont(m_tFont);
    painter.setPen(textcolor);
    QString ellipsis = "...";
    QFontMetrics fm = painter.fontMetrics();
    //整个label宽度-文字起始位置
    int tilenameW = labelRightX - m_borderframewX - (m_durationSize + 26);
    QString tSTitleName = m_ScheduleInfo->summary();
    tSTitleName.replace("\n", "");
    QString str = tSTitleName;
    QString tStr;

    for (int i = 0; i < str.count(); i++) {
        tStr.append(str.at(i));
        tStr.append(ellipsis);
        int widthT = fm.width(tStr);
        tStr.remove(ellipsis);
        if (widthT >= tilenameW) {
            tStr.chop(1);
            break;
        }
    }

    if (tStr != str) {
        tStr = tStr + "...";
    }
    this->setToolTip(str);
    painter.drawText(QRect(m_durationSize + 17 + 9, 6, tilenameW, labelBottomY), Qt::AlignLeft, tStr);

    //存在焦点时绘制边框
    if (m_tabFocus) {
        painter.setPen(Qt::NoPen);
        // 设置画刷颜色
        painter.setBrush(CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor());
        QPainterPath path;
        int w = 2; //边框宽度
        path.addRect(m_borderframewX, m_borderframewY, labelRightX - m_borderframewX, labelBottomY - m_borderframewY);
        path.addRect(m_borderframewX + w, m_borderframewY + w, labelRightX - 2 * w - m_borderframewX, labelBottomY - 2 * w - m_borderframewY);
        painter.drawPath(path);
    }

    painter.end();
}
void CScheduleSearchItem::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event);
    //判断是否为节假日日程
    if (CScheduleOperation::isFestival(m_ScheduleInfo))
        return;
    //在有些环境中弹出右击菜单不会触发leaveEvent，主动更新leave对应的事件处理
    m_mouseStatus = M_NONE;
    update();
    m_rightMenu->clear();
    m_rightMenu->addAction(m_editAction);
    m_rightMenu->addAction(m_deleteAction);
    //不可修改的日程删除按钮无效
    m_deleteAction->setEnabled(!CScheduleOperation::scheduleIsInvariant(m_ScheduleInfo));
    m_rightMenu->exec(QCursor::pos());
    //恢复按钮状态
    m_deleteAction->setEnabled(true);
}

void CScheduleSearchItem::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    CMyScheduleView dlg(m_ScheduleInfo, this);
    dlg.exec();
}

void CScheduleSearchItem::mousePressEvent(QMouseEvent *event)
{
    //鼠标点击取消焦点显示
    m_tabFocus = false;
    if (event->button() == Qt::LeftButton) {
        //注册为鼠标操作
        if (!CalendarGlobalEnv::getGlobalEnv()->registerKey("SearchItemEvent", "MousePress")) {
            CalendarGlobalEnv::getGlobalEnv()->reviseValue("SearchItemEvent", "MousePress");
        }
        emit signalSelectSchedule(m_ScheduleInfo);
    }
    DLabel::mousePressEvent(event);
}

void CScheduleSearchItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_mouseStatus = M_HOVER;
        update();
    }
    DLabel::mouseReleaseEvent(event);
}

void CScheduleSearchItem::enterEvent(QEvent *event)
{
    DLabel::enterEvent(event);
    m_mouseStatus = M_HOVER;
    update();
}

void CScheduleSearchItem::leaveEvent(QEvent *event)
{
    DLabel::leaveEvent(event);
    m_mouseStatus = M_NONE;
    update();
}

bool CScheduleSearchItem::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o);

    if (e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *m_press = dynamic_cast<QMouseEvent *>(e);
        if (m_press->button() == Qt::LeftButton) {
            m_mouseStatus = M_PRESS;
        }
    }
    update();
    return false;
}

void CScheduleSearchItem::focusInEvent(QFocusEvent *e)
{
    if (e->reason() == Qt::TabFocusReason) {
        //注册为键盘操作
        if (!CalendarGlobalEnv::getGlobalEnv()->registerKey("SearchItemEvent", "Keyboard")) {
            CalendarGlobalEnv::getGlobalEnv()->reviseValue("SearchItemEvent", "Keyboard");
        }
       emit signalSelectSchedule(m_ScheduleInfo);
        emit signalSelectCurrentItem(this, false);
        m_tabFocus = true;
    }
    if (e->reason() == Qt::ActiveWindowFocusReason || e->reason() == Qt::PopupFocusReason) {
        m_tabFocus = m_tabFocusBeforeActive;
    }
    DLabel::focusInEvent(e);
}

void CScheduleSearchItem::focusOutEvent(QFocusEvent *e)
{
   //只针对tab的情况生效
    if (e->reason() == Qt::TabFocusReason){
        emit signalSelectCurrentItem(this, true);
    }
   DLabel::focusOutEvent(e);
   m_tabFocusBeforeActive = m_tabFocus;
   m_tabFocus = false;
}

void CScheduleSearchItem::keyPressEvent(QKeyEvent *event)
{
    //回车显示我的日程详情
    if (event->key() == Qt::Key_Return) {
        CMyScheduleView dlg(m_ScheduleInfo, this);
        dlg.exec();
    }
    //获取Alt+m显示右击菜单
    if (event->modifiers() == Qt::ALT && event->key() == Qt::Key_M) {
        slotSchotCutClicked();
    }
    DLabel::keyPressEvent(event);
}

CScheduleSearchView::CScheduleSearchView(QWidget *parent)
    : DWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    m_gradientItemList = new CScheduleListWidget(parent);
    m_gradientItemList->setAlternatingRowColors(true);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_gradientItemList);
    // set default row
    m_gradientItemList->setCurrentRow(0);
    setLayout(layout);
    setMouseTracking(true);
    m_bBackgroundcolor.setAlphaF(0.03);
    m_gradientItemList->setLineWidth(0);
    m_labellist.clear();

    connect(m_gradientItemList, &CScheduleListWidget::signalListWidgetClicked, this, &CScheduleSearchView::slotListWidgetClicked);
    connect(gScheduleManager, &ScheduleManager::signalSearchScheduleUpdate, this, &CScheduleSearchView::slotScearedScheduleUpdate);
}

CScheduleSearchView::~CScheduleSearchView()
{
    for (int i = 0; i < m_gradientItemList->count(); i++) {
        QListWidgetItem *item11 = m_gradientItemList->takeItem(i);
        m_gradientItemList->removeItemWidget(item11);
        delete item11;
        item11 = nullptr;
    }
    m_gradientItemList->clear();
}

void CScheduleSearchView::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        m_bBackgroundcolor = "#000000";
        m_bBackgroundcolor.setAlphaF(0.03);
        m_btimecolor = "#526A7F";
        m_btTextColor = "#414D68";
        m_lBackgroundcolor = Qt::white;
        m_lTextColor = "#001A2E";
    } else if (type == 2) {
        m_bBackgroundcolor = "#FFFFFF";
        m_bBackgroundcolor.setAlphaF(0.05);
        m_btimecolor = "#6D7C88";
        m_btTextColor = "#C0C6D4";
        m_lBackgroundcolor = "#FFFFFF";
        m_lBackgroundcolor.setAlphaF(0.0);
        m_lTextColor = "#C0C6D4";
    }
    updateDateShow();
}

/**
 * @brief CScheduleSearchView::clearSearch      清空搜索
 */
void CScheduleSearchView::clearSearch()
{
    m_searchStr.clear();
    m_vlistData.clear();
    m_scheduleSearchItem.clear();
    //窗口隐藏后初始化对应的状态
    hasScheduleShow = false;
    m_selectItem = nullptr;
    for (int i = 0; i < m_gradientItemList->count(); i++) {
        QListWidgetItem *item11 = m_gradientItemList->takeItem(i);
        m_gradientItemList->removeItemWidget(item11);
        delete item11;
        item11 = nullptr;
    }
    m_labellist.clear();
    m_gradientItemList->clear();
    //清空搜索数据
    gScheduleManager->clearSearchSchedule();
}

void CScheduleSearchView::setMaxWidth(const int w)
{
    m_maxWidth = w;
}

/**
 * @brief CScheduleSearchView::getHasScheduleShow 是否搜索到日程
 */
bool CScheduleSearchView::getHasScheduleShow()
{
    return hasScheduleShow;
}

/**
 * @brief CScheduleSearchView::getScheduleStatus 获取当前item是否有焦点
 */
bool CScheduleSearchView::getScheduleStatus()
{
    return m_scheduleSearchItem.contains(m_selectItem);
}

/**
 * @brief CScheduleSearchView::deleteSchedule 删除选中item
 */
void CScheduleSearchView::deleteSchedule()
{
    currentDItemIndex = m_scheduleSearchItem.indexOf(m_selectItem);
    //如果存在该item且为节日日程不可操作
    if (currentDItemIndex >= 0 && !CScheduleOperation::isFestival(m_selectItem->getData())) {
        m_selectItem->slotDelete();
    }
}

/**
 * @brief CScheduleSearchView::updateDateShow   更新搜索日程显示
 */
void CScheduleSearchView::updateDateShow()
{
    m_currentItem = nullptr;
    //是否搜索到日程标志
    hasScheduleShow = true;

    //清空列表之前先断开信号响应
    for(auto widget : m_scheduleSearchItem){
        if(nullptr != widget){
            widget->disconnect();
        }
    }

    m_gradientItemList->clear();
    m_labellist.clear();
    m_scheduleSearchItem.clear();
    //找最近日程
    QDate tCurrentData = QDate::currentDate();
    //搜索日程过滤排序
    QMap<QDate, DSchedule::List> m_showData;
    qint64 offset = 1000;
    QDate topdate = tCurrentData;
    QMap<QDate, DSchedule::List>::const_iterator _iterator = m_vlistData.constBegin();
    DSchedule::List _showInfo {};
    for (; _iterator != m_vlistData.constEnd(); ++_iterator) {
        qint64 d = qAbs(_iterator.key().daysTo(tCurrentData));
        _showInfo.clear();
        for (int i = 0 ; i < _iterator.value().size(); ++i) {
            //如果开始时间日期为显示日期则显示,跨天日程只显示一个
            if (_iterator.value().at(i)->dtStart().date() == _iterator.key() || _iterator == m_vlistData.constBegin()) {
                _showInfo.append(_iterator.value().at(i));
            }
        }
        if (_showInfo.size() > 0) {
            //获取跟当前时间最近的日程时间
            if (d < offset) {
                offset = d;
                topdate = _iterator.key();
            }
            //排序
            std::sort(_showInfo.begin(), _showInfo.end());
            m_showData[_iterator.key()] = _showInfo;
        }
    }
    tCurrentData = topdate;
    QMap<QDate, DSchedule::List>::const_iterator _showIterator = m_showData.constBegin();
    for (; _showIterator != m_showData.constEnd(); ++_showIterator) {
        //创建显示日期项
        QListWidgetItem *dateItem = createItemWidget(_showIterator.key());
        if (_showIterator.key() == tCurrentData) {
            m_currentItem = dateItem;
        }
        if (_showIterator.value().size() == 1) {
            createItemWidget(_showIterator.value().at(0), _showIterator.key(), 1);
        } else if (_showIterator.value().size() == 2) {
            createItemWidget(_showIterator.value().at(0), _showIterator.key(), 3);
            createItemWidget(_showIterator.value().at(1), _showIterator.key(), 2);
        } else {
            createItemWidget(_showIterator.value().at(0), _showIterator.key(), 3);
            for (int j = 1; j < _showIterator->size() - 1; j++) {
                createItemWidget(_showIterator.value().at(j), _showIterator.key(), 0);
            }
            createItemWidget(_showIterator.value().at(_showIterator->size() - 1), _showIterator.key(), 2);
        }
    }
    if (m_gradientItemList->count() == 0) {
        hasScheduleShow = false;
        QListWidgetItem *listItem = new QListWidgetItem(m_gradientItemList);
        DLabel *gwi = new DLabel();
        gwi->setWordWrap(true);
        QFont font;
        font.setPixelSize(DDECalendar::FontSizeTwenty);
        gwi->setAlignment(Qt::AlignCenter);
        DPalette dayPalette = gwi->palette();
        QColor textcolor;
        int themtype = CScheduleDataManage::getScheduleDataManage()->getTheme();
        if (themtype == 2) {
            textcolor = "#C0C6D4";
            textcolor.setAlphaF(0.4);
        } else {
            textcolor = "#000000";
            textcolor.setAlphaF(0.3);
        }
        dayPalette.setColor(DPalette::WindowText, textcolor);
        dayPalette.setColor(DPalette::Window, m_lBackgroundcolor);
        gwi->setPalette(dayPalette);
        gwi->setForegroundRole(DPalette::WindowText);
        gwi->setFont(font);
        gwi->move(this->width() - 70, this->height() - 196);
        gwi->setText(tr("No search results"));
        gwi->setFixedSize(m_maxWidth - 20, 450);
        listItem->setSizeHint(QSize(m_maxWidth - 20, 450)); //每次改变Item的高度
        listItem->setFlags(Qt::ItemIsTristate);
        m_gradientItemList->addItem(listItem);
        m_gradientItemList->setItemWidget(listItem, gwi);
        m_labellist.append(gwi);
    }

    if (m_currentItem != nullptr) {
        m_gradientItemList->scrollToItem(m_currentItem, QAbstractItemView::PositionAtTop);
    }

    if (currentDItemIndex >= 0 && m_scheduleSearchItem.size() > 0) {
        //删除日程后,重新设置焦点
        if (currentDItemIndex < m_scheduleSearchItem.size()) {
            m_scheduleSearchItem.at(currentDItemIndex)->setFocus(Qt::TabFocusReason);
            m_selectItem = m_scheduleSearchItem.at(currentDItemIndex);
        } else {
            m_scheduleSearchItem.last()->setFocus(Qt::TabFocusReason);
            m_selectItem = m_scheduleSearchItem.last();
        }
        currentDItemIndex = -1;
    }
}

void CScheduleSearchView::createItemWidget(DSchedule::Ptr info, QDate date, int rtype)
{
    DSchedule::Ptr &gd = info;

    CScheduleSearchItem *gwi = new CScheduleSearchItem(this);
    QFont font;
    font.setPixelSize(DDECalendar::FontSizeFourteen);
    font.setWeight(QFont::Normal);
    gwi->setBackgroundColor(m_bBackgroundcolor);
    gwi->setText(m_btTextColor, font);
    font.setPixelSize(DDECalendar::FontSizeTwelve);

    gwi->setTimeC(m_btimecolor, font);
    gwi->setFixedSize(m_maxWidth - 15, 35);
    gwi->setData(gd, date);
    gwi->setRoundtype(rtype);
    //将搜索到的日程添加到容器
    m_scheduleSearchItem.append(gwi);
    connect(gwi, &CScheduleSearchItem::signalSelectSchedule, this, &CScheduleSearchView::slotSelectSchedule);
    connect(gwi, &CScheduleSearchItem::signalSelectCurrentItem, this, &CScheduleSearchView::slotSelectCurrentItem);

    QListWidgetItem *listItem = new QListWidgetItem;
    listItem->setSizeHint(QSize(m_maxWidth - 15, 36)); //每次改变Item的高度
    listItem->setFlags(Qt::ItemIsTristate);
    m_gradientItemList->addItem(listItem);
    m_gradientItemList->setItemWidget(listItem, gwi);
    m_labellist.append(gwi);
}

QListWidgetItem *CScheduleSearchView::createItemWidget(QDate date)
{
    CScheduleSearchDateItem *gwi = new CScheduleSearchDateItem();
    QFont font;
    font.setWeight(QFont::Medium);
    font.setPixelSize(DDECalendar::FontSizeSixteen);
    gwi->setBackgroundColor(m_lBackgroundcolor);
    gwi->setText(m_lTextColor, font);

    if (date == QDate::currentDate()) {
        gwi->setText(CScheduleDataManage::getScheduleDataManage()->getSystemActiveColor(), font);
    }
    gwi->setFixedSize(m_maxWidth - 15, 35);
    gwi->setDate(date);
    connect(gwi,
            &CScheduleSearchDateItem::signalLabelScheduleHide,
            this,
            &CScheduleSearchView::signalScheduleHide);
    QListWidgetItem *listItem = new QListWidgetItem;
    listItem->setSizeHint(QSize(m_maxWidth - 15, 36)); //每次改变Item的高度
    listItem->setFlags(Qt::ItemIsTristate);
    m_gradientItemList->addItem(listItem);
    m_gradientItemList->setItemWidget(listItem, gwi);
    m_labellist.append(gwi);
    return listItem;
}

/**
 * @brief CScheduleSearchView::slotsetSearch        需要搜索日程关键字
 * @param str
 */
void CScheduleSearchView::slotsetSearch(QString str)
{
    if (str.isEmpty())
        return;
    m_searchStr = str;
    QDateTime date = QDateTime::currentDateTime();
    QDateTime bDate = date.addMonths(-6);

    if (bDate.date() < QDate(1900, 1, 1)) {
        bDate.setDate(QDate(1900, 1, 1));
    }
    QDateTime eDate = date.addMonths(6);
    //查询搜索
    gScheduleManager->searchSchedule(str, bDate, eDate);
}

void CScheduleSearchView::slotScearedScheduleUpdate()
{
    m_vlistData = gScheduleManager->getAllSearchedScheduleMap();
    updateDateShow();
}

void CScheduleSearchView::slotSelectSchedule(const DSchedule::Ptr &scheduleInfo)
{
    emit signalSelectSchedule(scheduleInfo);
}

/**
 * @brief CScheduleSearchView::updateSearch     更新搜索信息
 */
void CScheduleSearchView::updateSearch()
{
    if (isVisible()) {
        slotsetSearch(m_searchStr);
    }
}

/**
 * @brief CScheduleSearchView::slotSelectCurrentItem 设置选中的item为嘴上面一个,实现滚动效果
 * @param item 选中的item
 * @param itemFocusOut 是否是focusout事件
 */
void CScheduleSearchView::slotSelectCurrentItem(CScheduleSearchItem *item, bool itemFocusOut)
{
    for (int i = 0; i < m_gradientItemList->count(); i++) {
        QListWidgetItem *cItem = m_gradientItemList->item(i);
        if (item == m_gradientItemList->itemWidget(cItem)) {
            m_selectItem = item;
            //设置选中的item为最上面一个
            m_gradientItemList->scrollToItem(cItem, QAbstractItemView::PositionAtTop);
            if (i == m_gradientItemList->count() - 1 && itemFocusOut && !keyPressUP) {
                //最后一个item,发送信号将焦点传递给搜索框
                emit signalSelectCurrentItem();
            }
        }
    }
    keyPressUP = false;
}

/**
 * @brief CScheduleSearchView::slotListWidgetClicked
 * 根据listwidget点击事件对当前tab焦点item做对应的处理
 */
void CScheduleSearchView::slotListWidgetClicked()
{
    //取消当前tab选中item
    m_selectItem = nullptr;
    emit signalScheduleHide();
}

void CScheduleSearchView::resizeEvent(QResizeEvent *event)
{
    for (int i = 0; i < m_gradientItemList->count(); i++) {
        QListWidgetItem *item11 = m_gradientItemList->item(i);
        item11->setSizeHint(QSize(m_maxWidth - 5, 36)); //每次改变Item的高度
    }
    for (int i = 0; i < m_labellist.count(); i++) {
        m_labellist.at(i)->setFixedSize(m_maxWidth - 20, 35);
        m_labellist.at(i)->update();
    }
    if (m_gradientItemList->count() == 1) {
        QListWidgetItem *item11 = m_gradientItemList->item(0);
        item11->setSizeHint(QSize(m_maxWidth - 20, qRound(height() * 0.7978))); //每次改变Item的高度
        m_labellist.at(0)->setFixedSize(m_maxWidth - 20, qRound(height() * 0.7978));
        m_labellist.at(0)->update();
    }
    if (m_gradientItemList->count() > 1) {
        if (m_currentItem != nullptr) {
            m_gradientItemList->scrollToItem(m_currentItem, QAbstractItemView::PositionAtTop);
        }
    }
    DWidget::resizeEvent(event);
}

void CScheduleSearchView::mousePressEvent(QMouseEvent *event)
{
    DWidget::mousePressEvent(event);
}

void CScheduleSearchView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up) {
        //上键,选中上一个item
        keyPressUP = true;
        int selectNum = m_scheduleSearchItem.indexOf(m_selectItem);
        if (selectNum > 0)
            m_scheduleSearchItem.at(selectNum - 1)->setFocus(Qt::TabFocusReason);
    }
    if (event->key() == Qt::Key_Down) {
        //下键,选中下一个item
        int selectNum = m_scheduleSearchItem.indexOf(m_selectItem);
        if (selectNum < m_scheduleSearchItem.count() - 1)
            m_scheduleSearchItem.at(selectNum + 1)->setFocus(Qt::TabFocusReason);
    }
    DWidget::keyPressEvent(event);
}

void CScheduleSearchView::focusInEvent(QFocusEvent *e)
{
    DWidget::focusInEvent(e);
    if (e->reason() == Qt::TabFocusReason) {
        if (m_scheduleSearchItem.size() > 0) {
            m_scheduleSearchItem.first()->setFocus(Qt::TabFocusReason);
        }
    }
}

CScheduleSearchDateItem::CScheduleSearchDateItem(QWidget *parent)
    : DLabel(parent)
{
    //设置对象名称和辅助显示名称
    this->setObjectName("CScheduleDateItem");
    this->setAccessibleName("CScheduleDateItem");
    setAutoFillBackground(true);
}

void CScheduleSearchDateItem::setBackgroundColor(QColor color1)
{
    m_Backgroundcolor = color1;
}

void CScheduleSearchDateItem::setText(QColor tColor, QFont font)
{
    m_textcolor = tColor;
    m_font = font;
}

void CScheduleSearchDateItem::setDate(QDate date)
{
    m_date = date;
}

void CScheduleSearchDateItem::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    int labelwidth = width();
    int labelheight = height();

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setBrush(QBrush(m_Backgroundcolor));
    painter.setPen(Qt::NoPen);
    painter.drawRect(this->rect());

    painter.setFont(m_font);
    painter.setPen(m_textcolor);

    QLocale locale;
    QString datestr;

    if (locale.language() == QLocale::Chinese) {
        datestr = QString::number(m_date.year()) + tr("Y") + QString::number(m_date.month()) + tr("M") + QString::number(m_date.day()) + tr("D");
    } else {
        datestr = m_date.toString("yyyy/M/d");
    }
    painter.drawText(QRect(12, 8, labelwidth, labelheight - 8), Qt::AlignLeft, datestr);
    painter.end();
}

void CScheduleSearchDateItem::mousePressEvent(QMouseEvent *event)
{
    emit signalLabelScheduleHide();
    DLabel::mousePressEvent(event);
}

CScheduleListWidget::CScheduleListWidget(QWidget *parent)
    : DListWidget(parent)
{
    //设置对象名称和辅助显示名称
    this->setObjectName("CScheduleListWidget");
    this->setAccessibleName("CScheduleListWidget");
    //接受触摸事件
    setAttribute(Qt::WA_AcceptTouchEvents);
    //截获相应的gesture手势
    grabGesture(Qt::TapGesture);
    grabGesture(Qt::TapAndHoldGesture);
    grabGesture(Qt::PanGesture);

    //日程类型发生改变，刷新界面
    connect(gAccountManager, &AccountManager::signalScheduleTypeUpdate, [&]() {
        this->viewport()->update();
    });
}

CScheduleListWidget::~CScheduleListWidget()
{
}

void CScheduleListWidget::mousePressEvent(QMouseEvent *event)
{
    DListWidget::mousePressEvent(event);
    emit signalListWidgetClicked();
}

void CScheduleListWidget::paintEvent(QPaintEvent *e)
{
    DListWidget::paintEvent(e);
    QPainter painter(this->viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    //绘制背景色
    DPalette _backgroundP;
    QColor _backgroundColor = _backgroundP.color(QPalette::Active, QPalette::Base);
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(_backgroundColor);
    painter.drawRect(this->rect());
    painter.restore();
    //绘制搜索右侧圆角效果
    QColor _outBorderColor = _backgroundP.color(QPalette::Active, QPalette::Window);
    const qreal _radios = 8;
    painter.setPen(Qt::NoPen);
    painter.setBrush(_outBorderColor);

//    const qreal _outWidth = 10;
    //不需要outwidth
    const qreal _outWidth = 0;
    const qreal _rectX = this->width() - _outWidth;
    QPainterPath _showPath;
    _showPath.moveTo(_rectX - _radios, 0);
    _showPath.lineTo(this->width(), 0);
    _showPath.lineTo(this->width(), this->height());
    _showPath.lineTo(_rectX - _radios, this->height());
    _showPath.arcTo(_rectX - _radios * 2, this->height() - _radios * 2, _radios * 2, _radios * 2, 270, 90);
    _showPath.lineTo(_rectX, _radios);
    _showPath.arcTo(_rectX - _radios * 2, 0, _radios * 2, _radios * 2, 0, 90);
    painter.drawPath(_showPath);
    painter.end();
}
