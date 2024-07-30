/*
    This file is part of KNewStuffCore.
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNS3_QUESTION_H
#define KNS3_QUESTION_H

#include <QObject>

#include "knewstuffcore_export.h"

#include <memory>

namespace KNSCore
{
class Entry;
class QuestionPrivate;
/**
 * @short A way to ask a user a question from inside a GUI-less library (like KNewStuffCore)
 *
 * Rather than using a message box (which is a UI thing), when you want to ask your user
 * a question, create an instance of this class and use that instead. The consuming library
 * (in most cases KNewStuff or KNewStuffQuick) will listen to any question being asked,
 * and act appropriately (that is, KNewStuff will show a dialog with an appropriate dialog
 * box, and KNewStuffQuick will either request a question be asked if the developer is using
 * the plugin directly, or ask the question using an appropriate method for Qt Quick based
 * applications)
 *
 * The following is an example of a question asking the user to select an item from a list.
 *
 * @code
QStringList choices() << "foo" << "bar";
Question question(Question::SelectFromListQuestion);
question.setTitle("Pick your option");
question.setQuestion("Please select which option you would like");
question.setList(choices);
question.setEntry(entry);
if (question.ask() == Question::OKResponse) {
    QString theChoice = question.response();
}
@endcode
 */
class KNEWSTUFFCORE_EXPORT Question : public QObject
{
    Q_OBJECT
public:
    enum Response {
        InvalidResponse = 0,
        YesResponse = 1,
        NoResponse = 2,
        ContinueResponse = 3,
        CancelResponse = 4,
        OKResponse = YesResponse,
    };
    Q_ENUM(Response)

    enum QuestionType {
        YesNoQuestion = 0,
        ContinueCancelQuestion = 1,
        InputTextQuestion = 2,
        SelectFromListQuestion = 3,
        PasswordQuestion = 4,
    };
    Q_ENUM(QuestionType)

    explicit Question(QuestionType = YesNoQuestion, QObject *parent = nullptr);
    ~Question() override;

    Response ask();

    void setQuestionType(QuestionType newType = YesNoQuestion);
    QuestionType questionType() const;

    void setQuestion(const QString &newQuestion);
    QString question() const;
    void setTitle(const QString &newTitle);
    QString title() const;
    void setList(const QStringList &newList);
    QStringList list() const;
    void setEntry(const Entry &entry);
    Entry entry() const;

    /**
     * When the user makes a choice on a question, that is a response. This is the return value in ask().
     * @param response This will set the response, and mark the question as answered
     */
    void setResponse(Response response);
    /**
     * If the user has any way of inputting data to go along with the response above, consider this a part
     * of the response. As such, you can set, and later get, that response as well. This does NOT mark the
     * question as answered ( @see setResponse(Response) ).
     * @param response This sets the string response for the question
     */
    void setResponse(const QString &response);
    QString response() const;

private:
    const std::unique_ptr<QuestionPrivate> d;
};
}

#endif // KNS3_QUESTION_H
