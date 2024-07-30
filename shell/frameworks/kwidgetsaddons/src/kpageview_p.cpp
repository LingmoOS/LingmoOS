/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2007 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpageview_p.h"

#include <QApplication>
#include <QHeaderView>
#include <QPainter>
#include <QScrollBar>
#include <QTextLayout>
#include <QVBoxLayout>

#include "kpagemodel.h"
#include "loggingcategory.h"

constexpr const auto viewWidth = 300;

using namespace KDEPrivate;

// KPagePlainView

KPagePlainView::KPagePlainView(QWidget *parent)
    : QAbstractItemView(parent)
{
    hide();
}

QModelIndex KPagePlainView::indexAt(const QPoint &) const
{
    return QModelIndex();
}

void KPagePlainView::scrollTo(const QModelIndex &, ScrollHint)
{
}

QRect KPagePlainView::visualRect(const QModelIndex &) const
{
    return QRect();
}

QModelIndex KPagePlainView::moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers)
{
    return QModelIndex();
}

int KPagePlainView::horizontalOffset() const
{
    return 0;
}

int KPagePlainView::verticalOffset() const
{
    return 0;
}

bool KPagePlainView::isIndexHidden(const QModelIndex &) const
{
    return false;
}

void KPagePlainView::setSelection(const QRect &, QFlags<QItemSelectionModel::SelectionFlag>)
{
}

QRegion KPagePlainView::visualRegionForSelection(const QItemSelection &) const
{
    return QRegion();
}

// KPageListView

KPageListView::KPageListView(QWidget *parent)
    : QListView(parent)
{
    if (layoutDirection() == Qt::RightToLeft) {
        setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::LeftEdge}));
    } else {
        setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::RightEdge}));
    }
    setViewMode(QListView::ListMode);
    setMovement(QListView::Static);
    setVerticalScrollMode(QListView::ScrollPerPixel);

    QFont boldFont(font());
    boldFont.setBold(true);
    setFont(boldFont);
}

KPageListView::~KPageListView()
{
}

void KPageListView::setModel(QAbstractItemModel *model)
{
    connect(model, &QAbstractItemModel::layoutChanged, this, &KPageListView::updateWidth);

    QListView::setModel(model);

    // Set our own selection model, which won't allow our current selection to be cleared
    setSelectionModel(new KDEPrivate::SelectionModel(model, this));

    updateWidth();
}

void KPageListView::changeEvent(QEvent *event)
{
    QListView::changeEvent(event);

    if (event->type() == QEvent::FontChange) {
        updateWidth();
    }
}

void KPageListView::setFlexibleWidth(bool flexibleWidth)
{
    m_flexibleWidth = flexibleWidth;
}

void KPageListView::updateWidth()
{
    if (!model()) {
        return;
    }
    if (m_flexibleWidth) {
        setFixedWidth(sizeHintForColumn(0) + verticalScrollBar()->sizeHint().width() + 5);
    } else {
        setFixedWidth(viewWidth);
    }
}

// KPageTreeView

KPageTreeView::KPageTreeView(QWidget *parent)
    : QTreeView(parent)
{
    if (layoutDirection() == Qt::RightToLeft) {
        setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::LeftEdge}));
    } else {
        setProperty("_breeze_borders_sides", QVariant::fromValue(QFlags{Qt::RightEdge}));
    }
    header()->hide();
}

void KPageTreeView::setModel(QAbstractItemModel *model)
{
    connect(model, &QAbstractItemModel::layoutChanged, this, &KPageTreeView::updateWidth);

    QTreeView::setModel(model);

    // Set our own selection model, which won't allow our current selection to be cleared
    setSelectionModel(new KDEPrivate::SelectionModel(model, this));

    updateWidth();
}

void KPageTreeView::updateWidth()
{
    if (!model()) {
        return;
    }

    int columns = model()->columnCount();

    expandItems();

    int width = 0;
    for (int i = 0; i < columns; ++i) {
        resizeColumnToContents(i);
        width = qMax(width, sizeHintForColumn(i));
    }

    setFixedWidth(qMax(viewWidth, width + 25));
}

void KPageTreeView::expandItems(const QModelIndex &index)
{
    setExpanded(index, true);

    const int count = model()->rowCount(index);
    for (int i = 0; i < count; ++i) {
        expandItems(model()->index(i, 0, index));
    }
}

// KPageTabbedView

KPageTabbedView::KPageTabbedView(QWidget *parent)
    : QAbstractItemView(parent)
{
    // hide the viewport of the QAbstractScrollArea
    const QList<QWidget *> list = findChildren<QWidget *>();
    for (int i = 0; i < list.count(); ++i) {
        list[i]->hide();
    }

    setFrameShape(NoFrame);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    mTabWidget = new QTabWidget(this);
    mTabWidget->setDocumentMode(true);
    connect(mTabWidget, &QTabWidget::currentChanged, this, &KPageTabbedView::currentPageChanged);

    layout->addWidget(mTabWidget);
}

KPageTabbedView::~KPageTabbedView()
{
    if (model()) {
        for (int i = 0; i < mTabWidget->count(); ++i) {
            QWidget *page = qvariant_cast<QWidget *>(model()->data(model()->index(i, 0), KPageModel::WidgetRole));

            if (page) {
                page->setVisible(false);
                page->setParent(nullptr); // reparent our children before they are deleted
            }
        }
    }
}

void KPageTabbedView::setModel(QAbstractItemModel *model)
{
    QAbstractItemView::setModel(model);

    connect(model, &QAbstractItemModel::layoutChanged, this, &KPageTabbedView::layoutChanged);

    layoutChanged();
}

QModelIndex KPageTabbedView::indexAt(const QPoint &) const
{
    if (model()) {
        return model()->index(0, 0);
    } else {
        return QModelIndex();
    }
}

void KPageTabbedView::scrollTo(const QModelIndex &index, ScrollHint)
{
    if (!index.isValid()) {
        return;
    }

    mTabWidget->setCurrentIndex(index.row());
}

QRect KPageTabbedView::visualRect(const QModelIndex &) const
{
    return QRect();
}

QSize KPageTabbedView::minimumSizeHint() const
{
    return mTabWidget->minimumSizeHint();
}

QModelIndex KPageTabbedView::moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers)
{
    return QModelIndex();
}

int KPageTabbedView::horizontalOffset() const
{
    return 0;
}

int KPageTabbedView::verticalOffset() const
{
    return 0;
}

bool KPageTabbedView::isIndexHidden(const QModelIndex &index) const
{
    return (mTabWidget->currentIndex() != index.row());
}

void KPageTabbedView::setSelection(const QRect &, QFlags<QItemSelectionModel::SelectionFlag>)
{
}

QRegion KPageTabbedView::visualRegionForSelection(const QItemSelection &) const
{
    return QRegion();
}

void KPageTabbedView::currentPageChanged(int index)
{
    if (!model()) {
        return;
    }

    QModelIndex modelIndex = model()->index(index, 0);

    selectionModel()->setCurrentIndex(modelIndex, QItemSelectionModel::ClearAndSelect);
}

void KPageTabbedView::layoutChanged()
{
    // save old position
    int pos = mTabWidget->currentIndex();

    // clear tab bar
    int count = mTabWidget->count();
    for (int i = 0; i < count; ++i) {
        mTabWidget->removeTab(0);
    }

    if (!model()) {
        return;
    }

    // add new tabs
    for (int i = 0; i < model()->rowCount(); ++i) {
        const QString title = model()->data(model()->index(i, 0)).toString();
        const QIcon icon = model()->data(model()->index(i, 0), Qt::DecorationRole).value<QIcon>();
        QWidget *page = qvariant_cast<QWidget *>(model()->data(model()->index(i, 0), KPageModel::WidgetRole));
        if (page) {
            QWidget *widget = new QWidget(this);
            QVBoxLayout *layout = new QVBoxLayout(widget);
            layout->setContentsMargins({});
            layout->addWidget(page);
            page->setVisible(true);
            mTabWidget->addTab(widget, icon, title);
        }
    }

    mTabWidget->setCurrentIndex(pos);
}

void KPageTabbedView::dataChanged(const QModelIndex &index, const QModelIndex &, const QList<int> &roles)
{
    if (!index.isValid()) {
        return;
    }

    if (index.row() < 0 || index.row() >= mTabWidget->count()) {
        return;
    }

    if (roles.isEmpty() || roles.contains(Qt::DisplayRole) || roles.contains(Qt::DecorationRole)) {
        const QString title = model()->data(index).toString();
        const QIcon icon = model()->data(index, Qt::DecorationRole).value<QIcon>();

        mTabWidget->setTabText(index.row(), title);
        mTabWidget->setTabIcon(index.row(), icon);
    }
}

// KPageListViewDelegate

KPageListViewDelegate::KPageListViewDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
}

static int layoutText(QTextLayout *layout, int maxWidth)
{
    qreal height = 0;
    int textWidth = 0;
    layout->beginLayout();
    while (true) {
        QTextLine line = layout->createLine();
        if (!line.isValid()) {
            break;
        }
        line.setLineWidth(maxWidth);
        line.setPosition(QPointF(0, height));
        height += line.height();
        textWidth = qMax(textWidth, qRound(line.naturalTextWidth() + 0.5));
    }
    layout->endLayout();
    return textWidth;
}

void KPageListViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }

    QStyleOptionViewItem opt(option);
    opt.showDecorationSelected = true;
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

    const QIcon::Mode iconMode = (option.state & QStyle::State_Selected) && (option.state & QStyle::State_Active) ? QIcon::Selected : QIcon::Normal;
    int iconSize = style->pixelMetric(QStyle::PM_IconViewIconSize);
    const QString text = index.model()->data(index, Qt::DisplayRole).toString();
    const QIcon icon = index.model()->data(index, Qt::DecorationRole).value<QIcon>();
    const QPixmap pixmap = icon.pixmap(iconSize, iconSize, iconMode);

    QFontMetrics fm = painter->fontMetrics();
    int wp = pixmap.width() / pixmap.devicePixelRatio();
    int hp = pixmap.height() / pixmap.devicePixelRatio();

    QTextLayout iconTextLayout(text, option.font);
    QTextOption textOption(Qt::AlignHCenter);
    iconTextLayout.setTextOption(textOption);
    int maxWidth = qMax(3 * wp, 8 * fm.height());
    layoutText(&iconTextLayout, maxWidth);

    QPen pen = painter->pen();
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active)) {
        cg = QPalette::Inactive;
    }

    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    painter->drawPixmap(option.rect.x() + (option.rect.width() / 2) - (wp / 2), option.rect.y() + 5, pixmap);
    if (!text.isEmpty()) {
        iconTextLayout.draw(painter, QPoint(option.rect.x() + (option.rect.width() / 2) - (maxWidth / 2), option.rect.y() + hp + 7));
    }

    painter->setPen(pen);

    drawFocus(painter, option, option.rect);
}

QSize KPageListViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QSize(0, 0);
    }

    QStyleOptionViewItem opt(option);
    opt.showDecorationSelected = true;
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

    int iconSize = style->pixelMetric(QStyle::PM_IconViewIconSize);
    const QString text = index.model()->data(index, Qt::DisplayRole).toString();
    const QIcon icon = index.model()->data(index, Qt::DecorationRole).value<QIcon>();
    const QPixmap pixmap = icon.pixmap(iconSize, iconSize);

    QFontMetrics fm = option.fontMetrics;
    int gap = fm.height();
    int wp = pixmap.width() / pixmap.devicePixelRatio();
    int hp = pixmap.height() / pixmap.devicePixelRatio();

    if (hp == 0) {
        // No pixmap loaded yet, we'll use the default icon size in this case.
        hp = iconSize;
        wp = iconSize;
    }

    QTextLayout iconTextLayout(text, option.font);
    int wt = layoutText(&iconTextLayout, qMax(3 * wp, 8 * fm.height()));
    int ht = iconTextLayout.boundingRect().height();

    int width;
    int height;
    if (text.isEmpty()) {
        height = hp;
    } else {
        height = hp + ht + 10;
    }

    width = qMax(wt, wp) + gap;

    return QSize(width, height);
}

void KPageListViewDelegate::drawFocus(QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect) const
{
    if (option.state & QStyle::State_HasFocus) {
        QStyleOptionFocusRect o;
        o.QStyleOption::operator=(option);
        o.rect = rect;
        o.state |= QStyle::State_KeyboardFocusChange;
        QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled;
        o.backgroundColor = option.palette.color(cg, (option.state & QStyle::State_Selected) ? QPalette::Highlight : QPalette::Window);

        QStyle *style = option.widget ? option.widget->style() : QApplication::style();
        style->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter, option.widget);
    }
}

// KPageListViewProxy

KPageListViewProxy::KPageListViewProxy(QObject *parent)
    : QAbstractProxyModel(parent)
{
}

KPageListViewProxy::~KPageListViewProxy()
{
}

int KPageListViewProxy::rowCount(const QModelIndex &) const
{
    return mList.count();
}

int KPageListViewProxy::columnCount(const QModelIndex &) const
{
    return 1;
}

QModelIndex KPageListViewProxy::index(int row, int column, const QModelIndex &) const
{
    if (column > 1 || row >= mList.count()) {
        return QModelIndex();
    } else {
        return createIndex(row, column, mList[row].internalPointer());
    }
}

QModelIndex KPageListViewProxy::parent(const QModelIndex &) const
{
    return QModelIndex();
}

QVariant KPageListViewProxy::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= mList.count()) {
        return QVariant();
    }

    return sourceModel()->data(mList[index.row()], role);
}

QModelIndex KPageListViewProxy::mapFromSource(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    for (int i = 0; i < mList.count(); ++i) {
        if (mList[i] == index) {
            return createIndex(i, 0, index.internalPointer());
        }
    }

    return QModelIndex();
}

QModelIndex KPageListViewProxy::mapToSource(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    return mList[index.row()];
}

void KPageListViewProxy::rebuildMap()
{
    mList.clear();

    const QAbstractItemModel *model = sourceModel();
    if (!model) {
        return;
    }

    for (int i = 0; i < model->rowCount(); ++i) {
        addMapEntry(model->index(i, 0));
    }

    for (int i = 0; i < mList.count(); ++i) {
        qCDebug(KWidgetsAddonsLog, "%d:0 -> %d:%d", i, mList[i].row(), mList[i].column());
    }

    Q_EMIT layoutChanged();
}

void KPageListViewProxy::addMapEntry(const QModelIndex &index)
{
    if (sourceModel()->rowCount(index) == 0) {
        mList.append(index);
    } else {
        const int count = sourceModel()->rowCount(index);
        for (int i = 0; i < count; ++i) {
            addMapEntry(sourceModel()->index(i, 0, index));
        }
    }
}

SelectionModel::SelectionModel(QAbstractItemModel *model, QObject *parent)
    : QItemSelectionModel(model, parent)
{
}

void SelectionModel::clear()
{
    // Don't allow the current selection to be cleared
}

void SelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
    // Don't allow the current selection to be cleared
    if (!index.isValid() && (command & QItemSelectionModel::Clear)) {
        return;
    }
    QItemSelectionModel::select(index, command);
}

void SelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
    // Don't allow the current selection to be cleared
    if (!selection.count() && (command & QItemSelectionModel::Clear)) {
        return;
    }
    QItemSelectionModel::select(selection, command);
}

#include "moc_kpageview_p.cpp"
