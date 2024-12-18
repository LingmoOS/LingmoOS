/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_ITEMS_TOOLTIP_PROXY_H
#define LINGMO_QUICK_ITEMS_TOOLTIP_PROXY_H

#include <QObject>
#include <QPoint>
#include <QPointF>
#include <qqml.h>

namespace LingmoUIQuick {

class ToolTip : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
public:
    explicit ToolTip(QObject *parent = nullptr);

    QString text() const;
    void setText(const QString &text);

    Q_INVOKABLE void show(int x = -1, int y = -1);
    Q_INVOKABLE void show(QPointF point);
    Q_INVOKABLE void show(QPoint point);
    Q_INVOKABLE void hide();

Q_SIGNALS:
    void textChanged();

private:
    QString m_text;
};

class ToolTipAttached : public QObject
{
    Q_OBJECT
public:
    static ToolTip *qmlAttachedProperties(QObject *object);
};

} // LingmoUIQuick

QML_DECLARE_TYPEINFO(LingmoUIQuick::ToolTipAttached, QML_HAS_ATTACHED_PROPERTIES)

#endif //LINGMO_QUICK_ITEMS_TOOLTIP_PROXY_H
