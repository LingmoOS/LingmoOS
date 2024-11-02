#include "kpartition.h"
#include <QApplication>
#include <QLibrary>
#include <QDebug>

KPartition::KPartition()
{
    this->setObjectName("KPartition");
}
KPartition::~KPartition()
{
}

QWidget* KPartition::CreateObject(QWidget *parent)
{
    m_mainPartFrame = new MainPartFrame(parent);
    return m_mainPartFrame;
}
MainPartFrame* KPartition::getWidget()
{
      return (MainPartFrame*)m_mainPartFrame;
}
QString KPartition::getWidgetName()
{
    QString s = "KPartition";
    return s;
}

void KPartition::next()
{

}
void KPartition::back()
{

}
void KPartition::onActive()
{

}
void KPartition::onLeave()
{

}
