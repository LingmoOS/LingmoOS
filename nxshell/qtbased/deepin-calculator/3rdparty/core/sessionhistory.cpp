// This file is part of the SpeedCrunch project
// Copyright (C) 2015 Pol Welter <polwelter@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

#include "sessionhistory.h"


HistoryEntry::HistoryEntry(const QJsonObject & json)
{
    deSerialize(json);
}

QString HistoryEntry::expr() const
{
    return m_expr;
}

Quantity HistoryEntry::result() const
{
    return m_result;
}

void HistoryEntry::setExpr(const QString & e)
{
    m_expr = e;
}

void HistoryEntry::setResult(const Quantity & n)
{
    m_result = n;
}

void HistoryEntry::serialize(QJsonObject & json) const
{
    json["expression"] = m_expr;
    QJsonObject result;
    m_result.serialize(result);
    json["result"] = result;
    return;
}

void HistoryEntry::deSerialize(const QJsonObject & json)
{
    *this = HistoryEntry();

    if (json.contains("expression"))
        m_expr = json["expression"].toString();

    if (json.contains("result"))
        m_result = Quantity(json["result"].toObject());
    return;
}
