// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULESEARCHVIEW_H
#define SCHEDULESEARCHVIEW_H

#include "dschedule.h"
#include "schedulemanager.h"

#include <DLabel>
#include <DMenu>
#include <DListWidget>

DWIDGET_USE_NAMESPACE
class QVBoxLayout;
class CScheduleListWidget;
class CScheduleSearchItem;
class CScheduleSearchDateItem;
/**
 * @brief The CScheduleSearchView class
 * 搜索结果展示界面
 */
class CScheduleSearchView : public DWidget
{
    Q_OBJECT
public:
    explicit CScheduleSearchView(QWidget *parent = nullptr);
    ~CScheduleSearchView() override;
    void setTheMe(int type = 0);
    //清空搜索
    void clearSearch();
    void setMaxWidth(const int w);
    bool getHasScheduleShow();
    bool getScheduleStatus();
    void deleteSchedule();
signals:
    void signalSelectSchedule(const DSchedule::Ptr &scheduleInfo);
    void signalScheduleHide();
    void signalSelectCurrentItem();
    void signalSchotCutClicked();
public slots:
    //需要搜索日程关键字
    void slotsetSearch(QString str);
    void slotSelectSchedule(const DSchedule::Ptr &scheduleInfo);
    //更新搜索信息
    void updateSearch();
    void slotSelectCurrentItem(CScheduleSearchItem *item, bool itemFocusOut);
    void slotListWidgetClicked();
    void slotScearedScheduleUpdate();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *e) override;

private:
    void updateDateShow();
    void createItemWidget(DSchedule::Ptr info, QDate date, int rtype);
    QListWidgetItem *createItemWidget(QDate date);
private:
    CScheduleListWidget *m_gradientItemList = nullptr; //下拉列表窗
    bool m_widgetFlag;
    QMap<QDate, DSchedule::List> m_vlistData;
    QVector<DLabel *> m_labellist;
    int m_type;
    QDate m_currentDate;
    QColor m_bBackgroundcolor = "#000000";
    QColor m_btimecolor = "#526A7F";
    QColor m_btTextColor = "#414D68";
    QColor m_lBackgroundcolor = Qt::white;
    QColor m_lTextColor = "#001A2E";
    QListWidgetItem *m_currentItem = nullptr;
    int m_maxWidth = 200;
    int currentDItemIndex = -1;
    QString     m_searchStr{""};
    bool    hasScheduleShow = false;
    bool    keyPressUP = false;
    CScheduleSearchItem *m_selectItem = nullptr;
    QVector<CScheduleSearchItem *> m_scheduleSearchItem{};
};

class CScheduleListWidget : public DListWidget
{
    Q_OBJECT
public:
    explicit CScheduleListWidget(QWidget *parent = nullptr);
    ~CScheduleListWidget() override;
signals:
    void signalListWidgetClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *e) override;
};

class CScheduleSearchItem : public DLabel
{
    Q_OBJECT
    enum MouseStatus { M_NONE,
                       M_PRESS,
                       M_HOVER
                     };

public:
    explicit CScheduleSearchItem(QWidget *parent = nullptr);
    void setBackgroundColor(QColor color1);
    void setText(QColor tColor, QFont font);
    void setTimeC(QColor tColor, QFont font);
    void setData(DSchedule::Ptr vScheduleInfo, QDate date);
    void setRoundtype(int rtype);
    void setTheMe(int type = 0);
    void setDurationSize(QFont font);
    const DSchedule::Ptr &getData() const
    {
        return m_ScheduleInfo;
    }
signals:
    void signalsDelete(CScheduleSearchItem *item);
    void signalsEdit(CScheduleSearchItem *item);
    void signalSelectSchedule(const DSchedule::Ptr &scheduleInfo);
    void signalSelectCurrentItem(CScheduleSearchItem *item, bool focusOutStatus);
public slots:
    void slotEdit();
    void slotDelete();
    void slotTimeFormatChanged(int value);
    void slotSchotCutClicked();
protected:
    void paintEvent(QPaintEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    bool eventFilter(QObject *o, QEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void keyPressEvent(QKeyEvent *event) override;
private:
    struct ColorStatus {
        QColor background;
        QColor timeColor;
        QColor textColor;
    };
    DSchedule::Ptr m_ScheduleInfo;
    QAction *m_editAction = nullptr;
    QAction *m_deleteAction = nullptr;
    QColor m_Backgroundcolor;
    ColorStatus m_presscolor;
    ColorStatus m_hovercolor;
    QColor m_timecolor;
    QFont m_timefont;
    QColor m_tTextColor;
    QFont m_tFont;
    QDate m_date;
    MouseStatus m_mouseStatus;
    DMenu *m_rightMenu = nullptr;
    int m_roundtype = 1;
    const int m_radius = 8;
    const int m_borderframewX = 1;  //绘制区域起始位置x坐标点
    const int m_borderframewY = 0;  //绘制区域起始位置y坐标点
    QString m_timeFormat = "hh:mm";
    int m_durationSize = 0;
    bool m_tabFocus {false};
    bool m_tabFocusBeforeActive {false}; //保存应用被激活前是否存在tab焦点效果
};
class CScheduleSearchDateItem : public DLabel
{
    Q_OBJECT

public:
    explicit CScheduleSearchDateItem(QWidget *parent = nullptr);
    void setBackgroundColor(QColor color1);
    void setText(QColor tColor, QFont font);
    void setDate(QDate date);
signals:
    void signalLabelScheduleHide();

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QColor m_Backgroundcolor;
    QColor m_textcolor;
    QFont m_font;
    QDate m_date;
};
#endif // SCHEDULESEARCHVIEW_H
