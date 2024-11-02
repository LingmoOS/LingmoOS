#ifndef COMBOXDELEGATE_H
#define COMBOXDELEGATE_H

#include <QObject>
#include <QPainter>
#include <QtWidgets/QStyledItemDelegate>
#include "comboxlistmodel.h"
namespace KServer {


class ComboxDelegate : public QStyledItemDelegate
{
public:
    ComboxDelegate();

    void paint(QPainter* paint, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};
}
#endif // COMBOXDELEGATE_H
