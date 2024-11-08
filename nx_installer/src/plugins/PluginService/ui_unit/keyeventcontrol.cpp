#include "keyeventcontrol.h"
#include <QWidget>
#include <QObject>
#include <QPushButton>
#include <QListWidget>

QPushButton* GetPBtnFocusCtrl(QWidget* widgets)
{
    QObjectList list = widgets->children();
    for(QObject* obj : list) {
        if(obj->inherits("QPushButton")) {
            QPushButton *b = qobject_cast<QPushButton*>(obj);
            if(b->hasFocus())
                return b;
        }

    }
}
QListWidget* GetLWFocusCtrl(QWidget *widget)
{
    QObjectList list = widget->children();
    for(QObject* obj : list) {
        if(obj->inherits("QListWidget")) {
            QListWidget *b = qobject_cast<QListWidget*>(obj);
            if(b->hasFocus())
                return b;
        }

    }
}
