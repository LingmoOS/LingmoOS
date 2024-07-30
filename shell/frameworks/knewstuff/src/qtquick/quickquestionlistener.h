/*
    This file is part of KNewStuffQuick.
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNSQ_QUICKQUESTIONLISTENER_H
#define KNSQ_QUICKQUESTIONLISTENER_H

#include "core/questionlistener.h"
#include <QPointer>

namespace KNewStuffQuick
{
class QuickQuestionListener : public KNSCore::QuestionListener
{
    Q_OBJECT
    Q_DISABLE_COPY(QuickQuestionListener)
public:
    static QuickQuestionListener *instance();
    ~QuickQuestionListener() override;

    Q_SLOT void askQuestion(KNSCore::Question *question) override;

    Q_SIGNAL void askListQuestion(QString title, QString question, QStringList list);
    Q_SIGNAL void askContinueCancelQuestion(QString title, QString question);
    Q_SIGNAL void askTextInputQuestion(QString title, QString question);
    Q_SIGNAL void askPasswordQuestion(QString title, QString question);
    Q_SIGNAL void askYesNoQuestion(QString title, QString question);

    Q_SLOT void passResponse(bool responseIsContinue, QString input);

    QuickQuestionListener() = default; // Only used by Q_GLOBAL_STATIC
private:
    QPointer<KNSCore::Question> m_question;
};
}

#endif // KNSQ_QUICKQUESTIONLISTENER_H
