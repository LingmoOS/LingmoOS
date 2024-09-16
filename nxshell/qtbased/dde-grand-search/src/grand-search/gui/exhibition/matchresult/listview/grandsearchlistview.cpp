// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grandsearchlistview.h"
#include "grandsearchlistmodel.h"
#include "grandsearchlistdelegate.h"
#include "utils/utils.h"
#include "global/matcheditem.h"
#include "global/builtinsearch.h"
#include "global/accessibility/acintelfunctions.h"

#include <DGuiApplicationHelper>
#include <DStandardPaths>

#include <QDebug>
#include <QFileInfo>
#include <QPainter>
#include <QMouseEvent>

using namespace GrandSearch;
DCORE_USE_NAMESPACE
DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

#define ICON_SIZE 24
#define ListItemTextMaxWidth      240      // 文本元素最大显示宽度

GrandSearchListView::GrandSearchListView(QWidget *parent)
    : DListView(parent)
{
    m_model = new GrandSearchListModel(0, 0, this);
    setModel(m_model);
    m_delegate = new GrandSearchListDelegate(this);
    setItemDelegate(m_delegate);
    setViewportMargins(0, 0, 0, 0);
    setUniformItemSizes(true);

    setViewMode(QListView::ListMode);
    setResizeMode(QListView::Adjust);
    setMovement(QListView::Static);
    setLayoutMode(QListView::Batched);
    setBatchSize(2000);
    setMouseTracking(true);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &GrandSearchListView::onSetThemeType);

    m_themeType = DGuiApplicationHelper::instance()->themeType();
}

GrandSearchListView::~GrandSearchListView()
{

}

void GrandSearchListView::setMatchedItems(const MatchedItems &items)
{
    m_matchedItems = items;
    m_model->clear();
    for (MatchedItem item : m_matchedItems) {
        addRow(item);
    }
}

void GrandSearchListView::addRow(const MatchedItem &item)
{
    QStandardItem *newItem = new QStandardItem;

    AC_SET_ACCESSIBLE_TEXT(newItem, item.name);

    m_model->appendRow(newItem);

    auto row = m_model->rowCount() - 1;
    QModelIndex index = m_model->index(row, 0, QModelIndex());

    setData(index, item);
}

void GrandSearchListView::addRow(const MatchedItem &item, const int level)
{
    int lastRow = levelItemLastRow(level);
    insertRow(lastRow, item);
}

void GrandSearchListView::addRows(const MatchedItems &items)
{
    for (auto item : items)
        addRow(item);
}

void GrandSearchListView::addRows(const MatchedItems &items, const int level)
{
    int lastRow = levelItemLastRow(level);
    insertRows(lastRow, items);
}

int GrandSearchListView::insertRow(int nRow, const MatchedItem &item)
{
    if (nRow < 0 || nRow >= m_model->rowCount()) {
        addRow(item);
        return m_model->rowCount() - 1;
    }

    QStandardItem *newItem = new QStandardItem;
    m_model->insertRow(nRow, newItem);

    QModelIndex index = m_model->index(nRow, 0, QModelIndex());
    setData(index, item);

    return nRow;
}

void GrandSearchListView::insertRows(int nRow, const MatchedItems &items)
{
    if (nRow < 0 || nRow >= m_model->rowCount()) {
        addRows(items);
        return;
    }

    for (int i = 0; i < items.size(); i++) {
        insertRow(nRow,items[i]);
    }
}

void GrandSearchListView::removeRows(int nRow, int nCount)
{
    m_model->removeRows(nRow, nCount);
}

int GrandSearchListView::levelItemCount(const int level)
{
    int count = 0;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        auto item = m_model->data(m_model->index(i, 0, QModelIndex()), DATA_ROLE).value<MatchedItem>();
        // 如果当前项不是分层项，则说明后续已经没有分层项
        int tempLevel = -1;
        if (!Utils::isLevelItem(item, tempLevel))
            break;
        if (-1 == level || level == tempLevel)
            count++;
    }

    return count;
}

int GrandSearchListView::rowCount()
{
    return m_model->rowCount();
}

int GrandSearchListView::getThemeType() const
{
    return m_themeType;
}

void GrandSearchListView::clear()
{
    if (m_model)
        m_model->clear();
}

void GrandSearchListView::updatePreviewItemState(const bool preview)
{
    m_isPreviewItem = preview;
}

bool GrandSearchListView::isPreviewItem() const
{
    return m_isPreviewItem;
}

void GrandSearchListView::onSetThemeType(int type)
{
    m_themeType = type;
}

bool GrandSearchListView::event(QEvent *event)
{
    Q_UNUSED(event)
    return QListView::event(event);
}

void GrandSearchListView::mousePressEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton: {
        const QModelIndex &index = indexAt(event->pos());
        if (!index.isValid())
            break;

        MatchedItem item = index.data(DATA_ROLE).value<MatchedItem>();
        if (event->modifiers() == Qt::ControlModifier) {
            Utils::openMatchedItemWithCtrl(item);
            emit sigItemClicked();
        } else {
            if (this->currentIndex() != index) {
                setCurrentIndex(index);
                // 通知选择已经改变
                emit sigCurrentItemChanged(item);
            } else {
                Utils::openMatchedItem(item);
                emit sigItemClicked();
            }
        }

        break;
    }
    default:
        break;
    }

    DListView::mousePressEvent(event);
}

QString GrandSearchListView::cacheDir()
{
    auto userCachePath = DStandardPaths::standardLocations(QStandardPaths::CacheLocation).value(0);
    return userCachePath;
}

void GrandSearchListView::setData(const QModelIndex& index, const MatchedItem &item)
{
    if (!index.isValid())
        return;

    QVariant searchMeta;
    searchMeta.setValue(item);
    m_model->setData(index, searchMeta, DATA_ROLE);

    // 添加悬浮提示
    QFontMetricsF fontWidth(DFontSizeManager::instance()->get(DFontSizeManager::T6));
    QString mtext = fontWidth.elidedText(item.name, Qt::ElideRight, ListItemTextMaxWidth);
    if (mtext != item.name) {
        m_model->setData(index, item.name, Qt::ToolTipRole);
    }

    // 设置icon
    QVariant iconVariantData;
    const QString &strIcon = item.icon;
    if (!strIcon.isEmpty()) {
        const int size = static_cast<int>(ICON_SIZE);
        const QSize iconSize(size, size);

        // 判断icon是路径还是图标名
        if (strIcon.contains('/')) {
            QFileInfo file(strIcon);
            if (file.exists())
                iconVariantData.setValue(QIcon(strIcon).pixmap(iconSize));
            else
                iconVariantData.setValue(Utils::defaultIcon(item).pixmap(iconSize));
        } else {
            QIcon icon = QIcon::fromTheme(strIcon);
            if (icon.isNull())
                iconVariantData.setValue(Utils::defaultIcon(item).pixmap(iconSize));
            else
                iconVariantData.setValue(icon.pixmap(iconSize));
        }
    }

    m_model->setData(index,iconVariantData, Qt::DecorationRole);
}

int GrandSearchListView::levelItemLastRow(const int level)
{
    int lastRow = 0;
    if (level < 1) {
        lastRow = m_model->rowCount();
    } else {
        for (int i = 1; i <= level; ++i) {
            lastRow += levelItemCount(i);
        }
    }

    return lastRow;
}
