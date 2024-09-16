// This file is part of the SpeedCrunch project
// Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>
// Copyright (C) 2005, 2006 Johan Thelin <e8johan@gmail.com>
// Copyright (C) 2007-2016 @heldercorreia
// Copyright (C) 2009 Wolf Lammen <ookami1@gmx.de>
// Copyright (C) 2014 Tey <teyut@free.fr>
// Copyright (C) 2015 Pol Welter <polwelter@gmail.com>
// Copyright (C) 2015 Hadrien Theveneau <theveneau@gmail.com>
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

#include "evaluator.h"
#include "session.h"
#include "settings.h"
#include "../math/rational.h"
#include "../math/units.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QStack>
#include <QDebug>

#define ALLOW_IMPLICIT_MULT

static constexpr int MAX_PRECEDENCE = INT_MAX;
static constexpr int INVALID_PRECEDENCE = INT_MIN;

#ifdef EVALUATOR_DEBUG
#include <QDebug>
#include <QFile>
#include <QTextStream>

QTextStream &operator<<(QTextStream &s, Quantity num)
{
    s << DMath::format(num, Quantity::Format::Fixed());
    return s;
}
#endif // EVALUATOR_DEBUG

static Evaluator *s_evaluatorInstance = 0;

static void s_deleteEvaluator()
{
    delete s_evaluatorInstance;
}

bool isMinus(const QChar &ch)
{
    return ch == QLatin1Char('-') || ch == QChar(0x2212);
}

bool isExponent(const QChar &ch, int base)
{
    switch (base) {
    case 2:
        return ch == QLatin1Char('b') || ch == QLatin1Char('B');
    case 8:
        return ch == QLatin1Char('o') || ch == QLatin1Char('O') || ch == QLatin1Char('C');
    case 10:
        return ch == QLatin1Char('e') || ch == QLatin1Char('E');
    case 16:
        return ch == QLatin1Char('h') || ch == QLatin1Char('H');
    default:
        return false;
    }
}

const Quantity &Evaluator::checkOperatorResult(const Quantity &n)
{
    switch (n.error()) {
    case Success: break;
    case NoOperand:
        if (!m_assignFunc)
            // The arguments are still NaN, so ignore this error.
            m_error = /*Evaluator::tr*/("cannot operate on a NaN");
        break;
    case Underflow:
        m_error = /*Evaluator::tr*/("underflow - tiny result is out "
                                    "of SpeedCrunch's number range");
        break;
    case Overflow:
        m_error = /*Evaluator::tr*/("overflow - huge result is out of "
                                    "SpeedCrunch's number range");
        break;
    case ZeroDivide:
        m_error = /*Evaluator::tr*/("division by zero");
        break;
    case OutOfLogicRange:
        m_error = /*Evaluator::tr*/("overflow - logic result exceeds "
                                    "maximum of 256 bits");
        break;
    case OutOfIntegerRange:
        m_error = /*Evaluator::tr*/("overflow - integer result exceeds "
                                    "maximum limit for integers");
        break;
    case TooExpensive:
        m_error = /*Evaluator::tr*/("too time consuming - "
                                    "computation was rejected");
        break;
    case DimensionMismatch:
        // We cannot make any assumptions about the dimension of the arguments,
        // so ignore this error when assigning a function.
        if (!m_assignFunc)
            m_error = /*Evaluator::tr*/("dimension mismatch - quantities with "
                                        "different dimensions cannot be "
                                        "compared, added, etc.");
        break;
    case InvalidDimension:
        m_error = /*Evaluator::tr*/("invalid dimension - operation might "
                                    "require dimensionless arguments");
        break;
    case EvalUnstable:
        m_error = /*Evaluator::tr*/("Computation aborted - encountered "
                                    "numerical instability");
        break;
    default:;
    }

    return n;
}

QString Evaluator::stringFromFunctionError(Function *function)
{
    if (!function->error())
        return QString();

    QString result = QString::fromLatin1("<b>%1</b>: ");

    switch (function->error()) {
    case Success: break;
    case InvalidParamCount:
        result += /*Evaluator::tr*/("wrong number of arguments");
        break;
    case NoOperand:
        result += /*Evaluator::tr*/("does not take NaN as an argument");
        break;
    case Overflow:
        result += /*Evaluator::tr*/("overflow - huge result is out of "
                                    "SpeedCrunch's number range");
        break;
    case Underflow:
        result += /*Evaluator::tr*/("underflow - tiny result is out of "
                                    "SpeedCrunch's number range");
        break;
    case OutOfLogicRange:
        result += /*Evaluator::tr*/("overflow - logic result exceeds "
                                    "maximum of 256 bits");
        break;
    case OutOfIntegerRange:
        result += /*Evaluator::tr*/("result out of range");
        break;
    case ZeroDivide:
        result += /*Evaluator::tr*/("division by zero");
        break;
    case EvalUnstable:
        result += /*Evaluator::tr*/("Computation aborted - "
                                    "encountered numerical instability");
        break;
    case OutOfDomain:
        result += /*Evaluator::tr*/("undefined for argument domain");
        break;
    case TooExpensive:
        result += /*Evaluator::tr*/("computation too expensive");
        break;
    case InvalidDimension:
        result += /*Evaluator::tr*/("invalid dimension - function "
                                    "might require dimensionless arguments");
        break;
    case DimensionMismatch:
        result += /*Evaluator::tr*/("dimension mismatch - quantities with "
                                    "different dimensions cannot be compared, "
                                    "added, etc.");
        break;
    case IONoBase:
    case BadLiteral:
    case InvalidPrecision:
    case InvalidParam:
        result += /*Evaluator::tr*/("internal error, please report a bug");
        break;
    default:
        result += /*Evaluator::tr*/("error");
        break;
    };

    return result.arg(function->identifier());
}

class TokenStack : public QVector<Token>
{
public:
    TokenStack();
    bool isEmpty() const;
    unsigned itemCount() const;
    Token pop();
    void push(const Token &token);
    const Token &top();
    const Token &top(unsigned index);
    bool hasError() const { return !m_error.isEmpty(); }
    QString error() const { return m_error; }
    void reduce(int count, int minPrecedence = INVALID_PRECEDENCE);
    void reduce(int count, Token &&top, int minPrecedence = INVALID_PRECEDENCE);
    void reduce(QList<Token> tokens, Token &&top,
                int minPrecedence = INVALID_PRECEDENCE);
private:
    void ensureSpace();
    int topIndex;
    QString m_error;
};

const Token Token::null;

static Token::Operator matchOperator(const QString &text)
{
    Token::Operator result = Token::Invalid;

    if (text.length() == 1) {
        QChar p = text.at(0);
        switch (p.unicode()) {
        case '+':
            result = Token::Addition;
            break;
        case 0x2212: // − MINUS SIGN
        case '-':
            result = Token::Subtraction;
            break;
        case 0x00D7: // × MULTIPLICATION SIGN
        case 0x22C5: // ⋅ DOT OPERATOR
        case '*':
            result = Token::Multiplication;
            break;
        case 0x00F7: // ÷ DIVISION SIGN
        case '/':
            if (Settings::instance()->programmerBase != 0)
                result = Token::IntegerDivision;
            else
                result = Token::Division;
            break;
        case '%':
            result = Token::Percent;
            break;
        case '^':
            result = Token::Exponentiation;
            break;
        case ';':
            result = Token::ListSeparator;
            break;
        case '(':
            result = Token::AssociationStart;
            break;
        case ')':
            result = Token::AssociationEnd;
            break;
        case '!':
            result = Token::Factorial;
            break;
        case '=':
            result = Token::Assignment;
            break;
        case '\\':
            result = Token::IntegerDivision;
            break;
        case '&':
            result = Token::BitwiseLogicalAND;
            break;
        case '|':
            result = Token::BitwiseLogicalOR;
            break;
        default:
            result = Token::Invalid;
        }
    } else if (text.length() == 2) {
        if (text == "**")
            result = Token::Exponentiation;
        else if (text == "<<")
            result = Token::ArithmeticLeftShift;
        else if (text == ">>")
            result = Token::ArithmeticRightShift;
        else if (text == "->" || text == "in")
            result = Token::UnitConversion;
        else if (text == "or")
            result = Token::BitwiseLogicalOR;
    } else if (text.length() == 3) {               //edit jingzhou 20200720 增加mod、yroot、log
        if (text == "mod")
            result = Token::Modulo;
        if (text == "log")
            result = Token::Log;
        if (text == "xor")
            result = Token::BitwiseLogicalXOR;
        if (text == "nor")
            result = Token::BitwiseLogicalNOR;
        if (text == "and")
            result = Token::BitwiseLogicalAND;
        if (text == "shl")
            result = Token::LogicalLeftShift;
        if (text == "shr")
            result = Token::LogicalRightShift;
        if (text == "sal")
            result = Token::ArithmeticLeftShift;
        if (text == "sar")
            result = Token::ArithmeticRightShift;
        if (text == "rol")
            result = Token::CircularLeftShift;
        if (text == "ror")
            result = Token::CircularRightShift;
        if (text == "rcl")
            result = Token::RotateCarryLeftShift;
        if (text == "rcr")
            result = Token::RotateCarryRightShift;
    } else if (text.length() == 4) {
        if (text == "nand")
            result = Token::BitwiseLogicalNAND;
    } else if (text.length() == 5) {
        if (text == "yroot")
            result = Token::Yroot;
    }

    return result;
}

// Helper function: give operator precedence e.g. "+" is 300 while "*" is 500.
static int opPrecedence(Token::Operator op)
{
    int prec;
    switch (op) {
    case Token::Factorial:
        prec = 800;
        break;
    case Token::Exponentiation:
        prec = 700;
        break;
    case Token::Function:
        // Not really operator but needed for managing shift/reduce conflicts.
        prec = 600;
        break;
    //edit jingzhou 20200831 取余的优先级与乘除相同
    case Token::Modulo:
    case Token::Multiplication:
    case Token::Division:
    case Token::IntegerDivision:
        prec = 500;
        break;
    //add jingzhou 20200720
    case Token::Yroot:
    case Token::Log:
        prec = 600;
        break;
    case Token::Percent:
        prec = 800;
        break;
    case Token::Addition:
    case Token::Subtraction:
        prec = 300;
        break;
    case Token::ArithmeticLeftShift:
    case Token::ArithmeticRightShift:
    case Token::LogicalLeftShift:
    case Token::LogicalRightShift:
    case Token::CircularLeftShift:
    case Token::CircularRightShift:
    case Token::RotateCarryLeftShift:
    case Token::RotateCarryRightShift:
        prec = 200;
        break;
    //逻辑运算优先级：&与 > ^异或 > |或
    case Token::BitwiseLogicalAND:
    case Token::BitwiseLogicalNAND:
        prec = 100;
        break;
    case Token::BitwiseLogicalXOR:
        prec = 75;
        break;
    case Token::BitwiseLogicalOR:
    case Token::BitwiseLogicalNOR:
        prec = 50;
        break;
    case Token::UnitConversion:
        prec = 0;
        break;
    case Token::AssociationEnd:
    case Token::ListSeparator:
        prec = -100;
        break;
    case Token::AssociationStart:
        prec = -200;
        break;
    default:
        prec = -200;
        break;
    }
    return prec;
}

Token::Token(Type type, const QString &text, int pos, int size)
{
    m_type = type;
    m_text = text;
    m_pos = pos;
    m_size = size;
    m_minPrecedence = MAX_PRECEDENCE;
}

Token::Token(const Token &token)
{
    m_type = token.m_type;
    m_text = token.m_text;
    m_pos = token.m_pos;
    m_size = token.m_size;
    m_minPrecedence = token.m_minPrecedence;
}

Token &Token::operator=(const Token &token)
{
    m_type = token.m_type;
    m_text = token.m_text;
    m_pos = token.m_pos;
    m_size = token.m_size;
    m_minPrecedence = token.m_minPrecedence;
    return*this;
}

Quantity Token::asNumber() const
{
    QString text = m_text;
    return isNumber() ? Quantity(CNumber((const char *)text.toLatin1()))
           : Quantity(0);
}

Token::Operator Token::asOperator() const
{
    return isOperator() ? matchOperator(m_text) : Invalid;
}

QString Token::description() const
{
    QString desc;

    switch (m_type) {
    case stxNumber:
        desc = "Number";
        break;
    case stxIdentifier:
        desc = "Identifier";
        break;
    case stxOpenPar:
    case stxClosePar:
    case stxSep:
    case stxOperator:
        desc = "Operator";
        break;
    default:
        desc = "Unknown";
        break;
    }

    while (desc.length() < 10)
        desc.prepend(' ');

    QString header;
    header.append(QString::number(m_pos) + ","
                  + QString::number(m_pos + m_size - 1));
    header.append("," + (m_minPrecedence == MAX_PRECEDENCE ?
                         "MAX" : QString::number(m_minPrecedence)));
    header.append("  ");

    while (header.length() < 10)
        header.append(' ');

    desc.prepend(header);
    desc.append(" : ").append(m_text);

    return desc;
}

static bool tokenPositionCompare(const Token &a, const Token &b)
{
    return (a.pos() < b.pos());
}

TokenStack::TokenStack() : QVector<Token>()
{
    topIndex = 0;
    m_error = "";
    ensureSpace();
}

bool TokenStack::isEmpty() const
{
    return topIndex == 0;
}

unsigned TokenStack::itemCount() const
{
    return topIndex;
}

void TokenStack::push(const Token &token)
{
    ensureSpace();
    (*this)[topIndex++] = token;
}

Token TokenStack::pop()
{
    if (topIndex > 0)
        return Token(at(--topIndex));

    m_error = "token stack is empty (BUG)";
    return Token();
}

const Token &TokenStack::top()
{
    return top(0);
}

const Token &TokenStack::top(unsigned index)
{
    return (topIndex > (int)index) ? at(topIndex - index - 1) : Token::null;
}

void TokenStack::ensureSpace()
{
    int length = size();
    while (topIndex >= length) {
        length += 10;
        resize(length);
    }
}

/** Remove \a count tokens from the top of the stack, add a stxAbstract token
 * to the top and adjust its text position and minimum precedence.
 *
 * \param minPrecedence minimum precedence to set the top token, or
 * \c INVALID_PRECEDENCE if this method should use the minimum value from
 * the removed tokens.
 */
void TokenStack::reduce(int count, int minPrecedence)
{
    // assert(itemCount() > count);

    QList<Token> tokens;
    for (int i = 0 ; i < count ; ++i)
        tokens.append(pop());

    reduce(tokens, Token(Token::stxAbstract), minPrecedence);
}

/** Remove \a count tokens from the top of the stack, push \a top to the top
 * and adjust its text position and minimum precedence.
 *
 * \param minPrecedence minimum precedence to set the top token, or
 * \c INVALID_PRECEDENCE if this method should use the minimum value
 * from the removed tokens.
 */
void TokenStack::reduce(int count, Token &&top, int minPrecedence)
{
    // assert(itemCount() >= count);

    QList<Token> tokens;
    for (int i = 0 ; i < count ; ++i)
        tokens.append(pop());

    reduce(tokens, std::forward<Token>(top), minPrecedence);
}

/** Push \a top to the top and adjust its text position and minimum precedence
 * using \a tokens.
 *
 * \param minPrecedence minimum precedence to set the top token, or
 * \c INVALID_PRECEDENCE if this method should use the minimum value from the
 * removed tokens.
 */
void TokenStack::reduce(QList<Token> tokens, Token &&top, int minPrecedence)
{
#ifdef EVALUATOR_DEBUG
    {
        const auto &_tokens = tokens;
        qDebug() << "reduce(" << _tokens.size() << ", " << top.description()
                 << ", " << minPrecedence << ")";
        for (const auto &t : _tokens)
            qDebug() << t.description();
    }
#endif // EVALUATOR_DEBUG

    std::sort(tokens.begin(), tokens.end(), tokenPositionCompare);

    bool computeMinPrec = (minPrecedence == INVALID_PRECEDENCE);
    int min_prec = computeMinPrec ? MAX_PRECEDENCE : minPrecedence;
    int start = -1, end = -1;
    const auto &_tokens = tokens;
    for (auto &token : _tokens) {
        if (computeMinPrec) {
            Token::Operator op = token.asOperator();
            if (op != Token::Invalid) {
                int prec = opPrecedence(op);
                if (prec < min_prec)
                    min_prec = prec;
            }
        }

        if (token.pos() == -1 && token.size() == -1)
            continue;

        if (token.pos() == -1 || token.size() == -1) {

#ifdef EVALUATOR_DEBUG
            qDebug() << "BUG: found token with either pos or size not set, "
                     "but not both.";
#endif  // EVALUATOR_DEBUG
            continue;
        }

        if (start == -1) {
            start = token.pos();
        } else {

#ifdef EVALUATOR_DEBUG
            if (token.pos() != end)
                qDebug() << "BUG: tokens expressions are not successive.";
#endif  // EVALUATOR_DEBUG

        }

        end = token.pos() + token.size();
    }

    if (start != -1) {
        top.setPos(start);
        top.setSize(end - start);
    }

    top.setMinPrecedence(min_prec);

#ifdef EVALUATOR_DEBUG
    qDebug() << "=> " << top.description();
#endif  // EVALUATOR_DEBUG

    push(top);
}

#ifdef EVALUATOR_DEBUG
void Tokens::append(const Token &token)
{
    qDebug() << QString("tokens.append: type=%1 text=%2")
             .arg(token.type())
             .arg(token.text());
    QVector<Token>::append(token);
}
#endif // EVALUATOR_DEBUG

// Helper function: return true for valid identifier character.
static bool isIdentifier(QChar ch)
{
    return ch.unicode() == '_' || ch.unicode() == '$' || ch.isLetter();
}

// Helper function: return true for valid radix characters.
bool Evaluator::isRadixChar(const QChar &ch)
{
    if (Settings::instance()->isRadixCharacterBoth())
        return ch.unicode() == '.' || ch.unicode() == ',';

    // There are more than 2 radix characters, actually:
    //     U+0027 ' apostrophe
    //     U+002C , comma
    //     U+002E . full stop
    //     U+00B7 · middle dot
    //     U+066B ٫ arabic decimal separator
    //     U+2396 ⎖ decimal separator key symbol

    return ch.unicode() == Settings::instance()->radixCharacter();
}

// Helper function: return true for valid thousand separator characters.
bool Evaluator::isSeparatorChar(const QChar &ch)
{
    // Match everything that is not alphanumeric or an operator or NUL.
    static const QRegExp s_separatorRE(
        "[^a-zA-Z0-9\\+\\-−\\*×⋅÷/\\^;\\(\\)%!=\\\\&\\|<>\\?#\\x0000]"
    );

    if (isRadixChar(ch))
        return false;

    return s_separatorRE.exactMatch(ch);
}

QString Evaluator::fixNumberRadix(const QString &number)
{
    int dotCount = 0;
    int commaCount = 0;
    QChar lastRadixChar;

    // First pass: count the number of dot and comma characters.
    for (int i = 0 ; i < number.size() ; ++i) {
        QChar c = number[i];
        if (isRadixChar(c)) {
            lastRadixChar = c;

            if (c == '.')
                ++dotCount;
            else if (c == ',')
                ++commaCount;
            else
                return QString(); // Should not happen.
        }
    }

    // Decide which radix characters to ignore based on their occurence count.
    bool ignoreDot = dotCount != 1;
    bool ignoreComma = commaCount != 1;
    if (!ignoreDot && !ignoreComma) {
        // If both radix characters are present once,
        // consider the last one as the radix point.
        ignoreDot = lastRadixChar != '.';
        ignoreComma = lastRadixChar != ',';
    }

    QChar radixChar; // Null character by default.
    if (!ignoreDot)
        radixChar = '.';
    else if (!ignoreComma)
        radixChar = ',';

    // Second pass: write the result.
    QString result = "";
    for (int i = 0 ; i < number.size() ; ++i) {
        QChar c = number[i];
        if (isRadixChar(c)) {
            if (c == radixChar)
                result.append('.');
        } else
            result.append(c);
    }

    return result;
}

Evaluator *Evaluator::instance()
{
    if (!s_evaluatorInstance) {
        s_evaluatorInstance = new Evaluator;
        qAddPostRoutine(s_deleteEvaluator);
    }
    return s_evaluatorInstance;
}

Evaluator::Evaluator()
{
    reset();
}

#define ADD_UNIT(name) \
    setVariable(QString::fromUtf8(#name), Units::name(), Variable::BuiltIn)

void Evaluator::initializeBuiltInVariables()
{
    setVariable(QLatin1String("e"), DMath::e(), Variable::BuiltIn); //edit 20200805 jingzhou for bug--41371,重新替换回小写e
    setVariable(QString::fromUtf8("ℯ"), DMath::e(), Variable::BuiltIn);

    setVariable(QLatin1String("pi"), DMath::pi(), Variable::BuiltIn);
    setVariable(QString::fromUtf8("π"), DMath::pi(), Variable::BuiltIn);

    if (Settings::instance()->complexNumbers) {
        setVariable(QLatin1String("j"), DMath::i(), Variable::BuiltIn);
    } else if (hasVariable("j")) {
        unsetVariable("j", ForceBuiltinVariableErasure(true));
    }

    QList<Unit> unitList(Units::getList());
    for (Unit &u : unitList) {
        setVariable(u.name, u.value, Variable::BuiltIn);
    }

    initializeAngleUnits();
}

void Evaluator::initializeAngleUnits()
{
    if (Settings::instance()->angleUnit == 'r') {
        setVariable("radian", 1, Variable::BuiltIn);
        setVariable("degree", HMath::pi() / HNumber(180), Variable::BuiltIn);
        setVariable("gradian", HMath::pi() / HNumber(200), Variable::BuiltIn);
        setVariable("gon", HMath::pi() / HNumber(200), Variable::BuiltIn);
    } else if (Settings::instance()->angleUnit == 'g') {
        setVariable("radian", HNumber(200) / HMath::pi(), Variable::BuiltIn);
        setVariable("degree", HNumber(200) / HNumber(180), Variable::BuiltIn);
        setVariable("gradian", 1, Variable::BuiltIn);
        setVariable("gon", 1, Variable::BuiltIn);
    } else {    // d
        setVariable("radian", HNumber(180) / HMath::pi(), Variable::BuiltIn);
        setVariable("degree", 1, Variable::BuiltIn);
        setVariable("gradian", HNumber(180) / HNumber(200), Variable::BuiltIn);
        setVariable("gon", HNumber(180) / HNumber(200), Variable::BuiltIn);
    }
}

void Evaluator::setExpression(const QString &expr)
{
    m_expression = expr;
    m_dirty = true;
    m_valid = false;
    m_error = QString();
}

QString Evaluator::expression() const
{
    return m_expression;
}

// Returns the validity. Note: empty expression is always invalid.
bool Evaluator::isValid()
{
    if (m_dirty) {
        Tokens tokens = scan(m_expression);
        if (!tokens.valid())
            compile(tokens);
        else
            m_valid = false;
    }
    return m_valid;
}

void Evaluator::reset()
{
    m_expression = QString();
    m_dirty = true;
    m_valid = false;
    m_error = QString();
    m_constants.clear();
    m_codes.clear();
    m_assignId = QString();
    m_assignFunc = false;
    m_assignArg.clear();
    m_session = nullptr;
    m_functionsInUse.clear();

    initializeBuiltInVariables();
}

void Evaluator::setSession(Session *s)
{
    m_session = s;
}

const Session *Evaluator::session()
{
    return m_session;
}

QString Evaluator::error() const
{
    return m_error;
}

// Returns list of token for the expression.
// This triggers again the lexical analysis step. It is however preferable
// (even when there's small performance penalty) because otherwise we need to
// store parsed tokens all the time which serves no good purpose.
Tokens Evaluator::tokens() const
{
    return scan(m_expression);
}

Tokens Evaluator::scan(const QString &expr) const
{
    // Associate character codes with the highest number base
    // they might belong to.
    constexpr unsigned DIGIT_MAP_COUNT = 128;
    static unsigned char s_digitMap[DIGIT_MAP_COUNT] = { 0 };

    if (s_digitMap[0] == 0) {
        // Initialize the digits map.
        std::fill_n(s_digitMap, DIGIT_MAP_COUNT, 255);
        for (int i = '0' ; i <= '9' ; ++i)
            s_digitMap[i] = i - '0' + 1;
        for (int i = 'a' ; i <= 'z' ; ++i)
            s_digitMap[i] = i - 'a' + 11;
        for (int i = 'A' ; i <= 'Z' ; ++i)
            s_digitMap[i] = i - 'A' + 11;
    }

    // Result.
    Tokens tokens;

    // Parsing state.
    enum {
        Init, Start, Finish, Bad, InNumberPrefix, InNumber, InExpIndicator,
        InExponentBase, InExponent, InIdentifier, InNumberEnd
    } state;

    // Initialize variables.
    state = Init;
    int i = 0;
    QString ex = expr;
    QString tokenText;
    int tokenStart = 0; // Includes leading spaces.
    Token::Type type;
    int numberBase = 10;
    int expBase = 0;
    int expStart = -1;  // Index of the exponent part in the expression.
    QString expText;    // Start of the exponent text matching /E[\+\-]*/

    // Force a terminator.
    ex.append(QChar());

#ifdef EVALUATOR_DEBUG
    qDebug() << "Scanning" << ex;
#endif // EVALUATOR_DEBUG

    // Main loop.
    while (state != Bad && state != Finish && i < ex.length()) {
        QChar ch = ex.at(i);

#ifdef EVALUATOR_DEBUG
        qDebug() << QString("state=%1 ch=%2 i=%3 tokenText=%4")
                 .arg(state).arg(ch).arg(i).arg(tokenText);
#endif // EVALUATOR_DEBUG

        switch (state) {
        case Init:
            tokenStart = i;
            tokenText = "";
            state = Start;

            // State variables reset
            expStart = -1;
            expText = "";
            numberBase = 10;
            expBase = 0;

        // No break here on purpose (make sure Start is the next case)

        case Start:
            // Skip any whitespaces.
            if (ch.isSpace())
                ++i;
            else if (ch == '?') // Comment.
                state = Finish;
            else if (ch.isDigit()) {
                // Check for number
                state = InNumberPrefix;
            } else if (ch == '#') {
                // Simple hexadecimal notation
                tokenText.append("0x");
                numberBase = 16;
                state = InNumber;
                ++i;
            } else if (isRadixChar(ch)) {
                // Radix character?
                tokenText.append(ch);
                numberBase = 10;
                state = InNumber;
                ++i;
            } else if (isSeparatorChar(ch)) {
                // Leading separator, probably a number
                state = InNumberPrefix;
            } else if (ch.isNull()) // Terminator character.
                state = Finish;
            else if (isIdentifier(ch)) // Identifier or alphanumeric operator
                state = InIdentifier;
            else { // Look for operator match.
                int op;
                QString s;
                s = QString(ch).append(ex.at(i + 1));
                op = matchOperator(s);
                // Check for one-char operator.
                if (op == Token::Invalid) {
                    s = QString(ch);
                    op = matchOperator(s);
                }
                // Any matched operator?
                if (op != Token::Invalid) {
                    switch (op) {
                    case Token::AssociationStart:
                        type = Token::stxOpenPar;
                        break;
                    case Token::AssociationEnd:
                        type = Token::stxClosePar;
                        break;
                    case Token::ListSeparator:
                        type = Token::stxSep;
                        break;
                    default: type = Token::stxOperator;
                    }
                    int len = s.length();
                    i += len;
                    int tokenSize = i - tokenStart;
                    tokens.append(Token(type, s.left(len),
                                        tokenStart, tokenSize));
                    state = Init;
                } else
                    state = Bad;
            }
            break;

        // Manage both identifier and alphanumeric operators.
        case InIdentifier:
            // Consume as long as alpha, dollar sign, underscore, or digit.
            //edit jingzhou 20200720 mod直接作为%取余使用，yroot,log处理方式同上
            if ((isIdentifier(ch) || ch.isDigit())
                    && tokenText != "mod" && tokenText != "yroot" && tokenText != "log"
                    && tokenText != "and" && tokenText != "or" && tokenText != "xor" && tokenText != "nand" && tokenText != "nor"
                    && tokenText != "shl" && tokenText != "shr" && tokenText != "sal" && tokenText != "sar"
                    && tokenText != "rol" && tokenText != "ror" && tokenText != "rcl" && tokenText != "rcr")
                tokenText.append(ex.at(i++));
            else { // We're done with identifier.
                int tokenSize = i - tokenStart;
                if (matchOperator(tokenText)) {
                    tokens.append(Token(Token::stxOperator, tokenText,
                                        tokenStart, tokenSize));
                } else {
                    // Normal identifier.
                    tokens.append(Token(Token::stxIdentifier, tokenText,
                                        tokenStart, tokenSize));
                }
                state = Init;
            }
            break;

        // Find out the number base.
        case InNumberPrefix:
            if (ch.isDigit()) {
                // Only consume the first digit and the second digit
                // if the first was 0.
                tokenText.append(ex.at(i++));
                if (tokenText != "0") {
                    numberBase = 10;
                    state = InNumber;
                }
            } else if (isExponent(ch, numberBase)) {
                if (tokenText.endsWith("0")) {
                    // Maybe exponent (tokenText is "0" or "-0").
                    numberBase = 10;
                    expText = ch.toUpper();
                    expStart = i;
                    ++i;
                    state = InExpIndicator;
                } else {
                    // Only leading separators.
                    state = Bad;
                }
            } else if (isRadixChar(ch)) {
                // Might be a radix point or a separator.
                // Collect it and decide later.
                tokenText.append(ch);
                numberBase = 10;
                state = InNumber;
                ++i;
            } else if (ch.toUpper() == 'X' && tokenText == "0" && Settings::instance()->programmerBase != 10) {
                // Hexadecimal number.
                numberBase = 16;
                tokenText.append('x');
                ++i;
                state = InNumber;
            } else if (ch.toUpper() == 'B' && tokenText == "0" && Settings::instance()->programmerBase != 10) {
                // Binary number.
                numberBase = 2;
                tokenText.append('b');
                ++i;
                state = InNumber;
            } else if (ch.toUpper() == 'O' && tokenText == "0" && Settings::instance()->programmerBase != 10) {
                // Octal number.
                numberBase = 8;
                tokenText.append('o');
                ++i;
                state = InNumber;
            } else if (ch.toUpper() == 'D' && tokenText == "0") {
                // Decimal number (with prefix).
                numberBase = 10;
                tokenText.append('d');
                ++i;
                state = InNumber;
            } else if (isSeparatorChar(ch)) {
                // Ignore thousand separators.
                ++i;
            } else if (tokenText.isEmpty() && (ch == '+' || isMinus(ch))) {
                // Allow expressions like "$-10" or "$+10".
                if (isMinus(ch))
                    tokenText.append('-');
                ++i;
            } else {
                if (tokenText.endsWith("0")) {
                    // Done with integer number (tokenText is "0" or "-0").
                    numberBase = 10;
                    state = InNumberEnd;
                } else {
                    // Only leading separators.
                    state = Bad;
                }
            }
            break;

        // Parse the number digits.
        case InNumber: {
            ushort c = ch.unicode();
            bool isDigit = c < DIGIT_MAP_COUNT && (s_digitMap[c] <= numberBase);

            if (isDigit) {
                // Consume as long as it's a digit
                tokenText.append(ex.at(i++).toUpper());
            } else if (isExponent(ch, numberBase)) {
                // Maybe exponent
                expText = ch.toUpper();
                expStart = i;
                ++i;
                tokenText = fixNumberRadix(tokenText);
                if (!tokenText.isNull()) {
                    state = InExpIndicator;
                } else
                    state = Bad;
            } else if (isRadixChar(ch)) {
                // Might be a radix point or a separator.
                // Collect it and decide later.
                tokenText.append(ch);
                ++i;
            } else if (isSeparatorChar(ch)) {
                // Ignore thousand separators.
                ++i;
            } else {
                // We're done with number.
                tokenText = fixNumberRadix(tokenText);
                if (!tokenText.isNull())
                    state = InNumberEnd;
                else
                    state = Bad;
            }

            break;
        }

        // Validate exponent start.
        case InExpIndicator: {
            ushort c = ch.unicode();
            bool isDigit = c < DIGIT_MAP_COUNT && (s_digitMap[c] <= numberBase);

            if (expBase == 0) {
                // Set the default exponent base (same as number)
                expBase = numberBase;
            }

            if (expText.length() == 1 && (ch == '+' || isMinus(ch))) {
                // Possible + or - right after E.
                expText.append(ch == QChar(0x2212) ? '-' : ch);
                ++i;
            } else if (isDigit) {
                if (ch == '0') {
                    // Might be a base prefix
                    expText.append(ch);
                    ++i;
                    state = InExponentBase;
                } else {
                    // Parse the exponent absolute value.
                    tokenText.append(expText);
                    state = InExponent;
                }
            } else if (isSeparatorChar(ch)) {
                // Ignore thousand separators.
                ++i;
            } else {
                // Invalid thing here. Rollback: might be an identifier
                // used in implicit multiplication.
                i = expStart;
                state = InNumberEnd;
            }

            break;
        }

        // Detect exponent base.
        case InExponentBase: {
            int base = -1;
            switch (ch.toUpper().unicode()) {
            case 'B':
                base = 2;
                break;
            case 'O':
                base = 8;
                break;
            case 'D':
                base = 10;
                break;
            case 'X':
                base = 16;
                break;
            }

            if (base != -1) {
                // Specific exponent base found
                expBase = base;
                tokenText.append(expText);
                tokenText.append(ch.toLower());
                ++i;
            } else {
                // No exponent base specified, use the default one
                tokenText.append(expText);
            }

            state = InExponent;

            break;
        }

        // Parse exponent.
        case InExponent: {
            ushort c = ch.unicode();
            bool isDigit = c < DIGIT_MAP_COUNT && (s_digitMap[c] <= expBase);

            if (isDigit) {
                // Consume as long as it's a digit.
                tokenText.append(ex.at(i++));
            } else if (isSeparatorChar(ch)) {
                // Ignore thousand separators.
                ++i;
            } else {
                // We're done with floating-point number.
                state = InNumberEnd;
            };

            break;
        }

        case InNumberEnd: {
            int tokenSize = i - tokenStart;
            tokens.append(Token(Token::stxNumber, tokenText,
                                tokenStart, tokenSize));

            // Make sure a number cannot be followed by another number.
            if (ch.isDigit() || isRadixChar(ch) || ch == '#')
                state = Bad;
            else
                state = Init;
            break;
        }

        case Bad:
            tokens.setValid(false);
            break;

        default:
            break;
        };
    }

    if (state == Bad)
        // Invalidating here too, because usually when we set state to Bad,
        // the case Bad won't be run.
        tokens.setValid(false);

    return tokens;
}

void Evaluator::compile(const Tokens &tokens)
{
#ifdef EVALUATOR_DEBUG
    QFile debugFile("eval.log");
    debugFile.open(QIODevice::WriteOnly);
    QTextStream dbg(&debugFile);
#endif

    // Initialize variables.
    m_dirty = false;
    m_valid = false;
    m_codes.clear();
    m_constants.clear();
    m_identifiers.clear();
    m_error = QString();

    // Sanity check.
    if (tokens.count() == 0)
        return;

    TokenStack syntaxStack;
    QStack<int> argStack;
    unsigned argCount = 1;

    for (int i = 0; i <= tokens.count() && !syntaxStack.hasError(); ++i) {
        // Helper token: Invalid is end-of-expression.
        auto token = (i < tokens.count()) ? tokens.at(i)
                     : Token(Token::stxOperator);
        auto tokenType = token.type();
        if (tokenType >= Token::stxOperator)
            tokenType = Token::stxOperator;

#ifdef EVALUATOR_DEBUG
        dbg << "\nToken: " << token.description() << "\n";
#endif

        // Unknown token is invalid.
        if (tokenType == Token::stxUnknown)
            break;

        // Try to apply all parsing rules.
#ifdef EVALUATOR_DEBUG
        dbg << "\tChecking rules..." << "\n";
#endif
        // Repeat until no more rule applies.
        bool argHandled = false;
        while (!syntaxStack.hasError()) {
            bool ruleFound = false;

            // Rule for function last argument: id (arg) -> arg.
            if (!ruleFound && syntaxStack.itemCount() >= 4) {
                Token par2 = syntaxStack.top();
                Token arg = syntaxStack.top(1);
                Token par1 = syntaxStack.top(2);
                Token id = syntaxStack.top(3);
                if (par2.asOperator() == Token::AssociationEnd
                        && arg.isOperand()
                        && par1.asOperator() == Token::AssociationStart
                        && id.isIdentifier()) {
                    ruleFound = true;
                    syntaxStack.reduce(4, MAX_PRECEDENCE);
                    m_codes.append(Opcode(Opcode::Function, argCount));
#ifdef EVALUATOR_DEBUG
                    dbg << "\tRule for function last argument "
                        << argCount << " \n";
#endif
                    argCount = argStack.empty() ? 0 : argStack.pop();
                }
            }

            // Are we entering a function? If token is operator,
            // and stack already has: id (arg.
            if (!ruleFound && !argHandled && tokenType == Token::stxOperator
                    && syntaxStack.itemCount() >= 3) {
                Token arg = syntaxStack.top();
                Token par = syntaxStack.top(1);
                Token id = syntaxStack.top(2);
                if (arg.isOperand()
                        && par.asOperator() == Token::AssociationStart
                        && id.isIdentifier()) {
                    ruleFound = true;
                    argStack.push(argCount);
#ifdef EVALUATOR_DEBUG
                    dbg << "\tEntering new function, pushing argcount="
                        << argCount << " of parent function\n";
#endif
                    argCount = 1;
                    break;
                }
            }

            // Rule for postfix operators: Y POSTFIX -> Y.
            // Condition: Y is not an operator, POSTFIX is a postfix op.
            // Since we evaluate from left to right,
            // we need not check precedence at this point.
            if (!ruleFound && syntaxStack.itemCount() >= 2) {
                Token postfix = syntaxStack.top();
                Token y = syntaxStack.top(1);
                if (postfix.isOperator() && y.isOperand()) {
                    switch (postfix.asOperator()) {
                    case Token::Factorial:
                        ruleFound = true;
                        syntaxStack.reduce(2);
                        m_codes.append(Opcode(Opcode::Fact));
                        break;
                    case Token::Percent:
                        m_singleNumPercent = false;
                        if (syntaxStack.top(2).type() == Token::stxOpenPar)
                            m_singleNumPercent = true;
                        ruleFound = true;
                        syntaxStack.reduce(2);
                        m_codes.append(Opcode(Opcode::Pct));
                        break;
                    default:;
                    }
                }
#ifdef EVALUATOR_DEBUG
                if (ruleFound) {
                    dbg << "\tRule for postfix operator "
                        << postfix.text() << "\n";
                }
#endif
            }

            // Rule for parenthesis: (Y) -> Y.
            if (!ruleFound && syntaxStack.itemCount() >= 3) {
                Token right = syntaxStack.top();
                Token y = syntaxStack.top(1);
                Token left = syntaxStack.top(2);
                if (y.isOperand()
                        && right.asOperator() == Token::AssociationEnd
                        && left.asOperator() == Token::AssociationStart) {
                    ruleFound = true;
                    syntaxStack.reduce(3, MAX_PRECEDENCE);
#ifdef EVALUATOR_DEBUG
                    dbg << "\tRule for (Y) -> Y" << "\n";
#endif
                }
            }

            // Rule for simplified syntax for function,
            // e.g. "sin pi" or "cos 1.2". Conditions:
            // *precedence of function reduction >= precedence of next token.
            // *or next token is not an operator.
            if (!ruleFound && syntaxStack.itemCount() >= 2) {
                Token arg = syntaxStack.top();
                Token id = syntaxStack.top(1);
                if (arg.isOperand() && isFunction(id)
                        && (!token.isOperator()
                            || opPrecedence(Token::Function) >=
                            opPrecedence(token.asOperator()))) {
                    ruleFound = true;
                    m_codes.append(Opcode(Opcode::Function, 1));
                    syntaxStack.reduce(2);
#ifdef EVALUATOR_DEBUG
                    dbg << "\tRule for simplified function syntax; function "
                        << id.text() << "\n";
#endif
                }
            }

            // Rule for unary operator in simplified function syntax.
            // This handles cases like "sin -90".
            if (!ruleFound && syntaxStack.itemCount() >= 3) {
                Token x = syntaxStack.top();
                Token op = syntaxStack.top(1);
                Token id = syntaxStack.top(2);
                if (x.isOperand() && isFunction(id)
                        && (op.asOperator() == Token::Addition
                            || op.asOperator() == Token::Subtraction)) {
                    ruleFound = true;
                    syntaxStack.reduce(2);
                    if (op.asOperator() == Token::Subtraction)
                        m_codes.append(Opcode(Opcode::Neg));
#ifdef EVALUATOR_DEBUG
                    dbg << "\tRule for unary operator in simplified "
                        "function syntax; function " << id.text() << "\n";
#endif
                }
            }

            // Rule for function arguments. If token is ; or ):
            // id (arg1 ; arg2 -> id (arg.
            // Must come before binary op rule, special case of the latter.
            if (!ruleFound && syntaxStack.itemCount() >= 5
                    && token.isOperator()
                    && (token.asOperator() == Token::AssociationEnd
                        || token.asOperator() == Token::ListSeparator)) {
                Token arg2 = syntaxStack.top();
                Token sep = syntaxStack.top(1);
                Token arg1 = syntaxStack.top(2);
                Token par = syntaxStack.top(3);
                Token id = syntaxStack.top(4);
                if (arg2.isOperand()
                        && sep.asOperator() == Token::ListSeparator
                        && arg1.isOperand()
                        && par.asOperator() == Token::AssociationStart
                        && id.isIdentifier()) {
                    ruleFound = true;
                    argHandled = true;
                    syntaxStack.reduce(3, MAX_PRECEDENCE);
                    ++argCount;
#ifdef EVALUATOR_DEBUG
                    dbg << "\tRule for function argument "
                        << argCount << " \n";
#endif
                }
            }

            // Rule for function call with parentheses,
            // but without argument, e.g. "2*PI()".
            if (!ruleFound && syntaxStack.itemCount() >= 3) {
                Token par2 = syntaxStack.top();
                Token par1 = syntaxStack.top(1);
                Token id = syntaxStack.top(2);
                if (par2.asOperator() == Token::AssociationEnd
                        && par1.asOperator() == Token::AssociationStart
                        && id.isIdentifier()) {
                    ruleFound = true;
                    syntaxStack.reduce(3, MAX_PRECEDENCE);
                    m_codes.append(Opcode(Opcode::Function, 0));
#ifdef EVALUATOR_DEBUG
                    dbg << "\tRule for function call with parentheses, "
                        "but without argument\n";
#endif
                }
            }

            // Rule for binary operator:  A (op) B -> A.
            // Conditions: precedence of op >= precedence of token.
            // Action: push (op) to result e.g.
            // "A * B" becomes "A" if token is operator "+".
            // Exception: for caret (power operator), if op is another caret
            // then it doesn't apply, e.g. "2^3^2" is evaluated as "2^(3^2)".
            // Exception: doesn't apply if B is a function name (to manage
            // shift/reduce conflict with simplified function syntax
            // (issue #600).
            if (!ruleFound && syntaxStack.itemCount() >= 3) {
                Token b = syntaxStack.top();
                Token op = syntaxStack.top(1);
                Token a = syntaxStack.top(2);
                if (a.isOperand() && b.isOperand() && op.isOperator()
                        && ( // Normal operator.
                            (token.isOperator()
                             && opPrecedence(op.asOperator()) >=
                             opPrecedence(token.asOperator())
                             && !(b.isIdentifier() && token.asOperator() == Token::AssociationStart)
                             && token.asOperator() != Token::Exponentiation)

                            || ( // May represent implicit multiplication.
                                token.isOperand()
                                && opPrecedence(op.asOperator()) >=
                                opPrecedence(Token::Multiplication)))
                        && !(isFunction(b))) {
                    ruleFound = true;
                    switch (op.asOperator()) {
                    // Simple binary operations.
                    case Token::Addition:
                        m_codes.append(Opcode::Add);
                        break;
                    case Token::Subtraction:
                        m_codes.append(Opcode::Sub);
                        break;
                    case Token::Multiplication:
                        m_codes.append(Opcode::Mul);
                        break;
                    case Token::Division:
                        m_codes.append(Opcode::Div);
                        break;
                    case Token::Exponentiation:
                        m_codes.append(Opcode::Pow);
                        break;
                    case Token::Modulo:
                        m_codes.append(Opcode::Modulo);
                        break;
                    //add jingzhou 20200720 增加yroot、log
                    case Token::Yroot:
                        m_codes.append(Opcode::Yroot);
                        break;
                    case Token::Log:
                        m_codes.append(Opcode::Log);
                        break;
                    case Token::IntegerDivision:
                        m_codes.append(Opcode::IntDiv);
                        break;
                    case Token::ArithmeticLeftShift:
                        m_codes.append(Opcode::Sal);
                        break;
                    case Token::ArithmeticRightShift:
                        m_codes.append(Opcode::Sar);
                        break;
                    case Token::LogicalLeftShift:
                        m_codes.append(Opcode::Shl);
                        break;
                    case Token::LogicalRightShift:
                        m_codes.append(Opcode::Shr);
                        break;
                    case Token::CircularLeftShift:
                        m_codes.append(Opcode::Rol);
                        break;
                    case Token::CircularRightShift:
                        m_codes.append(Opcode::Ror);
                        break;
                    case Token::RotateCarryLeftShift:
                        m_codes.append(Opcode::Rcl);
                        break;
                    case Token::RotateCarryRightShift:
                        m_codes.append(Opcode::Rcr);
                        break;
                    case Token::BitwiseLogicalAND:
                        m_codes.append(Opcode::BAnd);
                        break;
                    case Token::BitwiseLogicalOR:
                        m_codes.append(Opcode::BOr);
                        break;
                    //add jingzhou 20201201 增加逻辑运算nand，nor，xor
                    case Token::BitwiseLogicalNAND:
                        m_codes.append(Opcode::BNand);
                        break;
                    case Token::BitwiseLogicalNOR:
                        m_codes.append(Opcode::BNor);
                        break;
                    case Token::BitwiseLogicalXOR:
                        m_codes.append(Opcode::BXor);
                        break;
                    case Token::UnitConversion: {
                        static const QRegExp unitNameNumberRE(
                            "(^[0-9e\\+\\-\\.,]|[0-9e\\.,]$)",
                            Qt::CaseInsensitive);
                        QString unitName =
                            m_expression.mid(b.pos(), b.size()).simplified();
                        // Make sure the whole unit name can be used
                        // as a single operand in multiplications.
                        if (b.minPrecedence() <
                                opPrecedence(Token::Multiplication)) {
                            unitName = "(" + unitName + ")";
                        }
                        // Protect the unit name
                        // if it starts or ends with a number.
                        else if (unitNameNumberRE.indexIn(unitName) != -1)
                            unitName = "(" + unitName + ")";
                        m_codes.append(Opcode(Opcode::Conv, unitName));
                        break;
                    }
                    default: break;
                    };
                    syntaxStack.reduce(3);
#ifdef EVALUATOR_DEBUG
                    dbg << "\tRule for binary operator" << "\n";
#endif
                }
            }

#ifdef ALLOW_IMPLICIT_MULT
            // Rule for implicit multiplication.
            // Action: Treat as A * B.
            // Exception: doesn't apply if B is a function name
            // (to manage shift/reduce conflict with simplified
            // function syntax (fixes issue #600).
            if (!ruleFound && syntaxStack.itemCount() >= 2) {
                Token b = syntaxStack.top();
                Token a = syntaxStack.top(1);

                if (a.isOperand() && b.isOperand()
                        && token.asOperator() != Token::AssociationStart
                        && ( // Token is normal operator.
                            (token.isOperator()
                             && opPrecedence(Token::Multiplication) >=
                             opPrecedence(token.asOperator()))
                            || token.isOperand()) // Implicit multiplication.
                        && !isFunction(b)) {
                    ruleFound = true;
                    syntaxStack.reduce(2, opPrecedence(Token::Multiplication));
                    m_codes.append(Opcode::Mul);
#ifdef EVALUATOR_DEBUG
                    dbg << "\tRule for implicit multiplication" << "\n";
#endif
                }

            }
#endif

            // Rule for unary operator:  (op1) (op2) X -> (op1) X.
            // Conditions: op2 is unary.
            // Current token has lower precedence than multiplication.
            if (!ruleFound
                    && token.asOperator() != Token::AssociationStart
                    && syntaxStack.itemCount() >= 3) {
                Token x = syntaxStack.top();
                Token op2 = syntaxStack.top(1);
                Token op1 = syntaxStack.top(2);
                if (x.isOperand() && op1.isOperator()
                        && (op2.asOperator() == Token::Addition
                            || op2.asOperator() == Token::Subtraction)
                        && (token.isOperand()
                            || opPrecedence(token.asOperator()) <=
                            opPrecedence(Token::Multiplication))) {
                    ruleFound = true;
                    if (op2.asOperator() == Token::Subtraction)
                        m_codes.append(Opcode(Opcode::Neg));

                    syntaxStack.reduce(2);
#ifdef EVALUATOR_DEBUG
                    dbg << "\tRule for unary operator" << op2.text() << "\n";
#endif
                }
            }

            // Auxiliary rule for unary prefix operator:  (op) X -> X.
            // Conditions: op is unary, op is first in syntax stack.
            // Action: create code for (op). Unary MINUS or PLUS are
            // treated with the precedence of multiplication.
            if (!ruleFound
                    && token.asOperator() != Token::AssociationStart
                    && syntaxStack.itemCount() == 2) {
                Token x = syntaxStack.top();
                Token op = syntaxStack.top(1);
                if (x.isOperand()
                        && (op.asOperator() == Token::Addition
                            || op.asOperator() == Token::Subtraction)
                        && ((token.isOperator()
                             && opPrecedence(token.asOperator()) <=
                             opPrecedence(Token::Multiplication))
                            || token.isOperand())) {
                    ruleFound = true;
                    if (op.asOperator() == Token::Subtraction)
                        m_codes.append(Opcode(Opcode::Neg));
#ifdef EVALUATOR_DEBUG
                    dbg << "\tRule for unary operator (auxiliary)" << "\n";
#endif
                    syntaxStack.reduce(2);
                }
            }

            if (!ruleFound)
                break;
        }

        // Can't apply rules anymore, push the token.
        syntaxStack.push(token);

        // For identifier, generate code to load from reference.
        if (tokenType == Token::stxIdentifier) {
            m_identifiers.append(token.text());
            m_codes.append(Opcode(Opcode::Ref, m_identifiers.count() - 1));
#ifdef EVALUATOR_DEBUG
            dbg << "\tPush " << token.text() << " to identifier pools" << "\n";
#endif
        }

        // For constants, generate code to load from a constant.
        if (tokenType == Token::stxNumber) {
            m_constants.append(token.asNumber());
            m_codes.append(Opcode(Opcode::Load, m_constants.count() - 1));
#ifdef EVALUATOR_DEBUG
            dbg << "\tPush " << token.asNumber()
                << " to constant pools" << "\n";
#endif
        }
    }

    m_valid = false;
    if (syntaxStack.hasError())
        m_error = syntaxStack.error();
    // syntaxStack must left only one operand
    // and end-of-expression (i.e. Invalid).
    else if (syntaxStack.itemCount() == 2
             && syntaxStack.top().isOperator()
             && syntaxStack.top().asOperator() == Token::Invalid
             && !syntaxStack.top(1).isOperator()) {
        m_valid = true;
    }

#ifdef EVALUATOR_DEBUG
    dbg << "Dump: " << dump() << "\n";
    debugFile.close();
#endif

    // Bad parsing? Clean-up everything.
    if (!m_valid) {
        m_constants.clear();
        m_codes.clear();
        m_identifiers.clear();
    }
}

Quantity Evaluator::evalNoAssign()
{
    Quantity result;

    if (m_dirty) {
        // Reset.
        m_assignId = QString();
        m_assignFunc = false;
        m_assignArg.clear();

        Tokens tokens = scan(m_expression);

        // Invalid expression?
        if (!tokens.valid()) {
            m_error = /*tr*/("invalid expression");
            return Quantity(0);
        }

        // Variable assignment?
        if (tokens.count() > 2
                && tokens.at(0).isIdentifier()
                && tokens.at(1).asOperator() == Token::Assignment) {
            m_assignId = tokens.at(0).text();
            tokens.erase(tokens.begin());
            tokens.erase(tokens.begin());
        } else if (tokens.count() > 2
                   && tokens.at(0).isIdentifier()
                   && tokens.at(1).asOperator() == Token::AssociationStart) {
            // Check for function assignment.
            // Syntax:
            // ident opLeftPar (ident (opSemiColon ident)*)? opRightPar opEqual
            int t;
            if (tokens.count() > 4
                    && tokens.at(2).asOperator() == Token::AssociationEnd) {
                // Functions with no argument.
                t = 3;
                if (tokens.at(3).asOperator() == Token::Assignment)
                    m_assignFunc = true;
            } else {
                for (t = 2; t + 1 < tokens.count(); t += 2)  {
                    if (!tokens.at(t).isIdentifier())
                        break;

                    m_assignArg.append(tokens.at(t).text());

                    if (tokens.at(t + 1).asOperator() == Token::AssociationEnd) {
                        t += 2;
                        if (t < tokens.count()
                                && tokens.at(t).asOperator() == Token::Assignment) {
                            m_assignFunc = true;
                        }

                        break;
                    } else if (tokens.at(t + 1)
                               .asOperator() != Token::ListSeparator)
                        break;
                }
            }

            if (m_assignFunc) {
                m_assignId = tokens.at(0).text();
                for (; t >= 0; --t)
                    tokens.erase(tokens.begin());
            } else
                m_assignArg.clear();
        }

        compile(tokens);
        if (!m_valid) {
            if (m_error.isEmpty())
                m_error = /*tr*/("compile error");
            return CNumber(0);
        }
    }

    result = exec(m_codes, m_constants, m_identifiers);
    return result;
}

Quantity Evaluator::exec(const QVector<Opcode> &opcodes,
                         const QVector<Quantity> &constants,
                         const QStringList &identifiers)
{
    QStack<Quantity> stack;
    QHash<int, QString> refs;
    int index;
    Quantity val1, val2;
    QVector<Quantity> args;
    QString fname;
    Function *function;
    const UserFunction *userFunction = nullptr;

    for (int pc = 0; pc < opcodes.count(); ++pc) {
        const Opcode &opcode = opcodes.at(pc);
        index = opcode.index;
        switch (opcode.type) {
        // No operation.
        case Opcode::Nop:
            break;

        // Load a constant, push to stack.
        case Opcode::Load:
            val1 = constants.at(index);
            stack.push(val1);
            break;

        // Unary operation.
        case Opcode::Neg:
            if (stack.count() < 1) {
                m_error = /*tr*/("invalid expression");
                return CMath::nan();
            }
            val1 = stack.pop();
            val1 = checkOperatorResult(-val1);
            stack.push(val1);
            break;

        // Binary operation: take two values from stack,
        // do the operation, push the result to stack.
        case Opcode::Add:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return CMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            val2 = checkOperatorResult(val2 + val1);
            stack.push(val2);
            break;

        case Opcode::Sub:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return CMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            val2 = checkOperatorResult(val2 - val1);
            stack.push(val2);
            break;

        case Opcode::Mul:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return CMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            val2 = checkOperatorResult(val2 * val1);
            stack.push(val2);
            break;

        case Opcode::Div:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return CMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            //对于 A^(X/Y)类型的表达式，当X为偶数时，将A变为正数
            if ((pc + 1) < opcodes.count() && opcodes.at(pc + 1).type == Opcode::Pow) {
                if (val2.isEven()) {
                    Quantity temp = stack.pop();
                    temp *= temp.isNegative() ? -1 : 1;
                    stack.push(temp);
                }
            }

            val2 = checkOperatorResult(val2 / val1);
            stack.push(val2);
            break;

        case Opcode::Pow:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return CMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            val2 = checkOperatorResult(DMath::raise(val2, val1));
            stack.push(val2);
            break;

        //add jingzhou 20200720
        case Opcode::Yroot:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return CMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            val2 = checkOperatorResult(DMath::raise(val2, Quantity(1) / val1));
            stack.push(val2);
            break;

        case Opcode::Log:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return CMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            val2 = checkOperatorResult(DMath::log(val1, val2));
            stack.push(val2);
            break;

        case Opcode::Fact:
            if (stack.count() < 1) {
                m_error = /*tr*/("invalid expression");
                return CMath::nan();
            }
            val1 = stack.pop();
            val1 = checkOperatorResult(DMath::factorial(val1));
            stack.push(val1);
            break;

        case Opcode::Modulo:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return CMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            val2 = checkOperatorResult(val2 % val1);
            stack.push(val2);
            break;

        case Opcode::IntDiv:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return CMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            val2 = checkOperatorResult(val2 / val1);
            stack.push(DMath::integer(val2));
            break;

        case Opcode::Sal:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            if (val1.isNegative()) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val2 = checkOperatorResult(DMath::ashr(val2, -val1));
            stack.push(val2);
            break;

        case Opcode::Sar:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            if (val1.isNegative()) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val2 = checkOperatorResult(DMath::ashr(val2, val1));
            stack.push(val2);
            break;

        case Opcode::Shl:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            if (val1.isNegative()) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val2 = checkOperatorResult(DMath::lshr(val2, -val1));
            stack.push(val2);
            break;

        case Opcode::Shr:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            if (val1.isNegative()) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val2 = checkOperatorResult(DMath::lshr(val2, val1));
            stack.push(val2);
            break;

        case Opcode::Rol:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            if (val1.isNegative()) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val2 = checkOperatorResult(DMath::rosh(val2, -val1));
            stack.push(val2);
            break;

        case Opcode::Ror:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            if (val1.isNegative()) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val2 = checkOperatorResult(DMath::rosh(val2, val1));
            stack.push(val2);
            break;

        case Opcode::Rcl:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            if (val1.isNegative()) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val2 = checkOperatorResult(DMath::rcsh(val2, -val1));
            stack.push(val2);
            break;

        case Opcode::Rcr:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            if (val1.isNegative()) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val2 = checkOperatorResult(DMath::rcsh(val2, val1));
            stack.push(val2);
            break;

        case Opcode::BAnd:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            val2 &= val1;
            stack.push(val2);
            break;

        case Opcode::BOr:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            val2 |= val1;
            stack.push(val2);
            break;

        case Opcode::BNand:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            val2 &= val1;
            val2 = ~ val2;
            stack.push(val2);
            break;

        case Opcode::BNor:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            val2 |= val1;
            val2 = ~ val2;
            stack.push(val2);
            break;

        case Opcode::BXor:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return DMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            val2 ^= val1;
            stack.push(val2);
            break;

        case Opcode::Pct:
            if (stack.count() < 1) {
                m_error = /*tr*/("invalid expression");
                return CMath::nan();
            }
            val1 = stack.pop();

            switch (((pc + 1) < opcodes.count() && !m_singleNumPercent) ?
                    opcodes.at(pc + 1).type : Opcode::Nop) {
            case Opcode::Add:
            case Opcode::Sub:
                val2 = stack.pop();
                stack.push(val2);
                break;
            default:
                val2 = Quantity(1);
            }
            val1 = checkOperatorResult(val2 * (val1 * HNumber("0.01")));
            m_standardPercent = val1;
            stack.push(val1);
            break;

        case Opcode::Conv:
            if (stack.count() < 2) {
                m_error = /*tr*/("invalid expression");
                return HMath::nan();
            }
            val1 = stack.pop();
            val2 = stack.pop();
            if (val1.isZero()) {
                m_error = /*tr*/("unit must not be zero");
                return HMath::nan();
            }
            if (!val1.sameDimension(val2)) {
                m_error = /*tr*/("Conversion failed - dimension mismatch");
                return HMath::nan();
            }
            val2.setDisplayUnit(val1.numericValue(), opcode.text);
            stack.push(val2);
            break;

        // Reference.
        case Opcode::Ref:
            fname = identifiers.at(index);
            if (m_assignArg.contains(fname)) {
                // Argument.
                stack.push(CMath::nan());
            } else if (hasVariable(fname)) {
                // Variable.
                stack.push(getVariable(fname).value());
            } else {
                // Function.
                function = FunctionRepo::instance()->find(fname);
                if (function) {
                    stack.push(CMath::nan());
                    refs.insert(stack.count(), fname);
                } else if (m_assignFunc) {
                    // Allow arbitrary identifiers
                    // when declaring user functions.
                    stack.push(CMath::nan());
                    refs.insert(stack.count(), fname);
                } else if (hasUserFunction(fname)) {
                    stack.push(CMath::nan());
                    refs.insert(stack.count(), fname);
                } else {
                    m_error = "<b>" + fname + "</b>: "
                              + /*tr*/("unknown function or variable");
                    return CMath::nan();
                }
            }
            break;

        // Calling function.
        case Opcode::Function:
            // Must do this first to avoid crash
            // when using vars like functions.
            if (refs.isEmpty())
                break;

            fname = refs.take(stack.count() - index);
            function = FunctionRepo::instance()->find(fname);

            userFunction = nullptr;
            if (!function) {
                // Check if this is a valid user function call.
                userFunction = getUserFunction(fname);
            }

            if (!function && !userFunction && !m_assignFunc) {
                m_error = "<b>" + fname + "</b>: "
                          + /*tr*/("unknown function or variable");
                return CMath::nan();
            }

            if (stack.count() < index) {
                m_error = /*tr*/("invalid expression");
                return CMath::nan();
            }

            args.clear();
            for (; index; --index)
                args.insert(args.begin(), stack.pop());

            // Remove the NaN we put on the stack (needed to make the user
            // functions declaration work with arbitrary identifiers).
            stack.pop();

            // Show function signature if user has given no argument (yet).
            if (userFunction) {
                if (!args.count()
                        && userFunction->arguments().count() != 0) {
                    m_error = QString::fromLatin1("<b>%1</b>(%2)").arg(
                                  userFunction->name(),
                                  userFunction->arguments().join(";")
                              );
                    return CMath::nan();
                }
            } else if (function) {
                if (!args.count()) {
                    m_error = QString::fromLatin1("<b>%1</b>(%2)").arg(
                                  fname,
                                  function->usage()
                              );
                    return CMath::nan();
                }
            }

            if (m_assignFunc) {
                // Allow arbitrary identifiers for declaring user functions.
                stack.push(CMath::nan());
            } else if (userFunction) {
                stack.push(execUserFunction(userFunction, args));
                if (!m_error.isEmpty())
                    return CMath::nan();
            } else {
                stack.push(function->exec(args));
                if (function->error()) {
                    m_error = stringFromFunctionError(function);
                    return CMath::nan();
                }
            }
            break;

        default:
            break;
        }
    }

    // More than one value in stack? Unsuccessful execution.
    if (stack.count() != 1) {
        m_error = /*tr*/("invalid expression");
        return CMath::nan();
    }
    return stack.pop();
}

Quantity Evaluator::execUserFunction(const UserFunction *function,
                                     QVector<Quantity> &arguments)
{
    // TODO: Replace user variables by user functions (with no argument)?
    if (arguments.count() != function->arguments().count()) {
        m_error = "<b>" + function->name() + "</b>: "
                  + /*tr*/("wrong number of arguments");
        return CMath::nan();
    }

    if (m_functionsInUse.contains(function->name())) {
        m_error = "<b>" + function->name() + "</b>: "
                  + /*tr*/("recursion not supported");
        return CMath::nan();
    }
    m_functionsInUse.insert(function->name());

    QVector<Opcode> newOpcodes;
    auto newConstants = function->constants; // Copy.

    // Replace references to function arguments by constants.
    for (int i = 0; i < function->opcodes.count(); ++i) {
        Opcode opcode = function->opcodes.at(i);

        if (opcode.type == Opcode::Ref) {
            // Check if the identifier is an argument name.
            QString name = function->identifiers.at(opcode.index);
            int argIdx = function->arguments().indexOf(name);
            if (argIdx >= 0) {
                // Replace the reference by a constant value.
                opcode = Opcode(Opcode::Load, newConstants.count());
                newConstants.append(arguments.at(argIdx));
            }
        }

        newOpcodes.append(opcode);
    }

    auto result = exec(newOpcodes, newConstants, function->identifiers);
    if (!m_error.isEmpty()) {
        // Tell the user where the error happened.
        m_error = "<b>" + function->name() + "</b>: " + m_error;
    }

    m_functionsInUse.remove(function->name());
    return result;
}

bool Evaluator::isUserFunctionAssign() const
{
    return m_assignFunc;
}

bool Evaluator::isBuiltInVariable(const QString &id) const
{
    // Defining variables with the same name as existing functions
    // is not supported for now.
    if (FunctionRepo::instance()->find(id))
        return true;

    if (!m_session || !m_session->hasVariable(id))
        return false;

    return m_session->getVariable(id).type() == Variable::BuiltIn;
}

Quantity Evaluator::eval()
{
    Quantity result = evalNoAssign(); // This sets m_assignId.

    if (!m_error.isEmpty())
        return result;

    if (isBuiltInVariable(m_assignId)) {
        //m_error = tr("%1 is a reserved name, "
        //             "please choose another").arg(m_assignId);
        return CMath::nan();
    }
    // Handle user variable or function assignment.
    if (!m_assignId.isEmpty()) {
        if (m_assignFunc) {
            if (hasVariable(m_assignId)) {
                //    m_error = tr("%1 is a variable name, please choose another "
                //                 "or delete the variable").arg(m_assignId);
                return CMath::nan();
            }

            // Check that each argument is unique and not a reserved identifier.
            for (int i = 0; i < m_assignArg.count() - 1; ++i) {
                const QString &argName = m_assignArg.at(i);

                if (m_assignArg.indexOf(argName, i + 1) != -1) {
                    //       m_error = tr("argument %1 is used "
                    //                    "more than once").arg(argName);
                    return CMath::nan();
                }

                if (isBuiltInVariable(argName)) {
                    //       m_error = tr("%1 is a reserved name, "
                    //                    "please choose another").arg(argName);
                    return CMath::nan();
                }
            }

            if (m_codes.isEmpty())
                return CMath::nan();
            UserFunction userFunction(m_assignId, m_assignArg,
                                      m_expression.section("=", 1, 1).trimmed());
            userFunction.constants = m_constants;
            userFunction.identifiers = m_identifiers;
            userFunction.opcodes = m_codes;

            setUserFunction(userFunction);

        } else {
            if (hasUserFunction(m_assignId)) {
                //m_error = tr("%1 is a user function name, please choose "
                //             "another or delete the function").arg(m_assignId);
                return CMath::nan();
            }

            setVariable(m_assignId, result);
        }
    }

    return result;
}

Quantity Evaluator::evalUpdateAns()
{
    Settings::instance()->proRotateCarry.front() = Settings::instance()->proRotateCarry.at(1);
    auto result = eval();
    if (m_error.isEmpty() && !m_assignFunc)
        setVariable(QLatin1String("ans"), result, Variable::BuiltIn);
    return result;
}

void Evaluator::setVariable(const QString &id, Quantity value,
                            Variable::Type type)
{
    if (!m_session)
        m_session = new Session(this);
    m_session->addVariable(Variable(id, value, type));
}

Variable Evaluator::getVariable(const QString &id) const
{
    if (id.isEmpty() || !m_session)
        return Variable(QLatin1String(""), Quantity(0));

    return m_session->getVariable(id);
}

bool Evaluator::hasVariable(const QString &id) const
{
    if (id.isEmpty() || !m_session)
        return false;
    else
        return m_session->hasVariable(id);
}

void Evaluator::unsetVariable(const QString &id,
                              ForceBuiltinVariableErasure force)
{
    if (!m_session || (m_session->isBuiltInVariable(id) && !force))
        return;
    m_session->removeVariable(id);
}

QList<Variable> Evaluator::getVariables() const
{
    return m_session ? m_session->variablesToList() : QList<Variable>();
}

QList<Variable> Evaluator::getUserDefinedVariables() const
{
    auto result = getVariables();
    auto iter = result.begin();
    while (iter != result.end()) {
        if ((*iter).type() == Variable::BuiltIn)
            iter = result.erase(iter);
        else
            ++iter;
    }
    return result;
}

QList<Variable> Evaluator::getUserDefinedVariablesPlusAns() const
{
    auto result = getUserDefinedVariables();
    auto ans = getVariable(QLatin1String("ans"));
    if (!ans.identifier().isEmpty())
        result.append(ans);
    return result;
}

void Evaluator::unsetAllUserDefinedVariables()
{
    if (!m_session)
        return;
    auto ansBackup = getVariable(QLatin1String("ans")).value();
    m_session->clearVariables();
    setVariable(QLatin1String("ans"), ansBackup, Variable::BuiltIn);
    initializeBuiltInVariables();
}

static void replaceSuperscriptPowersWithCaretEquivalent(QString &expr)
{
    static const QRegularExpression s_superscriptPowersRE(
        "(\\x{207B})?[\\x{2070}¹²³\\x{2074}-\\x{2079}]+"
    );
    static const QHash<QChar, QChar> s_superscriptPowersHash {
        {L'\u207B', '-'},
        {L'\u2070', '0'},
        {L'\u00B9', '1'},
        {L'\u00B2', '2'},
        {L'\u00B3', '3'},
        {L'\u2074', '4'},
        {L'\u2075', '5'},
        {L'\u2076', '6'},
        {L'\u2077', '7'},
        {L'\u2078', '8'},
        {L'\u2079', '9'},
    };

    int offset = 0;
    while (true) {
        QRegularExpressionMatch match = s_superscriptPowersRE.match(expr, offset);
        if (!match.hasMatch())
            break;

        QString power = match.captured();
        for (int pos = power.size() - 1; pos >= 0; --pos) {
            QChar c = power.at(pos);
            power.replace(pos, 1, s_superscriptPowersHash.value(c, c));
        }

        bool isNegative = match.capturedStart(1) != -1;
        if (isNegative)
            power = "^(" + power + ")";
        else
            power = "^" + power;

        expr.replace(match.capturedStart(), match.capturedLength(), power);
        offset = match.capturedStart() + power.size();
    }
}

QList<UserFunction> Evaluator::getUserFunctions() const
{
    return m_session ? m_session->UserFunctionsToList()
           : QList<UserFunction>();
}

void Evaluator::setUserFunction(const UserFunction &f)
{
    if (!m_session)
        m_session = new Session;
    m_session->addUserFunction(f);
}

void Evaluator::unsetUserFunction(const QString &fname)
{
    m_session->removeUserFunction(fname);
}

void Evaluator::unsetAllUserFunctions()
{
    m_session->clearUserFunctions();
}

bool Evaluator::hasUserFunction(const QString &fname) const
{
    bool invalid = fname.isEmpty() || !m_session;
    return (invalid) ? false : m_session->hasUserFunction(fname);
}

Quantity Evaluator::getStandardPercentAns()
{
    return m_standardPercent;
}

const UserFunction *Evaluator::getUserFunction(const QString &fname) const
{
    if (hasUserFunction(fname))
        return m_session->getUserFunction(fname);
    else
        return nullptr;
}

QString Evaluator::autoFix(const QString &expr)
{
    int par = 0;
    QString result;

    // Strip off all funny characters.
    for (int c = 0; c < expr.length(); ++c)
        if (expr.at(c) >= QChar(32))
            result.append(expr.at(c));

    // No extra whitespaces at the beginning and at the end.
    result = result.trimmed();

    // Strip trailing equal sign (=).
    while (result.endsWith("="))
        result = result.left(result.length() - 1);

    replaceSuperscriptPowersWithCaretEquivalent(result);

    // Automagically close all parenthesis.
    Tokens tokens = Evaluator::scan(result);
    if (tokens.count()) {
        for (int i = 0; i < tokens.count(); ++i)
            if (tokens.at(i).asOperator() == Token::AssociationStart)
                ++par;
            else if (tokens.at(i).asOperator() == Token::AssociationEnd)
                --par;

        if (par < 0)
            par = 0;

        // If the scanner stops in the middle, do not bother to apply fix.
        const Token &lastToken = tokens.at(tokens.count() - 1);
        if (lastToken.pos() + lastToken.size() >= result.length())
            while (par--)
                result.append(')');
    }

    // Special treatment for simple function
    // e.g. "cos" is regarded as "cos(ans)".
    if (!result.isEmpty()) {
        Tokens tokens = Evaluator::scan(result);

        if (tokens.count() == 1
                && tokens.at(0).isIdentifier()
                && FunctionRepo::instance()->find(tokens.at(0).text())) {
            result.append("(ans)");
        }
    }

    return result;
}

QString Evaluator::dump()
{
    QString result;
    int c;

    if (m_dirty) {
        Tokens tokens = scan(m_expression);
        compile(tokens);
    }

    result = QString("Expression: [%1]\n").arg(m_expression);

    result.append("  Constants:\n");
    for (c = 0; c < m_constants.count(); ++c) {
        auto val = m_constants.at(c);
        result += QString("    #%1 = %2\n").arg(c).arg(
                      DMath::format(val, Quantity::Format::Fixed())
                  );
    }

    result.append("\n");
    result.append("  Identifiers:\n");
    for (c = 0; c < m_identifiers.count(); ++c) {
        result += QString("    #%1 = %2\n").arg(c).arg(m_identifiers.at(c));
    }

    result.append("\n");
    result.append("  Code:\n");
    for (int i = 0; i < m_codes.count(); ++i) {
        QString code;
        switch (m_codes.at(i).type) {
        case Opcode::Load:
            code = QString("Load #%1").arg(m_codes.at(i).index);
            break;
        case Opcode::Ref:
            code = QString("Ref #%1").arg(m_codes.at(i).index);
            break;
        case Opcode::Function:
            code = QString("Function (%1)").arg(m_codes.at(i).index);
            break;
        case Opcode::Add:
            code = "Add";
            break;
        case Opcode::Sub:
            code = "Sub";
            break;
        case Opcode::Mul:
            code = "Mul";
            break;
        case Opcode::Div:
            code = "Div";
            break;
        case Opcode::Neg:
            code = "Neg";
            break;
        case Opcode::Pow:
            code = "Pow";
            break;
        case Opcode::Fact:
            code = "Fact";
            break;
        case Opcode::Sal:
            code = "LSh";
            break;
        case Opcode::Sar:
            code = "RSh";
            break;
        case Opcode::BAnd:
            code = "BAnd";
            break;
        case Opcode::BOr:
            code = "BOr";
            break;
        case Opcode::Pct:
            code = "Pct";
            break;
        default:
            code = "Unknown";
            break;
        }
        result.append("   ").append(code).append("\n");
    }

    return result;
}
