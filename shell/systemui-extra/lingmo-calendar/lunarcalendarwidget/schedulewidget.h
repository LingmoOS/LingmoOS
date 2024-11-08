#ifndef SCHEDULEWIDGET_H
#define SCHEDULEWIDGET_H
#include <QApplication>
#include <QString>
#include <QPalette>
//#include <DArrowButton>
#include <QCheckBox>
#include <QDateEdit>
#include <QDialog>
#include "datetimeedit.h"
#include <QFrame>
#include <QLineEdit>
#include <QTextEdit>
#include <QTimeEdit>
#include <QButtonGroup>
#include <QAbstractItemView>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QObject>
#include <QRadioButton>
#include <QTimeEdit>
#include <QWidget>
#include <QComboBox>
#include <QDebug>
#include <QGSettings>
#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QRadioButton>
#include "schedulestruct.h"
#include "schedulemark.h"
//#include "switchbutton.h"

class CSchceduleDlg : public QDialog
{
    Q_OBJECT
public:
    //提醒规则
        enum AlarmType {
            Alarm_None, //从不
            Alarm_Begin, //日程开始时
            Alarm_5Min_Front, //提前5分钟
            Alarm_10Min_Front, //提前10分钟
            Alarm_15Min_Front, //提前15分钟
            Alarm_30Min_Front, //提前30分钟
            Alarm_1Hour_Front, //提前1小时
            Alarm_1Day_Front, //提前1天
            Alarm_2Day_Front, //提前2天
            Alarm_1Day9_Front, //1天前上午9点
            Alarm_Begin9_Front, //日程当天上午9点
            Alarm_2Day9_Front, //2天前上午9点
            Alarm_1Week9_Front, //1周前上午9点
        };
        //重复规则
        enum RRuleType {
            RRule_None, //从不
            RRule_Day, //每天
            RRule_Week, //每周
            RRule_Month, //每月
            RRule_Year, //每年
        };

    CSchceduleDlg(int type, QWidget *parent = nullptr, const bool isAllDay = true);
    ~CSchceduleDlg() Q_DECL_OVERRIDE;
    //设置提醒规则
    AlarmType alarm;
    RRuleType rrule;
    void setThemePalette();
    void initWindowTheme();
    //监听主题的Gsettings
    QGSettings * m_styleGsettings = nullptr;
    void setAlarmType(AlarmType type){
        alarm = type;
        qDebug()<<"当前选择的alarm:"<<alarm;

    }
    AlarmType getAlarmType() const {
        return alarm;
    }

    void setRRuleType(RRuleType type){
        rrule = type;
        qDebug()<<"当前选择的rrule:"<<rrule;
    }
    RRuleType getRRuleType() const {return rrule; }

    int m_height;
    QTextEdit *m_textEdit = nullptr;
    DateTimeEdit *m_beginDateEdit = nullptr;
    int m_beginHour;
    int m_endHour;
    int m_beginMinute;
    int m_endMinute;
    QString timemodel;
    int end_day;
    int end_month;
    QString timeLong;


    QTime m_startTime;
    QTime m_endTime;
    DateTimeEdit *m_endDateEdit = nullptr;
    // 设置小三角起始位置;
    void setStartPos(int startX);
    QPushButton *quxiao = nullptr;
    QPushButton *queding = nullptr;
    bool isDark = false; //是否是深色
    bool isDefault = false; //是否是深浅
    void Setinfo(MarkInfo info);
    QString getText();
    QString m_text;
    void updateMarkText(const MarkInfo &info);
    void updateCrontabEntry();
    QString escapeUrlToHtmlTags(QString &text);
    void setStartDate(const QDate &date);
    void setEndDate(const QDate &date); //用于后期使用
    QComboBox *m_beginTimeEdit = nullptr;
    // 设置中间区域widget;
    void setCenterWidget(QWidget *widget);
    void  setStartTimeMinute(const QString startTime);
    void  setEndTimeMinute(const QString endTime);
    void  setStartTimeHour(const QString tartTime);
    void  setEndTimeHour(const QString endTime);
    void setId(QString idd);
    QString idd;
    QList<MarkInfo> m_markInfoList;
    void setStatusFromId(QString id);
    //更新数据库中日程信息
    void updateScheduleInfo(const MarkInfo &info);

signals:
    void signalViewtransparentFrame(int type);

    void addManyDaysSignal(const MarkInfo &info);

    //void addMarkInfo(const MarkInfo &info);

    void updateMarkInfoSignal(const MarkInfo &info);

    void cancelScheduleEditSignal();

    void updateInfo(MarkInfo info);

    void addToDatabase(const MarkInfo &info);

    void timeIntervalSelection(int startTime,int endTime);
    void NeedDelete();

protected:
    void keyPressEvent(QKeyEvent *event) override; //键盘事件
    void paintEvent(QPaintEvent *) override;


private:
    void initWindow();
    void bindConnect();

private:
    MarkInfo m_info;//标记信息

    QWidget *mainWidget = nullptr;

    QLabel *m_typeLabel = nullptr;
    QComboBox *m_typeComBox = nullptr;
    QLabel *m_contentLabel = nullptr;

    QLabel *m_beginTimeLabel = nullptr;


    QLabel *m_endTimeLabel = nullptr;
    QLabel *m_appPicture = nullptr;
    QCheckBox *allDaycheckBox = nullptr;
    QLabel *allDayLabel =nullptr;
    QCheckBox *m_ifLunarcheckBox = nullptr;
    QToolButton *m_closeButton = nullptr;
    QComboBox *m_endTimeEdit = nullptr;
    QHBoxLayout *frequencyLayout = nullptr;
    QLabel *frequencyLabel = nullptr;
    QLabel *m_classLabel = nullptr;
    QComboBox *m_classCombox = nullptr;
    //QLabel *m_ifLunarLabel = nullptr;
    QLabel *m_remindSetLabel = nullptr;
    QComboBox *m_rmindCombox = nullptr;
    QComboBox *m_rmindComboxallday = nullptr;
    QComboBox *m_repeatCombox = nullptr;
    QLabel *m_beginrepeatLabel = nullptr;
    QComboBox *m_beginrepeatCombox = nullptr;
    QLabel *m_endrepeatLabel = nullptr;
    QComboBox *m_endrepeatCombox = nullptr;
    QLineEdit *m_endrepeattimes = nullptr;
    QLabel *m_endrepeattimesLabel = nullptr;
    QWidget *m_endrepeattimesWidget = nullptr;
    QDateEdit *m_endRepeatDate = nullptr;
    QWidget *m_endrepeatWidget = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_themeLabel = nullptr;

    QVBoxLayout *vlayout = nullptr;

//    SwitchButton *repeatButton = nullptr;
//    SwitchButton *allDaySwitchButton = nullptr;
//    SwitchButton *m_ifLunarButton = nullptr;
    QComboBox *frequencyBox = nullptr;
    DateTimeEdit *frequencytimeBox = nullptr;

    QString m_context;
    const bool m_createAllDay;

private:
    // 小三角起始位置;
    int m_startX;
    // 小三角的宽度;
    int m_triangleWidth;
    // 小三角高度;
    int m_triangleHeight;

    int m_type; // 1新建 0 编辑日程
    QGSettings *calendar_gsettings = nullptr;
    QGSettings *fontSetting = nullptr;
    QGSettings *colorGsettings =nullptr;
    QGSettings * styleGsettings = nullptr;
    QDate m_date;
    bool tanchuang = false;
    bool isStartTimeChanged = false;

    void addPeriodOfTime();
    void addRemindMe();
    void addRemindMeallday();
    void showLunar(const QDate &date);
    void ReminderTime(const QString &startTime);
    void onRepeatComboxTextChanged(const QString &text);
    void onfrequencyComboxTextChanged(const QString &text);
    void updateEndTimeValidation();
    void validateEndTimeFormat1();
    void validateEndTimeFormat2();
//    QString convertToCrontabFormat(int hour , int second,QString text);

public slots:
    void setAllDaySlot(bool isChecked);
    void setRepeatSlot(bool isChecked);
    void updateEndTime(const QString &startTime);
    void slotTextChanged();
    void handleComboBoxTextChanged(const QString &text); //用于设置提醒时间
    void handleComboBoxTextChangedallday(const QString &text);
    void slotConfirmClick();
    void setEndTime(const QString &endTime);
    void slotfrequencyBox(const QString &text);
    void updateEndDateFromBeginDate();
    void onPaletteChanged();
    void changedFontSlot();
//    void onPaletteChanged();
    void handleComboBoxDoubleClick(const QPoint &pos);
//    void formatSpecialText();
private:
    QString defaultTheme = QString(
                "QDateTimeEdit {"
                "    background-color: #E6E6E6;" // 背景色
                "    color: black;" // 字体颜色
                "    border-radius: 6px;" // 边框圆角
                "}"
                "QDateTimeEdit QCalendarWidget QWidget {"
                "    alternate-background-color: #F0F0F0;" // 周一到周五的背景色
                "    background-color: #E6E6E6;"
                "    color: black;" // 字体颜色（如果需要的话）
                "}"
                "QDateTimeEdit QCalendarWidget QAbstractItemView {"
                "    selection-background-color: #C0C0C0;" // 选中项的背景色
                "    selection-color: white;" // 选中项的字体颜色
                "}"
                "QDateTimeEdit QCalendarWidget QToolButton {"
                "    color: black;" // 工具按钮的字体颜色
                "}"
                "QDateTimeEdit QCalendarWidget QAbstractItemView QLabel {"
                "    color: black;" // 日历的日期文字颜色
                "}"
                "QDateTimeEdit QCalendarWidget QAbstractItemView QPushButton {"
                "    color: black;" // 日历的月份选择按钮文字颜色
                "}"
                "QDateTimeEdit QCalendarWidget QAbstractItemView QSpinBox {"
                "    color: black;" // 其他可能需要设置字体颜色的地方
                "}"
                "QDateTimeEdit::drop-down {"
                "   image: url(/usr/share/lingmo-user-guide/data/lingmo-down-symbolic.svg);"
                "   width: 20px;"
                "   height: 20px;"
                "}");
};
static void setFramePalette(QWidget  *widget, QPalette &pal) {
    QList<QLabel *> labelList = widget->findChildren<QLabel *>();
    for (int i = 0; i < labelList.count(); ++i) {
        labelList.at(i)->setPalette(pal);
    }
    QList<QComboBox *> comboBoxList = widget->findChildren<QComboBox *>();
    for (int i = 0; i < comboBoxList.count(); ++i) {
        comboBoxList.at(i)->setPalette(pal);
    if (comboBoxList.at(i)->view()) {
            comboBoxList.at(i)->view()->setPalette(pal);
        }
    }
    QList<QPushButton *> buttonList = widget->findChildren<QPushButton *>();
    for (int i = 0; i < buttonList.count(); ++i) {
        buttonList.at(i)->setPalette(pal);
    }
    QList<DateTimeEdit *> dateList = widget->findChildren<DateTimeEdit *>();
    for (int i = 0; i < dateList.count(); ++i) {
        dateList.at(i)->setPalette(pal);
    }
    QList<QTextEdit *> editList = widget->findChildren<QTextEdit *>();
    for (int i = 0; i < editList.count(); ++i) {
        editList.at(i)->setPalette(pal);
    }
    QList<QCheckBox *> checnboxList = widget->findChildren<QCheckBox *>();
    for (int i = 0; i < checnboxList.count(); ++i) {
        checnboxList.at(i)->setPalette(pal);
    }

}

static QPalette lightPalette(QWidget *widget)
{
    QPalette palette = qApp->palette();

    //lingmo-light palette LINGMO3.1 亮主题色板
    QColor windowText_at(38, 38, 38),
           windowText_iat(0, 0, 0, 255 * 0.55),
           windowText_dis(0, 0, 0, 255 * 0.3),
           button_at(230, 230, 230),
           button_iat(230, 230, 230),
           button_dis(233, 233, 233),
           light_at(255, 255, 255),
           light_iat(255, 255, 255),
           light_dis(242, 242, 242),
           midlight_at(218, 218, 218),
           midlight_iat(218, 218, 218),
           midlight_dis(230, 230, 230),
           dark_at(77, 77, 77),
           dark_iat(77, 77, 77),
           dark_dis(64, 64, 64),
           mid_at(115, 115, 115),
           mid_iat(115, 115, 115),
           mid_dis(102, 102, 102),
           text_at(38, 38, 38),
           text_iat(38, 38, 38),
           text_dis(0, 0, 0, 255 * 0.3),
           brightText_at(0, 0, 0),
           brightText_iat(0, 0, 0),
           brightText_dis(0, 0, 0),
           buttonText_at(38, 38, 38),
           buttonText_iat(38, 38, 38),
           buttonText_dis(150, 150, 150),
           base_at(255, 255, 255),
           base_iat(245, 245, 245),
           base_dis(237, 237, 237),
           window_at(245, 245, 245),
           window_iat(237, 237, 237),
           window_dis(230, 230, 230),
           shadow_at(0, 0, 0, 255 * 0.16),
           shadow_iat(0, 0, 0, 255 * 0.16),
           shadow_dis(0, 0, 0, 255 * 0.21),
           highLightText_at(255, 255, 255),
           highLightText_iat(255, 255, 255),
           highLightText_dis(179, 179, 179),
           alternateBase_at(245, 245, 245),
           alternateBase_iat(245, 245, 245),
           alternateBase_dis(245, 245, 245),
           noRale_at(240, 240, 240),
           noRole_iat(240, 240, 240),
           noRole_dis(217, 217, 217),
           toolTipBase_at(255, 255, 255),
           toolTipBase_iat(255, 255, 255),
           toolTipBase_dis(255, 255, 255),
           toolTipText_at(38, 38, 38),
           toolTipText_iat(38, 38, 38),
           toolTipText_dis(38, 38, 38),
           placeholderText_at(0, 0, 0, 255 * 0.35),
           placeholderText_iat(0, 0, 0, 255 * 0.35),
           placeholderText_dis(0, 0, 0, 255 * 0.3);

    palette.setColor(QPalette::Active, QPalette::WindowText, windowText_at);
    palette.setColor(QPalette::Inactive, QPalette::WindowText, windowText_iat);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, windowText_dis);

    palette.setColor(QPalette::Active, QPalette::Button, button_at);
    palette.setColor(QPalette::Inactive, QPalette::Button, button_iat);
    palette.setColor(QPalette::Disabled, QPalette::Button, button_dis);

    palette.setColor(QPalette::Active, QPalette::Light, light_at);
    palette.setColor(QPalette::Inactive, QPalette::Light, light_iat);
    palette.setColor(QPalette::Disabled, QPalette::Light, light_dis);

    palette.setColor(QPalette::Active, QPalette::Midlight, midlight_at);
    palette.setColor(QPalette::Inactive, QPalette::Midlight, midlight_iat);
    palette.setColor(QPalette::Disabled, QPalette::Midlight, midlight_dis);

    palette.setColor(QPalette::Active, QPalette::Dark, dark_at);
    palette.setColor(QPalette::Inactive, QPalette::Dark, dark_iat);
    palette.setColor(QPalette::Disabled, QPalette::Dark, dark_dis);

    palette.setColor(QPalette::Active, QPalette::Mid, mid_at);
    palette.setColor(QPalette::Inactive, QPalette::Mid, mid_iat);
    palette.setColor(QPalette::Disabled, QPalette::Mid, mid_dis);

    palette.setColor(QPalette::Active, QPalette::Text, text_at);
    palette.setColor(QPalette::Inactive, QPalette::Text, text_iat);
    palette.setColor(QPalette::Disabled, QPalette::Text, text_dis);

    palette.setColor(QPalette::Active, QPalette::BrightText, brightText_at);
    palette.setColor(QPalette::Inactive, QPalette::BrightText, brightText_iat);
    palette.setColor(QPalette::Disabled, QPalette::BrightText, brightText_dis);

    palette.setColor(QPalette::Active, QPalette::ButtonText, buttonText_at);
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, buttonText_iat);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, buttonText_dis);

    palette.setColor(QPalette::Active, QPalette::Base, base_at);
    palette.setColor(QPalette::Inactive, QPalette::Base, base_iat);
    palette.setColor(QPalette::Disabled, QPalette::Base, base_dis);

    palette.setColor(QPalette::Active, QPalette::Window, window_at);
    palette.setColor(QPalette::Inactive, QPalette::Window, window_iat);
    palette.setColor(QPalette::Disabled, QPalette::Window, window_dis);

    palette.setColor(QPalette::Active, QPalette::Shadow, shadow_at);
    palette.setColor(QPalette::Inactive, QPalette::Shadow, shadow_iat);
    palette.setColor(QPalette::Disabled, QPalette::Shadow, shadow_dis);

    palette.setColor(QPalette::Active, QPalette::HighlightedText, highLightText_at);
    palette.setColor(QPalette::Inactive, QPalette::HighlightedText, highLightText_iat);
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, highLightText_dis);

    palette.setColor(QPalette::Active, QPalette::AlternateBase, alternateBase_at);
    palette.setColor(QPalette::Inactive, QPalette::AlternateBase, alternateBase_iat);
    palette.setColor(QPalette::Disabled, QPalette::AlternateBase, alternateBase_dis);

    palette.setColor(QPalette::Active, QPalette::NoRole, noRale_at);
    palette.setColor(QPalette::Inactive, QPalette::NoRole, noRole_iat);
    palette.setColor(QPalette::Disabled, QPalette::NoRole, noRole_dis);

    palette.setColor(QPalette::Active, QPalette::ToolTipBase, toolTipBase_at);
    palette.setColor(QPalette::Inactive, QPalette::ToolTipBase, toolTipBase_iat);
    palette.setColor(QPalette::Disabled, QPalette::ToolTipBase, toolTipBase_dis);

    palette.setColor(QPalette::Active, QPalette::ToolTipText, toolTipText_at);
    palette.setColor(QPalette::Inactive, QPalette::ToolTipText, toolTipText_iat);
    palette.setColor(QPalette::Disabled, QPalette::ToolTipText, toolTipText_dis);

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    palette.setColor(QPalette::Active, QPalette::PlaceholderText, placeholderText_at);
    palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, placeholderText_iat);
    palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, placeholderText_dis);
#endif
    return palette;

}
#endif // SCHEDULEWIDGET_H
