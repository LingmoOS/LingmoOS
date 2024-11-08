#include "colorarea.h"
#include <QColor>
#include <QPainter>
#include "calendarcolor.h"
#include <QGSettings>
ColorArea::ColorArea(QWidget *parent) : QWidget(parent) {
    // 构造函数实现，如果需要的话，可以在这里添加初始化代码
//    const QByteArray i("org.lingmo.style");
//    if (QGSettings::isSchemaInstalled(i)){
//    StyleSetting = new QGSettings(i);
//    }
    color1 = CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED);

}
ColorArea::~ColorArea()
{
//  if(StyleSetting){
//      delete StyleSetting;
//      StyleSetting = nullptr;
//  }
}
void ColorArea::setcolor(const QColor &color) {
    this_Color = color;
    update(); // 调用update()来标记窗口部件需要重绘
}

void ColorArea::paintEvent(QPaintEvent *event)
{

    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(),color1); // 填充窗口部件的颜色
}
