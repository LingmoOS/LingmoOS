// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sessionpopupwidget.h"

#include <DStyle>
#include <DStandardItem>
#include <QStandardItemModel>

const int ITEM_WIDTH = 180;
const int ITEM_HEIGHT = 34;
const int ITEM_SPACING = 2;

SessionPopupWidget::SessionPopupWidget(QWidget *parent)
    : DListView(parent)
    , m_model(new QStandardItemModel(this))
{
    initUI();

    // enter键盘响应
    connect(this, &SessionPopupWidget::activated, this, &SessionPopupWidget::itemClicked);
    connect(this, &SessionPopupWidget::clicked, this, &SessionPopupWidget::itemClicked);
}

void SessionPopupWidget::setSessionInfo(const QMap<QString, QString> &infos, const QString &curSession)
{
    m_curSession = curSession;
    QMapIterator<QString, QString> it(infos);
    while (it.hasNext()) {
        it.next();
        addItem(it.value(), it.key());
    }

    resize(ITEM_WIDTH, ITEM_HEIGHT * m_model->rowCount() + m_model->rowCount() * 2 * ITEM_SPACING);
}

void SessionPopupWidget::updateCurrentSession(const QString &curSession)
{
    if (curSession == m_curSession) {
        return;
    }

    m_curSession = curSession;
    updateSelectedState();
}

void SessionPopupWidget::initUI()
{
    setAccessibleName(QStringLiteral("sessionListview"));
    setFrameShape(QFrame::NoFrame);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setViewportMargins(0, 0, 0, 0);
    setItemSpacing(0);
    setSpacing(ITEM_SPACING);
    setItemSize(QSize(ITEM_WIDTH, ITEM_HEIGHT));

    setModel(m_model);
}

void SessionPopupWidget::addItem(const QString &icon, const QString &itemName)
{
    DStandardItem *item = new DStandardItem();
    item->setFontSize(DFontSizeManager::T6);
    item->setIcon(QIcon(icon));
    item->setText(itemName);
    item->setEditable(false);

    QSize iconSize(12, 10);
    auto rightAction = new DViewItemAction(Qt::AlignVCenter, iconSize, iconSize, true);
    QIcon actIcon;
    if (itemName != m_curSession) {
        actIcon = QIcon();
    } else {
        DStyle *dStyle = qobject_cast<DStyle *>(style());
        actIcon = dStyle ? dStyle->standardIcon(DStyle::SP_MarkElement) : QIcon();
    }

    rightAction->setIcon(actIcon);
    item->setActionList(Qt::Edge::RightEdge, { rightAction });

    m_model->appendRow(item);
}

void SessionPopupWidget::itemClicked(const QModelIndex &index)
{
    QString session = index.data().toString();
    if (session == m_curSession) {
        return;
    }

    m_curSession = session;
    updateSelectedState();

    Q_EMIT sessionItemClicked(session);
}

void SessionPopupWidget::updateSelectedState()
{
    for (int i = 0; i < m_model->rowCount(); i++) {
        auto item = static_cast<DStandardItem *>(m_model->item(i));
        auto action = item->actionList(Qt::Edge::RightEdge).first();
        if (item->text() != m_curSession) {
            action->setIcon(QIcon());
            update(item->index());
            continue;
        }

        DStyle *dStyle = qobject_cast<DStyle *>(style());
        QIcon icon = dStyle ? dStyle->standardIcon(DStyle::SP_MarkElement) : QIcon();
        action->setIcon(icon);
        setCurrentIndex(item->index());
        update(item->index());
    }
}
