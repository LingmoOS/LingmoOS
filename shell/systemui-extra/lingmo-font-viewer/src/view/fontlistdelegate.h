#ifndef FONTLISTDELEGATE_H
#define FONTLISTDELEGATE_H

#include <QAbstractItemDelegate>
#include <QPoint>
#include <QPainter>

#include "fontlistmodel.h"
#include "fontlistview.h"
class FontListView;

class FontListDelegate : public QAbstractItemDelegate
{
public:
    FontListDelegate(FontListView *fontList);
    ~FontListDelegate();

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    // 根据消息内容获取消息框大小
    static QSize getTextRectSize(QString text, QFont font, int textWidth, int familyFontH);
    int m_fontSize;

    static QPoint m_collectPoint;
    static QSize m_collectSize;

private:
    FontListView *m_fontList = nullptr;
};

#endif // FONTLISTDELEGATE_H
