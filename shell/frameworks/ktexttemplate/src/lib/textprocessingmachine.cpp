/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2011 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "textprocessingmachine_p.h"

using namespace KTextTemplate;

void TextProcessingMachine::processCharacter(QString::const_iterator character)
{
    auto s = currentState();
    while (s) {
        if (!doProcessCharacter(character, s)) {
            s = s->parent();
        } else {
            return;
        }
    }
}

bool TextProcessingMachine::doProcessCharacter(QString::const_iterator character, State<CharTransitionInterface> *state)
{
    const auto transitions = state->transitions();
    auto it = transitions.constBegin();
    const auto end = transitions.constEnd();
    for (; it != end; ++it) {
        if ((*it)->characterTest(character)) {
            executeTransition(state, *it);
            return true;
        }
    }
    return false;
}
