/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010, 2011 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef KTEXTTEMPLATE_LEXER_P_H
#define KTEXTTEMPLATE_LEXER_P_H

#include "textprocessingmachine_p.h"
#include "token.h"

#include <QList>

namespace KTextTemplate
{

class Lexer
{
public:
    explicit Lexer(const QString &templateString);
    ~Lexer();

    enum TrimType { NoSmartTrim, SmartTrim };

    QList<Token> tokenize(TrimType type = NoSmartTrim);

    void markStartSyntax();
    void markEndSyntax();
    void markNewline();
    void clearMarkers();
    void finalizeToken();
    void finalizeTokenWithTrimmedWhitespace();

private:
    void reset();
    void finalizeToken(int nextPosition, bool processSyntax);

private:
    QString m_templateString;

    QList<Token> m_tokenList;
    int m_lineCount;
    int m_upto;
    int m_processedUpto;
    int m_startSyntaxPosition;
    int m_endSyntaxPosition;
    int m_newlinePosition;
};

struct NullLexerAction {
    static void doAction(Lexer *)
    {
    }
};

template<typename TType, typename Test, typename Action1 = NullLexerAction, typename Action2 = NullLexerAction>
class LexerObject : public TType
{
public:
    LexerObject(Lexer *lexer, State<typename TType::Type> *sourceState = {})
        : TType(sourceState)
        , m_lexer(lexer)
    {
    }

    bool characterTest(QString::const_iterator character)
    {
        return Test::characterTest(character);
    }

    void onTransition()
    {
        Action1::doAction(m_lexer);
    }

    void onEntry()
    {
        Action1::doAction(m_lexer);
    }

    void onExit()
    {
        Action2::doAction(m_lexer);
    }

private:
    Lexer *const m_lexer;
};

struct TokenFinalizer {
    static void doAction(Lexer *lexer)
    {
        lexer->finalizeToken();
    }
};

struct TokenFinalizerWithTrimming {
    static void doAction(Lexer *lexer)
    {
        lexer->finalizeTokenWithTrimmedWhitespace();
    }
};

struct TokenFinalizerWithTrimmingAndNewline {
    static void doAction(Lexer *lexer)
    {
        lexer->finalizeTokenWithTrimmedWhitespace();
        lexer->markNewline();
    }
};

struct MarkStartSyntax {
    static void doAction(Lexer *lexer)
    {
        lexer->markStartSyntax();
    }
};

struct FinalizeAndMarkStartSyntax {
    static void doAction(Lexer *lexer)
    {
        lexer->finalizeToken();
        lexer->markStartSyntax();
    }
};

struct MarksClearer {
    static void doAction(Lexer *lexer)
    {
        lexer->clearMarkers();
    }
};

struct MarkEndSyntax {
    static void doAction(Lexer *lexer)
    {
        lexer->markEndSyntax();
    }
};

struct MarkNewline {
    static void doAction(Lexer *lexer)
    {
        lexer->markNewline();
    }
};

template<char c, typename Action = NullLexerAction>
class CharacterTransition : public LexerObject<State<CharTransitionInterface>::Transition, CharacterTest<c>, Action>
{
public:
    CharacterTransition(Lexer *lexer, State<CharTransitionInterface> *sourceState = {})
        : LexerObject<State<CharTransitionInterface>::Transition, CharacterTest<c>, Action>(lexer, sourceState)
    {
    }
};

template<char c, typename Action = NullLexerAction>
class NegateCharacterTransition : public LexerObject<State<CharTransitionInterface>::Transition, Negate<CharacterTest<c>>, Action>
{
public:
    NegateCharacterTransition(Lexer *lexer, State<CharTransitionInterface> *sourceState = {})
        : LexerObject<State<CharTransitionInterface>::Transition, Negate<CharacterTest<c>>, Action>(lexer, sourceState)
    {
    }
};
}

#endif
