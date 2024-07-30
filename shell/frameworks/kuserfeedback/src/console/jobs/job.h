/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_JOB_H
#define KUSERFEEDBACK_CONSOLE_JOB_H

#include <QObject>

namespace KUserFeedback {
namespace Console {

class Job : public QObject
{
    Q_OBJECT
public:
    explicit Job(QObject *parent = nullptr);
    ~Job() override;

Q_SIGNALS:
    void info(const QString &msg);
    void error(const QString &msg);
    void finished();

protected:
    void emitError(const QString &msg);
    void emitFinished();
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_JOB_H
