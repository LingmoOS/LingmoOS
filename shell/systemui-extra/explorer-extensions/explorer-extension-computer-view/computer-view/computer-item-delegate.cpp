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
#include "computer-view.h"
#include "computer-proxy-model.h"
#include "computer-item-delegate.h"
#include "abstract-computer-item.h"

#include <QStyle>
#include <QDebug>
#include <QPainter>
#include <QListView>
#include <QApplication>
#include <QPainterPath>

#include <QTreeView>

ComputerItemDelegate::ComputerItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    m_styleIconView = new QListView;
    m_styleIconView->setViewMode(QListView::IconMode);

    m_styleTreeView = new QTreeView;
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
        opt.viewItemPosition = QStyleOptionViewItem::OnlyOne;
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
    bool isHover = (option.state & QStyle::State_MouseOver) && (option.state & ~QStyle::State_Selected);
    bool isSelected = option.state & QStyle::State_Selected;
    bool enable = option.state & QStyle::State_Enabled;

    QColor color = option.palette.color(enable? QPalette::Active: QPalette::Disabled,
                                        QPalette::NoRole);
    if (isHover && !isSelected) {
        color = option.palette.color(QPalette::BrightText);
        color.setAlpha(8);
    } else if (!isSelected){
        color.setAlpha(0);
    }
//    painter->save();
//    painter->setRenderHint(QPainter::Antialiasing);
//    painter->setPen(Qt::transparent);
//    painter->setBrush(color);
//    painter->drawRoundedRect(option.rect, 6, 6);
//    painter->restore();
    qApp->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, m_styleTreeView);

    if (index.parent().isValid()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        //draw pix map
        QIcon icon = option.icon;
        bool enable = option.state & QStyle::State_Enabled;
        bool selected = option.state &QStyle::State_Selected;
        QPixmap pix = icon.pixmap(QSize(64, 64), enable? QIcon::Normal: QIcon::Disabled);
        qApp->style()->drawItemPixmap(painter, option.rect.adjusted(5, 0, 0, 0), Qt::AlignVCenter|Qt::AlignLeft, pix);

        int textOffetX1 = 84;
        int textOffetX2 = -5;
        int progressX1 = 0;
        int progressX2 = -10;
        if (opt.direction == Qt::RightToLeft) {
            textOffetX1= 5;
            textOffetX2 = -84;
            progressX1 = 10;
            progressX2 = 0;
        }

        //draw text
        auto textRect = option.rect;
        textRect.adjust(textOffetX1, 10, textOffetX2, -25);
        textRect.translate(0, -option.fontMetrics.ascent());
        auto elideText = opt.fontMetrics.elidedText(opt.text, Qt::ElideMiddle, textRect.width());
        qApp->style()->drawItemText(painter, textRect, Qt::AlignLeft|Qt::AlignVCenter, option.palette, enable, elideText, QPalette::Text);

        //space
        bool shouldDrawProgress = false;
        QString spaceInfo;
        auto total = item->totalSpace();
        auto used = item->usedSpace();
        if (total > 0) {
            char *totalFormat = g_format_size_full(total,G_FORMAT_SIZE_IEC_UNITS);
            char *usedFormat = g_format_size_full(used,G_FORMAT_SIZE_IEC_UNITS);

            QString totalFormatString(totalFormat);
            QString usedFormatString(usedFormat);

            totalFormatString.replace("iB", "B");
            usedFormatString.replace("iB", "B");

            spaceInfo = QString("%1/%2").arg(usedFormatString).arg(totalFormatString);
            g_free(totalFormat);
            g_free(usedFormat);
            shouldDrawProgress = true;
        } else {
            if (!item->isMount()) {
                spaceInfo = tr("You should mount volume first");
            }
        }

        QRect spaceInfoArea = textRect;
        if (option.fontMetrics.ascent() <= option.font.pointSize() + 4) {
            spaceInfoArea = textRect.translated(0, 2.6 * option.fontMetrics.ascent());
            spaceInfoArea.setHeight (option.fontMetrics.height () + 3.0 * option.fontMetrics.ascent());
        } else {
            spaceInfoArea = textRect.translated(0, 1.5 * option.fontMetrics.ascent());
            spaceInfoArea.setHeight (option.fontMetrics.height () + 2.5 * option.fontMetrics.ascent());
        }

        qApp->style()->drawItemText(painter, spaceInfoArea, Qt::AlignLeft|Qt::AlignVCenter|Qt::TextWordWrap, option.palette, enable, spaceInfo, QPalette::PlaceholderText);

        if (shouldDrawProgress) {
            painter->save();

            QRect progressRect = spaceInfoArea.translated (0, spaceInfoArea.height ());
            progressRect.adjust (progressX1, -10, progressX2, 0);
            progressRect.setHeight(10);

//            QPainterPath clipPath;
//            clipPath.addRoundedRect(option.rect, 6, 6);
//            painter->setClipPath(clipPath);

            qreal percent = used*1.0/total*1.0;
            int progressBarWidth = progressRect.width() * percent;

            QStyleOptionProgressBar optBar;
            optBar.rect = progressRect;
            optBar.state = opt.state;
            optBar.palette = qApp->palette();
            optBar.minimum = 0;
            optBar.maximum = 100;
            optBar.progress =  percent * 100;
            //qApp->style()->drawControl(QStyle::CE_ProgressBar, &optBar, painter);

            painter->save();
            QPen pen;
            //pen.setColor(percent < 0.8?Qt::blue:Qt::red);
            pen.setCapStyle(Qt::PenCapStyle::RoundCap);
            pen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
            pen.setWidth(6);
            QColor color = option.palette.color(QPalette::BrightText);
            color.setAlpha(8);
            pen.setColor(color);
            painter->setPen(pen);
            //painter->drawLine(progressRect.topLeft(), progressRect.topRight());

            if (percent < 0.8) {
                pen.setColor(option.palette.highlight().color());
            } else {
                pen.setColor(Qt::red);
            }
            painter->setPen(pen);
            auto pos = progressRect.topLeft();
            pos.setX(pos.x() + progressBarWidth);
            //The percent is too small, resulting in incorrect coordinates.
            if((option.rect.bottomLeft() + QPoint(8, -5)).x() >= pos.x()){
                pos = progressRect.topLeft () + QPoint(9, -5);
            }
            //painter->drawLine(progressRect.topLeft (), pos);
            qApp->style()->drawControl(QStyle::CE_ProgressBar, &optBar, painter);
            painter->restore();
            painter->restore();
        }
        painter->restore();
    } else {
        //auto textRect = qApp->style()->subElementRect(QStyle::SE_ItemViewItemText, &option, m_styleIconView);
        drawTab(painter, option, index);
    }
}

void ComputerItemDelegate::paintRemoteItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, AbstractComputerItem *item) const
{
    if (index.parent().isValid()) {
        bool isHover = (option.state & QStyle::State_MouseOver) && (option.state & ~QStyle::State_Selected);
        bool isSelected = option.state & QStyle::State_Selected;
        bool enable = option.state & QStyle::State_Enabled;

        QColor color = option.palette.color(enable? QPalette::Active: QPalette::Disabled,
                                            QPalette::NoRole);
        if (isHover && !isSelected) {
            color = option.palette.color(QPalette::BrightText);
            color.setAlpha(8);
        } else if (!isSelected){
            color.setAlpha(0);
        }
//        painter->save();
//        painter->setRenderHint(QPainter::Antialiasing);
//        painter->setPen(Qt::transparent);
//        painter->setBrush(color);
//        painter->drawRoundedRect(option.rect, 6, 6);
//        painter->restore();
        qApp->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, m_styleTreeView);
        drawStyledItem(painter, option, index);
    } else {
        drawTab(painter, option, index);
    }
}

void ComputerItemDelegate::paintNetworkItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, AbstractComputerItem *item) const
{
    if (index.parent().isValid()) {
        bool isHover = (option.state & QStyle::State_MouseOver) && (option.state & ~QStyle::State_Selected);
        bool isSelected = option.state & QStyle::State_Selected;
        bool enable = option.state & QStyle::State_Enabled;

        QColor color = option.palette.color(enable? QPalette::Active: QPalette::Disabled,
                                            QPalette::NoRole);
        if (isHover && !isSelected) {
            color = option.palette.color(QPalette::BrightText);
            color.setAlpha(8);
        } else if (!isSelected){
            color.setAlpha(0);
        }
//        painter->save();
//        painter->setRenderHint(QPainter::Antialiasing);
//        painter->setPen(Qt::transparent);
//        painter->setBrush(color);
//        painter->drawRoundedRect(option.rect, 6, 6);
//        painter->restore();
        qApp->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, m_styleTreeView);
        drawStyledItem(painter, option, index);
    } else {
        drawTab(painter, option, index);
    }
}

void ComputerItemDelegate::drawTab(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    auto opt = option;
    auto titleFont = opt.font;
    if (titleFont.pixelSize() > 0) {
        titleFont.setPixelSize(titleFont.pixelSize()*1.25);
    } else {
        titleFont.setPointSizeF(titleFont.pointSizeF()*1.25);
    }
    opt.icon = QIcon();
    opt.decorationPosition = QStyleOptionViewItem::Right;
    opt.displayAlignment = Qt::AlignLeft|Qt::AlignVCenter;
    opt.font = titleFont;
    opt.fontMetrics = QFontMetrics(opt.font);
    painter->translate(QPoint(8, 0));
    qApp->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
    painter->restore();
    //auto textRect = qApp->style()->subElementRect(QStyle::SE_ItemViewItemText, &opt, m_styleIconView);
    //qApp->style()->drawItemText(painter, opt.rect.adjusted(5, 0, 0, 0), Qt::AlignTop|Qt::AlignVCenter, option.palette, enable, option.text, selected? QPalette::HighlightedText: QPalette::Text);
}

void ComputerItemDelegate::drawStyledItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    //draw pix map
    QIcon icon = option.icon;
    bool enable = option.state & QStyle::State_Enabled;
    bool selected = option.state &QStyle::State_Selected;
    QPixmap pix = icon.pixmap(QSize(64, 64), enable?  QIcon::Normal: QIcon::Disabled);
    qApp->style()->drawItemPixmap(painter, option.rect.adjusted(0, 5, 0, 0), Qt::AlignTop|Qt::AlignHCenter, pix);

    //draw text
    auto textRect = option.rect.adjusted(2, 74, -2, -2);
    auto elideText = option.fontMetrics.elidedText(option.text, Qt::ElideRight, 2 * textRect.width() - 10);
    qApp->style()->drawItemText(painter, textRect, Qt::ElideRight|Qt::TextWrapAnywhere|Qt::AlignTop|Qt::AlignHCenter, option.palette,
                                enable, elideText, QPalette::Text);
    painter->restore();
}
