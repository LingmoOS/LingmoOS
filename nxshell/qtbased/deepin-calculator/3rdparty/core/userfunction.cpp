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


#include "userfunction.h"
#include "evaluator.h"
#include "opcode.h"
#include <QJsonArray>


//#define SAVE_COMPILED_FORM



UserFunction::UserFunction(const QJsonObject &json) : UserFunction()
{
    if (json.contains("name"))
        m_name = json["name"].toString();

    if (json.contains("args")) {
        const QJsonArray args_json = json["args"].toArray();
        int n = json["args"].toArray().size();
        for (int i = 0; i < n; ++i)
            m_arguments.append(args_json.at(i).toString());
    }

    if (json.contains("expression"))
        m_expression = json["expression"].toString();
    if (json.contains("description"))
        m_description = json["description"].toString();

    if (json.contains("opcodes")) {
        const QJsonArray   &codes_json = json["opcodes"].toArray();
        for (int i = 0; i < codes_json.size(); ++i) {
            Opcode opcode(static_cast<Opcode::Type>(codes_json[i].toObject()["t"].toInt()),  codes_json[i].toObject()["i"].toInt());
            if (codes_json[i].toObject().contains("text"))
                opcode.text = codes_json[i].toObject()["text"].toString();
            opcodes.append(opcode);
        }

        const QJsonArray &const_json = json["constants"].toArray();
        for (int i = 0; i < const_json.size(); ++i) {
            CNumber hn(const_json[i].toObject());
            constants.append(hn);
        }

        const QJsonArray &id_json = json["identifiers"].toArray();
        for (int i = 0; i < id_json.size(); ++i) {
            identifiers.append(id_json[i].toString());
        }

    }
}

QString UserFunction::name() const
{
    return m_name;
}

QStringList UserFunction::arguments() const
{
    return m_arguments;
}

QString UserFunction::expression() const
{
    return m_expression;
}

QString UserFunction::description() const
{
    return m_description;
}

void UserFunction::setName(const QString &str)
{
    m_name = str;
}

void UserFunction::setArguments(const QStringList &args)
{
    m_arguments = args;
}

void UserFunction::setExpression(const QString &expr)
{
    m_expression = expr;
}

void UserFunction::setDescription(const QString &expr)
{
    m_description = expr;
}

void UserFunction::serialize(QJsonObject &json) const
{
    json["name"] = m_name;
    QJsonArray args;
    for (int i = 0; i < m_arguments.count(); ++i)
        args.append(m_arguments[i]);
    json["args"] = args;
    json["expression"] = m_expression;
    if (m_description != "")
        json["description"] = m_description;

#ifdef SAVE_COMPILED_FORM
    // if compiled form is available, save it as well
    if (!opcodes.isEmpty()) {
        QJsonArray opcodes_json;
        for (int i = 0; i < opcodes.size(); ++i) {
            QJsonObject curr_code_json;
            const Opcode &curr_code = opcodes.at(i);
            curr_code_json["t"] = curr_code.type;
            curr_code_json["i"] = int(curr_code.index);
            if (curr_code.text != "")
                curr_code_json["text"] = curr_code.text;
            opcodes_json.append(curr_code_json);
        }
        json["opcodes"] = opcodes_json;

        QJsonArray constants_json;
        for (int i = 0; i < constants.size(); ++i) {
            QJsonObject curr_const_json;
            constants.at(i).serialize(curr_const_json);
            constants_json.append(curr_const_json);
        }
        json["constants"] = constants_json;

        QJsonArray identifiers_json;
        for (int i = 0; i < identifiers.size(); ++i) {
            identifiers_json.append(identifiers.at(i));
        }
        json["identifiers"] = identifiers_json;
    }
#endif
}

void UserFunction::deSerialize(const QJsonObject &json)
{
    *this = UserFunction(json);

}
