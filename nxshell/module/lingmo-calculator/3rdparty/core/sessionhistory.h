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


#ifndef CORE_SESSIONHISTORY_H
#define CORE_SESSIONHISTORY_H

#include <QJsonArray>
#include <QString>
#include <QList>

#include "../math/quantity.h"


class HistoryEntry
{
private:
    QString m_expr;
    Quantity m_result;
public:
    HistoryEntry() : m_expr(""), m_result(0) {}
    HistoryEntry(const QJsonObject &json);
    HistoryEntry(const QString &expr, const Quantity &num) : m_expr(expr), m_result(num) {}
    HistoryEntry(const HistoryEntry &other) :  m_expr(other.m_expr), m_result(other.m_result) {}

    void setExpr(const QString &e);
    void setResult(const Quantity &n);

    QString expr() const;
    Quantity result() const;

    void serialize(QJsonObject &json) const;
    void deSerialize(const QJsonObject &json);
};

#endif // CORE_SESSIONHISTORY_H
