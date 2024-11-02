#include "kqpushbutton.h"

KQPushButton::KQPushButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{

}

KQPushButton::KQPushButton(QWidget *parent)
    : QPushButton(parent)
{

}

bool KQPushButton::event(QEvent *e) {
    if (e->type() == QEvent::HoverEnter) {
        emit hovered(true);
    }
    if (e->type() == QEvent::HoverLeave) {
        emit hovered(false);
    }
    return QPushButton::event(e);
}
