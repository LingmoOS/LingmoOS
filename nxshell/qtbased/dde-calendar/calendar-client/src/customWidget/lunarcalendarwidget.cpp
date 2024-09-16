// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lunarcalendarwidget.h"
#include "lunarmanager.h"

#include <DPalette>
#include <DStyle>
#include <DApplicationHelper>
#include <DApplicationHelper>

#include <QLabel>
#include <QTableView>
#include <QProxyStyle>
#include <QCalendarWidget>
#include <QVBoxLayout>
#include <QToolButton>
#include <QHeaderView>

/**
 * 实现的效果：保证原本日历不变的前提下，1.增加阴历年、2.增加阴历日、3.增加控件底部留白
 * 1.增加阴历年
 *  添加QLabel
 *
 * 2.增加阴历日
 *  重新绘制item
 *  相关算法：item.height =  2px空心圆 + 1.5px空白 + 实心圆.直径 + 2px空心圆 + 1.5px空白 + 阴历日.font.height
 *  实心圆.直径 = 阳历日.font.height = 阴历日.font.height
 *  实心圆.直径 = (item.height - 7) / 2
 *
 *  如下面代码：
 *  int d = (rect.height() - 7) / 2;//阳历日.font.height
 *  int h = d + 7;//阳历日在的rect的高度
 *
 * 3.增加控件底部留白
 *  添加固定高度的QWidget
**/

//底部留空10px
static const int BottomSpacing = 10;

DWIDGET_USE_NAMESPACE

class CalenderStyle : public QProxyStyle
{
public:
    CalenderStyle() {}

    /**
     * @brief getColor 获取对应palette的颜色
     */
    QColor getColor(const QStyleOption *option, DPalette::ColorType type, const QWidget *widget) const;

    QColor getColor(const QStyleOption *option, QPalette::ColorRole role) const;

    /**
     * @brief drawCalenderEllipse 绘制圆
     */
    void drawCalenderEllipse(QPainter *p, const QRect &rect, int offset) const;

    /**
     * @brief drawItemViewSelected 绘制选中样式
     */
    void drawItemViewSelected(QPainter *p, const QStyleOption *opt) const;

    /**
     * @brief drawItemViewFocus 绘制焦点样式
     */
    void drawItemViewFocus(QPainter *p, const QStyleOption *opt, const QWidget *w) const;

    /**
     * @brief getLunarYearDesc 获取阴历年描述
     */
    QString getLunarYearDesc(const QDate &date) const;

    /**
     * @brief getLunarDayName 获取阴历日
     */
    QString getLunarDayName(const QDate &date) const;

    /**
     * @brief updateLunarInfo 更新阴历信息
     */
    void updateLunarInfo(const QDate &date) const;

protected:
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;

private:
    mutable QMap<QDate, CaHuangLiDayInfo> m_caHuangLiDayMap;
};

QColor CalenderStyle::getColor(const QStyleOption *option, DPalette::ColorType type, const QWidget *widget) const
{
    const DPalette &pa = DApplicationHelper::instance()->palette(widget, option->palette);
    return pa.brush(type).color();
}

QColor CalenderStyle::getColor(const QStyleOption *option, QPalette::ColorRole role) const
{
    return option->palette.brush(role).color();
}

void CalenderStyle::drawCalenderEllipse(QPainter *p, const QRect &rect, int offset) const
{
    int d = (rect.height() - 7) / 2;
    int h = d + 7;
    QRect r = rect.adjusted(0, 0, 0, -(rect.height() - h));

    QRect ellipese = r;
    ellipese.setWidth(ellipese.height());
    ellipese.moveCenter(r.center());
    p->drawEllipse(ellipese.adjusted(offset, offset, -offset, -offset));
}

void CalenderStyle::drawItemViewSelected(QPainter *p, const QStyleOption *opt) const
{
    if (opt->state & QStyle::State_Selected) {
        QRect rect = opt->rect;

        int d = (rect.height() - 7) / 2;
        int h = d + 7;
        QRect r = rect.adjusted(0, 0, 0, -(rect.height() - h));

        QRect ellipese = r;
        ellipese.setWidth(ellipese.height());
        ellipese.moveCenter(r.center());
        p->setBrush(opt->palette.brush(QPalette::Highlight));
        p->setPen(Qt::NoPen);
        p->setRenderHint(QPainter::Antialiasing);
        p->drawEllipse(ellipese.adjusted(1, 1, -1, -1));
    }
}

void CalenderStyle::drawItemViewFocus(QPainter *p, const QStyleOption *opt, const QWidget *w) const
{
    const QColor &focus_color = getColor(opt, QPalette::Highlight);
    if (!focus_color.isValid() || focus_color.alpha() == 0)
        return;

    p->setRenderHint(QPainter::Antialiasing);
    QPen pen;
    pen.setWidth(2);
    pen.setColor(focus_color);
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);

    bool calendar = w && (w->objectName() == "qt_calendar_calendarview");

    pen.setWidth(2);
    int offset = 1;
    // 先画内框（黑or白）
    pen.setColor(getColor(opt, QPalette::Base));
    p->setPen(pen);
    if (calendar) {
        pen.setWidth(3);
        p->setPen(pen);
        offset = 2;
        drawCalenderEllipse(p, opt->rect, offset);
    }

    // 再画外框（活动色即焦点）
    pen.setColor(focus_color);
    p->setPen(pen);
    if (calendar) {
        pen.setWidth(2);
        p->setPen(pen);
        offset = 1;
        drawCalenderEllipse(p, opt->rect, offset);
    }
}

QString CalenderStyle::getLunarDayName(const QDate &date) const
{
    updateLunarInfo(date);
    CaHuangLiDayInfo &info = m_caHuangLiDayMap[date];

    return info.mLunarDayName;
}

QString CalenderStyle::getLunarYearDesc(const QDate &date) const
{
    updateLunarInfo(date);
    CaHuangLiDayInfo &info = m_caHuangLiDayMap[date];

    return info.mGanZhiYear + info.mZodiac + "年 " + info.mLunarMonthName;
}

void CalenderStyle::updateLunarInfo(const QDate &date) const
{
    if (m_caHuangLiDayMap.count() > 100)
        m_caHuangLiDayMap.clear();

    if (!m_caHuangLiDayMap.contains(date)) {
        CaHuangLiDayInfo info = gLunarManager->getHuangLiDay(date);
        m_caHuangLiDayMap[date] = info;
    }
}

void CalenderStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    const QWidget *w = widget;
    const QStyleOption *opt = option;
    QPainter *p = painter;

    while (element == CE_ItemViewItem) {
        if (w && w->objectName() == "qt_calendar_calendarview") {
            if (const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt)) {

                LunarCalendarWidget *calendar = qobject_cast<LunarCalendarWidget *>(w->parent());
                if (!calendar)
                    break;
                const QAbstractTableModel *model = qobject_cast<const QAbstractTableModel *>(vopt->index.model());
                if (!model)
                    break;

                //获取阳历和阴历
                QString dayName;
                QString dayNumber = vopt->index.data().toString();
                //获取该item的显示天。若为周x则为0
                int day = vopt->index.data().toInt();
                //该item的时间
                QDate curDate = QDate(calendar->yearShown(), calendar->monthShown(), 1);
                //显示天的item需要获取每个item的显示时间
                if (day > 0) {
                    //如果是第一行会显示上个月的日期
                    if (vopt->index.row() < 2 && day > 20) {
                        curDate = curDate.addMonths(-1);
                    } else if (vopt->index.row() > 3 && day < 15) {
                        //如果显示行数大于3且显示的天小于15则表示是下个月
                        curDate = curDate.addMonths(1);
                    }
                    curDate.setDate(curDate.year(), curDate.month(), day);
                    dayName = getLunarDayName(curDate);

                    //更新阴历年描述
                    int selectedYear = calendar->yearShown();
                    int selectedMonth = calendar->monthShown();

                    //如果为焦点状态，则根据该item的日期设置年份时间
                    if (vopt->state & QStyle::State_HasFocus) {
                        QDate selectedDate = QDate(selectedYear, selectedMonth, day);
                        calendar->setLunarYearText(getLunarYearDesc(selectedDate));
                    }
                }

                p->save();
                p->setClipRect(vopt->rect);

                //绘制禁用项
                if (!(vopt->state & QStyle::State_Enabled)) {
                    p->save();
                    p->setPen(Qt::NoPen);
                    p->setBrush(getColor(vopt, DPalette::Window));
                    p->drawRect(vopt->rect);
                    p->restore();
                }

                // 绘制当前选中项
                drawItemViewSelected(p, vopt);

                // draw the text
                if (!vopt->text.isEmpty()) {
                    QPalette::ColorGroup cg = (vopt->state & QStyle::State_Enabled)
                                                  ? QPalette::Normal
                                                  : QPalette::Disabled;
                    if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
                        cg = QPalette::Inactive;

                    QColor colorOrg;
                    QColor colorDisable = qApp->palette().brush(QPalette::Disabled, QPalette::Text).color();
                    if (vopt->state & QStyle::State_Selected) {
                        colorOrg = vopt->palette.color(cg, QPalette::HighlightedText);
                    } else {
                        colorOrg = vopt->palette.color(cg, QPalette::Text);
                    }

                    p->save();
                    const QRect rect = vopt->rect;
                    int d = (rect.height() - 7) / 2;
                    if (vopt->index.row() == 0) {
                        p->setPen(colorOrg);
                        p->drawText(vopt->rect, Qt::AlignCenter, dayNumber);
                    } else {
                        int h = d + 7;
                        p->setPen(colorOrg);
                        p->drawText(rect.adjusted(0, 0, 0, -(rect.height() - h)), Qt::AlignCenter, dayNumber);

                        p->setPen(colorDisable);
                        p->drawText(rect.adjusted(0, h, 0, 0), Qt::AlignCenter, dayName);
                    }
                    p->restore();
                }

                //绘制日历分割线
                if (vopt->index.row() == 0) {
                    p->save();
                    QColor color = getColor(vopt, DPalette::FrameBorder, w);
                    color.setAlpha(static_cast<int>(255 * 0.05));
                    QPen pen(color, 2);
                    p->setPen(pen);
                    int margins = DStyle::pixelMetric(proxy(), DStyle::PM_ContentsMargins);

                    if (vopt->index.column() == 0)
                        p->drawLine(vopt->rect.bottomLeft() + QPoint(margins, 0), vopt->rect.bottomRight());
                    else if (vopt->index.column() == model->columnCount() - 1) {
                        p->drawLine(vopt->rect.bottomLeft(), vopt->rect.bottomRight() - QPoint(margins, 0));
                    } else {
                        p->drawLine(vopt->rect.bottomLeft(), vopt->rect.bottomRight());
                    }

                    p->restore();
                }

                // draw the focus rect
                if (vopt->state & QStyle::State_HasFocus) {
                    QStyleOptionFocusRect o;
                    o.QStyleOption::operator=(*vopt);
                    o.rect = proxy()->subElementRect(SE_ItemViewItemFocusRect, vopt, w);
                    o.state |= QStyle::State_KeyboardFocusChange;
                    o.state |= QStyle::State_Item;
                    QPalette::ColorGroup cg = (vopt->state & QStyle::State_Enabled)
                                                  ? QPalette::Normal
                                                  : QPalette::Disabled;
                    o.backgroundColor = vopt->palette.color(cg, (vopt->state & QStyle::State_Selected)
                                                            ? QPalette::Highlight : QPalette::Window);
                    drawItemViewFocus(p, vopt, w);
                }
                p->restore();
                return;
            }
        }
        break;
    } //while
    QProxyStyle::drawControl(element, option, painter, widget);
}

LunarCalendarWidget::LunarCalendarWidget(QWidget *parent)
    : QCalendarWidget(parent)
    , m_style(new CalenderStyle)
{
    QWidget *w = findChild<QTableView *>("qt_calendar_calendarview");
    if (w)
        w->setStyle(m_style);

    QVBoxLayout *layoutV = qobject_cast<QVBoxLayout *>(layout());
    m_lunarLabel = new QLabel(this);
    m_lunarLabel->setObjectName("lunarLabel");
    m_lunarLabel->setAutoFillBackground(true);
    m_lunarLabel->setBackgroundRole(QPalette::Base);
    m_lunarLabel->setAlignment(Qt::AlignCenter);
    layoutV->insertWidget(1, m_lunarLabel);

    //底部留空10px
    QLabel *label = new QLabel(this);
    m_lunarLabel->setObjectName("BottomSpacingLabel");
    label->setAutoFillBackground(true);
    label->setBackgroundRole(QPalette::Base);
    label->setFixedHeight(BottomSpacing);
    layoutV->insertWidget(3, label);
}

LunarCalendarWidget::~LunarCalendarWidget()
{
    //Sets the widget's GUI style to style. The ownership of the style object is not transferred.
    //需手动删除
    delete m_style;
}

void LunarCalendarWidget::setLunarYearText(const QString &text)
{
    if (m_lunarLabel->text() != text)
        m_lunarLabel->setText(text);
}

QString LunarCalendarWidget::lunarYearText()
{
    return m_lunarLabel->text();
}

QSize LunarCalendarWidget::minimumSizeHint() const
{
    QTableView *view = findChild<QTableView *>("qt_calendar_calendarview");
    if (!view)
        return QSize();
    QAbstractItemModel *model = view->model();
    if (!model)
        return QSize();

    QWidget *navigationbar = findChild<QWidget *>("qt_calendar_navigationbar");
    if (!navigationbar)
        return QSize();

    int rowcount = model->rowCount();
    int hf = QFontMetrics(this->font()).height();

    //原生日历的宽度
    QSize old = QCalendarWidget::minimumSizeHint();
    int w = old.width();

    //navigationbar的高度
    int h = navigationbar->minimumSizeHint().height();
    //m_lunarLabel的高度
    h += m_lunarLabel->minimumSizeHint().height();
    //CalendarView的高度
    h += (hf * 2 + 7) * rowcount;
    //底部留空
    h += BottomSpacing;

    return QSize(w, h);
}
