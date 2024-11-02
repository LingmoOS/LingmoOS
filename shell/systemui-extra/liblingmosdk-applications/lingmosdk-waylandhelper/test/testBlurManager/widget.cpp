#include "widget.h"

#include "widget.h"
#include <lingmostylehelper/lingmostylehelper.h>
#include <windowmanager/windowmanager.h>
#include <blurmanager/blurmanager.h>
#include <KWindowEffects>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    m_btn = new QPushButton("switch",this);
    m_btn->setCheckable(true);
    m_btn->move(100,100);
    m_testWidget = new QWidget();

    connect(m_btn,&QPushButton::clicked,this,[=](bool checked)
    {
        if(checked)
        {
            m_testWidget->setAttribute(Qt::WA_TranslucentBackground);
            m_testWidget->show();
            kdk::BlurManager::setBlurBehindWithStrength(m_testWidget->windowHandle(),true, QRegion(m_testWidget->rect()),400);
        }
        else
            m_testWidget->hide();

    });
}

Widget::~Widget()
{
}
