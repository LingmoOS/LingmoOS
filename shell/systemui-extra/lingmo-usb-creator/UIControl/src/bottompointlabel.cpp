#include "bottompointlabel.h"
#include<QBrush>
#include<QPainter>
#include<QPalette>
bottompointlabel::bottompointlabel(const QString& text,QWidget* parent,int size,QColor backgroundColor):
    QLabel(parent)
{
    this->setFixedSize(QSize(size,size));
    setBackgroundColor(backgroundColor);

}
bottompointlabel::~bottompointlabel(){}
void bottompointlabel::setBackgroundColor(QColor backgroundColor){
    m_backgroundColor=backgroundColor;
    repaint();
}
void bottompointlabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    // 如果使用的是默认的灰色，则转换为调色板中的按钮背景色
    if (m_backgroundColor == QColor(0xCC, 0XCC, 0XCC)) {
        QColor background = this->palette().color(QPalette::Button);
        painter.setBrush(QBrush(background));
    } else if (m_backgroundColor == QColor(45,135,255)) {
        painter.setBrush(this->palette().highlight().color());
    } else {
        painter.setBrush(QBrush(m_backgroundColor));
    }
    painter.setPen(Qt::NoPen);
    QRect rect = this->rect();
    // 也可用QPainterPath 绘制代替 painter.drawRoundedRect(rect, 15, 15); { QPainterPath painterPath; painterPath.addRoundedRect(rect, 15, 15); p.drawPath(painterPath); }
    painter.drawRoundedRect(rect, rect.width()/2, rect.width()/2);

    // drawText
    QLabel::paintEvent(event);
}
