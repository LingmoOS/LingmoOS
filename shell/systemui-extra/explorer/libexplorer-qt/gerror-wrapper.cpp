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

#include "gerror-wrapper.h"
#include <QString>

#include <QDebug>

using namespace Peony;

GErrorWrapper::GErrorWrapper(GError *err)
{
    //qDebug()<<"Error";
    m_err = err;
}

GErrorWrapper::~GErrorWrapper()
{
    //qDebug()<<"~Error";
    if (m_err)
        g_error_free(m_err);
}

int GErrorWrapper::code()
{
    if (!m_err)
        return -1;
    return m_err->code;
}

QString GErrorWrapper::message()
{
    if (!m_err)
        return nullptr;
    return m_err->message;
}

QString GErrorWrapper::domain()
{
    if (!m_err)
        return nullptr;
    return g_quark_to_string(m_err->domain);
}

std::shared_ptr<GErrorWrapper> GErrorWrapper::wrapFrom(GError *err)
{
    return std::make_shared<GErrorWrapper>(err);
}
