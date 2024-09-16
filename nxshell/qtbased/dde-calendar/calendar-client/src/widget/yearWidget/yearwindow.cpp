// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "yearwindow.h"
#include "yearview.h"
#include "constants.h"
#include "schedulesearchview.h"
#include "yearscheduleview.h"
#include "calendarglobalenv.h"

#include <DPalette>
#include <DHiDPIHelper>

#include <QMessageBox>
#include <QMenuBar>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>

DGUI_USE_NAMESPACE
CYearWindow::CYearWindow(QWidget *parent)
    : CScheduleBaseWidget(parent)
{
    initUI();
    initConnection();
    setContentsMargins(0, 0, 0, 0);
    //设置接受触摸事件
    this->setAttribute(Qt::WA_AcceptTouchEvents);
    //截获相应的gesture手势
    grabGesture(Qt::TapGesture);
    grabGesture(Qt::TapAndHoldGesture);
    grabGesture(Qt::PanGesture);
    //设置年份显示
    setYearData();
}

CYearWindow::~CYearWindow()
{

}

/**
 * @brief eventFilter 过滤器，过滤返回今天的按钮事件
 * @param watched 事件对象
 * @param event 事件类型
 * @return false
 */
bool CYearWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_today) {
        if (event->type() == QEvent::MouseButtonPress) {
            slottoday();
        }
        if (event->type() == QEvent::KeyPress) {
            //点击 回车键 返回今天
            QKeyEvent *key = dynamic_cast<QKeyEvent *>(event);
            if (key->key() == Qt::Key_Return) {
                //返回今天
                slottoday();
            }
        }
    } else if (watched == m_yearWidget) {
        //上下键切换年份
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *key = dynamic_cast<QKeyEvent *>(event);
            if (key->key() == Qt::Key_Up) {
                //上一年
                slotprev();
            } else if (key->key() == Qt::Key_Down) {
                //下一年
                slotnext();
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

/**
 * @brief mousePressEvent 鼠标单击事件，单击日期区域外，隐藏日程浮框
 * @param event 鼠标事件
 */
void CYearWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        //如果为触摸点击则记录开始坐标
        m_touchBeginPoint = event->pos();
    }
    QWidget::mousePressEvent(event);
}

/**
 * @brief resizeEvent 窗口大小调整事件，搜索时，调整边框大小
 * @param event 窗口大小调整事件
 */
void CYearWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    m_topWidget->setGeometry(0, 0, this->width(), DDEMonthCalendar::M_YTopHeight);
}

void CYearWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void CYearWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        //如果为触摸移动状态
        if (m_touchState == 2) {
            //获取停止位置
            QPointF stopPoint = event->pos();
            //计算出移动状态
            TouchGestureData touchGData = calculateAzimuthAngle(m_touchBeginPoint, stopPoint);
            //如果方向为上下则切换年份
            switch (touchGData.movingDirection) {
            case TouchGestureData::T_TOP: {
                slotnext();
                break;
            }
            case TouchGestureData::T_BOTTOM: {
                slotprev();
                break;
            }
            default:
                break;
            }
        }
        m_touchState = 0;
    }
}

bool CYearWindow::event(QEvent *e)
{
    if (e->type() == QEvent::Gesture)
        return gestureEvent(dynamic_cast<QGestureEvent *>(e));
    return QWidget::event(e);
}

/**
 * @brief gestureEvent      触摸手势处理
 * @param event             手势事件
 * @return
 */
bool CYearWindow::gestureEvent(QGestureEvent *event)
{
    if (QGesture *tap = event->gesture(Qt::TapGesture))
        tapGestureTriggered(dynamic_cast<QTapGesture *>(tap));
    if (QGesture *pan = event->gesture(Qt::PanGesture))
        panTriggered(dynamic_cast<QPanGesture *>(pan));
    return true;
}

/**
 * @brief tapGestureTriggered       轻切手势处理
 * @param tap                       轻切手势事件
 */
void CYearWindow::tapGestureTriggered(QTapGesture *tap)
{
    switch (tap->state()) {
    case Qt::NoGesture: {
        break;
    }
    case Qt::GestureStarted: {
        m_touchState = 1;
        break;
    }
    case Qt::GestureUpdated: {
        m_touchState = 2;
        break;
    }
    case Qt::GestureFinished: {
        break;
    }
    default: {
        //GestureCanceled
    }
    }
}

/**
 * @brief panTriggered      多指滑动手势处理
 * @param pan               多指滑动手势
 */
void CYearWindow::panTriggered(QPanGesture *pan)
{
    switch (pan->state()) {
    case Qt::GestureFinished: {
        QPointF zeroPoint(0, 0);
        QPointF offset = pan->offset();
        TouchGestureData touchGData = calculateAzimuthAngle(zeroPoint, offset);
        switch (touchGData.movingDirection) {
        case TouchGestureData::T_TOP: {
            slotnext();
            break;
        }
        case TouchGestureData::T_BOTTOM: {
            slotprev();
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        //GestureCanceled
        break;
    }
}

/**
 * @brief calculateAzimuthAngle     计算方位角
 * @param startPoint            起始坐标
 * @param stopPoint            结束坐标
 * @return      触摸手势数据
 */
TouchGestureData CYearWindow::calculateAzimuthAngle(QPointF &startPoint,  QPointF &stopPoint)
{
    TouchGestureData _result{};
    qreal angle = 0.0000;
    qreal   dx = stopPoint.rx() - startPoint.rx();
    qreal   dy = stopPoint.ry() - startPoint.ry();
    //计算方位角
    angle = qAtan2(dy, dx) * 180 / M_PI;
    qreal line = qSqrt(dx * dx + dy * dy);
    //如果移动距离大于10则有效
    if (line > 10) {
        if ((angle <= -45) && (angle >= -135)) {
            _result.movingDirection = TouchGestureData::T_TOP;
        } else if ((angle > -45) && (angle < 45)) {
            _result.movingDirection = TouchGestureData::T_RIGHT;
        } else if ((angle >= 45) && (angle <= 135)) {
            _result.movingDirection = TouchGestureData::T_BOTTOM;
        } else {
            _result.movingDirection = TouchGestureData::T_LEFT;
        }
    }
    _result.angle = angle;
    _result.length = line;
    return  _result;
}

/**
 * @brief initUI 初始化年视图的界面显示
 */
void CYearWindow::initUI()
{
    m_today = new LabelWidget(this);
    m_today->setObjectName("yearToDay");
    m_today->setAccessibleName("yearToDay");
    m_today->installEventFilter(this);

    QFont todayfont;
    todayfont.setPixelSize(DDECalendar::FontSizeSixteen);
    m_today->setFont(todayfont);
    m_today->setAlignment(Qt::AlignCenter);
    m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    m_today->setFixedWidth(88);
    m_today->setAutoFillBackground(true);
    m_today->setFixedHeight(DDEYearCalendar::Y_MLabelHeight - 4);
    m_prevButton = new DIconButton(DStyle::SP_ArrowLeft, this);
    //设置对象名称和辅助显示名称
    m_prevButton->setObjectName("PrevButton");
    m_prevButton->setAccessibleName("PrevButton");
    m_prevButton->setFixedWidth(DDEYearCalendar::Y_MLabelHeight);
    m_prevButton->setFixedHeight(DDEYearCalendar::Y_MLabelHeight);

    m_nextButton = new DIconButton(DStyle::SP_ArrowRight, this);
    //设置对象名称和辅助显示名称
    m_nextButton->setObjectName("NextButton");
    m_nextButton->setAccessibleName("NextButton");
    m_nextButton->setFixedWidth(DDEYearCalendar::Y_MLabelHeight);
    m_nextButton->setFixedHeight(DDEYearCalendar::Y_MLabelHeight);

    m_yearLabel = new QLabel(this);
    m_yearLabel->setFixedHeight(DDEYearCalendar::Y_YLabelHeight);

    QFont t_labelF;
    t_labelF.setWeight(QFont::Medium);
    t_labelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_yearLabel->setFont(t_labelF);
    DPalette pa = m_yearLabel->palette();
    pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    m_yearLabel->setPalette(pa);

    m_yearLunarLabel = new QLabel(this);
    m_yearLunarLabel->setFixedSize(DDEMonthCalendar::M_YLunaLabelWindth, DDEMonthCalendar::M_YLunaLabelHeight);

    QFont yLabelF;
    yLabelF.setWeight(QFont::Medium);
    yLabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_yearLunarLabel->setFont(yLabelF);
    DPalette LunaPa = m_yearLunarLabel->palette();
    LunaPa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
    m_yearLunarLabel->setPalette(LunaPa);

    m_yearLunarDayLabel = new QLabel(this);
    m_yearLunarDayLabel->setFixedSize(108, DDEMonthCalendar::M_YLunaLabelHeight);
    m_yearLunarDayLabel->setFont(yLabelF);
    m_yearLunarDayLabel->setPalette(LunaPa);
    m_yearLunarDayLabel->setAlignment(Qt::AlignRight);

    QHBoxLayout *yeartitleLayout = new QHBoxLayout;
    yeartitleLayout->setMargin(0);
    yeartitleLayout->setSpacing(0);
    yeartitleLayout->addSpacing(10);
    yeartitleLayout->addWidget(m_yearLabel);
    yeartitleLayout->addWidget(m_dialogIconButton);

    QHBoxLayout *yeartitleLayout1 = new QHBoxLayout;
    yeartitleLayout1->setMargin(0);
    yeartitleLayout1->setSpacing(0);
    yeartitleLayout1->addWidget(m_yearLunarLabel);
    yeartitleLayout1->addStretch();
    yeartitleLayout1->addWidget(m_yearLunarDayLabel, 0, Qt::AlignVCenter);

    m_todayFrame = new CustomFrame(this);
    m_todayFrame->setContentsMargins(0, 0, 0, 0);
    m_todayFrame->setRoundState(true, true, true, true);
    m_todayFrame->setBColor(Qt::white);
    m_todayFrame->setFixedHeight(DDEYearCalendar::Y_MLabelHeight);
    m_todayFrame->setboreder(1);
    QHBoxLayout *todaylayout = new QHBoxLayout;
    todaylayout->setMargin(0);
    todaylayout->setSpacing(0);
    //设置tab选中顺序
    setTabOrder(m_prevButton, m_today);
    setTabOrder(m_today, m_nextButton);
    todaylayout->addWidget(m_prevButton);
    todaylayout->addWidget(m_today, 0, Qt::AlignCenter);
    todaylayout->addWidget(m_nextButton);
    m_todayFrame->setLayout(todaylayout);
    yeartitleLayout1->addSpacing(10);
    yeartitleLayout1->addWidget(m_todayFrame);
    yeartitleLayout->addSpacing(6);
    yeartitleLayout->addLayout(yeartitleLayout1);

    m_firstYearWidget = new YearFrame();
    m_secondYearWidget = new YearFrame();
    //设置过滤事件
    m_firstYearWidget->installEventFilter(this);
    m_secondYearWidget->installEventFilter(this);

    m_StackedWidget = new  AnimationStackedWidget(AnimationStackedWidget::TB);
    m_StackedWidget->addWidget(m_firstYearWidget);
    m_StackedWidget->addWidget(m_secondYearWidget);
    m_StackedWidget->setContentsMargins(0, 0, 0, 0);
    m_StackedWidget->setDuration(600);
    m_StackedWidget->setFrameShape(QFrame::NoFrame);
    m_StackedWidget->setFrameShadow(QFrame::Plain);

    m_yearWidget = qobject_cast<YearFrame *>(m_StackedWidget->widget(0));
    QVBoxLayout *hhLayout = new QVBoxLayout;
    hhLayout->setMargin(0);
    hhLayout->setSpacing(0);
    hhLayout->setContentsMargins(0, 0, 0, 0);
    hhLayout->addWidget(m_StackedWidget);

    m_tMainLayout = new QVBoxLayout;
    m_tMainLayout->setMargin(0);
    m_tMainLayout->setSpacing(0);
    m_tMainLayout->setContentsMargins(0, 0, 0, 0);
    m_tMainLayout->addLayout(hhLayout);
    this->setLayout(m_tMainLayout);

    m_topWidget = new DWidget(this);
    m_topWidget->setLayout(yeartitleLayout);

    m_scheduleView = new CYearScheduleOutView(this);
}

/**
 * @brief initConnection 初始化信号和槽的连接
 */
void CYearWindow::initConnection()
{
    connect(m_prevButton, &DIconButton::clicked, this, &CYearWindow::slotprev);
    connect(m_nextButton, &DIconButton::clicked, this, &CYearWindow::slotnext);
    connect(m_StackedWidget, &AnimationStackedWidget::signalIsFinished, this, &CYearWindow::setYearData);
    connect(m_firstYearWidget, &YearFrame::signalMousePress, this, &CYearWindow::slotMousePress);
    connect(m_secondYearWidget, &YearFrame::signalMousePress, this, &CYearWindow::slotMousePress);
    connect(m_scheduleView, &CYearScheduleOutView::signalsViewSelectDate, this, &CYearWindow::slotMousePress);
}

/**
 * @brief setTheMe 设置系统主题
 * @param type 主题类型
 */
void CYearWindow::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        DPalette todayPa = m_today->palette();
        todayPa.setColor(DPalette::WindowText, QColor("#000000"));
        todayPa.setColor(DPalette::Background, Qt::white);
        m_today->setPalette(todayPa);
        m_today->setForegroundRole(DPalette::WindowText);
        m_today->setBackgroundRole(DPalette::Background);

        m_todayFrame->setBColor(Qt::white);

        DPalette pa = m_yearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_yearLabel->setPalette(pa);
        m_yearLabel->setForegroundRole(DPalette::WindowText);

        DPalette LunaPa = m_yearLunarLabel->palette();
        LunaPa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_yearLunarLabel->setPalette(LunaPa);
        m_yearLunarLabel->setForegroundRole(DPalette::WindowText);

        m_yearLunarDayLabel->setPalette(LunaPa);
        m_yearLunarDayLabel->setForegroundRole(DPalette::WindowText);
    } else if (type == 2) {
        DPalette todayPa = m_today->palette();
        todayPa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        QColor tbColor = "#414141";
        tbColor.setAlphaF(0.0);
        todayPa.setColor(DPalette::Background, tbColor);
        m_today->setPalette(todayPa);
        m_today->setForegroundRole(DPalette::WindowText);
        m_today->setBackgroundRole(DPalette::Background);
        QColor tbColor2 = "#414141";
        tbColor2.setAlphaF(0.3);
        m_todayFrame->setBColor(tbColor2);
        DPalette pa = m_yearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_yearLabel->setPalette(pa);
        m_yearLabel->setForegroundRole(DPalette::WindowText);
        DPalette LunaPa = m_yearLunarLabel->palette();
        LunaPa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_yearLunarLabel->setPalette(LunaPa);
        m_yearLunarLabel->setForegroundRole(DPalette::WindowText);
        m_yearLunarDayLabel->setPalette(LunaPa);
        m_yearLunarDayLabel->setForegroundRole(DPalette::WindowText);
    }
    m_firstYearWidget->setTheMe(type);
    m_secondYearWidget->setTheMe(type);
    //设置提示框主题类型
    m_scheduleView->setTheMe(type);

    DPalette palette = m_topWidget->palette();
    palette.setBrush(DPalette::WindowText, palette.color(DPalette::Window));
    m_topWidget->setAutoFillBackground(true);
    m_topWidget->setPalette(palette);
}

/**
 * @brief CYearWindow::setSearchWFlag                   设置是否在进行搜索
 * @param flag
 */
void CYearWindow::setSearchWFlag(bool flag)
{
    m_searchFlag = flag;
}

/**
 * @brief CYearWindow::updateShowDate                   更新显示时间
 */
void CYearWindow::updateShowDate(const bool isUpdateBar)
{
    Q_UNUSED(isUpdateBar);
    m_scheduleView->setTimeFormat((m_calendarManager->getTimeShowType()?"AP ":"") + m_calendarManager->getTimeFormat());
    m_yearWidget->setShowDate(getSelectDate());
}

/**
 * @brief CYearWindow::updateShowScheduleData           更新显示日程数据
 */
void CYearWindow::updateShowSchedule()
{
    //获取显示日期中是否包含日程信息标志
    m_yearWidget->setDateHasScheduleSign(ScheduleManager::getInstace()->getAllScheduleDate());

}

/**
 * @brief CYearWindow::updateShowLunar                  更新显示农历信息
 */
void CYearWindow::updateShowLunar()
{
    //获取农历信息
    getLunarInfo();
    m_yearWidget->setLunarYearDate(m_lunarYear);
    //如果正在切换则退出
    if (m_StackedWidget->IsRunning())
        return;
    setLunarShow();
}

/**
 * @brief CYearWindow::updateSearchScheduleInfo         更新搜索日程信息
 */
void CYearWindow::updateSearchScheduleInfo()
{
    //获取搜索日程信息
    m_yearWidget->setSearchSchedule(gScheduleManager->getAllSearchedScheduleDate());
}

/**
 * @brief CYearWindow::setSelectSearchScheduleInfo      设置选中搜索日程
 * @param info
 */
void CYearWindow::setSelectSearchScheduleInfo(const DSchedule::Ptr &info)
{
    Q_UNUSED(info);
}

/**
 * @brief CYearWindow::slotSetScheduleHide              隐藏日程提示框
 */
void CYearWindow::slotSetScheduleHide()
{
    m_scheduleView->hide();
}

/**
 * @brief CYearWindow::slotprev     切换上一年
 */
void CYearWindow::slotprev()
{
    QDate minYear = getSelectDate();
    if (minYear.year() > 1900)
    {
        switchYear(-1);
    }
}

/**
 * @brief CYearWindow::slotnext     切换下一年
 */
void CYearWindow::slotnext()
{
    QDate maxYear = getSelectDate();
    if (maxYear.year() < 2100)
    {
        switchYear(1);
    }
}

/**
 * @brief CYearWindow::slottoday    返回当前时间
 */
void CYearWindow::slottoday()
{
    //隐藏提示
    slotSetScheduleHide();
    //设置选择时间为当前时间，切换年份信息
    setSelectDate(getCurrendDateTime().date(), true);
    //更新数据
    updateData();
    setYearData();
}

/**
 * @brief CYearWindow::switchYear   根据偏移值切换年份
 * @param offsetYear                偏移值
 */
void CYearWindow::switchYear(const int offsetYear)
{
    slotSetScheduleHide();
    //获取选择时间
    QDate _selectData = getSelectDate();
    //如果正在切换则退出
    if (m_StackedWidget->IsRunning())
        return;
    _selectData = _selectData.addYears(offsetYear);
    //获取当前显示控件的位置
    int index = m_StackedWidget->currentIndex();
    //当前选中的monthview的index
    int currentYearViewIndex = qobject_cast<YearFrame *>(m_StackedWidget->widget(index))->getViewFocusIndex();
    index = qAbs(index + offsetYear) % 2;
    m_yearWidget = qobject_cast<YearFrame *>(m_StackedWidget->widget(index));
    //设置年视图翻页后选中的monthview
    m_yearWidget->setViewFocus(currentYearViewIndex);
    //设置显示时间
    m_yearWidget->setShowDate(_selectData);
    updateData();
    if (offsetYear > 0) {
        //下一年
        m_StackedWidget->setNext();
    } else {
        //上一年
        m_StackedWidget->setPre();
    }
    //切换试图开始后再进行时间切换
    setSelectDate(_selectData);
}

/**
 * @brief CYearWindow::setLunarShow     显示农历信息
 */
void CYearWindow::setLunarShow()
{
    m_yearLunarLabel->setText(m_lunarYear);
    m_yearLunarDayLabel->setText(m_lunarDay);
}

/**
 * @brief CYearWindow::setYearData  设置选择时间年
 */
void CYearWindow::setYearData()
{
    //如果选择日期为本地时间日期则显示今天，否则显示返回当天
    if (getSelectDate() == getCurrendDateTime().date()) {
        m_today->setText(QCoreApplication::translate("today", "Today", "Today"));
    } else {
        m_today->setText(QCoreApplication::translate("Return", "Today", "Return"));
    }
    //如果是中文环境
    if (getShowLunar()) {
        m_yearLabel->setText(QString::number(getSelectDate().year()) + tr("Y"));
        //获取农历信息
        //获取农历信息
        getLunarInfo();
        //显示农历信息
        setLunarShow();
    } else {
        m_yearLabel->setText(QString::number(getSelectDate().year()));
    }
}

/**
 * @brief CYearWindow::slotMousePress       接收点击日期事件
 * @param selectDate                        选择的日期
 * @param pressType         鼠标点击类型 0:单击具体日期  1:双击具体日期  2:双击月份   3:提示框跳转周视图
 */
void CYearWindow::slotMousePress(const QDate &selectDate, const int pressType)
{
    slotSetScheduleHide();
    if (!selectDate.isValid())
        return;
    //设置选择时间
    setSelectDate(selectDate, this);
    setYearData();
    switch (pressType) {
    case 0: {
        // 0:单击
        DSchedule::List _scheduleInfo {};
        //获取选择日期的日程信息
        _scheduleInfo = gScheduleManager->getScheduleByDay(selectDate);

        m_scheduleView->setCurrentDate(selectDate);
        m_scheduleView->setData(_scheduleInfo);
        //使用设置的显示坐标
        QVariant variant;
        CalendarGlobalEnv::getGlobalEnv()->getValueByKey(DDECalendar::CursorPointKey, variant);
        QPoint pos22 = variant.value<QPoint>();
        // 因为将提示框从window改为widget，要转换为相对窗口的坐标
        auto rPos = this->mapFromGlobal(pos22);
        // 根据鼠标位置，决定悬浮框显示位置
        if (rPos.x() < this->width() / 2) {
            // 显示到右侧
            m_scheduleView->setDirection(DArrowRectangle::ArrowLeft);
            m_scheduleView->show(rPos.x()+10, rPos.y());
        } else {
            // 显示到左侧
            m_scheduleView->setDirection(DArrowRectangle::ArrowRight);
            m_scheduleView->show(rPos.x()-10, rPos.y());
        }
        update();
        break;
    }
    case 1: {
        // 1:双击时间
        signalSwitchView();
        break;
    }
    case 2: {
        // 2: 双击月
        signalSwitchView(1);
        break;
    }
    case 3: {
        // 3: 提示框跳转周视图
        signalSwitchView(2);
        break;
    }
    default:
        break;
    }
}

/**
 * @brief wheelEvent 通过鼠标中间的滚轮滚动切换年份，并刷新年视图下的所有内容。
 * @param event 鼠标滚轮事件
 */
void CYearWindow::wheelEvent(QWheelEvent *event)
{
    //如果为左右方向则退出
    if (event->orientation() == Qt::Orientation::Horizontal)
        return;
    if (event->delta() < 0) {
        slotnext();
    } else {
        slotprev();
    }
}

YearFrame::YearFrame(DWidget *parent)
    : QWidget(parent)
{
    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->setMargin(0);
    gridLayout->setSpacing(8);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            CYearView *view = new CYearView(this);
            //为每个monthview设置事件过滤器
            view->installEventFilter(this);
            gridLayout->addWidget(view, i, j);
            m_monthViewList.append(view);
            connect(view, &CYearView::signalMousePress, this, &YearFrame::signalMousePress);
        }
    }

    m_YearLabel = new QLabel(this);
    m_YearLabel->setFixedHeight(DDEYearCalendar::Y_YLabelHeight);

    QFont t_labelF;
    t_labelF.setWeight(QFont::Medium);
    t_labelF.setPixelSize(DDECalendar::FontSizeTwentyfour);
    m_YearLabel->setFont(t_labelF);
    DPalette pa = m_YearLabel->palette();
    pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
    m_YearLabel->setPalette(pa);

    m_YearLunarLabel = new QLabel();
    m_YearLunarLabel->setFixedSize(DDEMonthCalendar::M_YLunaLabelWindth, DDEMonthCalendar::M_YLunaLabelHeight);

    QFont yLabelF;
    yLabelF.setWeight(QFont::Medium);
    yLabelF.setPixelSize(DDECalendar::FontSizeFourteen);
    m_YearLunarLabel->setFont(yLabelF);
    DPalette LunaPa = m_YearLunarLabel->palette();
    LunaPa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
    m_YearLunarLabel->setPalette(LunaPa);

    QHBoxLayout *yeartitleLayout = new QHBoxLayout;
    yeartitleLayout->setMargin(0);
    yeartitleLayout->setSpacing(0);
    yeartitleLayout->setContentsMargins(11, 12, 8, 10);
    yeartitleLayout->addWidget(m_YearLabel);

    QHBoxLayout *yeartitleLayout1 = new QHBoxLayout;
    yeartitleLayout1->setMargin(0);
    yeartitleLayout1->setSpacing(0);
    yeartitleLayout1->setContentsMargins(4, 9, 0, 7);
    yeartitleLayout1->addWidget(m_YearLunarLabel);
    yeartitleLayout1->addSpacing(390);
    yeartitleLayout1->addStretch();
    yeartitleLayout1->addSpacing(10);
    yeartitleLayout->addLayout(yeartitleLayout1);

    m_topWidget = new DWidget();
    m_topWidget->setLayout(yeartitleLayout);
    m_topWidget->setFixedHeight(DDEMonthCalendar::M_YTopHeight);
    QVBoxLayout *hhLayout = new QVBoxLayout;
    hhLayout->setMargin(0);
    hhLayout->setSpacing(0);
    hhLayout->addWidget(m_topWidget);
    hhLayout->addLayout(gridLayout);
    this->setLayout(hhLayout);
}

YearFrame::~YearFrame()
{

}

/**
 * @brief YearFrame::setShowDate        设置显示时间
 * @param selectDate                    选择的时间
 * @param showDate                      需要显示一年的时间
 */
void YearFrame::setShowDate(const QDate &selectDate)
{
    QDate _showMonth(selectDate.year(), 1, 1);
    for (int i = 0; i < DDEYearCalendar::FrameSizeOfEveryYear; i++) {
        QDate _setShowMonth = _showMonth.addMonths(i);
        m_monthViewList.at(i)->setShowMonthDate(_setShowMonth);
    }
    m_selectDate = selectDate;
    //更新显示界面
    update();
    //设置年份显示
    setYearShow();
}

/**
 * @brief YearFrame::setLunarYearDate   设置阴历年显示
 * @param lunar                         显示数据
 */
void YearFrame::setLunarYearDate(const QString &lunar)
{
    m_YearLunarLabel->setText(lunar);
}

/**
 * @brief YearFrame::setDateHasScheduleSign     设置日期是否存在日程
 * @param hasSchedule
 */
void YearFrame::setDateHasScheduleSign(const QSet<QDate> &hasSchedule)
{
    QDate _startDate;
    QDate _stopDate;
    QDate _getDate;
    qint64 _offset = 0;
    QSet<QDate> _hasScheduleSet{};
    for (int i = 0; i < m_monthViewList.size(); ++i) {
        //如果时间有效
        if (m_monthViewList.at(i)->getStartAndStopDate(_startDate, _stopDate)) {
            _offset = _startDate.daysTo(_stopDate) + 1;
            _hasScheduleSet.clear();
            for (int j = 0 ; j < _offset; ++j) {
                _getDate = _startDate.addDays(j);
                if (hasSchedule.contains(_getDate)) {
                    _hasScheduleSet.insert(_getDate);
                }
            }
            m_monthViewList.at(i)->setHasScheduleSet(_hasScheduleSet);
        }
    }
}

/**
 * @brief YearFrame::setTheMe       设置不同主题颜色
 * @param type
 */
void YearFrame::setTheMe(int type)
{
    if (type == 0 || type == 1) {
        DPalette gpa = palette();
        gpa.setColor(DPalette::Background, "#F8F8F8");
        setPalette(gpa);
        setBackgroundRole(DPalette::Background);

        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#3B3B3B"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);

        DPalette LunaPa = m_YearLunarLabel->palette();
        LunaPa.setColor(DPalette::WindowText, QColor("#8A8A8A"));
        m_YearLunarLabel->setPalette(LunaPa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);
    } else if (type == 2) {
        DPalette gpa = palette();
        gpa.setColor(DPalette::Background, "#252525");
        setPalette(gpa);
        setBackgroundRole(DPalette::Background);

        DPalette pa = m_YearLabel->palette();
        pa.setColor(DPalette::WindowText, QColor("#C0C6D4"));
        m_YearLabel->setPalette(pa);
        m_YearLabel->setForegroundRole(DPalette::WindowText);
        DPalette LunaPa = m_YearLunarLabel->palette();
        LunaPa.setColor(DPalette::WindowText, QColor("#798BA8"));
        m_YearLunarLabel->setPalette(LunaPa);
        m_YearLunarLabel->setForegroundRole(DPalette::WindowText);
    }

    for (int i = 0; i < DDEYearCalendar::FrameSizeOfEveryYear; i++) {
        m_monthViewList.at(i)->setTheMe(type);
    }
}

/**
 * @brief YearFrame::setSearchSchedule      设置搜索日程
 * @param searchInfo
 */
void YearFrame::setSearchSchedule(const QSet<QDate> &hasSchedule)
{
    QDate _startDate;
    QDate _stopDate;
    QDate _getDate;
    qint64 _offset = 0;
    QSet<QDate> _hasSearchScheduleSet{};
    for (int i = 0; i < m_monthViewList.size(); ++i) {
        //如果时间有效
        if (m_monthViewList.at(i)->getStartAndStopDate(_startDate, _stopDate)) {
            _offset = _startDate.daysTo(_stopDate) + 1;
            _hasSearchScheduleSet.clear();
            for (int j = 0 ; j < _offset; ++j) {
                _getDate = _startDate.addDays(j);
                if (hasSchedule.contains(_getDate)) {
                    _hasSearchScheduleSet.insert(_getDate);
                }
            }
            m_monthViewList.at(i)->setHasSearchScheduleSet(_hasSearchScheduleSet);
        }
    }


}

/**
 * @brief YearFrame::setViewFocus 为选中的view设置焦点
 * @param index 选中的哪一个view
 */
void YearFrame::setViewFocus(int index)
{
    if (index >= 0 && index < m_monthViewList.size()) {
        //设置选中view的焦点类型
        m_monthViewList.at(index)->setFocus(Qt::FocusReason::TabFocusReason);
    }
}

/**
 * @brief YearFrame::getViewFocusIndex 获取选中view的index
 * @return 选中的哪一个view
 */
int YearFrame::getViewFocusIndex()
{
    return currentFocusView;
}

/**
 * @brief YearFrame::setYearShow    设置年信息显示
 */
void YearFrame::setYearShow()
{
    if (QLocale::system().language() == QLocale::Chinese) {
        m_YearLabel->setText(QString::number(m_selectDate.year()) + tr("Y"));
        m_YearLunarLabel->setText(m_LunarYear);
    } else {
        m_YearLabel->setText(QString::number(m_selectDate.year()));
        m_YearLunarLabel->setText("");
    }
}

/**
 * @brief YearFrame::eventFilter 事件过滤器，获取选中monthview的index
 */
bool YearFrame::eventFilter(QObject *watched, QEvent *event)
{
    //返回当前活动窗口中的焦点小部件
    CYearView *monthview = qobject_cast<CYearView *>(QApplication::focusWidget());
    //焦点小部件是否为12个月份中的一个
    if (m_monthViewList.contains(monthview)) {
        //焦点小部件在list中的index
        currentFocusView = m_monthViewList.indexOf(monthview);
    } else {
        currentFocusView = -1;
    }
    return QWidget::eventFilter(watched, event);
}
