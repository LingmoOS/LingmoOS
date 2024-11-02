#include "topborder.h"

topborder::topborder(QWidget *parent) : QWidget(parent)
{

}
void topborder::paintEvent(QPaintEvent *event)
{
    QPainter p;   //创建画家对象；
    p.begin(this);  //指定当前窗口为绘图设备；
    //定义画笔
    QPen pen;
    pen.setColor(QColor(45,135,255));
    p.setPen(pen);
    // 画矩形；
    p.drawRect(0,0,400,260);
    p.end();

}
