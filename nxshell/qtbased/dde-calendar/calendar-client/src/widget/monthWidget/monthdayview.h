// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MONTHDAYVIEW_H
#define MONTHDAYVIEW_H

#include "../touchgestureoperation.h"

#include <DFrame>

#include <QObject>
#include <QDate>

DWIDGET_USE_NAMESPACE

class CMonthWidget;
/**
 * @brief The CMonthDayView class       月视图月份自定义控件
 */
class CMonthDayView : public DFrame
{
    Q_OBJECT
public:
    explicit CMonthDayView(QWidget *parent = nullptr);
    ~CMonthDayView() override;
    //设置选择时间
    void setSelectDate(const QDate &date);
    //设置主题颜色
    void setTheMe(int type = 0);
    void setSearchflag(bool flag);

protected:
    void wheelEvent(QWheelEvent *e) override;
    bool event(QEvent *e) override;
signals:
    //切换月份修改选择时间
    void signalsSelectDate(QDate date);
    /**
     * @brief signalAngleDelta      发送滚动信号滚动相对量
     * @param delta     滚动相对量
     */
    void signalAngleDelta(int delta);
private:
    /**
     * @brief m_touchGesture        触摸手势处理
     */
    touchGestureOperation       m_touchGesture;
    CMonthWidget                *m_monthWidget = nullptr;
    QDate                       m_selectDate;
    QDate                       m_days[12];

    int                         m_fixwidth = 200;
    int                         m_realwidth = 100;
    bool m_searchFlag = false;
};

class CMonthRect;
class CMonthWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CMonthWidget(QWidget *parent = nullptr);
    ~CMonthWidget() override;
    void setDate(const QDate date[12]);
protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    /**
     * @brief mouseReleaseEvent 鼠标释放事件
     * @param event 鼠标事件
     */
    void mouseReleaseEvent(QMouseEvent *event) override;
    /**
     * @brief mouseMoveEvent 鼠标移动事件，设置hover状态
     * @param event 鼠标事件
     */
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void mousePress(const QPoint &point);
    //更新item大小
    void updateSize();
    //获取鼠标点击的区域编号
    int getMousePosItem(const QPointF &pos);
    //根据选择时间更新显示的月份
    void updateShowDate(const QDate &selectDate);
signals:
    void signalsSelectDate(QDate date);
private:
    QVector<CMonthRect *> m_MonthItem;
    QDate                       m_days[12];
    //触摸状态 0：原始  1：点击  2：移动
    int m_touchState {0};
    //触摸点击坐标
    QPoint m_touchBeginPoint;
    bool m_isFocus;
};

class CMonthRect
{
public:
    explicit CMonthRect(QWidget *parent = nullptr);
    //设置时间
    void setDate(const QDate &date);
    //获取时间
    QDate getDate()const;
    //获取矩阵大小
    QRectF rect() const;
    //设置矩阵大小
    void setRect(const QRectF &rect);
    //设置矩阵大小
    inline void setRect(qreal x, qreal y, qreal w, qreal h);
    //绘制
    void paintItem(QPainter *painter, const QRectF &rect, bool drawFocus = false);
    //设置设备缩放比例
    static void setDevicePixelRatio(const qreal pixel);
    //设置主题
    static void setTheMe(int type);
    //设置选择的矩阵
    static void setSelectRect(CMonthRect *selectRect);
    //获取选择的矩阵
    static CMonthRect *getSelectRect();

private:
    QRectF                              m_rect;
    QDate                               m_Date;
    static int                          m_themetype ;
    static qreal                        m_DevicePixelRatio;

    static QColor                       m_defaultTextColor;
    static QColor                       m_backgrounddefaultColor;
    static QColor                       m_currentDayTextColor;
    static QColor                       m_backgroundcurrentDayColor;
    static QColor                       m_fillColor;
    static QFont                        m_dayNumFont;
    static CMonthRect                  *m_SelectRect;
    QColor                              m_selectColor;
    QWidget *m_parentWidget;
};

#endif // MONTDAYVIEW_H
