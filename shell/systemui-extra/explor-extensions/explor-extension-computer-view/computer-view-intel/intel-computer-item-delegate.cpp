/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include <gio/gio.h>
#include "intel-computer-view.h"
#include "intel-computer-proxy-model.h"
#include "intel-computer-item-delegate.h"
#include "intel-abstract-computer-item.h"

#include <QStyle>
#include <QDebug>
#include <QPainter>
#include <QListView>
#include <QApplication>
#include <QPainterPath>

using namespace Intel;

ComputerItemDelegate::ComputerItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    m_styleIconView = new QListView;
    m_styleIconView->setViewMode(QListView::IconMode);
}

ComputerItemDelegate::~ComputerItemDelegate()
{
    m_styleIconView->deleteLater();
}

void ComputerItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    //use rounded rect primitive in lingmo style.
    if (index.parent().isValid()) {
        opt.decorationPosition = QStyleOptionViewItem::Top;
        opt.decorationSize = QSize(64, 64);
        opt.features = QStyleOptionViewItem::WrapText;
        opt.displayAlignment = Qt::AlignHCenter|Qt::AlignTop;
        opt.rect.adjust(2, 2, -2, -2);
        //opt.features.setFlag(QStyleOptionViewItem::WrapText);
    }

    auto view = qobject_cast<ComputerView *>(parent());
    auto model = qobject_cast<ComputerProxyModel *>(view->model());
    auto item = model->itemFromIndex(index);

    if (!item)
        return;
    switch (item->itemType()) {
    case AbstractComputerItem::Volume:
        paintVolumeItem(painter, opt, index, item);
        break;
    case AbstractComputerItem::RemoteVolume:
        paintRemoteItem(painter, opt, index, item);
        break;
    case AbstractComputerItem::Network:
        paintNetworkItem(painter, opt, index, item);
        break;
    default:
        break;
    }
}

void ComputerItemDelegate::paintVolumeItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, AbstractComputerItem *item) const
{
    auto opt = option;
    auto bg = opt.palette.highlight().color();
    int hue = bg.hue();
    //int s = bg.saturation();
    // int v = bg.value();
    bg.setHsv(hue, 10, 127);

    opt.palette.setColor(QPalette::Highlight, bg);
    //qApp->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, m_styleIconView);
    bool isHover = (option.state & QStyle::State_MouseOver) && (option.state & ~QStyle::State_Selected);
    bool isSelected = option.state & QStyle::State_Selected;
    bool enable = option.state & QStyle::State_Enabled;
    QColor color = option.palette.color(enable? QPalette::Active: QPalette::Disabled,
                                        QPalette::Highlight);

    color.setAlpha(0);
    if (isHover && !isSelected) {
        int h = color.hsvHue();
        //int s = color.hsvSaturation();
        auto base = option.palette.base().color();
        int v = color.value();
        color.setHsv(h, base.lightness(), v, 64);
    }
    if (isSelected) {
        color.setAlpha(127);
    }
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::transparent);
    painter->setBrush(color);
    painter->drawRoundedRect(option.rect, 6, 6);
    painter->restore();

    if (index.parent().isValid()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        //draw pix map
        QIcon icon = option.icon;
        bool enable = option.state & QStyle::State_Enabled;
        bool selected = option.state &QStyle::State_Selected;
        QPixmap pix = icon.pixmap(QSize(64, 64), enable? selected? QIcon::Selected: QIcon::Normal: QIcon::Disabled);
        qApp->style()->drawItemPixmap(painter, option.rect.adjusted(5, 0, 0, 0), Qt::AlignVCenter|Qt::AlignLeft, pix);

        //draw text
        auto textRect = option.rect;
        textRect.adjust(84, 10, -5, -10);
        textRect.translate(0, -option.fontMetrics.ascent());
        qApp->style()->drawItemText(painter, textRect, Qt::AlignLeft|Qt::AlignVCenter, option.palette, enable, option.text, QPalette::Text);

        //space
        bool shouldDrawProgress = false;
        QString spaceInfo;
        auto total = item->totalSpace();
        auto used = item->usedSpace();
        if (total > 0) {
            auto totalFormat = g_format_size(total);
            auto usedFormat = g_format_size(used);
            spaceInfo = QString("%1/%2").arg(usedFormat).arg(totalFormat);
            g_free(totalFormat);
            g_free(usedFormat);
            shouldDrawProgress = true;
        } else {
            if (!item->isMount()) {
                spaceInfo = tr("You should mount volume first");
            }
        }
        qApp->style()->drawItemText(painter, textRect.translated(0, 2*option.fontMetrics.ascent()), Qt::AlignLeft|Qt::AlignVCenter|Qt::TextWordWrap, option.palette, enable, spaceInfo, QPalette::WindowText);

        if (shouldDrawProgress) {
            painter->save();

            QPainterPath clipPath;
            clipPath.addRoundedRect(option.rect, 6, 6);
            painter->setClipPath(clipPath);
            qreal percent = used*1.0/total*1.0;
            int progressBarWidth = option.rect.width() * percent;
            painter->save();
            QPen pen;
            pen.setCapStyle(Qt::PenCapStyle::RoundCap);
            pen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
            pen.setWidth(6);
            pen.setColor(option.palette.midlight().color());
            painter->setPen(pen);
            painter->drawLine(option.rect.bottomLeft() + QPoint(5, -5), option.rect.bottomRight() + QPoint(-5, -5));

            //pen.setColor(percent < 0.8?Qt::blue:Qt::red);
            if (percent < 0.8) {
                pen.setColor(option.palette.highlight().color());
            } else {
                pen.setColor(Qt::red);
            }

            painter->setPen(pen);
            auto pos = option.rect.bottomLeft();
            pos.setX(pos.x() + progressBarWidth);
            pos.setY(pos.y() - 5);
            painter->drawLine(option.rect.bottomLeft() + QPoint(5, -5), pos);
            painter->restore();
            painter->restore();
        }
        painter->restore();
    } else {
        //auto textRect = qApp->style()->subElementRect(QStyle::SE_ItemViewItemText, &option, m_styleIconView);
        drawTab(painter, option, index, item);
    }
}

void ComputerItemDelegate::paintRemoteItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, AbstractComputerItem *item) const
{
    if (index.parent().isValid()) {
        qApp->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, nullptr);
        drawStyledItem(painter, option, index);
    } else {
        drawTab(painter, option, index, item);
    }
}

void ComputerItemDelegate::paintNetworkItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, AbstractComputerItem *item) const
{
    if (index.parent().isValid()) {
        qApp->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, nullptr);
        drawStyledItem(painter, option, index);
    } else {
        drawTab(painter, option, index, item);
    }
}

void ComputerItemDelegate::drawTab(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, AbstractComputerItem *item) const
{
    auto opt = option;
    auto titleFont = opt.font;

    // Modify tab name is smaller
    if (titleFont.pixelSize() > 0) {
        titleFont.setPixelSize(titleFont.pixelSize()*1.2);
    } else {
        titleFont.setPointSizeF(titleFont.pointSizeF()*1.2);
    }

    opt.icon = QIcon();
    opt.decorationPosition = QStyleOptionViewItem::Right;
    opt.displayAlignment = Qt::AlignLeft|Qt::AlignVCenter;
    opt.font = titleFont;
    opt.fontMetrics = QFontMetrics(opt.font);

    qApp->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
    //auto textRect = qApp->style()->subElementRect(QStyle::SE_ItemViewItemText, &opt, m_styleIconView);
    //qApp->style()->drawItemText(painter, opt.rect.adjusted(5, 0, 0, 0), Qt::AlignTop|Qt::AlignVCenter, option.palette, enable, option.text, selected? QPalette::HighlightedText: QPalette::Text);

    // add by wwn
    painter->setPen(QPen(QColor(125,125,125,125), 1));
    painter->drawLine(opt.rect.bottomLeft().x() + 110, opt.rect.center().y() + 3 ,
                      opt.rect.bottomRight().x() - 50, opt.rect.center().y() + 3);
    QRect rect = option.rect;
    rect.setTop(rect.top() + 12);
    rect.setX(rect.right() - 40);
    rect.setSize(QSize(16, 16));
    if (item->isExpanded())
        painter->drawPixmap(rect, QPixmap(":/img/view_show"));
    else
        painter->drawPixmap(rect, QPixmap(":/img/view_hide"));
}

void ComputerItemDelegate::drawStyledItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    //draw pix map
    QIcon icon = option.icon;
    bool enable = option.state & QStyle::State_Enabled;
    bool selected = option.state &QStyle::State_Selected;
    QPixmap pix = icon.pixmap(QSize(64, 64), enable? selected? QIcon::Selected: QIcon::Normal: QIcon::Disabled);
    qApp->style()->drawItemPixmap(painter, option.rect.adjusted(0, 5, 0, 0), Qt::AlignTop|Qt::AlignHCenter, pix);

    //draw text
    auto textRect = option.rect.adjusted(2, 74, -2, -2);
    qApp->style()->drawItemText(painter, textRect, Qt::ElideRight|Qt::TextWrapAnywhere|Qt::AlignTop|Qt::AlignHCenter, option.palette,
                                enable, option.text, selected? QPalette::HighlightedText: QPalette::Text);
    painter->restore();
}
