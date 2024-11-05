#include "mylabelicon.h"


mylabelicon::mylabelicon(QPixmap pix, QWidget *parent):QLabel(parent)
{
    m_pix=pix;

}

void mylabelicon::paintEvent(QPaintEvent *ev)
{
 pain.begin(this);
 pain.setPen(QPen(Qt::blue,1,Qt::SolidLine,Qt::FlatCap));
 pain.drawPixmap(0,0,m_pix);
 //pain.fillRect(rect(),Shadow);
 pain.end();
}
