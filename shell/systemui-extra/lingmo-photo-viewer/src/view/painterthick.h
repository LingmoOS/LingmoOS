#ifndef PAINTERTHICK_H
#define PAINTERTHICK_H

#include <QObject>
#include <QWidget>
#include <QCheckBox>
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>
class PainterThick : public QCheckBox
{
    Q_OBJECT
public:
    PainterThick(int diameter, bool type, QColor color, QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);
    int m_diameter;        //按钮直径或长宽
    bool m_type = false;   //判断是圆形粗细设置还是方形颜色设置--默认为圆形
    QColor m_color;        //按钮颜色
    QString m_themeChange; //主题变化

    void mousePressEvent(QMouseEvent *event);
    void resetSize(QSize realSize); //重设控件大小
    void resetColorSize(int diameter);
};

#endif // PAINTERTHICK_H
