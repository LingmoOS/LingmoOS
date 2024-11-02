#ifndef SIDEBARDELEGATE_H
#define SIDEBARDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QPainter>

class SideBarDelegate : public QStyledItemDelegate
{
public:
    SideBarDelegate();

    //    void paint(QPainter * painter,const QStyleOptionViewItem & option,const QModelIndex & index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // SIDEBARDELEGATE_H
