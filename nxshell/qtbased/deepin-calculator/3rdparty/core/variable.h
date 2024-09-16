// This file is part of the SpeedCrunch project
// Copyright (C) 2004, 2005, 2007 Ariya Hidayat <ariya@kde.org>
// Copyright (C) 2007-2009, 2013, 2014 @heldercorreia
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

#ifndef CORE_VARIABLE_H
#define CORE_VARIABLE_H

#include <QString>
#include <QSet>
#include <QList>

#include "../math/quantity.h"

class Variable
{
public:
    enum Type { BuiltIn, UserDefined };
private:
    QString m_identifier;
    Quantity m_value;
    Type m_type;
public:
    Variable() : m_identifier(""), m_value(0), m_type(UserDefined) {}
    Variable(const QJsonObject &json);
    Variable(const QString &id, const Quantity &val, Type t = UserDefined) : m_identifier(id), m_value(val), m_type(t) {}
    Variable(const Variable &other) :  m_identifier(other.m_identifier), m_value(other.m_value), m_type(other.m_type) {}

    Quantity value() const {return m_value;}
    QString identifier() const {return m_identifier;}
    Type type() const {return m_type;}

    void setValue(const Quantity &val) {m_value = val;}
    void set_identifier(const QString &str) {m_identifier = str;}
    void set_type(const Type t) {m_type = t;}

    void serialize(QJsonObject &json) const;
    void deSerialize(const QJsonObject &json);
    bool operator==(const Variable &other) const { return m_identifier == other.m_identifier; }
};

#endif // CORE_VARIABLE_H
