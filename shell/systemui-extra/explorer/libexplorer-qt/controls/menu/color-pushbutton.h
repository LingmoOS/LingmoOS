/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */


#ifndef COLORPUSHBUTTON_H
#define COLORPUSHBUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QPainter>
#include <QColor>
#include <QEnterEvent>
class ColorPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ColorPushButton(QColor color,QWidget *parent = nullptr);

    void setName(const QString& name);

Q_SIGNALS:
    void changeText(const QString &colorName);

protected:
    void paintEvent(QPaintEvent *e);

public:
    bool m_checkInMultiSelect = true;

private:
    QColor m_color = nullptr;
    QRectF m_pathrect;
    double m_pathrectwidth;
    double m_pathrectheigth;
    QRectF m_bigpathrect;
    double m_bigpathrectwidth;
    double m_bigpathrectheigth;
    QString m_name;
};

#endif // COLORPUSHBUTTON_H
