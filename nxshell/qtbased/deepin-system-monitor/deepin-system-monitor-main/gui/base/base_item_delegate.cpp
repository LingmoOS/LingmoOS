// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later


#include "base_item_delegate.h"
#include "process/process_icon_cache.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyle>
#include <DStyleHelper>

#include <QAbstractItemView>
#include <QDebug>
#include <QFontMetrics>
#include <QHelpEvent>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QToolTip>

DWIDGET_USE_NAMESPACE

// content margin
const int margin = 10;
// content spacing
const int spacing = 10;

// constructor
BaseItemDelegate::BaseItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

// paint method for this delegate
void BaseItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    // check index validity
    if (!index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    // initialize style option for drawing
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

#ifdef ENABLE_INACTIVE_DISPLAY
    QWidget *wnd = DApplication::activeWindow();
#endif
    DPalette::ColorGroup cg;
    if (!(opt.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
#ifdef ENABLE_INACTIVE_DISPLAY
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            if (wnd->isModal()) {
                cg = DPalette::Inactive;
            } else {
                cg = DPalette::Active;
            }
        }
#else
        cg = DPalette::Active;
//        cg = DPalette::Current;
#endif
    }

    // DStyle instance
    auto *style = dynamic_cast<DStyle *>(DApplication::style());
    // icon size
    auto iconSize = style->pixelMetric(DStyle::PM_ListViewIconSize, &option);

    // global palette
    auto palette = DApplicationHelper::instance()->applicationPalette();

    QPen forground;
    if (index.data(Qt::UserRole + 2).isValid()) {
        // user provided text color (custom color used in treeview)
        forground.setColor(palette.color(cg, static_cast<DPalette::ColorType>(index.data(Qt::UserRole + 2).toInt())));
    } else {
        // default text color
        forground.setColor(palette.color(cg, DPalette::Text));
    }
    // adjust forground color only when in enabled state
    if (opt.state & DStyle::State_Enabled) {
        if (opt.state & DStyle::State_Selected) {
            // selected row's color
            forground.setColor(palette.color(cg, DPalette::TextLively));
            if (opt.state & DStyle::State_Sunken) {
                // color used when current row pressed by mouse
                auto hlColor = opt.palette.highlight().color();
                hlColor.setAlphaF(.1);
                auto newColor = style->adjustColor(forground.color(), 0, 0, 0, 0, 0, 0, -40);
                forground = style->blendColor(newColor, hlColor);
            } else if (opt.state & DStyle::State_MouseOver) {
                // color used when current row hovered by mouse
                forground = style->adjustColor(forground.color(), 0, 0, 20);
            }
        } else {
            // color used when it's normal row & hovered by mouse
            if (opt.state & DStyle::State_MouseOver) {
                auto type = DApplicationHelper::instance()->themeType();
                forground = style->adjustColor(forground.color(), 0, 0, type == DApplicationHelper::DarkType ? 20 : -50);
            }
        }
    }

    QRect rect = opt.rect;
    QFontMetrics fm(opt.font);
    QRect textRect = rect;

    // adjust left/right most column's left/right margin
    switch (opt.viewItemPosition) {
    case QStyleOptionViewItem::Beginning: {
        rect.setX(rect.x() + margin);  // left margin
    } break;
    case QStyleOptionViewItem::Middle: {
        // whole rect
    } break;
    case QStyleOptionViewItem::End: {
        rect.setWidth(rect.width() - margin);  // right margin
    } break;
    case QStyleOptionViewItem::OnlyOne: { // when view has only one column
        rect.setX(rect.x() + margin);
        rect.setWidth(rect.width() - margin);
    } break;
    default: {
        painter->restore();
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    }

    QString text;
    QRect iconRect;
    if (opt.viewItemPosition == QStyleOptionViewItem::Beginning ||
            opt.viewItemPosition == QStyleOptionViewItem::OnlyOne) {
        // left most column or when only has one column
        if (opt.features & QStyleOptionViewItem::HasDecoration) {
            // icon decoration needed
            textRect = rect;
            // | margin - icon - spacing - text - margin |
            textRect.setX(textRect.x() + margin + spacing + iconSize);
            textRect.setWidth(textRect.width() - margin);
            text = fm.elidedText(opt.text, opt.textElideMode, textRect.width());

            iconRect = rect;
            iconRect.setX(rect.x() + margin);
            iconRect.setWidth(iconSize);
        } else {
            // no icon decoration needed
            // | margin - text - margin |
            textRect = rect;
            textRect.setX(textRect.x() + margin);
            textRect.setWidth(textRect.width() - margin);
            text = fm.elidedText(opt.text, opt.textElideMode, textRect.width());
        }
    } else {
        // | margin - text - margin |
        textRect = rect;
        textRect.setX(textRect.x() + margin);
        textRect.setWidth(textRect.width() - margin);
        text = fm.elidedText(opt.text, opt.textElideMode, textRect.width());
    }

    // draw icon when decoration needed
    if (opt.features & QStyleOptionViewItem::HasDecoration &&
            (opt.viewItemPosition == QStyleOptionViewItem::Beginning ||
             opt.viewItemPosition == QStyleOptionViewItem::OnlyOne)) {
        // vmargins between icon and edge
        const QIcon &icon = opt.icon;
        auto diff = (iconRect.height() - iconSize) / 2;
        iconRect.adjust(0, diff, 0, -diff);

        // Qt的QIcon::pixmap()接口返回QPixmap对象的内存不会回收，此处使用QPixmapCache类来管理进程图标内存资源
        // 原有QPixmapCache::find接口返回的pixmap指针为空，改为返回pixmap引用对象，保证能根据进程名从缓存中获取到图标资源
        // https://pms.uniontech.com/bug-view-239575.html
        const QString &procName = index.data(Qt::UserRole + 4).toString();

        QPixmap  iconPixmap;
        core::process::ProcessIconCache::instance()->iconPixmapCache.find(procName, iconPixmap);
        if (!iconPixmap.isNull()) {
            painter->drawPixmap(iconRect, iconPixmap);
        } else {
            const QPixmap &iconPix = icon.pixmap(iconRect.size());
            core::process::ProcessIconCache::instance()->iconPixmapCache.insert(procName, iconPix);
            painter->drawPixmap(iconRect, iconPix);
        }
    }
    // draw content text
    painter->setPen(forground);
    painter->drawText(textRect, static_cast<int>(opt.displayAlignment), text);

    // restore painter state
    painter->restore();
}

// editor for editing item data, not used here
QWidget *BaseItemDelegate::createEditor(QWidget *, const QStyleOptionViewItem &,
                                        const QModelIndex &) const
{
    return nullptr;
}

// size hint for this delegate
QSize BaseItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(std::max(36, size.height()));
    return size;
}

// handler for help events
bool BaseItemDelegate::helpEvent(QHelpEvent *e, QAbstractItemView *view,
                                 const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!e || !view)
        return false;

    // only process tooltip events for now
    if (e->type() == QEvent::ToolTip) {
        QRect rect = view->visualRect(index);
        QRect textRect = rect;

        // initialize style option
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        // icon size
        auto iconSize = view->style()->pixelMetric(DStyle::PM_ListViewIconSize);

        // calc textRect's size (exclude any decoration region)
        if (index.column() == 0) {
            if (opt.features & QStyleOptionViewItem::HasDecoration) {
                textRect.setX(textRect.x() + margin * 2 + spacing + iconSize);
            } else {
                textRect.setX(textRect.x() + 2 * margin);
            }
        } else {
            textRect.setX(textRect.x() + margin);
        }

        if (rect.x() + rect.width() >= view->width()) {
            textRect.setWidth(textRect.width() - margin * 2);
        } else {
            textRect.setWidth(textRect.width() - margin);
        }

        // max width of current text calculated with current option's font metric
        QFontMetrics fm(opt.font);
        int w = fm.size(Qt::TextSingleLine, opt.text).width();

        // show tooptip text when text is truncated (text width is larger than visible region)
        if (textRect.width() < w) {
            QVariant tooltip = index.data(Qt::DisplayRole);
            if (tooltip.canConvert<QString>()) {
                QToolTip::showText(e->globalPos(),
                                   QString("<div>%1</div>").arg(tooltip.toString().toHtmlEscaped()),
                                   view);
                return true;
            }
        }
        if (!QStyledItemDelegate::helpEvent(e, view, option, index))
            QToolTip::hideText();
        return true;
    }

    return QStyledItemDelegate::helpEvent(e, view, option, index);
}

// initialize style option
void BaseItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    option->showDecorationSelected = true;
    bool ok = false;
    // text alignment option
    if (index.data(Qt::TextAlignmentRole).isValid()) {
        uint value = index.data(Qt::TextAlignmentRole).toUInt(&ok);
        option->displayAlignment = static_cast<Qt::Alignment>(value);
    }
    // use default alignment
    if (!ok)
        option->displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    // text elide option
    option->textElideMode = Qt::ElideRight;
    // has display role
    option->features = QStyleOptionViewItem::HasDisplay;
    if (index.data(Qt::DisplayRole).isValid())
        option->text = index.data().toString();

    // check if has decoration role
    if (index.data(Qt::DecorationRole).isValid()) {
        option->features |= QStyleOptionViewItem::HasDecoration;
        option->icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    }
}
