// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ThumbnailDelegate.h"
#include "SideBarImageViewModel.h"
#include "Utils.h"
#include "Application.h"

#include <DGuiApplicationHelper>

#include <QPainter>
#include <QItemSelectionModel>
#include <QAbstractItemView>
#include <QPainterPath>

ThumbnailDelegate::ThumbnailDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
    m_parent = parent;
}

void ThumbnailDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.isValid()) {
        qreal pixscale = m_parent->property("adaptScale").toDouble();

        int rotate = index.data(ImageinfoType_e::IMAGE_ROTATE).toInt();

        bool bShowBookMark = index.data(ImageinfoType_e::IMAGE_BOOKMARK).toBool();

        QMatrix matrix;

        matrix.rotate(rotate);

        const QPixmap &pixmap = index.data(ImageinfoType_e::IMAGE_PIXMAP).value<QPixmap>().transformed(matrix);

        const int borderRadius = 6;

        QSize pageSize = index.data(ImageinfoType_e::IMAGE_PAGE_SIZE).toSize();

        if (rotate == 90 || rotate == 270)
            pageSize = QSize(pageSize.height(), pageSize.width());

        pageSize.scale(static_cast<int>(174 * pixscale * dApp->devicePixelRatio()), static_cast<int>(174 * pixscale * dApp->devicePixelRatio()), Qt::KeepAspectRatio);

        const QSize &scalePixSize = pageSize / dApp->devicePixelRatio();

        const QRect &rect = QRect(option.rect.center().x() - scalePixSize.width() / 2, option.rect.center().y() - scalePixSize.height() / 2, scalePixSize.width(), scalePixSize.height());

        if (!pixmap.isNull()) {
            //clipPath pixmap
            painter->save();
            QPainterPath clipPath;
            clipPath.addRoundedRect(rect, borderRadius, borderRadius);
            painter->setClipPath(clipPath);
            painter->drawPixmap(rect.x(), rect.y(), rect.width(), rect.height(), pixmap);
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
            painter->setPen(QPen(DTK_NAMESPACE::Gui::DGuiApplicationHelper::instance()->applicationPalette().windowText().color()));
        }
        painter->drawText(rect.x(), rect.bottom() + 4, rect.width(), option.rect.bottom() - rect.bottom(), Qt::AlignHCenter | Qt::AlignTop, QString::number(index.row() + 1));
        painter->restore();
        drawBookMark(painter, rect, bShowBookMark);
    }
}

QSize ThumbnailDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return DStyledItemDelegate::sizeHint(option, index);
}

void ThumbnailDelegate::drawBookMark(QPainter *painter, const QRect &rect, bool visible) const
{
    if (visible) {
        QPixmap pixmap(QIcon::fromTheme("dr_bookmark_checked").pixmap({36, 36}));
        painter->drawPixmap(rect.right() - 42, rect.y(), pixmap);
    }
}
