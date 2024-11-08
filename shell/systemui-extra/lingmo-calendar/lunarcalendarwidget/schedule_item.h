#ifndef SCHEDULE_ITEM_H
#define SCHEDULE_ITEM_H
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGSettings>
class LunarCalendarWidget;
//class QWidget;
class ColorArea; // 假设ColorArea是一个已经定义好的类
class schedule_item : public QWidget {
    Q_OBJECT

public:
    schedule_item(QWidget *parent = nullptr ,bool ishasSchedule=false);

    // 假设你想要设置开始时间和结束时间的函数
    void setStartTime(const QString &startTime);
    void setEndTime(int end_hour,int end_minute,QDate date,QString shangwu);
    QGSettings *StyleSetting = nullptr;
    void setColor(const QString &color);
    void setDescription(const QString &description);
    void adjustDate(QDate date);
    void setId(const QString &idd);
    QString getId();
    QLabel *id = nullptr;
    QString this_id;
    QMenu *m_menu = nullptr;
    QString timemode;

//    void setColor(const QString &color);
    bool wasSchedule = false;
    bool hasSchdule;
    ColorArea *colorArea= nullptr;
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
private:
    QLabel *dateTimeLabel= nullptr;
    QLabel *eventLabel= nullptr;
    QGSettings *styleGsettings = nullptr;

    QHBoxLayout *mainLayout= nullptr;
    QVBoxLayout *eventLayout= nullptr;
    QVBoxLayout *textLayout= nullptr;
    QColor selectBgColor;
//    LunarCalendarWidget *m_widget = nullptr;
signals:
    void updateWindow(QString id);
    void ScheduleClick(QString id);
    void EditSchedule(QString id);
    void updateSchedule1();
public slots:

    void slotEditSchedule();
    void slotDeleteSchedule();
};

#endif // SCHEDULE_ITEM_H

