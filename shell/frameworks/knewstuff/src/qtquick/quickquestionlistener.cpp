/*
    This file is part of KNewStuffQuick.
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "quickquestionlistener.h"

#include "core/question.h"

#include <QCoreApplication>
#include <QPointer>

using namespace KNewStuffQuick;

Q_GLOBAL_STATIC(QuickQuestionListener, s_quickQuestionListener)
QuickQuestionListener *QuickQuestionListener::instance()
{
    return s_quickQuestionListener;
}

QuickQuestionListener::~QuickQuestionListener()
{
    if (m_question) {
        m_question->setResponse(KNSCore::Question::CancelResponse);
    }
}

void QuickQuestionListener::askQuestion(KNSCore::Question *question)
{
    switch (question->questionType()) {
    case KNSCore::Question::SelectFromListQuestion:
        Q_EMIT askListQuestion(question->title(), question->question(), question->list());
        break;
    case KNSCore::Question::ContinueCancelQuestion:
        Q_EMIT askContinueCancelQuestion(question->title(), question->question());
        break;
    case KNSCore::Question::InputTextQuestion:
        Q_EMIT askTextInputQuestion(question->title(), question->question());
        break;
    case KNSCore::Question::PasswordQuestion:
        Q_EMIT askPasswordQuestion(question->title(), question->question());
        break;
    case KNSCore::Question::YesNoQuestion:
    default:
        Q_EMIT askYesNoQuestion(question->title(), question->question());
        break;
    }
    m_question = question;
}

void KNewStuffQuick::QuickQuestionListener::passResponse(bool responseIsContinue, QString input)
{
    if (m_question) {
        if (responseIsContinue) {
            m_question->setResponse(input);
            switch (m_question->questionType()) {
            case KNSCore::Question::ContinueCancelQuestion:
                m_question->setResponse(KNSCore::Question::ContinueResponse);
                break;
            case KNSCore::Question::YesNoQuestion:
                m_question->setResponse(KNSCore::Question::YesResponse);
                break;
            case KNSCore::Question::SelectFromListQuestion:
            case KNSCore::Question::InputTextQuestion:
            case KNSCore::Question::PasswordQuestion:
            default:
                m_question->setResponse(KNSCore::Question::OKResponse);
                break;
            }
        } else {
            switch (m_question->questionType()) {
            case KNSCore::Question::YesNoQuestion:
                m_question->setResponse(KNSCore::Question::NoResponse);
                break;
            case KNSCore::Question::SelectFromListQuestion:
            case KNSCore::Question::InputTextQuestion:
            case KNSCore::Question::PasswordQuestion:
            case KNSCore::Question::ContinueCancelQuestion:
            default:
                m_question->setResponse(KNSCore::Question::CancelResponse);
                break;
            }
        }
        m_question.clear();
    }
}

#include "moc_quickquestionlistener.cpp"
