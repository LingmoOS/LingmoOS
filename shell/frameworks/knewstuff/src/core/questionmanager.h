/*
    This file is part of KNewStuffCore.
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNS3_QUESTIONMANAGER_H
#define KNS3_QUESTIONMANAGER_H

#include <QObject>

#include "knewstuffcore_export.h"
namespace KNSCore
{
class Question;
class QuestionManagerPrivate;
/**
 * @short The central class handling Question redirection
 *
 * This class is used to ensure that KNSCore::Question instances get redirected
 * to the appropriate KNSCore::QuestionListener instances. It is a very dumb class
 * which only ensures the listeners have somewhere to listen to, and the
 * questions have somewhere to ask to be asked.
 */
class KNEWSTUFFCORE_EXPORT QuestionManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QuestionManager)
public:
    static QuestionManager *instance();
    ~QuestionManager() override;

Q_SIGNALS:
    void askQuestion(KNSCore::Question *question);

private:
    QuestionManager();
    const void *d; // Future BIC
};
}

#endif // KNS3_QUESTIONMANAGER_H
