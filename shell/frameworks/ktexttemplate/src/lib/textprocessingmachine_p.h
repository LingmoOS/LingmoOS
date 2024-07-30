/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2011 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef KTEXTTEMPLATE_TEXTPROCESSINGMACHINE_P_H
#define KTEXTTEMPLATE_TEXTPROCESSINGMACHINE_P_H

#include "statemachine_p.h"

namespace KTextTemplate
{

struct CharTransitionInterface {
    virtual bool characterTest(QString::const_iterator)
    {
        return false;
    }

    virtual void onTransition()
    {
    }

protected:
    virtual ~CharTransitionInterface()
    {
    }
};

class TextProcessingMachine : public StateMachine<CharTransitionInterface>
{
public:
    void processCharacter(QString::const_iterator character);

protected:
    bool doProcessCharacter(QString::const_iterator character, State<CharTransitionInterface> *state);
};

typedef CharTransitionInterface NullTest;

template<typename T, typename U>
struct AndTest {
    static bool characterTest(QString::const_iterator ch)
    {
        return T::characterTest(ch) && U::characterTest(ch);
    }
};

template<typename T, typename U>
struct OrTest {
    static bool characterTest(QString::const_iterator ch)
    {
        return T::characterTest(ch) || U::characterTest(ch);
    }
};

template<typename T>
struct Negate {
    static bool characterTest(QString::const_iterator ch)
    {
        return !T::characterTest(ch);
    }
};

struct IsSpace {
    static bool characterTest(QString::const_iterator ch)
    {
        return ch->isSpace();
    }
};

template<char c>
struct CharacterTest {
    static bool characterTest(QString::const_iterator ch)
    {
        return *ch == QLatin1Char(c);
    }
};
}

#endif
