#include "mymenu.h"
#include <QDebug>

MyMenu::MyMenu(QWidget *parent)
{

}

void MyMenu::keyPressEvent(QKeyEvent *event)
{
    // 快捷键Shift+F10 隐藏上下文菜单
    if ((event->modifiers() == Qt::ShiftModifier) && (event->key() == Qt::Key_F10))
    {
        if (isActiveWindow()) {
//            qDebug() << "MyMenu::keyPressEvent(QKeyEvent *event) "  << "if isActiveWindow() = " << isActiveWindow();
            this->hide();
        } else {
//            qDebug() << "MyMenu::keyPressEvent(QKeyEvent *event) "  << "else m_oMenu->isActiveWindow() = " << isActiveWindow();
        }

    }

}
