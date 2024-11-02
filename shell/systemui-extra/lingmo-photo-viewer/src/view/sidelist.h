#ifndef SIDELIST_H
#define SIDELIST_H

#include <QObject>
#include <QWidget>
#include <QListView>
#include <klistview.h>
class SideList : public kdk::KListView
{
    Q_OBJECT
public:
    SideList(QWidget *parent = nullptr);

protected:
    void mouseReleaseEvent(QMouseEvent *e);
};

#endif // SIDELIST_H
