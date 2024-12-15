// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SearchResDelegate.h"
#include "SideBarImageViewModel.h"
#include "Utils.h"
#include "Application.h"

#include <DGuiApplicationHelper>

#include <QPainter>
#include <QItemSelectionModel>
#include <QAbstractItemView>
#include <QPainterPath>

SearchResDelegate::SearchResDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
    m_parent = parent;
}

void SearchResDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid()) {
        const QPixmap &pixmap = index.data(ImageinfoType_e::IMAGE_PIXMAP).value<QPixmap>();
        QSize pageSize = index.data(ImageinfoType_e::IMAGE_PAGE_SIZE).toSize();

        const int borderRadius = 6;
        pageSize.scale(static_cast<int>(62 * dApp->devicePixelRatio()), static_cast<int>(62 * dApp->devicePixelRatio()), Qt::KeepAspectRatio);
        const QSize &scalePixSize = pageSize / dApp->devicePixelRatio();
        const QRect &rect = QRect(option.rect.x() + 10, option.rect.center().y() - scalePixSize.height() / 2, scalePixSize.width(), scalePixSize.height());


        if (!pixmap.isNull()) {
            const QPixmap &scalePix = pixmap.scaled(pageSize);
            //clipPath pixmap
            painter->save();
            QPainterPath clipPath;
            clipPath.addRoundedRect(rect, borderRadius, borderRadius);
            painter->setClipPath(clipPath);
            painter->drawPixmap(rect.x(), rect.y(), scalePix);
            painter->restore();
        }

        //drawText RoundRect
        painter->save();
        painter->setBrush(Qt::NoBrush);
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        if (m_parent->selectionModel()->isRowSelected(index.row(), index.parent())) {
            painter->setPen(QPen(DTK_NAMESPACE::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color(), 2));
            painter->drawRoundedRect(rect, borderRadius, borderRadius);
        } else {
            painter->setPen(QPen(DTK_NAMESPACE::Gui::DGuiApplicationHelper::instance()->applicationPalette().frameShadowBorder().color(), 1));
            painter->drawRoundedRect(rect, borderRadius, borderRadius);
        }
        painter->restore();

        //drawPagetext
        int margin = 2;
        int textStartX = rect.right() + 18;
        painter->save();
        painter->setPen(QPen(DTK_NAMESPACE::Gui::DGuiApplicationHelper::instance()->applicationPalette().windowText().color()));
        QFont font = painter->font();
        font = DFontSizeManager::instance()->t8(font);
        painter->setFont(font);
        const QString &pageText = index.data(ImageinfoType_e::IMAGE_INDEX_TEXT).toString();
        int pagetextHeight = painter->fontMetrics().height();
        painter->drawText(textStartX, option.rect.y() + margin, option.rect.width(), pagetextHeight, Qt::AlignVCenter | Qt::AlignLeft, pageText);
        painter->restore();

        //drawSearchCount
        painter->save();
        painter->setPen(QPen(DTK_NAMESPACE::Gui::DGuiApplicationHelper::instance()->applicationPalette().textTips().color()));
        QFont sfont = painter->font();
        sfont = DFontSizeManager::instance()->t10(sfont);
        painter->setFont(sfont);
        const QString &searchCountText = index.data(ImageinfoType_e::IMAGE_SEARCH_COUNT).toString();
        int searchCountTextWidth = painter->fontMetrics().width(searchCountText);
        painter->drawText(option.rect.right() - searchCountTextWidth - 20, option.rect.y() + margin, option.rect.width(), pagetextHeight, Qt::AlignVCenter | Qt::AlignLeft, searchCountText);
        painter->restore();

        //drawPageContenttext
        painter->save();
        painter->setPen(QPen(DTK_NAMESPACE::Gui::DGuiApplicationHelper::instance()->applicationPalette().brightText().color()));
        QFont cfont = painter->font();
        cfont = DFontSizeManager::instance()->t9(cfont);
        painter->setFont(cfont);
        QString contentText = index.data(ImageinfoType_e::IMAGE_CONTENT_TEXT).toString();
        contentText.replace(QChar('\n'), QString(""));
        contentText.replace(QChar('\t'), QString(""));
        QTextOption contentOption;
        contentOption.setAlignment(Qt::AlignTop | Qt::AlignLeft);
        contentOption.setWrapMode(QTextOption::WrapAnywhere);

        QSize contentSize(option.rect.right() - textStartX, option.rect.height() - pagetextHeight - 2 * margin);
        const QString &elidedContentText = Utils::getElidedText(painter->fontMetrics(), contentSize, contentText, contentOption.alignment());
        painter->drawText(QRect(textStartX, option.rect.y() + margin + pagetextHeight, contentSize.width(), contentSize.height()), elidedContentText, contentOption);
        painter->restore();

        //drawBottomLine
        painter->save();
        int bottomlineHeight = 1;
        painter->setPen(QPen(DTK_NAMESPACE::Gui::DGuiApplicationHelper::instance()->applicationPalette().frameBorder().color(), bottomlineHeight));
        painter->drawLine(textStartX, option.rect.bottom() - bottomlineHeight, option.rect.right(), option.rect.bottom() - bottomlineHeight);
        painter->restore();
    }
}

QSize SearchResDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return DStyledItemDelegate::sizeHint(option, index);
}

