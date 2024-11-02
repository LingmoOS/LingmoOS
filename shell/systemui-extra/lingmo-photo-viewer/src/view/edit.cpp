#include "edit.h"

Edit::Edit(QWidget *parent) : QLineEdit(parent) {}
void Edit::focusOutEvent(QFocusEvent *event)
{
    if(m_rightButtonPress){
        return;
    }
    clearFocus();
    close();
    Q_EMIT showOrigName();
    QLineEdit::focusOutEvent(event);
}
void Edit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        Text = this->text();
        close();
        Q_EMIT renamefinished();
    }
    QLineEdit::keyPressEvent(event);
}

bool Edit::event(QEvent *event)
{
    if (event->type() == QEvent::HoverLeave) {
        Q_EMIT dealDoubleClicked(false);
    } else if (event->type() == QEvent::HoverEnter) {
        Q_EMIT dealDoubleClicked(true);
    }
    if(event->type() == QEvent::MouseButtonPress){
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            m_rightButtonPress = true;
        }
    } else if (event->type() == QEvent::ChildRemoved){
        m_rightButtonPress = false;
    }
    return QLineEdit::event(event);
}
