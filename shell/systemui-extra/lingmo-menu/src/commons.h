/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

#ifndef LINGMO_MENU_COMMONS_H
#define LINGMO_MENU_COMMONS_H

#include "data-entity.h"

namespace LingmoUIMenu {

class Display {
    Q_GADGET
public:
    enum Type {
        IconOnly,
        TextOnly,
        TextBesideIcon,
        TextUnderIcon
    };

    Q_ENUM(Type)
};

} // LingmoUIMenu

#endif //LINGMO_MENU_COMMONS_H
