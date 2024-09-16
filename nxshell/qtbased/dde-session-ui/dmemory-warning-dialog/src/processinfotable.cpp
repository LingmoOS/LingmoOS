// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processinfotable.h"
#include "processinfomodel.h"

#include <QHeaderView>
#include <QDebug>
#include <QPainter>

ProcessInfoView::ProcessInfoView(QWidget *parent)
    : QTreeView(parent)
{
    setProperty("CheckAccessibleName", false);
    header()->setVisible(false);
    setRootIsDecorated(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(QFrame::NoFrame);
    setSelectionMode(NoSelection);
    setStyleSheet("QTreeView {"
                  "border: 1px solid #ccc;"
                  "border-radius: 5px;"
                  "}"
                  ""
                  "QTreeView::branch:!has-children {"
                  "border: none;"
                  "}"
                  ""
                  "QTreeView::branch:has-children {"
//                  "border: 1px solid red;"
                  "}"
                  "");
}

void ProcessInfoView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);

    QHeaderView *header = this->header();
    header->setAccessibleName("HeaderView");
    header->setSectionsMovable(false);
    header->setStretchLastSection(false);
    header->setSectionResizeMode(COLUMN_ICON, QHeaderView::Fixed);
    header->setSectionResizeMode(COLUMN_NAME, QHeaderView::Stretch);
//    header->setSectionResizeMode(COLUMN_MEM, QHeaderView::ResizeToContents);

    header->resizeSection(COLUMN_ICON, 24 + 10 + 20);
    header->resizeSection(COLUMN_MEM, 50);
    header->resizeSection(COLUMN_FREE_BTN, 80);

    connect(model, &QAbstractItemModel::layoutChanged, this, [=] { expandAll(); });
}

void ProcessInfoView::mouseReleaseEvent(QMouseEvent *e)
{
    QTreeView::mouseReleaseEvent(e);

    static_cast<ProcessInfoModel *>(model())->clearPressed();
}

void ProcessInfoView::drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    if (index.parent().isValid())
    {
        const QRect r = options.rect.marginsRemoved(QMargins(57, 0, 0, 0));

        painter->fillRect(r, QColor(0, 0, 0, 255 * .035));
        painter->drawText(r,
                          Qt::AlignVCenter | Qt::AlignLeft,
                          index.sibling(index.row(), 1).data(ProcessInfoModel::DisplayRole).toString());

        return;
    }

    QTreeView::drawRow(painter, options, index);
}
