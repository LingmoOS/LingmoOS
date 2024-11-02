#include "maskwidget.h"

#include <QPainter>

MaskWidget::MaskWidget(QWidget *parent) :
    QWidget(parent)
{
//    setAttribute(Qt::WA_TranslucentBackground);
    pWidth = parent->width();
    pHeigh = parent->height();
    pRadius = 6;
    pColor = QString("#ffffff");
    pBorder = 2;
}

MaskWidget::~MaskWidget()
{
}


void MaskWidget::paintEvent(QPaintEvent *event){
    Q_UNUSED(event)
    QPainter painter(this);

    painter.setRenderHint(QPainter:: Antialiasing, true);  //设置渲染,启动反锯齿

    painter.setPen(QPen(QColor(palette().color(QPalette::Base)), pBorder));

    painter.drawRect(0, 0, pWidth, pHeigh);
    painter.drawRoundedRect(0, 0, pWidth, pHeigh, pRadius, pRadius);
}
