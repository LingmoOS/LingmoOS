#include "widget.h"
#include <QHBoxLayout>
#include <QDBusConnection>
#include <QDebug>
#include <QProcess>


Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);

    QPushButton *btn1 = new QPushButton("常规通知",this);
    QPushButton *btn2 = new QPushButton("通知内容替换",this);
    QPushButton *btn3 = new QPushButton("常驻通知",this);
    QPushButton *btn4 = new QPushButton("关闭通知",this);

    hLayout->addWidget(btn1);
    hLayout->addWidget(btn2);
    hLayout->addWidget(btn3);
    hLayout->addWidget(btn4);

    QDBusConnection::sessionBus().connect("org.freedesktop.Notifications",
                                          "/org/freedesktop/Notifications",
                                          "org.freedesktop.Notifications", "ActionInvoked", this,
                                          SLOT(onActionInvoked(quint32, QString))
    );

    connect(btn1,&QPushButton::clicked,this,[=](){
        KNotifier notifier;
        notifier.setAppName("lingmo-control-center");
        notifier.setAppIcon("lingmo-control-center");
        notifier.setBodyText("bodyText");
        notifier.setSummary("summary");
        notifier.setDefaultAction("lingmo-control-center");
        notifier.addAction("lingmo-music","music");
        m_id = notifier.notify();
        qDebug() << "notifacation id:"<<m_id;
    });

    connect(btn2,&QPushButton::clicked,this,[=](){
        KNotifier notifier;
        notifier.setAppName("lingmo-control-center");
        notifier.setAppIcon("lingmo-control-center");
        notifier.setBodyText("replaced bodyText");
        notifier.setSummary("summary");

        notifier.addAction("lingmo-music","music");
        notifier.setDefaultAction("lingmo-control-center");
        notifier.setReplaceId(1);
        m_id = notifier.notify();
        qDebug() << "notifacation id:"<<m_id;
    });

    connect(btn3,&QPushButton::clicked,this,[=](){
        KNotifier notifier;
        notifier.setAppName("lingmo-control-center");
        notifier.setAppIcon("lingmo-control-center");
        notifier.setBodyText("bodyText");
        notifier.setSummary("summary");
        notifier.setDefaultAction("lingmo-control-center");
        notifier.addAction("lingmo-music","music");
        notifier.setShowTime(KNotifier::AllTheTime);
        m_id = notifier.notify();
        qDebug() << "notifacation id:"<<m_id;
    });


    connect(btn4,&QPushButton::clicked,this,[=](){
        KNotifier::closeNotification(m_id);
    });
}

Widget::~Widget()
{
}

void Widget::onActionInvoked(quint32 id, QString actionKey)
{
    qDebug()<<id <<actionKey;
    QProcess *process = new QProcess(this);
    QStringList l;
    if(actionKey == "lingmo-music")
    {
        l <<"lingmo-music";
        process->start("lingmo-music",l);
    }
    else
    {
        l <<"lingmo-control-center";
        process->start("lingmo-control-center",l);
    }
}

