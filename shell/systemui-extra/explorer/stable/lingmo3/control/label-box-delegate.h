/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: MeihongHe <hemeihong@kylinos.cn>
 *
 */

#ifndef DEVELLABELBOXDELEGATE_H
#define DEVELLABELBOXDELEGATE_H

#include <QStyledItemDelegate>

class LabelBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit LabelBoxDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index);

Q_SIGNALS:

};
#endif // DEVELLABELBOXDELEGATE_H
