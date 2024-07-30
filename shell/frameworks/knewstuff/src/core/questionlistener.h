/*
    This file is part of KNewStuffCore.
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNS3_QUESTIONLISTENER_H
#define KNS3_QUESTIONLISTENER_H

#include <QObject>

#include "knewstuffcore_export.h"

namespace KNSCore
{
class Question;
/**
 * @short Implementation-side handler class for questions sent from KNewStuffCore
 *
 * When implementing anything on top of KNewStuffCore, you will need to be able
 * to react to questions asked from inside the framework. This is done by creating
 * an instance of a QuestionListener, and reacting to any calls to the askQuestion
 * slot, which you must extend and implement. Two examples of this exist, in the
 * form of the KNS3::WidgetQuestionListener and KNewStuffQuick::QuickQuestionListener
 * and should you need to create your own, take inspiration from them.
 */
class KNEWSTUFFCORE_EXPORT QuestionListener : public QObject
{
    Q_OBJECT
public:
    explicit QuestionListener(QObject *parent = nullptr);
    ~QuestionListener() override;

    virtual void askQuestion(Question *question) = 0;
};
}

#endif // KNS3_QUESTIONLISTENER_H
