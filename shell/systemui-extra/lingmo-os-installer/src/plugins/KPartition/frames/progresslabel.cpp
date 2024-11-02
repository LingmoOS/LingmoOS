#include "progresslabel.h"
#include <QPainter>
#include <QDebug>

namespace KInstaller {
QString ColorNamedir = ":/res/pic/color";
ProgressLabel::ProgressLabel(QWidget *parent) : QWidget(parent)
{
    widget = new QWidget(this);
    initProgressLabel();
}


void ProgressLabel::initProgressLabel()
{
    gridLayout = new QGridLayout(widget);
    progressBar = new QProgressBar(widget);
    gridLayout->setMargin(0);
    gridLayout->addWidget(progressBar, 0, 0, 1, parts.size());
    updateStructLabel();
}

void ProgressLabel::setDevice(Device::Ptr dev)
{
    m_dev = dev;
    update();
}

void ProgressLabel::insetStructLabel(QPixmap pix, QString str)
{
    StrPartProgressView *p1 = new StrPartProgressView;
    p1->color  = new QLabel(widget);
    p1->color->setPixmap(pix);
    p1->color->setMaximumHeight(8);
    p1->text = new QLabel(widget);
    p1->text->setObjectName("Nomal");
    p1->text->setProperty("level","little");
    p1->text->setMaximumHeight(10);
    p1->text->setText(str);
    parts.append(p1);
    updateStructLabel();
}
//分区完成后,进行updateStructLabel,更新分区显示label
void ProgressLabel::updateStructLabel()
{
    for(int i = 0; i < parts.size(); i++)
    {
        QHBoxLayout *hlayout = new QHBoxLayout(widget);
        hlayout->addWidget(parts.at(i)->color);
        hlayout->addWidget(parts.at(i)->text);
        gridLayout->addLayout(hlayout, 1, i, Qt::AlignLeft);
     }
    gridLayout->addItem(new QSpacerItem(10, 15, QSizePolicy::Expanding, QSizePolicy::Maximum),1,parts.size());
}
//设置进度条中的不同颜色显示
QString ProgressLabel::calcValue(int nTotal)
{


//    list.append("rgba(0, 255, 0, 255)");
//    list.append("rgba(255, 255, 0, 255)");
    QList <float> data;
#if 0
    QString strStyle = QString("qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, \
stop:0 rgba(255, 0, 0, 255), stop:0.2 rgba(255, 0, 0, 255), \
stop:0.2001 rgba(255, 255, 0, 255), stop:0.5 rgba(255, 255, 0, 255), \
stop:0.5001 rgba(255, 0, 0, 255), stop:0.7 rgba(255, 0, 0, 255), \
stop:0.7001 rgba(255, 123, 255, 255), stop:0.9 rgba(255, 123, 255, 255), \
stop:0.9001 rgba(0, 0, 255, 255), stop:1 rgba(0, 0, 255, 255))");
#endif
    QString strStyle = QString("qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, ");
    for(int i = 0; i < progressValue.size(); i++) {
        QString stmp = progressValue.at(i);
        float scale = stmp.toFloat() / (float)nTotal;
        if(i>0)
            data.append(scale + data.at(i-1));
        else
            data.append(scale);
        qDebug() << "ProgressLabel::data" << data.at(i);
    }
    for(int i = 0; i < colorList.size(); i++) {
        QString sm;
        if(i == 0)
            sm= QString("stop:%1 %2, stop:%3 %4,").arg(0).arg(colorList.at(i)).arg(data.at(i)).arg(colorList.at(i));
        else
            sm= QString("stop:%1 %2, stop:%3 %4,").arg(data.at(i-1)+0.0001).arg(colorList.at(i)).arg(data.at(i)).arg(colorList.at(i));
        strStyle.append(sm);
        qDebug() << "ProgressLabel::calcValue" <<i << strStyle;
    }


    strStyle.append(QString("stop:%1 rgba(255, 255, 255, 255), stop:1 rgba(255, 255, 255, 255))").arg(data.at(data.size()-1)+0.0001));
    return strStyle;
}
void ProgressLabel::paintEvent(QPaintEvent *)
{
//    QPainter p(this);
//    QRect rect = QRect(0, 0, width(), height()/2);
//    QRect textRect = QRect(0, height()/2, width(), height()/2);

//    const double k = (double)(value() - minimum()) / (maximum()-minimum());
//    int x = (int)(rect.width() * k);
//    QRect fillRect = rect.adjusted(0, 0, x-rect.width(), 0);

//    QString valueStr = QString("%1%").arg(QString::number(value()));
//    QPixmap buttomMap = QPixmap(":/resource/radius_back.png");
//    QPixmap fillMap = QPixmap(":/resource/radius_front.png");

//    //画进度条
//    p.drawPixmap(rect, buttomMap);
//    p.drawPixmap(fillRect, fillMap, fillRect);

//    //画文字
//    QFont f = QFont("Microsoft YaHei", 15, QFont::Bold);
//    p.setFont(f);
//    p.setPen(QColor("#555555"));
//    p.drawText(textRect, Qt::AlignCenter, valueStr);
}

void ProgressLabel::slotDevPartitionColor(Device::Ptr dev)
{
    if(dev) {
        const PartitionList& partitions = dev->partitions;
        int i = 1;
        for(Partition::Ptr partition : partitions) {
            QString dir = QString(":/res/pic/color/0_%1.png").arg(i);
            insetStructLabel(QPixmap(dir), partition->m_partitionPath);
            i++;
        }
    }
}
}
