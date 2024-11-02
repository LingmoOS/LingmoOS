#include "pushbuttoncheck.h"
#include <QHBoxLayout>
#include <QDebug>

PushButtonCheck::PushButtonCheck(QString str, QWidget *parent) : QPushButton(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    m_labelpix = new QLabel(this);
    m_labelpix->setPixmap(QPixmap(":/res/png/clicked.svg").scaled(24, 24));
    this->setLayout(layout);
    layout->setAlignment(this, Qt::AlignLeft);
    this->setText(str);
    layout->addStretch();
    layout->addWidget(m_labelpix);

    m_labelpix->setVisible(false);
    connect(this,&PushButtonCheck::clicked,this,&PushButtonCheck::clicked_slot);
    setFixedSize(320, 48);
}

void PushButtonCheck::clicked_slot()
{
    emit clicked_obj(this);
}

void PushButtonCheck::other_clicked_slot(QString objname)
{
    qDebug()<<"objectname===="<<objectName()<<"======"<<objname;
    if(this->objectName().compare(objname)!=0)
    {
        setLabelShow(false);
    }
    else
    {
        setLabelShow(true);
    }
}
