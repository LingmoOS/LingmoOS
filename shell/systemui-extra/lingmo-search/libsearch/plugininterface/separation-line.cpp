/*
 *
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "separation-line.h"
#include "global-settings.h"
#include <QApplication>

#define NOMORL_LINE_STYLE "QFrame{background: rgba(0,0,0,0.1);}"
#define DARK_LINE_STYLE "QFrame{background: rgba(255, 255, 255, 0.16);}"

using namespace LingmoUISearch;
SeparationLine::SeparationLine(QWidget *parent) : QFrame(parent)
{
    this->setLineWidth(0);
    this->setFixedHeight(1);
    setLineStyle();
    connect(qApp, &QApplication::paletteChanged, this, &SeparationLine::setLineStyle);
}

void SeparationLine::setLineStyle()
{
    QString type = GlobalSettings::getInstance().getValue(STYLE_NAME_KEY).toString();
    if (type == "lingmo-dark") {
        this->setStyleSheet(DARK_LINE_STYLE);
    } else {
        this->setStyleSheet(NOMORL_LINE_STYLE);
    }
}
