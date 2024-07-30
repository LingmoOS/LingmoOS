/*
    SPDX-FileCopyrightText: 2012-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katecolortreewidget.h"

#include "katecategorydrawer.h"

#include <QPainter>
#include <QStyledItemDelegate>

#include <KLocalizedString>

#include <QColorDialog>
#include <QEvent>
#include <QKeyEvent>
#include <qdrawutil.h>

// BEGIN KateColorTreeItem
class KateColorTreeItem : public QTreeWidgetItem
{
public:
    KateColorTreeItem(const KateColorItem &colorItem, QTreeWidgetItem *parent = nullptr)
        : QTreeWidgetItem(parent)
        , m_colorItem(colorItem)
    {
        setText(0, m_colorItem.name);
        if (!colorItem.whatsThis.isEmpty()) {
            setData(1, Qt::WhatsThisRole, colorItem.whatsThis);
        }
        if (!colorItem.useDefault) {
            setData(2, Qt::ToolTipRole, i18n("Use default color from the color theme"));
        }
    }

    QColor color() const
    {
        return m_colorItem.color;
    }

    void setColor(const QColor &c)
    {
        m_colorItem.color = c;
    }

    QColor defaultColor() const
    {
        return m_colorItem.defaultColor;
    }

    bool useDefaultColor() const
    {
        return m_colorItem.useDefault;
    }

    void setUseDefaultColor(bool useDefault)
    {
        m_colorItem.useDefault = useDefault;
        QString tooltip = useDefault ? QString() : i18n("Use default color from the color theme");
        setData(2, Qt::ToolTipRole, tooltip);
    }

    QString key() const
    {
        return m_colorItem.key;
    }

    KateColorItem colorItem() const
    {
        return m_colorItem;
    }

private:
    KateColorItem m_colorItem;
};
// END KateColorTreeItem

// BEGIN KateColorTreeDelegate
class KateColorTreeDelegate : public QStyledItemDelegate
{
public:
    KateColorTreeDelegate(KateColorTreeWidget *widget)
        : QStyledItemDelegate(widget)
        , m_tree(widget)
    {
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QSize sh = QStyledItemDelegate::sizeHint(option, index);
        if (!index.parent().isValid()) {
            sh.rheight() += 2 * m_categoryDrawer.leftMargin();
        } else {
            sh.rheight() += m_categoryDrawer.leftMargin();
        }
        if (index.column() == 0) {
            sh.rwidth() += m_categoryDrawer.leftMargin();
        } else if (index.column() == 1) {
            sh.rwidth() = 150;
        } else {
            sh.rwidth() += m_categoryDrawer.leftMargin();
        }

        return sh;
    }

    QRect fullCategoryRect(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QModelIndex i = index;
        if (i.parent().isValid()) {
            i = i.parent();
        }

        QTreeWidgetItem *item = m_tree->itemFromIndex(i);
        QRect r = m_tree->visualItemRect(item);

        // adapt width
        r.setLeft(m_categoryDrawer.leftMargin());
        r.setWidth(m_tree->viewport()->width() - m_categoryDrawer.leftMargin() - m_categoryDrawer.rightMargin());

        // adapt height
        if (item->isExpanded() && item->childCount() > 0) {
            const int childCount = item->childCount();
            const int h = sizeHint(option, index.model()->index(0, 0, index)).height();
            r.setHeight(r.height() + childCount * h);
        }

        r.setTop(r.top() + m_categoryDrawer.leftMargin());

        return r;
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_ASSERT(index.isValid());
        Q_ASSERT(index.column() >= 0 && index.column() <= 2);

        // BEGIN: draw toplevel items
        if (!index.parent().isValid()) {
            QStyleOptionViewItem opt(option);
            const QRegion cl = painter->clipRegion();
            painter->setClipRect(opt.rect);
            opt.rect = fullCategoryRect(option, index);
            m_categoryDrawer.drawCategory(index, 0, opt, painter);
            painter->setClipRegion(cl);
            return;
        }
        // END: draw toplevel items

        // BEGIN: draw background of category for all other items
        {
            QStyleOptionViewItem opt(option);
            opt.rect = fullCategoryRect(option, index);
            const QRegion cl = painter->clipRegion();
            QRect cr = option.rect;
            if (index.column() == 0) {
                if (m_tree->layoutDirection() == Qt::LeftToRight) {
                    cr.setLeft(5);
                } else {
                    cr.setRight(opt.rect.right());
                }
            }
            painter->setClipRect(cr);
            m_categoryDrawer.drawCategory(index, 0, opt, painter);
            painter->setClipRegion(cl);
            painter->setRenderHint(QPainter::Antialiasing, false);
        }
        // END: draw background of category for all other items

        painter->setClipRect(option.rect);

        // BEGIN: draw color button
        if (index.column() == 1) {
            KateColorTreeItem *item = static_cast<KateColorTreeItem *>(m_tree->itemFromIndex(index));
            QColor color = item->useDefaultColor() ? item->defaultColor() : item->color();

            QStyleOptionButton opt;
            opt.rect = option.rect;
            opt.palette = m_tree->palette();

            m_tree->style()->drawControl(QStyle::CE_PushButton, &opt, painter, m_tree);
            opt.rect = m_tree->style()->subElementRect(QStyle::SE_PushButtonContents, &opt, m_tree);
            opt.rect.adjust(1, 1, -1, -1);
            painter->fillRect(opt.rect, color);

            qDrawShadePanel(painter, opt.rect, opt.palette, true, 1, nullptr);
        }
        // END: draw color button

        // BEGIN: draw reset icon
        if (index.column() == 2 && !static_cast<KateColorTreeItem *>(m_tree->itemFromIndex(index))->useDefaultColor()) {
            // get right pixmap
            const bool enabled = (option.state & QStyle::State_MouseOver || option.state & QStyle::State_HasFocus);
            const QPixmap p = QIcon::fromTheme(QStringLiteral("edit-undo")).pixmap(16, 16, enabled ? QIcon::Normal : QIcon::Disabled);

            // compute rect with scaled sizes
            const QRect rect(option.rect.left() + 10,
                             option.rect.top() + (option.rect.height() - p.height() / p.devicePixelRatio() + 1) / 2,
                             p.width() / p.devicePixelRatio(),
                             p.height() / p.devicePixelRatio());
            painter->drawPixmap(rect, p);
        }
        // END: draw reset icon

        // paint the text
        QStyledItemDelegate::paint(painter, option, index);
    }

private:
    KateColorTreeWidget *m_tree;
    KateCategoryDrawer m_categoryDrawer;
};
// END KateColorTreeDelegate

KateColorTreeWidget::KateColorTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    setItemDelegate(new KateColorTreeDelegate(this));

    QStringList headers;
    headers << QString() // i18nc("@title:column the color name", "Color Role")
            << QString() // i18nc("@title:column a color button", "Color")
            << QString(); // i18nc("@title:column use default color", "Reset")
    setHeaderLabels(headers);
    setHeaderHidden(true);
    setRootIsDecorated(false);
    setIndentation(25);
    setSelectionMode(QAbstractItemView::NoSelection);
}

bool KateColorTreeWidget::edit(const QModelIndex &index, EditTrigger trigger, QEvent *event)
{
    if (m_readOnly) {
        return false;
    }

    // accept edit only for color buttons in column 1 and reset in column 2
    if (!index.parent().isValid() || index.column() < 1) {
        return QTreeWidget::edit(index, trigger, event);
    }

    bool accept = false;
    if (event && event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        accept = (ke->key() == Qt::Key_Space); // allow Space to edit
    }

    switch (trigger) {
    case QAbstractItemView::DoubleClicked:
    case QAbstractItemView::SelectedClicked:
    case QAbstractItemView::EditKeyPressed: // = F2
        accept = true;
        break;
    default:
        break;
    }

    if (accept) {
        KateColorTreeItem *item = static_cast<KateColorTreeItem *>(itemFromIndex(index));
        const QColor color = item->useDefaultColor() ? item->defaultColor() : item->color();

        if (index.column() == 1) {
            const QColor selectedColor = QColorDialog::getColor(color, this, QString(), QColorDialog::ShowAlphaChannel);

            if (selectedColor.isValid()) {
                item->setUseDefaultColor(false);
                item->setColor(selectedColor);
                viewport()->update();
                Q_EMIT changed();
            }
        } else if (index.column() == 2 && !item->useDefaultColor()) {
            item->setUseDefaultColor(true);
            viewport()->update();
            Q_EMIT changed();
        }

        return false;
    }
    return QTreeWidget::edit(index, trigger, event);
}

void KateColorTreeWidget::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    Q_UNUSED(painter)
    Q_UNUSED(rect)
    Q_UNUSED(index)
}

void KateColorTreeWidget::selectDefaults()
{
    bool somethingChanged = false;

    // use default colors for all selected items
    for (int a = 0; a < topLevelItemCount(); ++a) {
        QTreeWidgetItem *top = topLevelItem(a);
        for (int b = 0; b < top->childCount(); ++b) {
            KateColorTreeItem *it = static_cast<KateColorTreeItem *>(top->child(b));
            Q_ASSERT(it);
            if (!it->useDefaultColor()) {
                it->setUseDefaultColor(true);
                somethingChanged = true;
            }
        }
    }

    if (somethingChanged) {
        viewport()->update();
        Q_EMIT changed();
    }
}

void KateColorTreeWidget::addColorItem(const KateColorItem &colorItem)
{
    QTreeWidgetItem *categoryItem = nullptr;
    for (int i = 0; i < topLevelItemCount(); ++i) {
        if (topLevelItem(i)->text(0) == colorItem.category) {
            categoryItem = topLevelItem(i);
            break;
        }
    }

    if (!categoryItem) {
        categoryItem = new QTreeWidgetItem();
        categoryItem->setText(0, colorItem.category);
        addTopLevelItem(categoryItem);
        expandItem(categoryItem);
    }

    new KateColorTreeItem(colorItem, categoryItem);

    resizeColumnToContents(0);
}

void KateColorTreeWidget::addColorItems(const QList<KateColorItem> &colorItems)
{
    for (const KateColorItem &item : colorItems) {
        addColorItem(item);
    }
}

QList<KateColorItem> KateColorTreeWidget::colorItems() const
{
    QList<KateColorItem> items;
    for (int a = 0; a < topLevelItemCount(); ++a) {
        QTreeWidgetItem *top = topLevelItem(a);
        for (int b = 0; b < top->childCount(); ++b) {
            KateColorTreeItem *item = static_cast<KateColorTreeItem *>(top->child(b));
            Q_ASSERT(item);
            items.append(item->colorItem());
        }
    }
    return items;
}

QColor KateColorTreeWidget::findColor(const QString &key) const
{
    for (int a = 0; a < topLevelItemCount(); ++a) {
        QTreeWidgetItem *top = topLevelItem(a);
        for (int b = 0; b < top->childCount(); ++b) {
            KateColorTreeItem *item = static_cast<KateColorTreeItem *>(top->child(b));
            if (item->key() == key) {
                if (item->useDefaultColor()) {
                    return item->defaultColor();
                } else {
                    return item->color();
                }
            }
        }
    }
    return QColor();
}

bool KateColorTreeWidget::readOnly() const
{
    return m_readOnly;
}

void KateColorTreeWidget::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
}

#include "moc_katecolortreewidget.cpp"
