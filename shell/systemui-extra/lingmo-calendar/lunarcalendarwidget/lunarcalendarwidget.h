/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef LUNARCALENDARWIDGET_H
#define LUNARCALENDARWIDGET_H

#include <QGSettings/QGSettings>
#include <QWidget>
#include <QDate>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>
#include "qfontdatabase.h"
#include <QScreen>
#include "qdatetime.h"
#include "qlayout.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include "qtoolbutton.h"
#include "qcombobox.h"
#include "qdebug.h"
#include <QPainter>
#include <QApplication>
#include "customscrollarea.h"

#include <QPalette>
#include "picturetowhite.h"
#include "morelabel.h"
#include <QJsonParseError>
#include <QJsonObject>
#include <QRadioButton>
#include <QLabel>
#include <QObject>
#include <QMouseEvent>
#include "lunarcalendarinfo.h"
#include "lunarcalendaritem.h"
#include "lunarcalendaryearitem.h"
#include "lunarcalendarmonthitem.h"
#include "customstylePushbutton.h"
#include <QCheckBox>
#include <QLocale>
#include <QProcess>
#include <QListWidget>
#include <QListWidgetItem>
#include <QScrollArea>
#include "calendardatabase.h"
#include "schedulewidget.h"
#include "schedule_item.h"
#include "schedulemark.h"
#include "schedulestruct.h"
#include <lingmosdk/applications/kborderlessbutton.h>

class QGestureEvent;
class QPanGesture;
class QPinchGesture;
class QSwipeGesture;
class CSchceduleDlg;
class QLabel;
class statelabel;
class QComboBox;
class LunarCalendarYearItem;
class LunarCalendarMonthItem;
class LunarCalendarItem;
class CalendarDataBase;
class PartLineWidget;

#ifdef quc
#if (QT_VERSION < QT_VERSION_CHECK(5,7,0))
#include <QtDesigner/QDesignerExportWidget>
#else
#include <QtUiPlugin/QDesignerExportWidget>
#endif

class QDESIGNER_WIDGET_EXPORT LunarCalendarWidget : public QWidget
#else
class LunarCalendarWidget : public QWidget
#endif

{
    Q_OBJECT
    Q_ENUMS(CalendarStyle)
    Q_ENUMS(WeekNameFormat)
    Q_ENUMS(SelectType)

    Q_PROPERTY(CalendarStyle calendarStyle READ getCalendarStyle WRITE setCalendarStyle)
    Q_PROPERTY(QDate date READ getDate WRITE setDate)

    Q_PROPERTY(QColor weekTextColor READ getWeekTextColor WRITE setWeekTextColor)
    Q_PROPERTY(QColor weekBgColor READ getWeekBgColor WRITE setWeekBgColor)

    Q_PROPERTY(QString bgImage READ getBgImage WRITE setBgImage)
    Q_PROPERTY(SelectType selectType READ getSelectType WRITE setSelectType)

    Q_PROPERTY(QColor borderColor READ getBorderColor WRITE setBorderColor)
    Q_PROPERTY(QColor weekColor READ getWeekColor WRITE setWeekColor)
    Q_PROPERTY(QColor superColor READ getSuperColor WRITE setSuperColor)
    Q_PROPERTY(QColor lunarColor READ getLunarColor WRITE setLunarColor)

    Q_PROPERTY(QColor currentTextColor READ getCurrentTextColor WRITE setCurrentTextColor)
    Q_PROPERTY(QColor otherTextColor READ getOtherTextColor WRITE setOtherTextColor)
    Q_PROPERTY(QColor selectTextColor READ getSelectTextColor WRITE setSelectTextColor)
    Q_PROPERTY(QColor hoverTextColor READ getHoverTextColor WRITE setHoverTextColor)

    Q_PROPERTY(QColor currentLunarColor READ getCurrentLunarColor WRITE setCurrentLunarColor)
    Q_PROPERTY(QColor otherLunarColor READ getOtherLunarColor WRITE setOtherLunarColor)
    Q_PROPERTY(QColor selectLunarColor READ getSelectLunarColor WRITE setSelectLunarColor)
    Q_PROPERTY(QColor hoverLunarColor READ getHoverLunarColor WRITE setHoverLunarColor)

    Q_PROPERTY(QColor currentBgColor READ getCurrentBgColor WRITE setCurrentBgColor)
    Q_PROPERTY(QColor otherBgColor READ getOtherBgColor WRITE setOtherBgColor)
    Q_PROPERTY(QColor selectBgColor READ getSelectBgColor WRITE setSelectBgColor)
    Q_PROPERTY(QColor hoverBgColor READ getHoverBgColor WRITE setHoverBgColor)

public:
    enum CalendarStyle {
        CalendarStyle_Red = 0
    };

    enum WeekNameFormat {
        WeekNameFormat_Short = 0,   //短名称
        WeekNameFormat_Normal = 1,  //普通名称
        WeekNameFormat_Long = 2,    //长名称
        WeekNameFormat_En = 3       //英文名称
    };

    enum SelectType {
        SelectType_Rect = 0,        //矩形背景
        SelectType_Circle = 1,      //圆形背景
        SelectType_Triangle = 2,    //带三角标
        SelectType_Image = 3        //图片背景
    };

    QVBoxLayout *m_verLayoutCalendar = nullptr;
    QHBoxLayout *m_horLayoutWidgetTop = nullptr;
    schedule_item *scheduleWidget =nullptr;
    QWidget *allButtom = nullptr;
    QDate date;                         //当前日期
    QScreen *screen = nullptr;
    static QDate s_clickDate;                    //保存点击日期

    void updateLayout();
    int m_num=0;
    bool isClicked  = false;
    bool getBool();
    void initGsettings();
    bool yijistate = false;

    bool ScrollbarUnderTheResolution = false; //极限分辨率下的滑动区域
    bool ScrollAreaNeedChange = false;
CustomScrollArea *scrollArea = nullptr;
QString ScrollAreaStyleSheet;
void DisplayAtResolution();//极限分辨率下的显示

    int position;
    explicit LunarCalendarWidget(QWidget *parent = 0);
    ~LunarCalendarWidget();
    void getFontInfo(QString &fontName, int &fontSize);
    bool getShowLunar();
    bool compare(MarkInfo info1 ,MarkInfo info2);
//    void updateSchedule();
    void updateAllSchedule();
    int primaryScreenHeight = 0; // 存储主屏幕高度的变量
    void PrimaryScreenHeight(int height);
    bool isRTLLanguage();

    bool isWayland;
    void updateSchedule();
    QList<MarkInfo> twiceInfo;
    QList<MarkInfo> remainInfo;
    QVBoxLayout *item_layout ;
//    QVBoxLayout *verLayoutCalendar;
//    ShowMoreLabel *label;
    int panelHeight;//任务栏高度
    int panelSize;
    bool isExceedScreenHeight = false;
    QStringList m_markIdList;
    bool isMore = false;
    QGSettings *m_panelGsettings;
    QList<MarkInfo> m_markInfoList;
//    CalendarDataBase *m_database ;
private:


    ShowMoreLabel *label = nullptr;
    CSchceduleDlg *dlg = nullptr;
//    CSchceduleDlg *demo = nullptr;
    QMessageBox *msg = nullptr;
//    schedule_item *scheduleWidget = nullptr;
    QDate m_date;      //当前日期
    QWidget *m_scheduleWidget = nullptr;
    QListWidget *m_tableWidget = nullptr;
    QList<MarkInfo> *allinfo = nullptr;

    QLabel *datelabel = nullptr;
    QLabel *timelabel = nullptr;
    QLabel *lunarlabel = nullptr;
    QTimer *timer = nullptr;
    QVBoxLayout *m_verLayoutTopLeft = nullptr;
    QWidget *widgetTime = nullptr;
    QPushButton *btnYear = nullptr;
    QPushButton *btnMonth = nullptr;
    QPushButton *btnToday = nullptr;
    QPushButton *yesButton = nullptr;
    QPushButton *noButton = nullptr;
    QWidget *labWidget = nullptr;
    QLabel *labBottom = nullptr;
    int screenHeight;
    QHBoxLayout *labLayout = nullptr;
    PartLineWidget *lineUp = nullptr;
    PartLineWidget *lineDown = nullptr;
    QGSettings * yijiGsettings = nullptr;
    PartLineWidget *lineSchedule = nullptr;
    kdk::KBorderlessButton *btnPrevYear;
    kdk::KBorderlessButton *btnNextYear;
    QLabel *yijichooseLabel = nullptr;
    QCheckBox *yijichoose = nullptr;
    QVBoxLayout *yijiLayout = nullptr;
    QWidget *yijiWidget = nullptr;
    QLabel *yiLabel = nullptr;
    QLabel *jiLabel = nullptr;
    QWidget *widgetWeek = nullptr;
    QWidget *m_widgetDayBody = nullptr;
    QWidget *widgetYearBody = nullptr;
    QWidget *widgetmonthBody = nullptr;
    QProcess *myProcess = nullptr;
    QWidget *m_widgetTop = nullptr;
    QString timemodel = 0;
    bool lunarstate =false;
    bool oneRun = true;
//    ILINGMOPanelPlugin *mPlugin;
//    QString dateShowMode;
    QMap<QString,QString> worktimeinside;
    QMap<QString,QMap<QString,QString>> worktime;
    void analysisWorktimeJs();       //解析js文件
    void downLabelHandle(const QDate &date);
    QFont iconFont;                     //图形字体
    bool btnClick;                      //按钮单击,避开下拉选择重复触发
    QComboBox *cboxYearandMonth = nullptr;        //年份下拉框
    QLabel *cboxYearandMonthLabel = nullptr;
    QList<QLabel *> labWeeks;           //顶部星期名称
    QList<LunarCalendarItem *> dayItems;            //日期元素
    QList<LunarCalendarYearItem *> yearItems;       //年份元素
    QList<LunarCalendarMonthItem *> monthItems;     //月份元素
    QFont m_font;

    CalendarStyle calendarStyle;        //整体样式
    bool FirstdayisSun;                 //首日期为周日
    bool isRightToLeftLanguage;
    int pro;
    QColor weekTextColor;               //星期名称文字颜色
    QColor weekBgColor;                 //星期名称背景色
    QString bgImage;                    //背景图片
    SelectType selectType;              //选中模式



    QColor borderColor;                 //边框颜色
    QColor weekColor;                   //周末颜色
    QColor superColor;                  //角标颜色
    QColor lunarColor;                  //农历节日颜色

    QColor currentTextColor;            //当前月文字颜色
    QColor otherTextColor;              //其他月文字颜色
    QColor selectTextColor;             //选中日期文字颜色
    QColor hoverTextColor;              //悬停日期文字颜色

    QColor currentLunarColor;           //当前月农历文字颜色
    QColor otherLunarColor;             //其他月农历文字颜色
    QColor selectLunarColor;            //选中日期农历文字颜色
    QColor hoverLunarColor;             //悬停日期农历文字颜色

    QColor currentBgColor;              //当前月背景颜色
    QColor otherBgColor;                //其他月背景颜色
    QColor selectBgColor;               //选中日期背景颜色
    QColor hoverBgColor;                //悬停日期背景颜色

    QGSettings *calendar_gsettings = nullptr;
//    CustomScrollArea *scrollArea = nullptr;

    void setColor(bool mdark_style);
    void _timeUpdate();
    void setBool(bool more);
    void yijihandle(const QDate &date);
    QString getSettings();
    void setSettings(QString arg);
    QGSettings *style_settings = nullptr;
    bool dark_style;
    int getPrimaryScreenHeight() const;
    void updatePrimaryScreenHeight();

    QStringList getLocale(QString &language, QString &locale);
    void setLocaleCalendar();
    void changeMonth(bool forward);
    void changeYear(bool forward);
    //void insertNewItem(const QString &title, const QStringList &details);

    //控制农历、黄历、假期显示功能
    bool m_showLunar = false;
    //设置子控件是否显示农历信息
    void setShowLunar(bool showLunar);
    //根据gsetting和语言获取show lunar的值

    int m_currentType = 0;//0:日，1:月, 2:年
    void changeMode(int mode);
    QString m_fontName;

    int m_fontSize = 0;
    int m_start_y = -1;
    QWidget *m_clickedWidget = nullptr;
    int m_savedYear = 0;
    void initTransparency();
    QGSettings *m_transparencySetting = nullptr;
    int m_trans = 75;
    void controlDownLine();
    MarkInfo info;

    QPushButton *m_createScheduleButton = nullptr ;

    void updateLayout1();
    QGSettings *styleGsettings = nullptr;
    void noSchedule();
    void twoLessSchedule();
    void twoMoreSchedule();
    void ThreeMoreSchedule();
    QMap<QString,QString> m_zh_HK_dict; //繁中宜忌翻译字典
    QString toHK(QString str);
protected :
    void wheelEvent(QWheelEvent *event)override;
    bool eventFilter(QObject *watched, QEvent *event)override;
    void paintEvent(QPaintEvent*)override;
    void keyPressEvent(QKeyEvent *ev)override;
//    bool event(QEvent *event)override;
    //void mouseDoubleClickEvent(QMouseEvent *event) override;

private Q_SLOTS:
    void initWidget();
//    void slotClickedMore();
//    void slotClickedCollapse();
    void initStyle();
    void initDate();

    void yearChanged(const QString &arg1);
    void monthChanged(const QString &arg1);
    void labClicked(const QDate &date, const LunarCalendarItem::DayType &dayType);


    void timerUpdate();
    void customButtonsClicked(int x);
    void yearWidgetChange();
    void monthWidgetChange();
    void updateYearMonthString(int year, int month);
    void onPrimaryScreenChanged(const QRect &newGeometry);
    void onScreenGeometryChanged();

public:
    QDate getCurrentDate(){return date;}
    static QDate getClickedDate(){
        qDebug () << s_clickDate;
        return s_clickDate;
    }

    CalendarStyle getCalendarStyle()    const;
    QDate getDate()                     const;

    QColor getWeekTextColor()           const;
    QColor getWeekBgColor()             const;

    QString getBgImage()                const;
    SelectType getSelectType()          const;

    QColor getBorderColor()             const;
    QColor getWeekColor()               const;
    QColor getSuperColor()              const;
    QColor getLunarColor()              const;

    QColor getCurrentTextColor()        const;
    QColor getOtherTextColor()          const;
    QColor getSelectTextColor()         const;
    QColor getHoverTextColor()          const;

    QColor getCurrentLunarColor()       const;
    QColor getOtherLunarColor()         const;
    QColor getSelectLunarColor()        const;
    QColor getHoverLunarColor()         const;

    QColor getCurrentBgColor()          const;
    QColor getOtherBgColor()            const;
    QColor getSelectBgColor()           const;
    QColor getHoverBgColor()            const;

    QSize sizeHint()                    const;
    QSize minimumSizeHint()             const;
    QString locale                     ;
    void showCalendar(bool showToday);
public Q_SLOTS:
    void dateChanged(int year, int month, int day);
    void changeDate(const QDate &date);
    void dayChanged(const QDate &date,const QDate &m_date);
    void updateFromItem(QString id);
    void updateYearClicked(const QDate &date, const LunarCalendarYearItem::DayType &dayType);
    void updateMonthClicked(const QDate &date, const LunarCalendarMonthItem::DayType &dayType);
    //上一年,下一年
    void showPreviousYear();
    void showNextYear();
    void slotShowMore();

    void onGSettingsChanged(const QString &key);

    //上一月,下一月
    void showPreviousMonth(bool date_clicked = true);
    void showNextMonth(bool date_clicked = true);

    //转到今天
    void showToday();

    //设置整体样式
    void setCalendarStyle(const CalendarStyle &calendarStyle);
    //设置星期名称格式
    void setWeekNameFormat(bool FirstDayisSun);

    //设置日期
    void setDate(const QDate &date);

    //设置顶部星期名称文字颜色+背景色
    void setWeekTextColor(const QColor &weekTextColor);
    void setWeekBgColor(const QColor &weekBgColor);


    //设置背景图片
    void setBgImage(const QString &bgImage);
    //设置选中背景样式
    void setSelectType(const SelectType &selectType);

    //设置边框颜色
    void setBorderColor(const QColor &borderColor);
    //设置周末颜色
    void setWeekColor(const QColor &weekColor);
    //设置角标颜色
    void setSuperColor(const QColor &superColor);
    //设置农历节日颜色
    void setLunarColor(const QColor &lunarColor);

    //设置当前月文字颜色
    void setCurrentTextColor(const QColor &currentTextColor);
    //设置其他月文字颜色
    void setOtherTextColor(const QColor &otherTextColor);
    //设置选中日期文字颜色
    void setSelectTextColor(const QColor &selectTextColor);
    //设置悬停日期文字颜色
    void setHoverTextColor(const QColor &hoverTextColor);

    //设置当前月农历文字颜色
    void setCurrentLunarColor(const QColor &currentLunarColor);
    //设置其他月农历文字颜色
    void setOtherLunarColor(const QColor &otherLunarColor);
    //设置选中日期农历文字颜色
    void setSelectLunarColor(const QColor &selectLunarColor);
    //设置悬停日期农历文字颜色
    void setHoverLunarColor(const QColor &hoverLunarColor);

    //设置当前月背景颜色
    void setCurrentBgColor(const QColor &currentBgColor);
    //设置其他月背景颜色
    void setOtherBgColor(const QColor &otherBgColor);
    //设置选中日期背景颜色
    void setSelectBgColor(const QColor &selectBgColor);
    //设置悬停日期背景颜色
    void setHoverBgColor(const QColor &hoverBgColor);
    //新建日程弹窗
    void CreateSchedule(QDate date);
    void slotAddSchedule();
    void slotEditSchedule(QString id);
    void slotEditSchedulefromMenu(QString id);
//    void onDialogAccepted();

Q_SIGNALS:
    void scheduleNum(int num);
    void NeedToHide();
//    void signalLabel
    void clicked(const QDate &date);
    void showMore(bool isshow);
    void selectionChanged();
    void yijiChangeUp(int num);
    void Up(int n);
    void allschedule();
    void yijiChangeDown(int num);
    //  true：显示农历、黄历 , false：不显示农历、黄历
    void almanacChanged(bool);
    void DayClick(QDate date);//月视图内每一个日小框双击信号
    void newschdule(int n);
};


class PartLineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PartLineWidget(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);

};

class statelabel : public QLabel
{
    Q_OBJECT
public:
    statelabel();

protected:
    void mousePressEvent(QMouseEvent *event);
Q_SIGNALS :
    void labelclick();
};


#endif // LUNARCALENDARWIDGET_H
