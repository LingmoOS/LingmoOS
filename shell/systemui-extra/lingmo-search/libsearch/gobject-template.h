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
#ifndef GT_H
#define GT_H
#include <glib-object.h>
#include <gio/gio.h>
#include <memory>

template<class T>

class gobjecttemplate {
public:
    //do not use this constructor.
    gobjecttemplate();
    gobjecttemplate(T *obj, bool ref = false) {
        m_obj = obj;
        if(ref) {
            g_object_ref(obj);
        }
    }

    ~gobjecttemplate() {
        //qDebug()<<"~GObjectTemplate";
        if(m_obj)
            g_object_unref(m_obj);
    }

    T *get() {
        return m_obj;
    }

private:
    mutable T *m_obj = nullptr;
};


std::shared_ptr<gobjecttemplate<GFile>> wrapGFile(GFile *file);
std::shared_ptr<gobjecttemplate<GFileInfo>> wrapGFileInfo(GFileInfo *info);


#endif // GT_H
