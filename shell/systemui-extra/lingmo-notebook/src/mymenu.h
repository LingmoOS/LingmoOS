#ifndef MYMENU_H
#define MYMENU_H
#include <QObject>
#include <QMenu>
#include <qevent.h>


class MyMenu : public QMenu
{
public:
    explicit MyMenu(QWidget *parent = nullptr);
protected:
    void keyPressEvent(QKeyEvent* event);
};

#endif // MYMENU_H
