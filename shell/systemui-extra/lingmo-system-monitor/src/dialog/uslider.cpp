#include "uslider.h"
#include <QDebug>

USlider::USlider(Qt::Orientation orientation, QWidget *parent):QSlider(orientation,parent)
{

}

USlider::~USlider()
{

}

void USlider::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        QMouseEvent a(e->type(),e->localPos(),Qt::MiddleButton,
                      QFlags<Qt::MouseButton>(Qt::MidButton|Qt::MiddleButton),e->modifiers());
        QSlider::mousePressEvent(&a); //左键事件转换为中键事件，实现滑动条点击移动
        return;
    } else {
        QSlider::mousePressEvent(e);
        return;
    }
}
