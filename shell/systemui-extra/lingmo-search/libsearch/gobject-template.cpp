/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: zhangjiaping <zhangjiaping@kylinos.cn>
 *
 */
#include "gobject-template.h"
#include "glib-object.h"

std::shared_ptr<gobjecttemplate<GFile>> wrapGFile(GFile *file) {
    return std::make_shared<gobjecttemplate<GFile>>(file);
}

std::shared_ptr<gobjecttemplate<GFileInfo>> wrapGFileInfo(GFileInfo *info) {
    return std::make_shared<gobjecttemplate<GFileInfo>>(info);
}
