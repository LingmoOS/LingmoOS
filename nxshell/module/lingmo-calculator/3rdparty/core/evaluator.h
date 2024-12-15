// This file is part of the SpeedCrunch project
// Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>
// Copyright (C) 2008-2016 @heldercorreia
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

#ifndef CORE_EVALUATOR_H
#define CORE_EVALUATOR_H

#include "functions.h"
#include "opcode.h"
#include "variable.h"
#include "userfunction.h"

#include "../math/hmath.h"
#include "../math/cmath.h"
#include "../math/quantity.h"

#include <QHash>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

class Session;

class Token
{
public:
    enum Operator {
        Invalid = 0,
        Addition, Subtraction, Multiplication,
        Division, IntegerDivision, Exponentiation,
        Super0, Super1, Super2, Super3, Super4,
        Super5, Super6, Super7, Super8, Super9,
        AssociationStart, AssociationEnd, Percent,
        ListSeparator, Factorial, Assignment, Modulo,
        ArithmeticLeftShift, ArithmeticRightShift,
        BitwiseLogicalAND, BitwiseLogicalOR,
        UnitConversion,
        Function, // For managing shift/reduce conflicts.
        Yroot, Log,//add jingzhou 20200720
        BitwiseLogicalXOR, BitwiseLogicalNAND, BitwiseLogicalNOR,//add jingzhou 20201201
        LogicalLeftShift, LogicalRightShift, // add 20201204
        CircularLeftShift, CircularRightShift, RotateCarryLeftShift, RotateCarryRightShift//add 20201205
    };
    enum Type {
        stxUnknown, stxNumber, stxIdentifier, stxAbstract, // isOperand
        stxOperator, stxOpenPar, stxClosePar, stxSep // isOperator
    };

    static const Token null;

    Token(Type = stxUnknown, const QString & = QString(), int pos = -1,
          int size = -1);
    Token(const Token &);

    Quantity asNumber() const;
    Operator asOperator() const;
    QString description() const;
    bool isNumber() const { return m_type == stxNumber; }
    bool isOperator() const { return m_type >= stxOperator; }
    bool isIdentifier() const { return m_type == stxIdentifier; }
    bool isAbstract() const { return m_type == stxAbstract; }
    bool isOperand() const
    {
        return isNumber() || isIdentifier()
               || isAbstract();
    }
    int pos() const { return m_pos; }
    void setPos(int pos) { m_pos = pos; }
    int size() const { return m_size; }
    void setSize(int size) { m_size = size; }
    QString text() const { return m_text; }
    Type type() const { return m_type; }
    int minPrecedence() const { return m_minPrecedence; }
    void setMinPrecedence(int value) { m_minPrecedence = value; }

    Token &operator=(const Token &);

protected:
    // Start position of the text that token represents in the expression
    // (might include extra space characters).
    int m_pos;
    // Size of text that token represents in the expression
    // (might include extra space characters).
    int m_size;
    // Precedence of the operator with the lowest precedence contained
    // in this token.
    int m_minPrecedence;
    // Normalized version of that token text (only valid when the token
    // represents a single token).
    QString m_text;
    Type m_type;
};

class Tokens : public QVector<Token>
{
public:
    Tokens()
        : QVector<Token>()
        , m_valid(true)
    { }

    bool valid() const { return m_valid; }
    void setValid(bool v) { m_valid = v; }

#ifdef EVALUATOR_DEBUG
    void append(const Token &);
#endif  /* EVALUATOR_DEBUG */

protected:
    bool m_valid;
};

class Evaluator : public QObject
{
    Q_OBJECT
    using ForceBuiltinVariableErasure = bool;

public:
    static Evaluator *instance();
    void reset();

    void setSession(Session *);
    const Session *session();

    static bool isSeparatorChar(const QChar &);
    static bool isRadixChar(const QChar &);
    static QString fixNumberRadix(const QString &);

    QString autoFix(const QString &);
    QString dump();
    QString error() const;
    Quantity eval();
    Quantity evalNoAssign();
    Quantity evalUpdateAns();
    QString expression() const;
    bool isValid();
    Tokens scan(const QString &) const;
    void setExpression(const QString &);
    Tokens tokens() const;
    bool isUserFunctionAssign() const;

    Variable getVariable(const QString &) const;
    QList<Variable> getVariables() const;
    QList<Variable> getUserDefinedVariables() const;
    QList<Variable> getUserDefinedVariablesPlusAns() const;
    void setVariable(const QString &, Quantity,
                     Variable::Type = Variable::UserDefined);
    void unsetVariable(const QString &, ForceBuiltinVariableErasure = false);
    void unsetAllUserDefinedVariables();
    bool isBuiltInVariable(const QString &) const;
    bool hasVariable(const QString &) const;
    void initializeBuiltInVariables();
    void initializeAngleUnits();

    QList<UserFunction> getUserFunctions() const;
    void setUserFunction(const UserFunction &);
    void unsetUserFunction(const QString &);
    void unsetAllUserFunctions();
    bool hasUserFunction(const QString &) const;
    Quantity getStandardPercentAns(); //获取标准模式下的百分号处理

protected:
    void compile(const Tokens &);

private:
    Q_DISABLE_COPY(Evaluator)
    Evaluator();

    bool m_dirty;
    QString m_error;
    QString m_expression;
    bool m_valid;
    QString m_assignId;
    bool m_assignFunc;
    QStringList m_assignArg;
    QVector<Opcode> m_codes;
    QVector<Quantity> m_constants;
    QStringList m_identifiers;
    Session *m_session;
    QSet<QString> m_functionsInUse;
    Quantity m_standardPercent = Quantity(); //标准模式下的百分号结果
    bool m_singleNumPercent = false;//true:100-(50%) = 100 - 0.5;

    const Quantity &checkOperatorResult(const Quantity &);
    static QString stringFromFunctionError(Function *);
    Quantity exec(const QVector<Opcode> &opcodes,
                  const QVector<Quantity> &constants,
                  const QStringList &identifiers);
    Quantity execUserFunction(const UserFunction *function,
                              QVector<Quantity> &arguments);
    const UserFunction *getUserFunction(const QString &) const;

    bool isFunction(Token token)
    {
        return token.isIdentifier()
               && (FunctionRepo::instance()->find(token.text())
                   || hasUserFunction(token.text()));
    }
};

#endif
