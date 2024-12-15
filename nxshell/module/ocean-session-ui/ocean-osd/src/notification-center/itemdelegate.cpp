// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemdelegate.h"
#include "bubbletitlewidget.h"
#include "bubbleitem.h"
#include "../notification/constants.h"
#include "overlapwidet.h"
#include "notifylistview.h"

#include <QDebug>

ItemDelegate::ItemDelegate(NotifyListView *view, NotifyModel *model, QObject *parent)
    : QStyledItemDelegate(parent)
    , m_model(model)
    , m_view(view)
{
}

QWidget *ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)

    EntityPtr notify = index.data().value<EntityPtr>();
    if (!notify)
        return nullptr;

    if(notify->isTitle()) {
        BubbleTitleWidget *titleWidget = new BubbleTitleWidget(m_model, notify, parent);
        titleWidget->setParentView(m_view);
        return titleWidget;
    }

    if (notify->hideCount() != 0) {
        OverLapWidet *widget = new OverLapWidet(m_model, notify, parent);
        widget->setParentView(m_view);
        return widget;
    }

    BubbleItem *bubble = new BubbleItem(parent, notify);
    bubble->setParentModel(m_model);
    bubble->setParentView(m_view);
    return bubble;
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);

    EntityPtr notify = index.data().value<EntityPtr>();
    QSize bubbleSize(BubbleItemWidth, BubbleItem::bubbleItemHeight() + BubbleSpacing);

    if(notify && notify->isTitle())
        bubbleSize = QSize(BubbleTitleWidth, BubbleTitleWidget::bubbleTitleWidgetHeight());
    else if(notify->hideCount() != 0)
        bubbleSize = bubbleSize + QSize(0,notify->hideCount()*10);

    return bubbleSize;
}

void ItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)

    QRect rect = option.rect;
    QSize size = sizeHint(option, index);
    editor->setGeometry(rect.x(), rect.y(), size.width(), size.height() - BubbleSpacing);
}
