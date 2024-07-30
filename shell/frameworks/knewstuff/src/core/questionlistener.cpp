/*
    This file is part of KNewStuffCore.
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "questionlistener.h"
#include "question.h"
#include "questionmanager.h"

using namespace KNSCore;

QuestionListener::QuestionListener(QObject *parent)
    : QObject(parent)
{
    connect(QuestionManager::instance(), &QuestionManager::askQuestion, this, &QuestionListener::askQuestion);
}

QuestionListener::~QuestionListener() = default;

#include "moc_questionlistener.cpp"
