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



#ifndef CORE_USERFUNCTION_H
#define CORE_USERFUNCTION_H

#include<QString>
#include<QStringList>
#include<QVector>
#include "opcode.h"
#include "../math/quantity.h"

class UserFunction
{
private:
    QString m_name;
    QStringList m_arguments;
    QString m_expression;
    QString m_description;

public:
    QVector<Quantity> constants;
    QStringList identifiers;
    QVector<Opcode> opcodes;

    UserFunction(QString name, QStringList arguments, QString expression)
        : m_name(name), m_arguments(arguments), m_expression(expression) {}
    UserFunction() {}
    UserFunction(const QJsonObject &json);

    QString name() const;
    QStringList arguments() const;
    QString expression() const;
    QString description() const;

    void setName(const QString &str);
    void setArguments(const QStringList &args);
    void setExpression(const QString &expr);
    void setDescription(const QString &expr);

    void serialize(QJsonObject &json) const;
    void deSerialize(const QJsonObject &json);
};

#endif // CORE_USERFUNCTION_H
