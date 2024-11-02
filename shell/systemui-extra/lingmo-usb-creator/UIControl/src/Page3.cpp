#include "Page3.h"
#include <QDir>
#include <QDebug>
#include <QDBusConnection>
#include <QLabel>

Page3::Page3(QString strImgDir, QWidget *parent)
    : QWidget(parent)
{
    lab=new QLabel(this);
    setImgDir(strImgDir);
}
void Page3::setImgDir(QString strImgDir)
{
    if(m_strImgDir == strImgDir)
        return ;
    else
        m_strImgDir = strImgDir;
    QDir tmpDir(m_strImgDir);
    QFileInfoList listInfo = tmpDir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name);
    if(listInfo.isEmpty()){
        qWarning()<<"AE Dir is Empty!";
        return ;
    }
    m_strListImg.clear();
    foreach(QFileInfo strFileInfo,listInfo){
        m_strListImg.append(strFileInfo.filePath());
    }
    this->resize(QImage(m_strListImg.first()).size());
    this->show();
    }

void Page3::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    if(m_nIndex > (m_strListImg.size() - 1))
        return ;
    QPainter painter(this);
    painter.drawImage(QPointF(0,0),QImage(m_strListImg[m_nIndex]));
    m_nIndex++;
    m_nIndex = (m_nIndex)%m_strListImg.size();
}

