// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CatalogTreeView.h"
#include "Application.h"
#include "Utils.h"
#include "DocSheet.h"
#include "MsgHeader.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#include <QProxyStyle>
#include <QPainter>
#include <QStylePainter>
#include <QScroller>
#include <QHeaderView>
#include <QDebug>

class ActiveProxyStyle : public QProxyStyle
{
public:
    explicit ActiveProxyStyle(DWidget *parent)
    {
        this->setParent(parent);
    }

    ~ActiveProxyStyle();

    void drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget = nullptr) const
    {
        QStyleOptionComplex *op = const_cast<QStyleOptionComplex *>(option);
        op->state = option->state | QStyle::State_Active;
        QProxyStyle::drawComplexControl(control, op, painter, widget);
    }

    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const
    {
        QStyleOption *op = const_cast<QStyleOption *>(option);
        op->state = option->state | QStyle::State_Active;
        QProxyStyle::drawControl(element, op, painter, widget);
    }

    void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const
    {
        if (element == QStyle::PE_IndicatorBranch)
            painter->setPen(DTK_NAMESPACE::Gui::DGuiApplicationHelper::instance()->applicationPalette().text().color());
        QStyleOption *op = const_cast<QStyleOption *>(option);
        op->state = option->state | QStyle::State_Active;
        QProxyStyle::drawPrimitive(element, op, painter, widget);
    }
};

ActiveProxyStyle::~ActiveProxyStyle()
{
    //NotTodo
}

class CatalogModel: public QStandardItemModel
{
public:
    explicit CatalogModel(QAbstractItemView *parent): QStandardItemModel(parent)
    {
        m_parent = parent;
    }

protected:
    QVariant data(const QModelIndex &index, int role) const;

private:
    QAbstractItemView *m_parent;
};

QVariant CatalogModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::SizeHintRole) {
        QSize size = QStandardItemModel::data(index, role).toSize();
        size.setHeight(m_parent->fontMetrics().height());
        return size;
    }
    return QStandardItemModel::data(index, role);
}

CatalogTreeView::CatalogTreeView(DocSheet *sheet, DWidget *parent)
    : DTreeView(parent), m_sheet(sheet)
{
    ActiveProxyStyle *style = new ActiveProxyStyle(this);
    setStyle(style);
    setFrameShape(QFrame::NoFrame);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    CatalogModel *pModel = new CatalogModel(this);
    this->setModel(pModel);
    pModel->setColumnCount(2);

    this->header()->setHidden(true);
    this->header()->setDefaultSectionSize(18);
    this->header()->setMinimumSectionSize(18);
    this->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    this->viewport()->setAutoFillBackground(false);
    this->setContentsMargins(0, 0, 0, 0);

    connect(this, SIGNAL(collapsed(const QModelIndex &)), SLOT(slotCollapsed(const QModelIndex &)));
    connect(this, SIGNAL(expanded(const QModelIndex &)), SLOT(slotExpanded(const QModelIndex &)));
    connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));

    connect(dApp, &DApplication::fontChanged, this, &CatalogTreeView::onFontChanged);

    QScroller::grabGesture(this, QScroller::TouchGesture);//滑动
}

CatalogTreeView::~CatalogTreeView()
{

}

void CatalogTreeView::setRightControl(bool hasControl)
{
    rightnotifypagechanged = hasControl;
}

void CatalogTreeView::parseCatalogData(const deepin_reader::Section &ol, QStandardItem *parentItem)
{
    foreach (auto s, ol.children) { //  2级显示
        if (s.nIndex >= 0) {
            auto itemList = getItemList(s.title, s.nIndex, s.offsetPointF.x(), s.offsetPointF.y());
            parentItem->appendRow(itemList);

            foreach (auto s1, s.children) { //  3级显示
                auto itemList1 = getItemList(s1.title, s1.nIndex, s1.offsetPointF.x(), s1.offsetPointF.y());
                itemList.at(0)->appendRow(itemList1);
            }
        }
    }
}

QList<QStandardItem *> CatalogTreeView::getItemList(const QString &title, const int &index, const qreal  &realleft, const qreal &realtop)
{
    QStandardItem *item = new QStandardItem(title);
    item->setData(index);
    item->setData(realleft, Qt::UserRole + 2);
    item->setData(realtop, Qt::UserRole + 3);
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QStandardItem *item1 = new QStandardItem(QString::number(index + 1));
    item1->setData(index);
    item1->setData(realleft, Qt::UserRole + 2);
    item1->setData(realtop, Qt::UserRole + 3);
    item1->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QColor color = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().textTips().color();
    item1->setForeground(QBrush(color));

    return QList<QStandardItem *>() << item << item1;
}

void CatalogTreeView::handleOpenSuccess()
{
    auto model = reinterpret_cast<QStandardItemModel *>(this->model());
    if (model) {
        model->clear();

        if (nullptr == m_sheet)
            return;

        m_index = m_sheet->currentIndex();
        const deepin_reader::Outline &ol = m_sheet->outline();
        for (const deepin_reader::Section &s : ol) {   //root
            if (s.nIndex >= 0) {
                auto itemList = getItemList(s.title, s.nIndex, s.offsetPointF.x(), s.offsetPointF.y());
                model->appendRow(itemList);
                parseCatalogData(s, itemList.at(0));
            }
        }
        setIndex(m_index);
    }
    resizeCoulumnWidth();
}

void CatalogTreeView::slotCollapsed(const QModelIndex &index)
{
    Q_UNUSED(index);

    if (nullptr == m_sheet)
        return;

    setIndex(m_index, m_title);
}

void CatalogTreeView::slotExpanded(const QModelIndex &index)
{
    Q_UNUSED(index);

    if (nullptr == m_sheet)
        return;

    setIndex(m_index, m_title);
}

void CatalogTreeView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if (!rightnotifypagechanged) {

        if (nullptr == m_sheet)
            return;

        int nIndex = current.data(Qt::UserRole + 1).toInt();
        double left = current.data(Qt::UserRole + 2).toDouble();
        double top = current.data(Qt::UserRole + 3).toDouble();
        m_title = current.data(Qt::DisplayRole).toString();
        if (nIndex >= 0)
            m_sheet->jumpToOutline(left, top, nIndex);
    }
    rightnotifypagechanged = false;

    return DTreeView::currentChanged(current, previous);
}

void CatalogTreeView::onItemClicked(const QModelIndex &current)
{
    if (nullptr == m_sheet)
        return;

    int nIndex = current.data(Qt::UserRole + 1).toInt();
    double left = current.data(Qt::UserRole + 2).toDouble();
    double top = current.data(Qt::UserRole + 3).toDouble();
    m_title = current.data(Qt::DisplayRole).toString();

    if (nIndex >= 0)
        m_sheet->jumpToOutline(left, top, nIndex);
}

void CatalogTreeView::resizeEvent(QResizeEvent *event)
{
    DTreeView::resizeEvent(event);
    resizeCoulumnWidth();
}

void CatalogTreeView::mousePressEvent(QMouseEvent *event)
{
    rightnotifypagechanged = false;
    DTreeView::mousePressEvent(event);
}

void CatalogTreeView::keyPressEvent(QKeyEvent *event)
{
    rightnotifypagechanged = false;
    DTreeView::keyPressEvent(event);
}

void CatalogTreeView::setIndex(int index, const QString &title)
{
    m_index = index;
    m_title = title;
    this->clearSelection();

    auto model = reinterpret_cast<QStandardItemModel *>(this->model());
    if (model) {
        const QList<QStandardItem *> &itemList = model->findItems("*", Qt::MatchWildcard | Qt::MatchRecursive);
        foreach (QStandardItem *item, itemList) {
            int itemIndex = item->data().toInt();
            if (itemIndex == index && (title.isEmpty() || title == item->data(Qt::DisplayRole).toString())) {
                QList<QStandardItem *> parentlst;
                parentlst << item;
                QStandardItem *parent = item->parent();
                while (parent) {
                    parentlst << parent;
                    parent = parent->parent();
                }

                for (int i = parentlst.size() - 1; i >= 0; i--) {
                    const QModelIndex &modelIndex = parentlst.at(i)->index();
                    if (!this->isExpanded(modelIndex) || item->index() == modelIndex) {
                        this->selectionModel()->select(modelIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                        break;
                    }
                }
                break;
            }
        }
    }
}

void CatalogTreeView::resizeCoulumnWidth()
{
    if (m_sheet) {
        int maxPageColumnWid = this->fontMetrics().width(QString::number(m_sheet->pageCount())) + 34;
        this->setColumnWidth(0, this->width() - maxPageColumnWid);
        this->setColumnWidth(1, maxPageColumnWid);
    }
}

void CatalogTreeView::nextPage()
{
    const QModelIndex &newCurrent = this->moveCursor(QAbstractItemView::MoveDown, Qt::NoModifier);
    scrollToIndex(newCurrent);
}

void CatalogTreeView::pageDownPage()
{
    const QModelIndex &newCurrent = this->moveCursor(QAbstractItemView::MovePageDown, Qt::NoModifier);
    scrollToIndex(newCurrent);
}

void CatalogTreeView::prevPage()
{
    const QModelIndex &newCurrent = this->moveCursor(QAbstractItemView::MoveUp, Qt::NoModifier);
    scrollToIndex(newCurrent);
}

void CatalogTreeView::pageUpPage()
{
    const QModelIndex &newCurrent = this->moveCursor(QAbstractItemView::MovePageUp, Qt::NoModifier);
    scrollToIndex(newCurrent);
}

void CatalogTreeView::scrollToIndex(const QModelIndex &newIndex)
{
    if (newIndex.isValid()) {
        rightnotifypagechanged = false;
        currentChanged(newIndex, currentIndex());
        this->selectionModel()->select(newIndex, QItemSelectionModel::SelectCurrent);
        this->setCurrentIndex(newIndex);
    }
}

void CatalogTreeView::onFontChanged(const QFont &font)
{
    Q_UNUSED(font);
    resizeCoulumnWidth();
}
