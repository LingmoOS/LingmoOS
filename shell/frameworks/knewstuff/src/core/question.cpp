/*
    This file is part of KNewStuffCore.
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "question.h"

#include "entry.h"
#include "questionmanager.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <optional>

using namespace KNSCore;

class KNSCore::QuestionPrivate
{
public:
    QuestionPrivate()
        : questionType(Question::YesNoQuestion)
        , response()
    {
    }
    QString question;
    QString title;
    QStringList list;
    Entry entry;

    QEventLoop loop;
    Question::QuestionType questionType;
    std::optional<Question::Response> response;
    QString textResponse;
};

Question::Question(QuestionType questionType, QObject *parent)
    : QObject(parent)
    , d(new QuestionPrivate)
{
    d->questionType = questionType;
}

Question::~Question() = default;

Question::Response Question::ask()
{
    Q_EMIT QuestionManager::instance()->askQuestion(this);
    if (!d->response.has_value()) {
        d->loop.exec(); // Wait for the setResponse method to quit the event loop
    }
    return *d->response;
}

Question::QuestionType Question::questionType() const
{
    return d->questionType;
}

void Question::setQuestionType(Question::QuestionType newType)
{
    d->questionType = newType;
}

void Question::setQuestion(const QString &newQuestion)
{
    d->question = newQuestion;
}

QString Question::question() const
{
    return d->question;
}

void Question::setTitle(const QString &newTitle)
{
    d->title = newTitle;
}

QString Question::title() const
{
    return d->title;
}

void Question::setList(const QStringList &newList)
{
    d->list = newList;
}

QStringList Question::list() const
{
    return d->list;
}

void Question::setResponse(Response response)
{
    d->response = response;
    d->loop.quit();
}

void Question::setResponse(const QString &response)
{
    d->textResponse = response;
}

QString Question::response() const
{
    return d->textResponse;
}

void Question::setEntry(const Entry &entry)
{
    d->entry = entry;
}

Entry Question::entry() const
{
    return d->entry;
}

#include "moc_question.cpp"
