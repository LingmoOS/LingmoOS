#include "tableviewdelegate.h"
#include "UI/base/widgetstyle.h"
#include "UIControl/player/player.h"
#include "UIControl/base/musicDataBase.h"
#include "UI/search/musicsearchlistview.h"
#include "UI/tableview/tablebaseview.h"

#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

TableViewDelegate::TableViewDelegate(QWidget *parent)
    : QStyledItemDelegate(parent),
      m_pOpenButton(new QPushButton()),
      m_pDeleteButton(new QPushButton()),
      m_nSpacing(5),
      m_nWidth(25),
      m_nHeight(20),
      m_hoverrow(-1)
{
}

TableViewDelegate::~TableViewDelegate()
{

}

// 绘制按钮
void TableViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const TableBaseView *tableview = qobject_cast<const TableBaseView *>(option.widget);
    QString listName = tableview->getNowPlayListName();

    QStyleOptionViewItem opt = option;
    if (index.row() == m_hoverrow ) {
        opt.state |= QStyle::State_Selected;
    } else {
        opt.state &= ~QStyle::State_MouseOver;
    }
    //字体高亮颜色
    opt.palette.setColor(QPalette::HighlightedText,index.data(Qt::ForegroundRole).value<QColor>());
    if (WidgetStyle::themeColor == 0) {
        opt.palette.setColor(QPalette::Highlight,QColor(240,240,240));
    } else {
        opt.palette.setColor(QPalette::Highlight,QColor(54,54,55));
    }

    QString playListName = tableview->getSearchListName();
    if (listName != playListName || playListName == "") {
        QStyledItemDelegate::paint(painter,opt,index);
        return;
    }

    //以下是对搜索结果列表的处理

    //设置选中和悬停样式
    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(opt.rect, opt.palette.color(QPalette::Highlight));
    }

    // 根据关键字高亮
    QString searchText = tableview->getSearchText();
    QColor textColor;
    QColor lightColor;

    //设置非高亮字颜色
    if (WidgetStyle::themeColor == 1) {
        textColor = QColor("#FFFFFF");
    } else if (WidgetStyle::themeColor == 0) {
        textColor = QColor("#000000");
    }

    //如果文件不存在需要置灰
    if (index.data(Qt::StatusTipRole).toString() == QString("nofile")) {
        textColor = QColor(Qt::gray);
    }

    //设置高亮字的颜色
    QPalette pa = option.palette;
    QBrush selectBrush = pa.brush(QPalette::Active, QPalette::Highlight);
    QColor selectColor = selectBrush.color();
    lightColor = selectColor;

    //获取该单元格文本
    QString mtext = index.data(Qt::DisplayRole).value<QString>();

    //超过单元格宽度，末尾换成...
    QFontMetricsF fontWidth(tableview->font());
    mtext = fontWidth.elidedText(mtext, Qt::ElideRight, 136);//魔鬼数字，且没考虑全屏布局，此处待优化

    // 设置文字边距，保证绘制文字居中
    QTextDocument document;
    document.setDocumentMargin(0);
    document.setPlainText(mtext);
    QTextCursor highlight_cursor(&document);
    QTextCursor cursor(&document);
    cursor.beginEditBlock();
    QTextCharFormat color_format(highlight_cursor.charFormat());
    color_format.setForeground(lightColor);

    // 搜索字体高亮
    QTextCursor testcursor(&document);
    testcursor.select(QTextCursor::LineUnderCursor);
    QTextCharFormat fmt;
    fmt.setForeground(textColor);
    testcursor.mergeCharFormat(fmt);
    testcursor.clearSelection();
    testcursor.movePosition(QTextCursor::EndOfLine);

    //当前正在播放的歌曲全部高亮
    if (index.data(Qt::StatusTipRole).toString() == QString("heightLight")) {
        searchText = mtext;
    }

    //关键字高亮
    while (!highlight_cursor.isNull() && !highlight_cursor.atEnd()) {
        highlight_cursor = document.find(searchText, highlight_cursor);
        if (!highlight_cursor.isNull()) {
            highlight_cursor.mergeCharFormat(color_format);
        }
    }
    cursor.endEditBlock();

    //调整布局
    QStyle *pStyle = option.widget ? option.widget->style() : QApplication::style();
    QAbstractTextDocumentLayout::PaintContext paintContext;
    QRect textRect = pStyle->subElementRect(QStyle::SE_ItemViewItemText, &option);

    double moveX = 25;//x轴偏移量
    double moveY = opt.rect.height() / 2 - fontWidth.height() / 2;//y轴偏移量
    if (index.column()+1 == index.model()->columnCount()) {
        moveX = 77 - fontWidth.width(mtext);
    }

    //绘制
    painter->save();
    painter->translate(double(textRect.topLeft().rx()) + moveX ,double(textRect.topLeft().ry()) + moveY );
    document.documentLayout()->draw(painter, paintContext);
    painter->restore();
}


void TableViewDelegate::onHoverIndexChanged(const QModelIndex& index)
{
    m_hoverrow = index.row();
}
void TableViewDelegate::onLeaveFromItemEvent()
{
    m_hoverrow = -1;
}

QSize TableViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(690, 40);
}
