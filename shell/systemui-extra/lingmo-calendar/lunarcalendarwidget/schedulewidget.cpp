#include "schedulewidget.h"
#include <QStandardItemModel>
#include <QObject>
#include <QMessageBox>
#include <QPalette>
#include <QCalendarWidget>
#include <QHBoxLayout>
#include <QIcon>
#include "datetimeedit.h"
#include <QPalette>
#include <QApplication>
#include <QIntValidator>
#include <QListView>
#include <QMessageBox>
#include <QToolButton>
#include <QAbstractItemView>
#include <QRadioButton>
#include <QShortcut>
#include "custommessagebox.h"
#include <QTextBlock>
#include <QToolButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include "schedulemark.h"
#include "switchbutton.h"
#include <QString>
#include <QEventLoop>
#include <QFrame>
#include<QStringList>
//#include <QDBusConnection>
#include <cstdlib>
//#include <QDBusMessage>
#include <QProcess>
#include <QTime>
#include "schedule_item.h"
//#include "../UI/crontabscheduler.h"
#include "calendardatabase.h"
const QString websiteTemplate = R"(<a href="%1">%2</a>)";
#define SHADOW_WIDTH 15    // 窗口阴影宽度;
#define TRIANGLE_WIDTH 15  // 小三角的宽度;
#define TRIANGLE_HEIGHT 10 // 小三角的高度;
#define BORDER_RADIUS 5    // 窗口边角的弧度;
#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "styleName"
#define LUNAR_KEY                 "calendar"
#define FIRST_DAY_KEY "firstday"
#define PANEL_CONTROL_IN_CALENDAR "org.lingmo.control-center.panel.plugins"

CSchceduleDlg::CSchceduleDlg(int type, QWidget *parent, const bool isAllDay)//1,新建日程 0.编辑日程
    : QDialog(parent), m_startX(50), m_triangleWidth(TRIANGLE_WIDTH), m_triangleHeight(TRIANGLE_HEIGHT),
      m_createAllDay(isAllDay)
{
    m_type = type;//m_type可以当做当前日期是否含有日程的

    m_date = QDate::currentDate();
    initWindow();

    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
        styleGsettings = new QGSettings(style_id, QByteArray(), this);
        QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
        if(currentTheme == "lingmo-default"){
            isDefault = true;
        }else if(currentTheme == "lingmo-dark"){
            isDark = true;
        }
        connect(styleGsettings, &QGSettings::changed,[=](QString key) {
            if ("styleName" ==key) {
                onPaletteChanged();
            }
        });
    }
    if(isDefault){
        QPalette pal ;
        if (styleGsettings!=nullptr) {
            QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
            if(currentTheme == "lingmo-default"){
                pal = lightPalette(this);

            }
        }
        pal.setColor(QPalette::Background, pal.background().color());
        pal.setColor(QPalette::Base, pal.base().color());
        pal.setColor(QPalette::Text, pal.text().color());
        pal.setColor(QPalette::Button, pal.button().color());
        pal.setColor(QPalette::ButtonText, pal.buttonText().color());
        setFramePalette(this, pal);
        m_closeButton->setStyleSheet("QToolButton {"
                                     "color: black;"
                                     "}"
                                     "QToolTip {"
                                     "    background-color: #FFFFFF;"
                                     "    color: black;"
                                     "}");
        initWindowTheme();
        setThemePalette();
        bindConnect();
        setFocusPolicy(Qt::WheelFocus);
    }
    const QByteArray id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(id)) {
        colorGsettings = new QGSettings(id, QByteArray(), this);
        connect(colorGsettings, &QGSettings::changed, this, [=](QString key){
            if ("themeColor" == key) {
                onPaletteChanged();
            }
        });
    }

    const QByteArray calendar_id(PANEL_CONTROL_IN_CALENDAR);
    if(QGSettings::isSchemaInstalled(calendar_id)){
        calendar_gsettings = new QGSettings(calendar_id);
    }
    if(calendar_gsettings!=nullptr){
        //农历切换监听与日期显示格式
        connect(calendar_gsettings, &QGSettings::changed, this, [=] (const QString &key){
            //后期可能在控制面板农历公历有用
            if(key == LUNAR_KEY){
            }
            else if(key == "date") {
            } else if (key == FIRST_DAY_KEY) {
            }
        });
        //监听12/24小时制
        connect(calendar_gsettings, &QGSettings::changed, this, [=] (const QString &keys){
            timemodel = calendar_gsettings->get("hoursystem").toString();
        });
        timemodel = calendar_gsettings->get("hoursystem").toString();

    }

    //changedFontSlot();
    bindConnect();
    setFocusPolicy(Qt::WheelFocus);
}
CSchceduleDlg::~CSchceduleDlg()
{
   if(calendar_gsettings){
       delete calendar_gsettings;
       calendar_gsettings = nullptr;
   }
   if(colorGsettings){
       delete colorGsettings;
       colorGsettings = nullptr;
   }
   if(fontSetting){
       delete fontSetting;
       fontSetting = nullptr;
   }
   if(styleGsettings){
       delete styleGsettings;
       styleGsettings = nullptr;
   }
}

void CSchceduleDlg::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
//    painter.setBrush(QColor(255, 255, 255));

    QPalette pal ;
    QGSettings *styleGsettings = nullptr;
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
        styleGsettings = new QGSettings(style_id, QByteArray(), this);
        QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
        if(currentTheme == "lingmo-default" ){
            pal = lightPalette(this);
            painter.setBrush(QColor(255, 255, 255));
            setFramePalette(this, pal);
        }else if(currentTheme == "lingmo-dark"){
            painter.setBrush(QColor(0, 0, 0));
        }else if (currentTheme == "lingmo-light"){
            painter.setBrush(QColor(255, 255, 255));
        }
    }



    QPainterPath drawPath;
    drawPath.addRoundedRect(QRect(SHADOW_WIDTH, m_triangleHeight + SHADOW_WIDTH, width() - SHADOW_WIDTH * 2,
                                  height() - SHADOW_WIDTH * 2 - m_triangleHeight),
                            BORDER_RADIUS, BORDER_RADIUS);

    painter.drawPath(drawPath);
    if (styleGsettings != nullptr) {
        delete styleGsettings;
        styleGsettings = nullptr;
    }
}

void CSchceduleDlg::showLunar(const QDate &date)
{
    QString strHoliday;
    QString strSolarTerms;
    QString strLunarFestival;
    QString strLunarYear;
    QString strLunarMonth;
    QString strLunarDay;

    //CalendarInfo::getInstance().getLunarCalendarInfo(date.year(), date.month(), date.day(), strHoliday, strSolarTerms,
    //                                                 strLunarFestival, strLunarYear, strLunarMonth, strLunarDay);

    QString labBottomarg = strLunarYear + "年  " + strLunarMonth + strLunarDay;
    qDebug() << "农历日:" << labBottomarg;

}
/**
 * @brief CSchceduleDlg::addRemindMe
 * 添加提醒时间
 */
void CSchceduleDlg::addRemindMe()
{
    m_rmindCombox->addItem(tr("No prompt"));
    m_rmindCombox->addItem(tr("Schedule Begin"));
    m_rmindCombox->addItem(tr("5 minutes in advance"));
    m_rmindCombox->addItem(tr("10 minutes in advance"));
    m_rmindCombox->addItem(tr("15 minutes in advance"));
    m_rmindCombox->addItem(tr("30 minutes in advance"));
    m_rmindCombox->addItem(tr("1 hour in advance"));
    m_rmindCombox->addItem(tr("1 day in advance"));
    m_rmindCombox->addItem(tr("2 day in advance"));
    setAlarmType(AlarmType::Alarm_None);
    CalendarDataBase::getInstance().AlarmType = 0;
    connect(m_rmindCombox,SIGNAL(currentTextChanged(QString)),this,SLOT(handleComboBoxTextChanged(QString)));
}

//点击全天按钮后的提醒下拉框
void CSchceduleDlg::addRemindMeallday()
{
    m_rmindComboxallday->addItem(tr("1 day ago at 9am"));
    m_rmindComboxallday->addItem(tr("No prompt"));
    m_rmindComboxallday->addItem(tr("At 9am on the same day of the schedule"));
    m_rmindComboxallday->addItem(tr("2 days ago at 9am"));
    m_rmindComboxallday->addItem(tr("1 week ago at 9am"));
    setAlarmType(AlarmType::Alarm_1Day9_Front);
    CalendarDataBase::getInstance().AlarmType =9;
    connect(m_rmindComboxallday,SIGNAL(currentTextChanged(QString)),this,SLOT(handleComboBoxTextChangedallday(QString)));
}

void CSchceduleDlg::handleComboBoxTextChanged(const QString &text)
{
    if(text == tr("No prompt")){
        setAlarmType(Alarm_None);
        CalendarDataBase::getInstance().AlarmType=0;
    }
    else if(text == tr("Schedule Begin")){
        setAlarmType(Alarm_Begin);
        CalendarDataBase::getInstance().AlarmType=1;
    }
    else if(text ==tr("5 minutes in advance")){
        setAlarmType(Alarm_5Min_Front);
        CalendarDataBase::getInstance().AlarmType=2;
    }
    else if(text ==tr("10 minutes in advance")){
        setAlarmType(Alarm_10Min_Front);
        CalendarDataBase::getInstance().AlarmType=3;
    }
    else if(text ==tr("15 minutes in advance")){
        setAlarmType(Alarm_15Min_Front);
        CalendarDataBase::getInstance().AlarmType=4;
    }
    else if(text ==tr("30 minutes in advance")){
        setAlarmType(Alarm_30Min_Front);
        CalendarDataBase::getInstance().AlarmType=5;
    }
    else if(text ==tr("1 hour in advance")){
        setAlarmType(Alarm_1Hour_Front);
        CalendarDataBase::getInstance().AlarmType=6;
    }
    else if(text ==tr("1 day in advance")){
        setAlarmType(Alarm_1Day_Front);
        CalendarDataBase::getInstance().AlarmType=7;
    }
    else if(text ==tr("2 day in advance")){
        setAlarmType(Alarm_2Day_Front);
        CalendarDataBase::getInstance().AlarmType=8;
    }
}

void CSchceduleDlg::handleComboBoxTextChangedallday(const QString &text)
{

   if(text == tr("1 day ago at 9am")){
       setAlarmType(Alarm_1Day9_Front);
       CalendarDataBase::getInstance().AlarmType=9;
   }
   else if(text ==tr("No prompt")){
       setAlarmType(Alarm_None);
       CalendarDataBase::getInstance().AlarmType=0;
//        qDebug()<<"当前的alarm"<<alarm;
   }
   else if(text ==tr("At 9am on the same day of the schedule")){
       setAlarmType(Alarm_Begin9_Front);
       CalendarDataBase::getInstance().AlarmType=10;
   }
   else if(text ==tr("2 days ago at 9am")){
       setAlarmType(Alarm_2Day9_Front);
       CalendarDataBase::getInstance().AlarmType=11;
   }
   else if(text ==tr("1 week ago at 9am")){
       setAlarmType(Alarm_1Week9_Front);
       CalendarDataBase::getInstance().AlarmType=12;
   }
}

/**
 * @brief CSchceduleDlg::setStartTimeMinute
 * @param startTime
 * 设置开始时间的分钟
 */
void CSchceduleDlg::setStartTimeMinute(const QString startTime)
{
    QTime time =QTime::fromString(startTime,"h:mm");
    QTime end = time.addSecs(1800);
    m_endHour = end.hour();//设置默认
    m_endMinute = end.minute();//设置默认

    m_beginMinute=time.minute();
}
/**
 * @brief CSchceduleDlg::setStartTimeHour
 * @param startTime
 * 设置开始时间的小时
 */
void  CSchceduleDlg::setStartTimeHour(const QString startTime)
{
     QTime time =QTime::fromString(startTime,"h:mm");
     m_beginHour=time.hour();
}
/**
 * @brief CSchceduleDlg::setEndTimeHour
 * @param startTime
 * 设置结束时间的小时
 */
void CSchceduleDlg::setEndTimeHour(const QString endTime)
{
     QTime time =QTime::fromString(endTime,"h:mm");
     m_endHour=time.hour();
}
/**
 * @brief CSchceduleDlg::setEndTimeMinute
 * @param startTime
 * 设置结束时间的分钟
 */
void CSchceduleDlg::setEndTimeMinute(const QString endTime)
{
     QTime time =QTime::fromString(endTime,"h:mm");
     m_endMinute=time.minute();
}
void CSchceduleDlg::setEndTime(const QString &endTime)
{

    m_endTime = QTime::fromString(endTime,"hh:mm");
    setEndTimeHour(endTime);
    setEndTimeMinute(endTime);
}

//检查手动输入结束时间，结束时间小于开始时间弹出警告框，点击确认后结束时间恢复为比开始时间晚半小时的
void CSchceduleDlg::updateEndTimeValidation() {
    QString beginTimeStr = m_beginTimeEdit->currentText();
    QString endTimeStr = m_endTimeEdit->currentText();
    QTime userEndTime = QTime::fromString(endTimeStr, "hh:mm");
    QTime selectedStartTime = QTime::fromString(beginTimeStr, "hh:mm");
    QTime halfAnHourLater = selectedStartTime.addSecs(30 * 60);
    QDateTime beginDateTime = m_beginDateEdit->dateTime();
    QDateTime endDateTime = m_endDateEdit->dateTime();
    // 检查结束时间是否早于开始时间
    if(beginDateTime.date() < endDateTime.date()){
    }
    else
    {
        if(tanchuang==false){
            tanchuang = true;
            if (userEndTime.isValid() && userEndTime < m_startTime) {
                CustomMessageBox *msgBox = new CustomMessageBox(0,this);
                connect(msgBox, &CustomMessageBox::yesClicked, [&](){
                    msgBox->accept();
                });
                connect(msgBox, &CustomMessageBox::noClicked, [&](){
                    msgBox->accept();
                });
                //disconnect(m_endTimeEdit, QOverload<int>::of(&QComboBox::activated),this,&CSchceduleDlg::updateEndTimeValidation);
                //disconnect(m_endTimeEdit->lineEdit(), &QLineEdit::editingFinished, this,&CSchceduleDlg::updateEndTimeValidation);
                QString defaultEndTimeStr = halfAnHourLater.toString("hh:mm");
                m_endTimeEdit->setCurrentText(defaultEndTimeStr);
                msgBox->exec();

            }
        }else{
            tanchuang = false;
            return ;
        }
    }

    // 当开始时间发生变化时触发
    QObject::connect(m_beginTimeEdit, QOverload<const QString &>::of(&QComboBox::currentTextChanged), [=]() {
        isStartTimeChanged = true; // 标记开始时间已改变
        m_startTime = QTime::fromString(m_beginTimeEdit->lineEdit()->text(), "hh:mm");
        QTime halfAnHourLater = m_startTime.addSecs(30 * 60);
        QString defaultEndTimeStr = halfAnHourLater.toString("hh:mm");
        m_endTimeEdit->setCurrentText(defaultEndTimeStr);
    });

    // 当结束时间从下拉框中选择时触发
    connect(m_endTimeEdit, QOverload<int>::of(&QComboBox::activated), [=](int) {
        updateEndTimeValidation();
    });
    connect(m_endTimeEdit->lineEdit(), &QLineEdit::editingFinished, [=]() {
        if (isStartTimeChanged == true && tanchuang == false) { // 检查开始时间是否刚被改变
            updateEndTimeValidation();
        }
    });
}

/**
 * @brief CSchceduleDlg::updateEndTime
 * @param startTime
 * 当选择完开始时间，结束时间要始终比开始时间晚半小时，结束时间下拉框更新为只显示开始时间之后的时间
 */
void CSchceduleDlg::updateEndTime(const QString &startTime)
{
    m_startTime= QTime::fromString(startTime,"hh:mm");
    setStartTimeMinute(startTime);
    setStartTimeHour(startTime);
    //这个参数可以用户设置提醒时间
    QTime selectedStartTime = QTime::fromString(startTime, "hh:mm");
    QTime halfAnHourLater = selectedStartTime.addSecs(30 * 60);

    m_endTimeEdit->clear();
    for (int hour = 0; hour <= 23; ++hour) {
        for (int minute = 0; minute <= 59; ++minute) {
            QString timeStr = QStringLiteral("%1:%2").arg(hour, 2, 10, QLatin1Char('0')).arg(minute, 2, 10, QLatin1Char('0'));
            QTime timeValue = QTime::fromString(timeStr, "hh:mm");

            // 只添加比开始时间晚半小时及以后的时间
            if (timeValue >= selectedStartTime) {
                m_endTimeEdit->addItem(timeStr);
            }
        }
    }

    //更改开始时间后，输入结束时间，点击回车保存时确认合法性
    QObject::connect(m_endTimeEdit, QOverload<const QString &>::of(&QComboBox::activated), [=]() {
        updateEndTimeValidation();
    });

    // 设置结束时间下拉框的当前选择为比开始时间晚半小时的时间（如果有）
    QString endTimeStr = halfAnHourLater.toString("hh:mm");
    int index = m_endTimeEdit->findText(endTimeStr);
    if (index != -1) {
        m_endTimeEdit->setCurrentIndex(index);
    }

    connect(m_endTimeEdit,&QComboBox::currentTextChanged,this,&CSchceduleDlg::setEndTime);
}

void CSchceduleDlg::validateEndTimeFormat1() {
    qDebug() << "时间错了";

    const auto inputText = m_beginTimeEdit->currentText();
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString currentEndTimeText = currentDateTime.time().toString("HH:mm"); // 只获取当前时间部分

    if(tanchuang == false){
        tanchuang = true;
//        if (!inputText.isEmpty()) {
            QTime time = QTime::fromString(inputText, "HH:mm");
            if (!time.isValid() || time.hour() > 23 || time.minute() > 59) {
                CustomMessageBox *msgBox = new CustomMessageBox(2,this);
                connect(msgBox, &CustomMessageBox::yesClicked, [&](){
                   msgBox->accept();
                });
                connect(msgBox, &CustomMessageBox::noClicked, [&](){
                   msgBox->accept();
                });
                tanchuang = true;
                m_beginTimeEdit->clearEditText();
                m_beginTimeEdit->setFocus();

                // 设置为当前时间
                m_beginTimeEdit->addItem(currentEndTimeText);
                m_beginTimeEdit->setCurrentText(currentEndTimeText); // 设置为当前选中项
                QString beginTimeStr = m_beginTimeEdit->currentText();
                QTime selectedStartTime = QTime::fromString(beginTimeStr, "hh:mm");
                QTime halfAnHourLater = selectedStartTime.addSecs(30 * 60);
                QString defaultEndTimeStr = halfAnHourLater.toString("hh:mm");
                m_endTimeEdit->setCurrentText(defaultEndTimeStr);
                msgBox->exec();
//                QMessageBox::warning(nullptr, tr("Warning"), tr("The time entered is not in the correct format, please make sure that the hours are between 0-23 and the minutes are between 0-59."));

            }
//        }else{
//            tanchuang = false;
//            return;
//        }
    }
    tanchuang = false;
}

void CSchceduleDlg::validateEndTimeFormat2() {
    const auto inputText = m_endTimeEdit->currentText();
    if(tanchuang == false){
        tanchuang = true;
//        if (!inputText.isEmpty()) {
            QTime time = QTime::fromString(inputText, "HH:mm");
            if (!time.isValid() || time.hour() > 23 || time.minute() > 59) {
                CustomMessageBox *msgBox = new CustomMessageBox(2,this);
                connect(msgBox, &CustomMessageBox::yesClicked, [&](){
                   msgBox->accept();
                });
                connect(msgBox, &CustomMessageBox::noClicked, [&](){
                   msgBox->accept();
                });
                m_endTimeEdit->clearEditText();
                m_endTimeEdit->setFocus();
                tanchuang = true;
                // 设置为当前时间
                QString beginTimeStr = m_beginTimeEdit->currentText();
                QTime selectedStartTime = QTime::fromString(beginTimeStr, "hh:mm");
                QTime halfAnHourLater = selectedStartTime.addSecs(30 * 60);
                QString defaultEndTimeStr = halfAnHourLater.toString("hh:mm");
                m_endTimeEdit->setCurrentText(defaultEndTimeStr);
                tanchuang = false;
                msgBox->exec();
//                QMessageBox::warning(nullptr, tr("Warning"), tr("The time entered is not in the correct format, please make sure that the hours are between 0-23 and the minutes are between 0-59."));

            }
//        }else{
//            tanchuang = false;
//            return;
//        }
    }
     tanchuang = false;
}
/**
 * @brief CSchceduleDlg::initWindow
 * 绘制整改弹窗，按设计稿重修绘制弹窗部分
 */
void CSchceduleDlg::initWindow()
{
    this->setWindowIcon(QIcon::fromTheme("lingmo-calendar", QIcon(":/png/png/calendar_32.png")));
    this->setWindowTitle(tr("Add Schedule"));
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog); //加个QDialog
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(444,524);
    // 设置日程弹出框的阴影边框;
    auto shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0, 0);
    shadowEffect->setColor(Qt::gray);
    shadowEffect->setBlurRadius(SHADOW_WIDTH);
    setGraphicsEffect(shadowEffect);
    this->mainWidget = new QWidget(this);
    //mainWidget->setFixedSize(424,504);
    mainWidget->setFixedWidth(424);
    mainWidget->setMinimumHeight(504);
    vlayout = new QVBoxLayout();

    //添加日历图标图片
    if(m_type == 0){
        m_typeLabel = new QLabel(tr("Edit Event"),this);
    }else{
        m_typeLabel = new QLabel(tr("New Event"), this);
    }
//    m_appPicture = new QPushButton(this); //窗体左上角标题图片
//    m_appPicture->setIcon(QIcon::fromTheme("lingmo-calendar", QIcon(":/png/png/calendar_32.png")));
//    m_appPicture->setFixedSize(25, 25);
//    m_appPicture->setIconSize(QSize(25, 25)); //重置图标大小

    m_appPicture = new QLabel(this); //窗体左上角标题图片
    QIcon icon =QIcon::fromTheme("lingmo-calendar", QIcon(":/png/png/calendar_32.png"));
    QPixmap pixmap = icon.pixmap(QSize(25,25));
    m_appPicture->setPixmap(pixmap);

    //关闭按钮
    m_closeButton = new QToolButton(this);
    m_closeButton->setFixedSize(30, 30);
    m_closeButton->setToolTip(tr("Close"));
    m_closeButton->setIcon(QIcon::fromTheme("window-close-symbolic")); //主题库的叉子图标
    m_closeButton->setProperty("isWindowButton", 0x2);
    m_closeButton->setProperty("useIconHighlightEffect", 0x8);
    m_closeButton->setAutoRaise(true);
    connect(m_closeButton, &QToolButton::clicked, [=]() {
        this->deleteLater();
    });

    m_appPicture->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_appPicture->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QHBoxLayout *titlelayout = new QHBoxLayout();
    QWidget *titlewidget = new QWidget(this);
    titlelayout->addWidget(m_appPicture);
    titlelayout->addSpacing(2);
    titlelayout->addWidget(m_typeLabel);
    titlelayout->addWidget(m_closeButton);
    //titlelayout->addSpacing(8); //内部按钮间距8px
    titlelayout->setContentsMargins(0, 8, 0, 0);
    titlewidget->setLayout(titlelayout);
    //titlewidget->setFixedWidth(400);
    titlewidget->setFixedSize(400,40);

    QHBoxLayout *contentLabellayout = new QHBoxLayout();
    m_textEdit = new QTextEdit(this);
    m_textEdit->setFixedSize(378, 90);
    m_textEdit->setAcceptRichText(false);
    //设置关联控件，用于QTextEdit控件捕获MouseButtonPress等事件
//    QWidget *mpContentWidget = m_textEdit->viewport();
    //这一行创建了一个指向m_textEdit的视口（viewport）的指针。在Qt
    //中，视口是负责绘制控件内容并处理输入事件的窗口部件。QTextEdit有一个视口，它负责显示文本并处理与文本编辑相关的输入事件，如鼠标点击、键盘输入等。
//    mpContentWidget->installEventFilter(this);
    m_textEdit->setWordWrapMode(QTextOption::WrapAnywhere);
    //设置事件过滤器
//    m_textEdit->installEventFilter(this);
    contentLabellayout->addWidget(m_textEdit);
    // 如果是占位符文本
    m_textEdit->setPlaceholderText(tr("Enter the content of the event"));
    // 连接textChanged信号和槽函数
    connect(m_textEdit, &QTextEdit::textChanged,this,&CSchceduleDlg::slotTextChanged, Qt::QueuedConnection );
    //添加默认显示文案
    //添加全天和农历选择框
    QHBoxLayout *alldayLabellayout = new QHBoxLayout();
    alldayLabellayout->setSpacing(0);
    alldayLabellayout->setMargin(0);
    allDayLabel = new QLabel();
    allDayLabel->setText(tr("All Day"));
    allDayLabel->setFixedSize(65,25);
    allDaycheckBox = new QCheckBox(this);
    allDaycheckBox->setFixedSize(18,18);
    allDaycheckBox->installEventFilter(this);
//    m_ifLunarLabel = new QLabel(tr("Lunar"));

//    m_ifLunarLabel->setFixedSize(38,19);
//    m_ifLunarcheckBox = new QCheckBox(this);
//    m_ifLunarcheckBox->setFixedSize(16,16);
//    m_ifLunarcheckBox->installEventFilter(this);
    alldayLabellayout->addWidget(allDaycheckBox);
    alldayLabellayout->addSpacing(10);
    alldayLabellayout->addWidget(allDayLabel);
//    alldayLabellayout->addSpacing(31);
//    alldayLabellayout->addWidget(m_ifLunarcheckBox);
//    alldayLabellayout->addSpacing(8);
    //alldayLabellayout->addWidget(m_ifLunarLabel);
    alldayLabellayout->setContentsMargins(102,0,0,0);
    alldayLabellayout->addStretch();

    QHBoxLayout *beginLabellayout = new QHBoxLayout();
    beginLabellayout->setSpacing(0);
    beginLabellayout->setMargin(0);
    m_beginTimeLabel = new QLabel(tr("Starts:"));
    m_beginTimeLabel->setContentsMargins(15,0,0,0);
    m_beginTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_beginTimeLabel->setFixedWidth(75);

    m_beginDateEdit = new DateTimeEdit(this);
    m_beginDateEdit->setFixedSize(140, 36);
    m_beginDateEdit->setCalendarPopup(true);
    m_beginDateEdit->setDisplayFormat("yyyy-MM-dd");
    // 假设你已经有了QDateEdit的指针，名为dateEdit
    QDateTime currentDateTime = QDateTime::currentDateTime();
    // 设置最小日期时间
    m_beginDateEdit->setMinimumDateTime(currentDateTime);

    // 设置日期时间范围，不允许选择早于当前的时间
    m_beginDateEdit->setDateTimeRange(currentDateTime, QDateTime::currentDateTime().addDays(365)); // 或者使用QDateTime::max()来允许选择任意未来时间

    // 更新界面
    m_beginDateEdit->update();


    //解决开始时间显示不全问题
    m_beginDateEdit->setDate(m_date);
    //重新绘制time选择下拉框
    m_beginTimeEdit = new QComboBox(this);
    m_beginTimeEdit->installEventFilter(this);
    m_beginTimeEdit->setFixedSize(135, 36);
    m_beginTimeEdit->setEditable(true);


    //重新绘制time选择下拉框
    m_endTimeEdit = new QComboBox(this);
    m_endTimeEdit->setFixedSize(135, 36);
    m_endTimeEdit->setEditable(true);

    connect(m_beginTimeEdit, &QComboBox::customContextMenuRequested, this, &CSchceduleDlg::handleComboBoxDoubleClick);
    connect(m_endTimeEdit, &QComboBox::customContextMenuRequested, this, &CSchceduleDlg::handleComboBoxDoubleClick);


    beginLabellayout->addWidget(m_beginTimeLabel);
    beginLabellayout->addSpacing(28);
    beginLabellayout->addWidget(m_beginDateEdit);
    beginLabellayout->addSpacing(11);
    beginLabellayout->addWidget(m_beginTimeEdit);
    beginLabellayout->addStretch();

    QHBoxLayout *enQLabellayout = new QHBoxLayout();
    enQLabellayout->setSpacing(0);
    enQLabellayout->setMargin(0);
    m_endTimeLabel = new QLabel(tr("Ends:"));
    m_endTimeLabel->setContentsMargins(15,0,0,0);
    m_endTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_endTimeLabel->setFixedWidth(75);

    m_endDateEdit = new DateTimeEdit(this);
    m_endDateEdit->setFixedSize(140, 36);
    m_endDateEdit->setCalendarPopup(true);
    m_endDateEdit->setDisplayFormat("yyyy-MM-dd");

    //解决开始时间显示不全问题
    m_endDateEdit->setDate(m_date);
    QDateTime newStartTime = m_beginDateEdit->dateTime();
    // 保持结束时间的小时、分钟和秒不变，仅更新日期部分
    QDateTime newEndTime = QDateTime(newStartTime.date(), m_endDateEdit->time());
    m_endDateEdit->setDateTime(newEndTime);
    m_endDateEdit->setMinimumDateTime(m_beginDateEdit->dateTime());
    addPeriodOfTime();
    // 连接开始时间更改信号到槽函数,保证结束时间自动更新为和开始时间一样，并不能早于开始时间
    QObject::connect(m_beginDateEdit, &QDateTimeEdit::dateTimeChanged, [=]() {
        QDateTime newStartTime = m_beginDateEdit->dateTime();
        // 保持结束时间的小时、分钟和秒不变，仅更新日期部分
        QDateTime newEndTime = QDateTime(newStartTime.date(), m_endDateEdit->time());
        m_endDateEdit->setDateTime(newEndTime);
        m_endDateEdit->setMinimumDateTime(m_beginDateEdit->dateTime());
    });

    enQLabellayout->addWidget(m_endTimeLabel);
    enQLabellayout->addSpacing(28);
    enQLabellayout->addWidget(m_endDateEdit);
    enQLabellayout->addSpacing(11);
    enQLabellayout->addWidget(m_endTimeEdit);
    enQLabellayout->addStretch();


    QHBoxLayout *rminQLabellayout = new QHBoxLayout();
    rminQLabellayout->setSpacing(0);
    rminQLabellayout->setMargin(0);
    m_remindSetLabel = new QLabel(this);
    m_remindSetLabel->setText(tr("Remind Me:"));
    m_remindSetLabel->setContentsMargins(15,0,0,0);
//    m_remindSetLabel->setStyleSheet(QStringLiteral("color: balck;"));
    m_remindSetLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_remindSetLabel->setFixedWidth(75);

    m_rmindCombox = new QComboBox(this);
    m_rmindCombox->setFixedSize(220, 36);
    m_rmindCombox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_rmindComboxallday = new QComboBox(this);
    m_rmindComboxallday->setFixedSize(220, 36);
    m_rmindComboxallday->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    rminQLabellayout->addWidget(m_remindSetLabel);
    rminQLabellayout->setSpacing(28);
    rminQLabellayout->addWidget(m_rmindCombox);
    rminQLabellayout->addWidget(m_rmindComboxallday); // 注意：这里也需要添加m_rmindComboxallday，但设置为不可见
    rminQLabellayout->addStretch();
    m_rmindComboxallday->setVisible(false);
    m_rmindCombox->setVisible(true);
    addRemindMe();

    connect(allDaycheckBox, &QCheckBox::stateChanged, [=](int state){
        if (state == Qt::Checked) {
            m_rmindComboxallday->clear();
            m_rmindCombox->setVisible(false);
            m_rmindComboxallday->setVisible(true);
            addRemindMeallday();
            } else {
            m_rmindCombox->clear();
            m_rmindComboxallday->setVisible(false);
            m_rmindCombox->setVisible(true);
            addRemindMe();
            }
    });

    QHBoxLayout *repeatLabellayout = new QHBoxLayout();
    repeatLabellayout->setSpacing(0);
    repeatLabellayout->setMargin(0);
    m_beginrepeatLabel = new QLabel(tr("Repeat:"));
    m_beginrepeatLabel->setContentsMargins(15,0,0,0);
    m_beginrepeatLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_beginrepeatLabel->setFixedWidth(75);

    m_repeatCombox = new QComboBox(this);
    m_repeatCombox->setFixedSize(220, 36);
    m_repeatCombox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_repeatCombox->setFixedSize(220,36);
    m_repeatCombox->addItem(tr("No Repeat"));
    m_repeatCombox->addItem(tr("Daily"));
    m_repeatCombox->addItem(tr("Weekly"));
    m_repeatCombox->addItem(tr("Monthly"));
    m_repeatCombox->addItem(tr("Yearly"));
    setRRuleType(RRuleType::RRule_None);
    CalendarDataBase::getInstance().RRuleType=0;
    connect(m_repeatCombox,&QComboBox::currentTextChanged,this,&CSchceduleDlg::slotfrequencyBox);

    repeatLabellayout->addWidget(m_beginrepeatLabel);
    repeatLabellayout->setSpacing(28);
    repeatLabellayout->addWidget(m_repeatCombox);
    repeatLabellayout->addStretch();
    //repeatLabellayout->addWidget(repeatButton);

    frequencyLayout = new QHBoxLayout();
    frequencyLabel = new QLabel(this);
    frequencyBox = new QComboBox(this);

    frequencyBox->setDisabled(false);
    frequencyBox->addItem(tr("Infinite repetition"));
    frequencyBox->addItem(tr("Customize"));
    frequencytimeBox = new DateTimeEdit(this);
    frequencytimeBox->setFixedSize(145, 36);
    frequencytimeBox->setCalendarPopup(true);
    frequencytimeBox->setDisplayFormat("yyyy-MM-dd");
    // 初始化完成后立即更新结束时间
    updateEndDateFromBeginDate();
    // 然后再连接开始时间控件的dateTimeChanged信号
    QObject::connect(m_beginDateEdit, &QDateTimeEdit::dateTimeChanged, this, [=]() {
        updateEndDateFromBeginDate();
    });

    frequencyBox->setVisible(false);  //默认不显示
    frequencyLabel->setVisible(false);  //默认不显示
    frequencytimeBox->setVisible(false);  //默认不显示

    frequencyLabel->setText(tr("Frequency:"));
    frequencyLabel->setFixedWidth(95);
    frequencyLabel->setContentsMargins(15,0,0,0);
    frequencyLayout->addWidget(frequencyLabel);
    frequencyLayout->addSpacing(0);
    frequencyLayout->addWidget(frequencyBox);
    frequencyLayout->addSpacing(0);
    frequencyLayout->addWidget(frequencytimeBox);
    frequencyLayout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    //调整确认取消按钮布局排列及按钮三态
    queding = new QPushButton(tr("Save"));
    queding->setFixedSize(108,36);
    connect(queding,&QPushButton::clicked,this,&CSchceduleDlg::slotConfirmClick);

    quxiao = new QPushButton(tr("Cancel"));
    quxiao->setFixedSize(108,36);
    connect(quxiao,&QPushButton::clicked,[&](){
         this->deleteLater();
    });

    buttonLayout->setContentsMargins(0, 0, 0, 0); // 清除左右两侧的默认内边距
    buttonLayout->setAlignment(Qt::AlignRight); // 设置整体布局右对齐
    // 让按钮之间有18像素间距
    QSpacerItem *spacer = new QSpacerItem(18, 0, QSizePolicy::Fixed, QSizePolicy::Ignored);
    buttonLayout->addItem(spacer);
    buttonLayout->addWidget(quxiao);
    buttonLayout->addWidget(queding);

    vlayout->addWidget(titlewidget);
    vlayout->addSpacing(20);
    vlayout->addLayout(contentLabellayout);
    vlayout->addSpacing(16);
    vlayout->addLayout(alldayLabellayout);
    vlayout->addSpacing(10);
    vlayout->addLayout(beginLabellayout);
    vlayout->addLayout(enQLabellayout);
    vlayout->addLayout(rminQLabellayout);
    vlayout->addLayout(repeatLabellayout);
    vlayout->addLayout(frequencyLayout);
    vlayout->addSpacing(30);
    vlayout->addLayout(buttonLayout);
    vlayout->setContentsMargins(15, 15, 15, 15);

    //设置重复时，才显示结束重复
    connect(m_repeatCombox, qOverload<const QString &>(&QComboBox::currentTextChanged),
            this, &CSchceduleDlg::onRepeatComboxTextChanged);
    connect(frequencyBox, qOverload<const QString &>(&QComboBox::currentTextChanged),
            this, &CSchceduleDlg::onfrequencyComboxTextChanged);

    mainWidget->setLayout(vlayout);
    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(mainWidget);
    hlayout->setSizeConstraint(QLayout::SetDefaultConstraint);
    setLayout(hlayout);
    this->adjustSize();
//    setStyleSheet(".QWidget{border-top-left-radius:5px;border-top-right-radius:5px;border-bottom-left-radius:5px;"
//                  "border-bottom-right-radius:5px;}");


}

//设置自定义结束重复日期始终是开始日期的后一天
void CSchceduleDlg::updateEndDateFromBeginDate() {
    // 同样获取开始时间并计算结束时间，然后设置到frequencytimeBox
    QDateTime beginDateTime = m_beginDateEdit->dateTime();
    QDate nextDay = beginDateTime.date().addDays(1);
    QDateTime endDate = QDateTime(nextDay, beginDateTime.time());
    frequencytimeBox->setDateTime(endDate);
    frequencytimeBox->setMinimumDateTime(m_beginDateEdit->dateTime());
}

//实现双击时间进入编辑状态
void CSchceduleDlg::handleComboBoxDoubleClick(const QPoint &pos)
    {
        //if (m_beginTimeEdit->view()->rect().contains(pos))
        //    return; // 如果双击在下拉列表内，忽略此次事件

        m_beginTimeEdit->lineEdit()->selectAll();
        m_beginTimeEdit->lineEdit()->setFocus(Qt::OtherFocusReason);
    }


//重写开始时间和结束时间精确到分钟，并获取当前时间为默认开始时间，结束时间默认比开始时间晚半小时
void CSchceduleDlg::addPeriodOfTime()
{
    //初始化不改变开始时间时更改结束时间立即开始检查合法性
    updateEndTimeValidation();
    QDateTime now = QDateTime::currentDateTime();
    QTime currentTime = now.time();
    m_beginHour = currentTime.hour();//设置默认
    m_beginMinute = currentTime.minute();//设置默认
    for (int hour = 0; hour <= 23; ++hour) {
        for (int minute = 0; minute <= 59; ++minute) {
            QString timeStr = QStringLiteral("%1:%2").arg(hour, 2, 10, QLatin1Char('0')).arg(minute, 2, 10, QLatin1Char('0'));
            int index = m_beginTimeEdit->count();
            m_beginTimeEdit->addItem(timeStr);
            if (currentTime.hour() == hour && currentTime.minute() == minute) {
                m_beginTimeEdit->setCurrentIndex(index);
            }
        }
    }

    QObject::connect(m_beginTimeEdit, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        QString selectedTime = m_beginTimeEdit->itemText(index);
        QTime selectedTimeValue = QTime::fromString(selectedTime, "hh:mm");
        QDateTime selectedDateTime = QDateTime::currentDateTime().addSecs(selectedTimeValue.secsTo(QTime::currentTime()));
        // 更新显示当前选中时间的控件，比如：
        QTimeEdit timeEdit;
        timeEdit.setDisplayFormat("hh:mm");
        timeEdit.setTime(selectedDateTime.time());
    });

    //设置默认结束时间比开始时间晚半小时
    QTime defaultEndTime = currentTime.addSecs(30 * 60);
    m_endHour = defaultEndTime.hour(); //设置默认
    m_endMinute = defaultEndTime.minute();//设置默认
    // 初始化结束时间下拉框并设置默认选中项
    for (int hour = 0; hour <= 23; ++hour) {
        for (int minute = 0; minute <= 59; ++minute) {
            QString timeStr = QStringLiteral("%1:%2").arg(hour, 2, 10, QLatin1Char('0')).arg(minute, 2, 10, QLatin1Char('0'));
            int index = m_endTimeEdit->count();
            m_endTimeEdit->addItem(timeStr);
            if (defaultEndTime.hour() == hour && defaultEndTime.minute() == minute) {
                m_endTimeEdit->setCurrentIndex(index);
                break;
            }
        }
    }

    tanchuang = false;
    QObject::connect(m_beginTimeEdit->lineEdit(), &QLineEdit::editingFinished, [=]()  {
        tanchuang = false;
        validateEndTimeFormat1();
    });
    QObject::connect(m_endTimeEdit->lineEdit(), &QLineEdit::editingFinished, [=]() {
        tanchuang = false;
        validateEndTimeFormat2();
    });
    connect(m_beginTimeEdit,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(updateEndTime(const QString&)));
    connect(m_endTimeEdit,&QComboBox::currentTextChanged,this,&CSchceduleDlg::setEndTime);
    //connect(m_beginTimeEdit,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(validateEndTimeFormat()));
    //connect(m_endTimeEdit,SIGNAL(currentIndexChanged(const QString&)),this,SLOT(updateBeginTime(const QString&)));
}

//判断重复选项是否变化为重复，重复时出现结束重复layout
void CSchceduleDlg::onRepeatComboxTextChanged(const QString &text)
{
    bool showFrequencyLayout = (text != "不重复"); // 当选项不是"不重复"时显示frequencyLayout
    frequencyBox->setVisible(showFrequencyLayout);
    frequencyLabel->setVisible(showFrequencyLayout);
    frequencyBox->setFixedSize(220, 36);

    //解决显示frequencyLayout后弹窗窗口高度增加，不可见时恢复默认
    if (showFrequencyLayout) {
            QTimer::singleShot(0, this, [=](){
                mainWidget->setFixedHeight(546);
                this->setFixedHeight(564);
               //                this->adjustSize();

            });
        } else {
            QTimer::singleShot(0, this, [=](){
                mainWidget->adjustSize();
                mainWidget->setFixedHeight(504);
                //this->resize(444,524);
                this->setFixedHeight(524);
                this->adjustSize();
                frequencytimeBox->setVisible(false);
            });
        }
}

//判断结束重复选项是否变化为自定义，自定义时出现时间box
void CSchceduleDlg::onfrequencyComboxTextChanged(const QString &text)
{
    bool showFrequencyLayout = (text == tr("Customize")); // 当选项不是"不重复"时显示frequencyLayout

    frequencytimeBox->setVisible(showFrequencyLayout);
    frequencyBox->setFixedSize(135,36);
}

/**
 * @brief CSchceduleDlg::slotfrequencyBox
 * @param text
 * 设置重复规则的
 */
void CSchceduleDlg::slotfrequencyBox(const QString &text)
{

    if(text.compare(tr("不重复")) == 0){
        CalendarDataBase::getInstance().RRuleType=0;
        setRRuleType(RRule_None);
    }
    else if(text.compare(tr("Daily"))==0){
        CalendarDataBase::getInstance().RRuleType=1;
        setRRuleType(RRule_Day);
    }
    else if(text.compare(tr("Weekly"))==0){
        CalendarDataBase::getInstance().RRuleType=2;
        setRRuleType(RRule_Week);
    }
    else if(text.compare(tr("Monthly"))==0){
        CalendarDataBase::getInstance().RRuleType=3;
        setRRuleType(RRule_Month);
    }
    else if(text.compare(tr("Yearly"))==0){
        CalendarDataBase::getInstance().RRuleType=4;
        setRRuleType(RRule_Year);
    }
}
/**
 * @brief CSchceduleDlg::slotConfirmClick
 * 点击"确定"，保存当前日程到数据库 并创建QTimer ，到时间后调用通知DBus信号发送通知
 */
void CSchceduleDlg::slotConfirmClick(){
    if(m_textEdit->toPlainText().trimmed().isEmpty()){
       m_textEdit->setText(tr("Enter the content of the event"));
    }
    QString uniqueId = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");//日程id为14位
    //QString uniqueId = m_info.m_markId;
    qDebug()<<"日程的唯一id是"<<uniqueId;
    AlarmType currentAlarmType = getAlarmType();
    RRuleType currentRRuleType = getRRuleType();
    MarkInfo info;
    info.m_markId = uniqueId;
    info.m_descript=  m_textEdit->toPlainText();
    info.minute =info.Alarm_end_minute= m_beginMinute;
    info.hour=info.Alarm_end_hour=m_beginHour;
    info.day= m_beginDateEdit->date().day();

    info.week = m_beginDateEdit->date().dayOfWeek();
    info.month = m_beginDateEdit->date().month();
    info.end_day = m_endDateEdit->date().day();
    info.end_month = m_endDateEdit->date().month();
    info.m_markStartDate = m_beginDateEdit->date();
    info.m_markEndDate = m_endDateEdit->date();
    info.end_hour = m_endHour;
    info.end_minute = m_endMinute;
    info.startTime = m_startTime;
    info.endTime = m_endTime;
    info.beginDateTime = m_beginDateEdit->dateTime();
    info.endDateTime = m_endDateEdit->dateTime();
    info.m_isAllDay = allDaycheckBox->isChecked();
    int secs1 = m_startTime.secsTo(QTime(0, 0, 0));
    int secs2 = m_endTime.secsTo(QTime(0, 0, 0));
    int timeDiffSecs = secs2 - secs1;
    // 转换为小时、分钟和秒
    int hours = timeDiffSecs / 3600;
    int minutes = (timeDiffSecs % 3600) / 60;
    timeLong = QString("%1 hours,%2 minutes").arg(hours).arg(minutes);
    /**
      开始的时候提醒
      */
    if(currentAlarmType == 1){
    }
    /**
      5分钟前提醒
      */
    // 根据提前的时间调整开始时间
    else if (currentAlarmType == 2) {
        if (m_beginMinute >= 5) {
            info.Alarm_end_minute = m_beginMinute - 5;
        } else {
            // 处理分钟数变为负数的情况
            info.Alarm_end_minute = m_beginMinute + 55; // 60分钟减去提前的5分钟
            if (m_beginHour > 0) {
                info.Alarm_end_hour = m_beginHour - 1;
            } else {
                info.Alarm_end_hour = 23; // 如果小时为0，则调整为23
                // 还需要调整日期
                QDate adjustedDate = m_beginDateEdit->date().addDays(-1);
                info.day = adjustedDate.day();
                info.month = adjustedDate.month();
            }
        }
    }
    /**
       10分钟前提醒
       */
     else if (currentAlarmType == 3) {
         if (m_beginMinute >= 10) {
             info.Alarm_end_minute = m_beginMinute - 10;
         } else {
             // 处理分钟数变为负数的情况
             info.Alarm_end_minute = m_beginMinute + 50; // 60分钟减去提前的10分钟
             if (m_beginHour > 0) {
                 info.Alarm_end_hour = m_beginHour - 1;
             } else {
                 info.Alarm_end_hour = 23; // 如果小时为0，则调整为23
                 // 还需要调整日期
                 QDate adjustedDate = m_beginDateEdit->date().addDays(-1);
                 info.day = adjustedDate.day();
                 info.month = adjustedDate.month();
             }
         }
     }
    /**
      15分钟前提醒
      */
    else if (currentAlarmType == 4) {
        if (m_beginMinute >= 15) {
            info.Alarm_end_minute = m_beginMinute - 15;
        } else {
            // 处理分钟数变为负数的情况
            info.Alarm_end_minute = m_beginMinute + 45; // 60分钟减去提前的15分钟
            if (m_beginHour > 0) {
                info.Alarm_end_hour = m_beginHour - 1;
            } else {
                info.Alarm_end_hour = 23; // 如果小时为0，则调整为23
                // 还需要调整日期
                QDate adjustedDate = m_beginDateEdit->date().addDays(-1);
                info.day = adjustedDate.day();
                info.month = adjustedDate.month();
            }
        }
    }
    /**
      30分钟前提醒
      */
    else if (currentAlarmType == 5) {
        if (m_beginMinute >= 30) {
            info.Alarm_end_minute = m_beginMinute - 30;
        } else {
            // 处理分钟数变为负数的情况
            info.Alarm_end_minute = m_beginMinute + 30; // 60分钟减去提前的30分钟
            if (m_beginHour > 0) {
                info.Alarm_end_hour = m_beginHour - 1;
            } else {
                info.Alarm_end_hour = 23; // 如果小时为0，则调整为23
                // 还需要调整日期
                QDate adjustedDate = m_beginDateEdit->date().addDays(-1);
                info.day = adjustedDate.day();
                info.month = adjustedDate.month();
            }
        }
    }
    /**
      1小时前提醒
      */
    else if (currentAlarmType == 6) {
        if (m_beginHour >= 1) {
            info.Alarm_end_hour = m_beginHour - 1;
        } else {
            // 处理小时数变为负数的情况
            info.Alarm_end_hour = m_beginHour + 23; // 24小时减去提前的1小时
            // 还需要调整日期
            QDate adjustedDate = m_beginDateEdit->date().addDays(-1);
            info.day = adjustedDate.day();
            info.month = adjustedDate.month();
        }
    }
    /**
      1天前提醒
      */
    else if (currentAlarmType == 7) {
        // 处理提前一天的情况
        QDate adjustedDate = m_beginDateEdit->date().addDays(-1);
        info.day = adjustedDate.day();
        info.month = adjustedDate.month();
    }
    /**
          2天前提醒
          */
    else if (currentAlarmType == 8) {
        // 处理提前两天的情况
        QDate adjustedDate = m_beginDateEdit->date().addDays(-2);
        info.day = adjustedDate.day();
        info.month = adjustedDate.month();
    }
    /**
          1天前上午9点
          */
    else if (currentAlarmType == 9){
        QDate currentDate = m_beginDateEdit->date();
        QDateTime dateTime(currentDate.addDays(-1)); // 减少一天
        dateTime.setTime(QTime(9, 0)); // 设置时间为上午9点
        info.day = dateTime.date().day();
        info.month = dateTime.date().month();
        info.hour = 0; //因为是全天的 所以开始时间和结束时间就从00:00 - 23:59这样就避免全天的也会置灰现象
        info.minute = 0;
        info.end_hour = 23;
        info.end_minute = 59;
    }
    /**
          日程发生当天上午9点
          */
    else if (currentAlarmType == 10){
        QDate currentDate = m_beginDateEdit->date();
        QDateTime dateTime(currentDate); // 使用当前日期
        dateTime.setTime(QTime(9, 0)); // 设置时间为上午9点
        info.day = dateTime.date().day();
        info.month = dateTime.date().month();
        info.hour = 0;//因为是全天的 所以开始时间和结束时间就从00:00 - 23:59这样就避免全天的也会置灰现象
        info.minute = 0;
        info.end_hour = 23;
        info.end_minute = 59;
    }
    /**
          2天前上午9点
          */
    else if (currentAlarmType == 11){
        QDate currentDate = m_beginDateEdit->date();
        QDateTime dateTime(currentDate.addDays(-2)); // 减少一天
        dateTime.setTime(QTime(9, 0)); // 设置时间为上午9点
        info.day = dateTime.date().day();
        info.month = dateTime.date().month();
        info.hour = 0;//因为是全天的 所以开始时间和结束时间就从00:00 - 23:59这样就避免全天的也会置灰现象
        info.minute = 0;
        info.end_hour = 23;
        info.end_minute = 59;
    }
    /**
          1周前上午9点
          */
    else if (currentAlarmType == 12){
        QDate currentDate = m_beginDateEdit->date();
        QDateTime dateTime(currentDate.addDays(-7)); // 减少一天
        dateTime.setTime(QTime(9, 0)); // 设置时间为上午9点
        info.day = dateTime.date().day();
        info.month = dateTime.date().month();
        info.hour = 0;//因为是全天的 所以开始时间和结束时间就从00:00 - 23:59这样就避免全天的也会置灰现象
        info.minute = 0;
        info.end_hour = 23;
        info.end_minute = 59;
    }

    if(timemodel == "12")
    {
        if(info.hour <= 12){
            info.shangwu = "上午";
        }else{
            info.shangwu = "下午";
        }
    }
    else if(timemodel == "24")
    {
        info.shangwu = "";
    }

    info.m_isAllDay = allDaycheckBox->isChecked();
//    info.m_isLunar =m_ifLunarcheckBox->isChecked();
    info.remind = m_rmindCombox->currentText();
    info.AlldayRemind = m_rmindComboxallday->currentText();
    info.repeat = m_repeatCombox->currentText();
    info.beginrepeat = frequencyBox->currentText();
    info.endrepeat = frequencytimeBox->dateTime();
    if(info.m_descript.contains("\n")){
           qDebug()<<"包含\n,不符合crontab标准,消除所有的\n";
           info.m_descript = info.m_descript.replace("\n"," ");//和设计沟通过了 可以用空格
       }
       qDebug()<<"markInfo:"<<info.m_descript;


    if(m_type == 1){
        if(currentAlarmType ==0){
            CalendarDataBase::getInstance().insertDataSlot(info);
            CalendarDataBase::getInstance().searchDataSlot(info.m_markStartDate);
        }else{
            CalendarDataBase::getInstance().add(info); //crontab
            CalendarDataBase::getInstance().insertDataSlot(info);
            CalendarDataBase::getInstance().searchDataSlot(info.m_markStartDate);
        }




    }else if(m_type ==0){
        info.m_markId = idd;
        if(currentAlarmType ==0){
            CalendarDataBase::getInstance().update(info);
            CalendarDataBase::getInstance().updateDataSlot(info);
        }else{
            CalendarDataBase::getInstance().update(info);
            CalendarDataBase::getInstance().updateDataSlot(info);
        }


    }
//    this->accept(); //接受并关闭
//    delete this;
    this->deleteLater();
}
void CSchceduleDlg::setStartDate(const QDate &date)
{
    m_beginDateEdit->setDate(date);
}

void CSchceduleDlg::setEndDate(const QDate &date)
{
    m_endDateEdit->setDate(date);
}
QString CSchceduleDlg::getText()
{
    return m_textEdit->toPlainText();
}
void CSchceduleDlg::bindConnect()
{
    connect(allDaycheckBox, &QCheckBox::toggled, this, &CSchceduleDlg::setAllDaySlot);
    //connect(repeatButton, &SwitchButton::checkedChanged, this, &CSchceduleDlg::setRepeatSlot);
}

//设置打开全天，不能操作时间，且时间选择框置灰
void CSchceduleDlg::setAllDaySlot(bool isChecked)
{
    if (isChecked) {
        m_beginTimeEdit->setDisabled(true);
        m_endTimeEdit->setDisabled(true);
    } else {
        m_beginTimeEdit->setDisabled(false);
        m_endTimeEdit->setDisabled(false);
    }
}

void CSchceduleDlg::setRepeatSlot(bool isChecked)
{
    if (isChecked) {
        frequencyBox->setDisabled(false);


    } else {
        frequencyBox->setDisabled(true);
    }
}

void CSchceduleDlg::updateScheduleInfo(const MarkInfo &info)
{
    m_info = info;
    m_beginDateEdit->setDate(info.m_markStartDate);
    m_endDateEdit->setDate(info.m_markEndDate);
    m_textEdit->setText(info.m_descript);
    qDebug() << "更新开始时间:" << info.m_markStartDate << "更新结束时间:" << info.m_markEndDate
             << "更新信息:" << info.m_descript << "更新开始:" << info.m_markStartTime
             << "更新结束:" << info.m_markEndTime << "更新宽度:" << info.m_markWidth
             << "更新高度:" << info.m_markHeight;
}
/**
 * @brief CSchceduleDlg::Setinfo
 * 给弹窗设置info，这样可以弹出来一个已知的日程信息.
 */
void CSchceduleDlg::Setinfo(MarkInfo info)
{
   m_info = info;
}
void CSchceduleDlg::setId(QString idd)
{
    this->idd = idd;

}
//BUG:218784
void CSchceduleDlg::setStatusFromId(QString id)
{
   QString date;
   QString searchCmd =
       QString("select start_date from Schedule where id = '%1'").arg(id);
   QSqlQuery searchQuery;
   searchQuery.exec(searchCmd);
   if (!searchQuery.exec(searchCmd)) {
       qDebug() << "query error :" << searchQuery.lastError();

   } else {
       while (searchQuery.next()) {
           date = searchQuery.value(0).toString();
       }
   }
   QDate mubiaoDate = QDate::fromString(date,"yyyy-MM-dd");
   int x = QDate::currentDate().daysTo(mubiaoDate);
   CalendarDataBase::getInstance().searchDataSlot(QDate::currentDate().addDays(x));
   m_markInfoList = CalendarDataBase::getInstance().m_markInfoList;
   foreach (MarkInfo info, m_markInfoList) {
        if(info.m_markId == id){
            m_textEdit->setText(info.m_descript);
            m_beginDateEdit->setDateTime(info.beginDateTime);
            m_endDateEdit->setDateTime(info.endDateTime);

            QString starttimeString = QString::number(info.hour)+":"+QString::number(info.minute);// 确保这是有效的格式
            QTime m_startTime = QTime::fromString(starttimeString, "h:m");
            QString start = m_startTime.toString("hh:mm");
            int index = m_beginTimeEdit->findText(start);
            m_beginTimeEdit->setCurrentIndex(index);

            QString timeString = QString::number(info.end_hour)+":"+QString::number(info.end_minute);// 确保这是有效的格式
            QTime m_endTime = QTime::fromString(timeString, "h:m");
            QString end = m_endTime.toString("hh:mm");
            int index1 = m_endTimeEdit->findText(end);
            m_endTimeEdit->setCurrentIndex(index1);

            allDaycheckBox->setChecked(info.m_isAllDay);
//            m_ifLunarcheckBox->setChecked(info.m_isLunar);

            int index2 = m_rmindCombox->findText(info.remind);
            m_rmindCombox->setCurrentIndex(index2);

            int index3 = m_rmindComboxallday->findText(info.AlldayRemind);
            m_rmindComboxallday->setCurrentIndex(index3);

            int index4 = m_repeatCombox->findText(info.repeat);
            m_repeatCombox->setCurrentIndex(index4);
            if(index4 == 0){
                return;

            }
            int index5 = frequencyBox->findText(info.beginrepeat);
            frequencyBox->setCurrentIndex(index5);

//            m_beginDateEdit->setDateTime(info.beginDateTime);
//            m_endDateEdit->setDateTime(info.endDateTime);
            frequencytimeBox->setDateTime(info.endrepeat);
        }
   }
}
void CSchceduleDlg::setThemePalette()
{

}
void CSchceduleDlg::initWindowTheme()
{

}
QString CSchceduleDlg::escapeUrlToHtmlTags(QString &text)
{
    QString escapedText = text;

    QRegularExpression regExp(R"((?:https|http|ftp)://[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,}(?:[0-9]{1,5})?(?:/[\S]*)?)");
    QRegularExpressionMatchIterator iterator = regExp.globalMatch(text);
    while (iterator.hasNext()) {
        QStringList capturedTextList = iterator.next().capturedTexts();
        QString url = capturedTextList.value(0);

        escapedText.replace(url, websiteTemplate.arg(url, url));
    }

    text = escapedText;
    return text;
}
void CSchceduleDlg::slotTextChanged()
{
    QTextCursor cursor = m_textEdit->textCursor();
    int previousPosition = cursor.position();

    m_textEdit->setPlaceholderText(this->m_textEdit->toPlainText().isEmpty() ? tr("Enter the content of the event") : "");
    m_textEdit->blockSignals(true);

    QString plainText = m_textEdit->toPlainText();
    plainText.replace(" ","&nbsp;");
    plainText = escapeUrlToHtmlTags(plainText);
    plainText.replace("\n", "<br>");
    m_textEdit->setHtml(plainText);
//    m_textEdit->moveCursor(QTextCursor::End);
    cursor.setPosition(previousPosition);
    m_textEdit->setTextCursor(cursor);


    m_textEdit->blockSignals(false);
}
void CSchceduleDlg::onPaletteChanged()
{
    QPalette pal = qApp->palette();

    QGSettings * styleGsettings = nullptr;
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
       styleGsettings = new QGSettings(style_id);
       QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
       if(currentTheme == "lingmo-default" || currentTheme == "lingmo-light" ){
           QPalette pal ;
           pal = lightPalette(this);
           pal.setColor(QPalette::Background, pal.background().color());
           pal.setColor(QPalette::Base, pal.base().color());
           pal.setColor(QPalette::Text, pal.text().color());
           pal.setColor(QPalette::Button, pal.button().color());
           pal.setColor(QPalette::ButtonText, pal.buttonText().color());
           setFramePalette(this, pal);
           m_closeButton->setStyleSheet("QToolButton {"
                                        "color: black;"
                                        "}"
                                        "QToolTip {"
                                        "    background-color: #FFFFFF;"
                                        "    color: black;"
                                        "}");
       }
       if(currentTheme == "lingmo-dark")
       {
           QPalette palette = allDaycheckBox->palette();
           palette.setColor(QPalette::Button, QColor("#222222")); // 红色背景
//           m_ifLunarcheckBox->setPalette(palette);
           allDaycheckBox->setPalette(palette);

           m_closeButton->setStyleSheet("QToolButton {"
                                        "color: white;"
                                        "}"
                                        "QToolTip {"
                                        "    background-color: #FFFFFF;"
                                        "    color: white;"
                                        "}");
//           m_appPicture->setStyleSheet("QPushButton{border:0px;border-radius:4px;background:transparent;}");
//           m_ifLunarcheckBox->setStyleSheet("QCheckBox {"
//                                            "   background-color:#222222;"
//                                            "   border: none; /* 移除边框，默认边框可能会盖住背景色 */"
//                                            "   border-radius: 6px; "
//                                            "}");
           allDaycheckBox->setStyleSheet("QCheckBox {"
                                         "   background-color:#222222;"
                                         "   border: none; /* 移除边框，默认边框可能会盖住背景色 */"
                                         "   border-radius: 6px; "
                                         "}");
           setFramePalette(this, pal);

       }
    }
    this->setPalette(pal);
    if (styleGsettings != nullptr) {
        delete styleGsettings;
        styleGsettings = nullptr;
    }
}
void CSchceduleDlg::changedFontSlot()
{
    const QByteArray id("org.lingmo.style");
    if(QGSettings::isSchemaInstalled(id)){
        QGSettings * fontSetting = new QGSettings(id, QByteArray(), this);
        QVariant fontVariant =  fontSetting->get("systemFontSize");
        QFont font;
        font.setPointSize(fontVariant.toInt()*0.85);
        QList<QComboBox *> comboBoxList = this->findChildren<QComboBox *>();
        for (int i = 0; i < comboBoxList.count(); ++i) {
            comboBoxList.at(i)->setFont(font);
        if (comboBoxList.at(i)->view()) {
                comboBoxList.at(i)->view()->setFont(font);
            }
        }
        QList<QPushButton *> buttonList = this->findChildren<QPushButton *>();
        for (int i = 0; i < buttonList.count(); ++i) {
            buttonList.at(i)->setFont(font);
        }
        QList<DateTimeEdit *> dateList = this->findChildren<DateTimeEdit *>();
        for (int i = 0; i < dateList.count(); ++i) {
            dateList.at(i)->setFont(font);
        }
        QList<QTextEdit *> editList = this->findChildren<QTextEdit *>();
        for (int i = 0; i < editList.count(); ++i) {
            editList.at(i)->setFont(font);
        }
        QList<QLabel *> labelList = this->findChildren<QLabel *>();
        for (int i = 0; i < labelList.count(); ++i) {
            labelList.at(i)->setFont(font);
        }
//        this->setFont(font);
        if (fontSetting != nullptr) {
            delete fontSetting;
            fontSetting = nullptr;
        }
    }

}
void CSchceduleDlg::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter){
        qDebug()<<"event->key() == Qt::Key_Return ||";
        event->ignore();
    }else{
        QDialog::keyPressEvent(event);
    }
}
