/*
 *
 * Copyright: 2020 KylinSoft Co., Ltd.
 * Authors:
 *   huanhuan zhang <zhanghuanhuan@kylinos.cn>
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
 */
#ifndef CAPTUREBUTTONCAP_H
#define CAPTUREBUTTONCAP_H


#include <QPushButton>
#include <QMap>
#include <QVector>
#include <QLabel>
#include <QHBoxLayout>

class QWidget;
class QPropertyAnimation;
class CaptureToolCap;

class CaptureButtonCap : public QPushButton {
    Q_OBJECT

public:
    enum ButtonType
    {
        TYPE_SCREENCUT,
        TYPE_SCREENCAP,
        TYPE_SCREENMOUSECURSOR,
        TYPE_SCREENVOICE,
        TYPE_SCREENMOUSE,
        TYPE_SCREENCAPOPTION,
        TYPE_SCREENCAPEXIT,
        TYPE_SCREENCAPSTART,
    };
public:
    CaptureButtonCap() = delete;
    explicit CaptureButtonCap(const ButtonType t, QWidget *parent = nullptr);
};

#endif // CAPTUREBUTTONCAP_H
