#include "widget.h"
#include <QDebug>

using namespace kdk;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    IdleNotifier *notifier = new IdleNotifier(this);
    notifier->setIdleInterval(10);
    connect(notifier, &IdleNotifier::idled,[=](){
        qDebug() <<"idled";
    });
    connect(notifier, &IdleNotifier::resumed,[=](){
        qDebug() <<"resumed";
    });
}

Widget::~Widget()
{
}

