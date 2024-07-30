/*
    This file is part of KNewStuffCore.
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "questionmanager.h"

#include "question.h"

namespace KNSCore
{
class QuestionManagerHelper
{
public:
    QuestionManagerHelper() = default;
    ~QuestionManagerHelper()
    {
        delete q;
    }
    QuestionManager *q = nullptr;
};
Q_GLOBAL_STATIC(QuestionManagerHelper, s_kns3_questionManager)

QuestionManager *QuestionManager::instance()
{
    if (!s_kns3_questionManager()->q) {
        s_kns3_questionManager()->q = new QuestionManager;
    }
    return s_kns3_questionManager()->q;
}

QuestionManager::QuestionManager()
    : QObject()
{
    Q_UNUSED(d)
}

QuestionManager::~QuestionManager() = default;
}

#include "moc_questionmanager.cpp"
