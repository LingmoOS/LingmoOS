#include "comboxdelegate.h"
namespace KServer {


ComboxDelegate::ComboxDelegate()
{

}

void ComboxDelegate::paint(QPainter *paint, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(paint, option, index);
//    option.widget->style()->drawItemText(
//                paint, option.rect,  Qt::AlignVCenter,
//                option.palette, false, index.data().toString());
}
}
