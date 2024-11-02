/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef GIOERROR_H
#define GIOERROR_H

#include "explor-core_global.h"
#include <gio/gio.h>
#include <memory>

#include <QMetaType>

namespace Peony {

class PEONYCORESHARED_EXPORT GErrorWrapper
{
public:
    GErrorWrapper();//do not use this constructor.
    GErrorWrapper(GError *err);
    ~GErrorWrapper();
    int code();
    QString message();
    QString domain();

    static std::shared_ptr<GErrorWrapper> wrapFrom(GError *err);

private:
    GError *m_err = nullptr;
};

typedef std::shared_ptr<GErrorWrapper> GErrorWrapperPtr;

}

Q_DECLARE_METATYPE(Peony::GErrorWrapper)
Q_DECLARE_METATYPE(Peony::GErrorWrapperPtr)

#endif // GIOERROR_H
