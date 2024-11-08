#include "customscrollarea.h"
#include <QScrollBar>
#include <QDebug>
#include <QGSettings>
#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "styleName"
CustomScrollArea::CustomScrollArea(QWidget *parent)
    : QScrollArea(parent)
{
}

void CustomScrollArea::wheelEvent(QWheelEvent *event)
{
    int delta = event->angleDelta().y();
    QScrollBar *vBar = verticalScrollBar();

    // 判断滚动条是否在最顶部或最底部
    bool atTop = (vBar->value() == vBar->minimum());
    bool atBottom = (vBar->value() == vBar->maximum());

    if ((delta > 0 && atTop) || (delta < 0 && atBottom)) {
        TopOrButtom = true;
        // 如果滚动到最顶部且继续向上滚动，或者滚动到最底部且继续向下滚动，则忽略事件
        event->ignore();
    } else {
        // 否则，正常处理事件
        QScrollArea::wheelEvent(event);
    }
}
