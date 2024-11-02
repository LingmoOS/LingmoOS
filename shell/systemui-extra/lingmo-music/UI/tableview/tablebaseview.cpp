
#include "tablebaseview.h"
#include "UI/base/widgetstyle.h"
#include "UI/globalsignal.h"

TableBaseView::TableBaseView(QTableView *parent)
{
    m_delegate = new TableViewDelegate();
    setItemDelegate(m_delegate);
    connect(this,&TableBaseView::hoverIndexChanged,m_delegate,&TableViewDelegate::onHoverIndexChanged);
    connect(this,&TableBaseView::leaveFromItem,m_delegate,&TableViewDelegate::onLeaveFromItemEvent);
    this->setMouseTracking(true);
    initStyle();
}

void TableBaseView::initStyle()
{
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);//设置选中模式为选中行
    setSelectionMode(QAbstractItemView::ExtendedSelection);//设置按ctrl键选中多个
//    setAutoScroll(false);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
//    setAutoFillBackground(true);
    setAlternatingRowColors(false);
    this->setAutoScroll(false);
//    this->verticalScrollBarPolicy();
//    this->setAutoFillBackground(false);
    //限制应用内字体固定大小
//    QFont sizeFont;
//    sizeFont.setPixelSize(14);
//    this->setFont(sizeFont);

}

TableBaseView::~TableBaseView()
{
    if(m_delegate!=nullptr) {
        m_delegate->deleteLater();
    }
}

QString TableBaseView::getSearchText()const
{
    return m_searchText;
}

void TableBaseView::setSearchText(QString text)
{
    m_searchText = text;
}

QString TableBaseView::getSearchListName()const
{
    return m_searchListName;
}

void TableBaseView::setSearchListName(QString listName)
{
    m_searchListName = listName;
}

void TableBaseView::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndex index = this->indexAt(event->pos());
    Q_EMIT hoverIndexChanged(index);
}



void TableBaseView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QModelIndex index = this->indexAt(event->pos());
    Q_EMIT doubleClicked(index);

    //QWidget::mouseDoubleClickEvent(event);
}

void TableBaseView::leaveEvent(QEvent *event)
{
    Q_EMIT leaveFromItem();
    viewport()->update();
}

QString TableBaseView::getNowPlayListName()const
{
    return m_nowListName;
}

void TableBaseView::setNowPlayListName(QString listName)
{
    m_nowListName = listName;
}
