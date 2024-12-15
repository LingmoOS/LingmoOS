// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file btinfodelegate.cpp
 *
 * @brief bt窗口中表格item代理类
 *
 * @date 2020-06-09 10:49
 *
 * Author: bulongwei  <bulongwei@uniontech.com>
 *
 * Maintainer: bulongwei  <bulongwei@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "btinfodelegate.h"

#include "btinfodialog.h"

#include <QMouseEvent>
#include <QPainter>
#include <QApplication>
#include <QFileIconProvider>
#include <QTemporaryFile>
#include <QEvent>
#include <QAbstractItemModel>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QCheckBox>
#include <QDir>

BtInfoDelegate::BtInfoDelegate(DDialog *dialog):
    m_dialog(dialog),
    m_checkBtn(new QCheckBox)
{
}

BtInfoDelegate::~BtInfoDelegate()
{
    delete m_checkBtn;
}

void BtInfoDelegate::setHoverColor(QBrush c)
{
    m_hoverColor = c;
}

void BtInfoDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    if (index.row() == m_hoverRow) {
        painter->fillRect(option.rect, m_hoverColor); //QColor(0,0,0,13)QColor(255,255,255,26)
    } else {
        if (index.row() % 2 == 1) {
            painter->fillRect(option.rect, option.palette.base());
        } else {
            painter->fillRect(option.rect, option.palette.alternateBase());
        }
    }
    painter->restore();
    painter->save();
    QFont font;
    painter->setFont(font);

    if (index.column() == 0) {
        QStyleOptionButton checkBoxStyle;
        checkBoxStyle.state = index.data().toString() == "1" ? QStyle::State_On : QStyle::State_Off;
        checkBoxStyle.state |= QStyle::State_Enabled;
        checkBoxStyle.rect = option.rect;
        checkBoxStyle.rect.setX(option.rect.x() + 5); //option.rect.width() / 2 - 6
        checkBoxStyle.rect.setWidth(15);

        QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxStyle, painter, m_checkBtn);

        QString ext = index.model()->data(index.model()->index(index.row(), 2)).toString();
        QFileIconProvider prov;
        QString tempFilePath = QDir::tempPath() + QDir::separator() + QCoreApplication::applicationName() + "_temp.";
        QFileInfo fi(tempFilePath + ext);
        QIcon icon = prov.icon(fi);
        if (icon.isNull()) {
            icon = prov.icon(QFileIconProvider::File);
        }
        QPixmap pic = icon.pixmap(20, 20);
        painter->drawPixmap(option.rect.x() + 32, option.rect.y() + (option.rect.height() - 20)/2, pic);

        painter->setPen(Qt::darkGray);
        QString text = painter->fontMetrics().elidedText(index.model()->data(index.model()->index(index.row(), 1)).toString(), Qt::ElideRight, option.rect.width() - 55);
        painter->drawText(option.rect.marginsRemoved(QMargins(55, 2, 0, 2)), Qt::AlignVCenter | Qt::AlignLeft, text);
    } else {
        painter->setPen(Qt::darkGray);
        QString text = painter->fontMetrics().elidedText(index.data().toString(), Qt::ElideRight, option.rect.width() - 25);
        painter->drawText(option.rect.marginsRemoved(QMargins(5, 2, 0, 2)), Qt::AlignVCenter | Qt::AlignLeft, text);
    }
    painter->restore();
}

bool BtInfoDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    if (index.column() == 0) {
        QRect rect(option.rect);
        rect.setX(10);
        rect.setWidth(15);
        if (event->type() == QEvent::MouseButtonPress
            && mouseEvent->button() == Qt::LeftButton
            && rect.contains(mouseEvent->pos())) {
            QString v = index.data().toString();
            model->setData(index, QVariant(v == "1" ? "0" : "1"), Qt::EditRole);

            ((BtInfoDialog *)m_dialog)->updateSelectedInfo();
            return true;
        }
    }
    return false;
}

void BtInfoDelegate::onhoverChanged(const QModelIndex &index)
{
    m_hoverRow = index.row();
}
