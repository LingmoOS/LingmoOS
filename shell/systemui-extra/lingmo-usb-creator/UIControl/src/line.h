#ifndef LINE_H
#define LINE_H
#include <QLineEdit>
#include <QDebug>
#include <QMouseEvent>
class line : public QLineEdit
{
    Q_OBJECT
public:
    line();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *e);

    void mouseDoubleClickEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);
};

#endif // LINE_H
