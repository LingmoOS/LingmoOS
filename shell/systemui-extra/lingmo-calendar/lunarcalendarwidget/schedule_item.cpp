#include "schedule_item.h"
#include "schedulewidget.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScreen>
#include <QLabel>
#include <QPainter>
#include <QAction>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QMenu>
#include "calendardatabase.h"
#include <QColor>
#include <QGSettings>
#include "calendarcolor.h"
#include "colorarea.h"
#include "schedulewidget.h"
#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "themeColor"
#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "styleName"
#ifndef Q_UNUSED


#define Q_UNUSED(x) (void)x;

#endif
schedule_item::schedule_item(QWidget *parent,bool ishasSchedule) : QWidget(parent) {
    // 初始化布局
    mainLayout = new QHBoxLayout(this);
    this->setMouseTracking(true);
    const QByteArray i("org.lingmo.style");
    StyleSetting = new QGSettings(i, QByteArray(), this);
    // 创建颜色区域
    colorArea = new ColorArea(this);
    colorArea->setFixedSize(4, 44); // 设置大小
    //实时监听主题变化

    mainLayout->addWidget(colorArea);
    hasSchdule = ishasSchedule;
    dateTimeLabel = new QLabel(this);
    dateTimeLabel->setFixedHeight(20);
//    dateTimeLabel->setStyleSheet("QLabel { font-size: 14px; }");
    dateTimeLabel->setMargin(0); // 设置内边距
    id = new QLabel(this);
    id->hide();
    eventLabel = new QLabel(this);
//    eventLabel->setStyleSheet("QLabel { font-size: 14px; }");
    eventLabel->setMargin(0); // 设置内边距

    // 添加间距
    mainLayout->addSpacing(4);

    // 创建水平布局用于日期、时间和日程内容
    eventLayout = new QVBoxLayout();

    textLayout = new QVBoxLayout();

    textLayout->addWidget(dateTimeLabel);
    textLayout->addWidget(eventLabel);

    eventLayout->addLayout(textLayout);

    mainLayout->addLayout(eventLayout);

    // 添加间距
    eventLayout->addSpacing(3);
    // 设置布局
    setLayout(mainLayout);

}
void schedule_item::enterEvent(QEvent *event)
{

}
void schedule_item::setId(const QString &idd)
{
    id->setText(idd);
}
void schedule_item::setStartTime(const QString &startTime) {
//    QString star = QString(startTime);
    dateTimeLabel->setText(tr("Schedule:")+startTime);
    if(!hasSchdule){
         dateTimeLabel->setText(tr(" No Schedule:")+startTime);
    }
}
//设置 已经结束了的日程置灰 但是还没有做到日程ASC功能
void schedule_item::setEndTime(int end_hour,int end_minute,QDate date,QString shangwu) {
//    if(shangwu == "afternoon")
    if(timemode == "12"){
        if(end_hour < 12){
            if(shangwu == "下午"){
               end_hour = end_hour+12;
            }
        }else{
//            end_hour = end_hour+12;
        }
    }
    QString endTimeStr = QString("%1:%2").arg(end_hour, 2, 10, QChar('0')).arg(end_minute, 2, 10, QChar('0'));
    // 将结束时间字符串转换为 QDateTime 对象
    QDateTime endTimeObj = QDateTime(QDate::currentDate(), QTime(end_hour, end_minute), Qt::LocalTime);
    QDateTime currentTime = QDateTime::currentDateTime();
//    QDateTime endTimeObj = QDateTime::fromString(endTime, "hh:mm");
    if(date == QDate::currentDate()){
        if (currentTime > endTimeObj) {
            wasSchedule=true;
        }
    }
    if(wasSchedule){
        // 遍历所有子控件，查找并置灰标签
        eventLabel->setStyleSheet("color: gray;"); // 将标签文字置灰
        dateTimeLabel->setStyleSheet("color: grey ;");
    }


}
void schedule_item::adjustDate(QDate date)
{
    qDebug()<<"传进来的date是"<<date<<" QDate::currentDate()="<< QDate::currentDate();
    if(date < QDate::currentDate() ){
        wasSchedule = true;
    }else if (date > QDate::currentDate()){
        wasSchedule = false;
    }
}
//修改超过显示区域用省略号表示，并只有显示省略号的内容才显示悬浮框
void schedule_item::setDescription(const QString &description) {
    QFont font;
//    font.setPointSize(14);
    QFontMetrics metrics(font);
//    eventLabel->setFont(font);
    eventLabel->setFixedHeight(27);
    QString descriptionTrimmed = description.trimmed(); // 去除两端空白字符，确保准确比较长度
    QString elided = metrics.elidedText(descriptionTrimmed, Qt::ElideRight, 400);//400后用...省略

    eventLabel->setText(elided);

    const QByteArray i("org.lingmo.style");
    QGSettings *ColorSetting = new QGSettings(i, QByteArray(), this);
    QColor color1 = ColorSetting->get("themeColor").toString();
    color1 = CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED);
    QString styleSheet;
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
        styleGsettings = new QGSettings(style_id, QByteArray(), this);
        QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
        if (currentTheme == "lingmo-dark" || currentTheme == "lingmo-default") {
            styleSheet = QString("QToolTip {"
                                 "    background-color: #222222;"
                                 "    border: 1px solid #888888;" // 设置一个透明的边框，用于显示渐变效果
                                 "    border-radius: 10px;" // 设置边框圆角半径
                                 "}"
                                 "QLabel {  }").arg(color1.name());
            if(wasSchedule){
                styleSheet = QString("QToolTip {"
                                     "    background-color: #222222;"
                                     "    border: 1px solid #888888;" // 设置一个透明的边框，用于显示渐变效果
                                     "    border-radius: 10px;" // 设置边框圆角半径
                                     "}"
                                     "QLabel {color: gray; }").arg(color1.name());

            }
        } else {
            styleSheet = QString("QToolTip {"
                                 "    background-color: white;"
                                 "    border: 1px solid #C0C0C0;" // 设置一个透明的边框，用于显示渐变效果
                                 "    border-radius: 10px;" // 设置边框圆角半径
                                 "}"
                                 "QLabel {  }").arg(color1.name());
            if(wasSchedule){
                styleSheet = QString("QToolTip {"
                                     "    background-color: #222222;"
                                     "    border: 1px solid #888888;" // 设置一个透明的边框，用于显示渐变效果
                                     "    border-radius: 10px;" // 设置边框圆角半径
                                     "}"
                                     "QLabel { color: gray; }").arg(color1.name());

            }
        }

    }
    eventLabel->setStyleSheet(styleSheet);
    if (ColorSetting != nullptr) {
        delete ColorSetting;
        ColorSetting = nullptr;
    }
}

void schedule_item::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton){
        if(hasSchdule){
        this_id = getId();
        m_menu = new QMenu(this);
        const QByteArray i("org.lingmo.style");
        QGSettings *fontSetting3 = new QGSettings(i, QByteArray(), this);
        QColor color1 = fontSetting3->get("themeColor").toString();
        QPalette pal = palette();
        color1 = CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED);

        QString styleSheet;
        const QByteArray style_id(THEME_SCHAME);
        if (QGSettings::isSchemaInstalled(style_id)) {
            styleGsettings = new QGSettings(style_id, QByteArray(), this);
            QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
            if (currentTheme == "lingmo-dark" || currentTheme == "lingmo-default") {
                styleSheet = QString("QMenu {"
                                     "    background-color: #222222;"
                                     "    border: 1px solid #888888;" // 设置一个透明的边框，用于显示渐变效果
                                     "    border-radius: 10px;" // 设置边框圆角半径
                                     "}"
                                     "QMenu::item {"
                                     "    padding-left: 5px; "
                                     "    padding-right: 80px;"
                                     "    background-color: transparent;"
                                     "}"
                                     "QMenu::item:selected {"
                                     "    background-color: %1;"
                                     "    border-radius: 4px;"
                                     "    color: white;"
                                     "}").arg(color1.name());
            } else {
                styleSheet = QString("QMenu {"
                                     "    background-color: white;"
                                     "    border: 1px solid #C0C0C0;" // 设置一个透明的边框，用于显示渐变效果
                                     "    border-radius: 10px;" // 设置边框圆角半径
                                     "}"
                                     "QMenu::item {"
                                     "    padding-left : 5px; "
                                     "    padding-right: 80px;"
                                     "    background-color: transparent;"
                                     "}"
                                     "QMenu::item:selected {"
                                     "    background-color: %1;"
                                     "    border-radius: 4px;"
                                     "    color: black;"
                                     "}").arg(color1.name());
            }
        }
        m_menu->setStyleSheet(styleSheet);

        // 添加“编辑日程”动作
        QAction *editAction = new QAction(tr("Edit"));
        m_menu->addAction(editAction);
        // 添加“删除日程”动作
        QAction *deleteAction =  new QAction(tr("Delete"));
        m_menu->addAction(deleteAction);
        // 显示菜单
        // 检查用户选择了哪个动作
        connect(editAction,&QAction::triggered,this,&schedule_item::slotEditSchedule);
        connect(deleteAction,&QAction::triggered,this,&schedule_item::slotDeleteSchedule);
        m_menu->exec(event->globalPos());
    }else if(!hasSchdule){

        }
}
}
void schedule_item::mouseDoubleClickEvent(QMouseEvent *event)
{
    QString id = getId();
    if(event->button() == Qt::LeftButton){
        Q_EMIT ScheduleClick(id);
    }

}
void schedule_item::slotEditSchedule()
{
    Q_EMIT EditSchedule(this_id);
}
void schedule_item::slotDeleteSchedule()
{

    Q_EMIT updateWindow(this_id);



}
QString schedule_item::getId()
{
    return id->text();
}
