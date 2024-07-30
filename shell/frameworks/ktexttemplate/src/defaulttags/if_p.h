/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2016 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef IF_P_H
#define IF_P_H

#include <QList>
#include <QStringList>

#include "../lib/exception.h"
#include "filterexpression.h"
#include "node.h"
#include "util.h"

namespace KTextTemplate
{
class Parser;
}

class IfToken;

class IfParser
{
public:
    IfParser(KTextTemplate::Parser *parser, const QStringList &args);

    QSharedPointer<IfToken> parse();

    QSharedPointer<IfToken> expression(int rbp = {});

private:
    QSharedPointer<IfToken> createNode(const QString &content) const;

    QSharedPointer<IfToken> consumeToken();

private:
    KTextTemplate::Parser *mParser;
    QList<QSharedPointer<IfToken>> mParseNodes;
    int mPos = 0;
    QSharedPointer<IfToken> mCurrentToken;
};

static bool contains(const QVariant &needle, const QVariant &var)
{
    if (KTextTemplate::isSafeString(var)) {
        return KTextTemplate::getSafeString(var).get().contains(KTextTemplate::getSafeString(needle));
    } else if (var.canConvert<QVariantList>()) {
        auto container = var.value<QVariantList>();
        if (KTextTemplate::isSafeString(needle)) {
            return container.contains(KTextTemplate::getSafeString(needle).get());
        }
        return container.contains(needle);
    }
    if (var.canConvert<QVariantHash>()) {
        auto container = var.value<QVariantHash>();
        if (KTextTemplate::isSafeString(needle)) {
            return container.contains(KTextTemplate::getSafeString(needle).get());
        }
        return container.contains(needle.toString());
    }
    return false;
}

class IfToken
{
public:
    enum OpCode { Invalid, Literal, OrCode, AndCode, NotCode, InCode, NotInCode, EqCode, NeqCode, GtCode, GteCode, LtCode, LteCode, Sentinal };

    static QSharedPointer<IfToken> makeSentinal()
    {
        return QSharedPointer<IfToken>::create(0, QString(), Sentinal);
    }

    using ArgsType = std::pair<QSharedPointer<IfToken>, QSharedPointer<IfToken>>;

    IfToken(int lbp, const QString &tokenName, OpCode opCode)
        : mArgs()
    {
        mLbp = lbp;
        mTokenName = tokenName;
        mOpCode = opCode;
    }

    IfToken(const KTextTemplate::FilterExpression &fe)
        : mFe(fe)
    {
        mLbp = 0;
        mTokenName = QStringLiteral("literal");
        mOpCode = Literal;
    }

    void nud(IfParser *parser);
    void led(QSharedPointer<IfToken> left, IfParser *parser);

    QVariant evaluate(KTextTemplate::Context *c) const;

    int lbp() const
    {
        return mLbp;
    }

    int mLbp;
    QString mTokenName;

    KTextTemplate::FilterExpression mFe;
    ArgsType mArgs;

    OpCode mOpCode;

    QString tokenName() const
    {
        return mTokenName;
    }
};

void IfToken::nud(IfParser *parser)
{
    switch (mOpCode) {
    default:
        break;
    case IfToken::Literal:
        return;
    case IfToken::NotCode:
        mArgs.first = parser->expression(mLbp);
        mArgs.second.clear();
        return;
    }

    throw KTextTemplate::Exception(KTextTemplate::TagSyntaxError, QStringLiteral("Not expecting '%1' in this position in if tag.").arg(mTokenName));
}

void IfToken::led(QSharedPointer<IfToken> left, IfParser *parser)
{
    switch (mOpCode) {
    default:
        break;
    case IfToken::OrCode:
    case IfToken::AndCode:
    case IfToken::InCode:
    case IfToken::NotInCode:
    case IfToken::EqCode:
    case IfToken::NeqCode:
    case IfToken::GtCode:
    case IfToken::GteCode:
    case IfToken::LtCode:
    case IfToken::LteCode:
        mArgs.first = left;
        mArgs.second = parser->expression(mLbp);
        return;
    }

    throw KTextTemplate::Exception(KTextTemplate::TagSyntaxError, QStringLiteral("Not expecting '%1' as infix operator in if tag.").arg(mTokenName));
}

IfParser::IfParser(KTextTemplate::Parser *parser, const QStringList &args)
    : mParser(parser)
{
    mParseNodes.reserve(args.size());
    if (args.size() > 1) {
        auto it = args.begin() + 1, end = args.end() - 1;
        for (; it != end; ++it) {
            if (*it == QLatin1String("not") && *std::next(it) == QLatin1String("in")) {
                mParseNodes.push_back(createNode(QStringLiteral("not in")));
                ++it;
                if (it == end) {
                    break;
                }
            } else {
                mParseNodes.push_back(createNode(*it));
            }
        }
        mParseNodes.push_back(createNode(*it));
    }
    mPos = 0;
    mCurrentToken = consumeToken();
}

QSharedPointer<IfToken> IfParser::consumeToken()
{
    if (mPos >= mParseNodes.size()) {
        return IfToken::makeSentinal();
    }
    auto t = mParseNodes[mPos];
    ++mPos;
    return t;
}

QSharedPointer<IfToken> IfParser::expression(int rbp)
{
    auto t = mCurrentToken;
    mCurrentToken = consumeToken();
    t->nud(this);
    auto left = t;
    while (rbp < mCurrentToken->lbp()) {
        t = mCurrentToken;
        mCurrentToken = consumeToken();
        t->led(left, this);
        left = t;
    }
    return left;
}

QSharedPointer<IfToken> IfParser::parse()
{
    auto r = expression();

    if (mCurrentToken->mOpCode != IfToken::Sentinal) {
        throw KTextTemplate::Exception(KTextTemplate::TagSyntaxError, QStringLiteral("Unused '%1' at end of if expression.").arg(mCurrentToken->tokenName()));
    }

    return r;
}

QSharedPointer<IfToken> IfParser::createNode(const QString &content) const
{
    if (content == QLatin1String("or")) {
        return QSharedPointer<IfToken>::create(6, content, IfToken::OrCode);
    }
    if (content == QLatin1String("and")) {
        return QSharedPointer<IfToken>::create(7, content, IfToken::AndCode);
    }
    if (content == QLatin1String("in")) {
        return QSharedPointer<IfToken>::create(9, content, IfToken::InCode);
    }
    if (content == QLatin1String("not in")) {
        return QSharedPointer<IfToken>::create(9, content, IfToken::NotInCode);
    }
    if (content == QLatin1String("==")) {
        return QSharedPointer<IfToken>::create(10, content, IfToken::EqCode);
    }
    if (content == QLatin1String("!=")) {
        return QSharedPointer<IfToken>::create(10, content, IfToken::NeqCode);
    }
    if (content == QLatin1String(">")) {
        return QSharedPointer<IfToken>::create(10, content, IfToken::GtCode);
    }
    if (content == QLatin1String(">=")) {
        return QSharedPointer<IfToken>::create(10, content, IfToken::GteCode);
    }
    if (content == QLatin1String("<")) {
        return QSharedPointer<IfToken>::create(10, content, IfToken::LtCode);
    }
    if (content == QLatin1String("<=")) {
        return QSharedPointer<IfToken>::create(10, content, IfToken::LteCode);
    }
    if (content == QStringLiteral("not")) {
        return QSharedPointer<IfToken>::create(8, content, IfToken::NotCode);
    }
    return QSharedPointer<IfToken>::create(KTextTemplate::FilterExpression(content, mParser));
}

QVariant IfToken::evaluate(KTextTemplate::Context *c) const
{
    try {
        switch (mOpCode) {
        case Literal:
            return mFe.resolve(c);
        case OrCode:
            return KTextTemplate::variantIsTrue(mArgs.first->evaluate(c)) || KTextTemplate::variantIsTrue(mArgs.second->evaluate(c));
        case AndCode:
            return KTextTemplate::variantIsTrue(mArgs.first->evaluate(c)) && KTextTemplate::variantIsTrue(mArgs.second->evaluate(c));
        case NotCode:
            return !KTextTemplate::variantIsTrue(mArgs.first->evaluate(c));
        case InCode:
            return contains(mArgs.first->evaluate(c), mArgs.second->evaluate(c));
        case NotInCode:
            return !contains(mArgs.first->evaluate(c), mArgs.second->evaluate(c));
        case EqCode:
            return KTextTemplate::equals(mArgs.first->evaluate(c), mArgs.second->evaluate(c));
        case NeqCode:
            return !KTextTemplate::equals(mArgs.first->evaluate(c), mArgs.second->evaluate(c));
        case GtCode:
        case GteCode:
        case LtCode:
        case LteCode: {
            auto f = mArgs.first->evaluate(c);
            auto s = mArgs.second->evaluate(c);

            auto comp = QVariant::compare(f, s);
            if (comp != QPartialOrdering::Unordered) {
                if (mOpCode == GtCode && comp == QPartialOrdering::Greater)
                    return true;
                if (mOpCode == GteCode && comp != QPartialOrdering::Less)
                    return true;
                if (mOpCode == LtCode && comp == QPartialOrdering::Less)
                    return true;
                if (mOpCode == LteCode && comp != QPartialOrdering::Greater)
                    return true;
            }
            return false;
        }
        default:
            Q_ASSERT(!"Invalid OpCode");
            return QVariant();
        }
    } catch (const KTextTemplate::Exception &) {
        return false;
    }
}

#endif
