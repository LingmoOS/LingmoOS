#ifndef FONTLISTDELEGATEUG_H
#define FONTLISTDELEGATEUG_H

#include <QObject>
#include <QAbstractItemDelegate>
#include <QPoint>
#include <QPainter>

#include "fontlistmodel.h"
#include "fontlistview.h"
class FontListView;

class FontListDelegateug : public QAbstractItemDelegate
{
public:
    FontListDelegateug(FontListView *fontList);
    ~FontListDelegateug();

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
#endif // FONTLISTDELEGATEUG_H
