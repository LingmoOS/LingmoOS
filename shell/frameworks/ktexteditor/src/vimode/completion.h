/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_COMPLETION_H
#define KATEVI_COMPLETION_H

#include <QList>
#include <QString>

namespace KateVi
{
class Completion
{
public:
    enum CompletionType { PlainText, FunctionWithoutArgs, FunctionWithArgs };

    explicit Completion(const QString &completedText, bool removeTail, CompletionType completionType);
    QString completedText() const;
    bool removeTail() const;
    CompletionType completionType() const;

private:
    QString m_completedText;
    bool m_removeTail;
    CompletionType m_completionType;
};

typedef QList<Completion> CompletionList;
}

#endif // KATEVI_COMPLETION_H
