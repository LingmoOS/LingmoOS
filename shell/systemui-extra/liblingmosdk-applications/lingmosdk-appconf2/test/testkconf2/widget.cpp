#include "widget.h"
#include <QPushButton>
#include <QObject>
#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>
#include <QProgressBar>
#include <QLineEdit>
#include "kconf2.h"
using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{

    if (KConf2::isSettingsAvailable("control-center.apt.proxy")) {
        KConf2 *settings = new KConf2("control-center.apt.proxy");
        connect(settings, &KConf2::changed, this, [=](const QString &key) {
            qDebug()<<"恭喜你，gsetting切换成功了,变化的key是"<<key<<"style-name is"<<settings->get("style-name").toString();
        });
    }

    QVBoxLayout* layout = new QVBoxLayout(this);
    QPushButton* btn1 = new QPushButton("get value");
    QPushButton* btn2 = new QPushButton("set enabled");
    QPushButton* btn3 = new QPushButton("get keys");
    QPushButton* btn4 = new QPushButton("schemas");
    QPushButton* btn5 = new QPushButton("getDescrition");
    QPushButton* btn6 = new QPushButton("get summary");
    QPushButton* btn7 = new QPushButton("get type");
    QPushButton* btn8 = new QPushButton("is Writable");
    QPushButton* btn9 = new QPushButton("get id ");
    QPushButton* btn10 = new QPushButton("get version ");
    QPushButton* btn11 = new QPushButton("get child ");
    QPushButton* btn12 = new QPushButton("children ");


    layout->addWidget(btn1);
    layout->addWidget(btn2);
    layout->addWidget(btn3);
    layout->addWidget(btn4);
    layout->addWidget(btn5);
    layout->addWidget(btn6);
    layout->addWidget(btn7);
    layout->addWidget(btn8);
    layout->addWidget(btn9);
    layout->addWidget(btn10);
    layout->addWidget(btn11);
    layout->addWidget(btn12);

    connect(btn1,&QPushButton::clicked,this,[=]{
        onPushbutton_1Clicked();
    });

    connect(btn2,&QPushButton::clicked,this,[=]{
        onPushbutton_2Clicked();
    });

    connect(btn3,&QPushButton::clicked,this,[=]{
        onPushbutton_3Clicked();
    });

    connect(btn4,&QPushButton::clicked,this,[=]{
        onPushbutton_4Clicked();
    });

    connect(btn5,&QPushButton::clicked,this,[=]{
        onPushbutton_5Clicked();
    });

    connect(btn6,&QPushButton::clicked,this,[=]{
        onPushbutton_6Clicked();
    });

    connect(btn7,&QPushButton::clicked,this,[=]{
        onPushbutton_7Clicked();
    });

    connect(btn8,&QPushButton::clicked,this,[=]{
        onPushbutton_8Clicked();
    });

    connect(btn9,&QPushButton::clicked,this,[=]{
        onPushbutton_9Clicked();
    });

    connect(btn10,&QPushButton::clicked,this,[=]{
        onPushbutton_10Clicked();
    });

    connect(btn11,&QPushButton::clicked,this,[=]{
        onPushbutton_11Clicked();
    });

    connect(btn12,&QPushButton::clicked,this,[=]{
        onPushbutton_12Clicked();
    });

}

Widget::~Widget()
{
}

void Widget::onPushbutton_1Clicked()
{
    if (KConf2::isSettingsAvailable("control-center.apt.proxy")) {
        KConf2 *settings = new KConf2("control-center.apt.proxy");
        auto stylename = settings->get("port").toString();
        qDebug()<<"stylename ="<<stylename;
    }
}

void Widget::onPushbutton_2Clicked()
{

    if (KConf2::isSettingsAvailable("control-center.apt.proxy")) {
        KConf2 *settings = new KConf2("control-center.apt.proxy");
        if(settings->get("enabled").toBool())
            settings->set("enabled", "false");
        else
            settings->set("enabled", "true");
        qDebug()<<"success"<<settings->get("enabled").toString();
    }
}

void Widget::onPushbutton_3Clicked()
{
    if (KConf2::isSettingsAvailable("control-center.apt.proxy")) {
        KConf2 *settings = new KConf2("control-center.apt.proxy");
        auto list = settings->keys();
        qDebug()<<"keys list is"<<list;
    }
}

void Widget::onPushbutton_4Clicked()
{
    if (KConf2::isSettingsAvailable("control-center")) {
        KConf2 *settings = new KConf2("control-center");
        auto list = settings->schemas("control-center1","3.20.1.31-0k2.20");
        qDebug()<<"schemas list is"<<list;
    }
}

void Widget::onPushbutton_5Clicked()
{
    if (KConf2::isSettingsAvailable("control-center.apt.proxy")) {
        KConf2 *settings = new KConf2("control-center.apt.proxy");
        QString str("enabled");
        QString str1 = settings->getDescrition(str);
        qDebug()<<"getDescrition is"<<str1;
    }
}

void Widget::onPushbutton_6Clicked()
{
    if (KConf2::isSettingsAvailable("control-center.apt.proxy")) {
        KConf2 *settings = new KConf2("control-center.apt.proxy");
        QString str("enabled");
        QString str1 = settings->getSummary(str);
        qDebug()<<"getSummary is"<<str1;
    }
}

void Widget::onPushbutton_7Clicked()
{
    if (KConf2::isSettingsAvailable("control-center.apt.proxy")) {
        KConf2 *settings = new KConf2("control-center.apt.proxy");
        QString str("enabled");
        QString str1 = settings->getType(str);
        qDebug()<<"getType is"<<str1;
    }
}

void Widget::onPushbutton_8Clicked()
{
    if (KConf2::isSettingsAvailable("control-center.apt.proxy")) {
        KConf2 *settings = new KConf2("control-center.apt.proxy");
        QString str("enabled");
        settings->isWritable(str);
        qDebug()<<"isWritable "<<settings->isWritable(str);
    }
}

void Widget::onPushbutton_9Clicked()
{
    if (KConf2::isSettingsAvailable("control-center.apt.proxy")) {
        KConf2 *settings = new KConf2("control-center.apt.proxy");
        qDebug()<<"getId is"<<settings->getId();
    }
}

void Widget::onPushbutton_10Clicked()
{
    if (KConf2::isSettingsAvailable("control-center.apt.proxy")) {
        KConf2 *settings = new KConf2("control-center.apt.proxy");
        qDebug()<<"getVersion is"<<settings->getVersion();
    }
}

void Widget::onPushbutton_11Clicked()
{
    if (KConf2::isSettingsAvailable("control-center")) {
        KConf2 *settings = new KConf2("control-center");
        QString str("apt");
        KSettings* gsetting = settings->getChild(str);
        qDebug()<<"getChild is"<<gsetting;
    }
}

void Widget::onPushbutton_12Clicked()
{
    if (KConf2::isSettingsAvailable("control-center")) {
        KConf2 *settings = new KConf2("control-center");
        auto list = settings->children();
        qDebug()<<"children list is"<<list;
    }
}
