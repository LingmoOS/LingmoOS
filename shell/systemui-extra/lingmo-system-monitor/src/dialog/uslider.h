#ifndef USLIDER_H
#define USLIDER_H
#include <QSlider>
#include <QMouseEvent>

class USlider:public QSlider
{
public :
    USlider(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~USlider();

protected:
    void mousePressEvent(QMouseEvent *e);
};

#endif // USLIDER_H
