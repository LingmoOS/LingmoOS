/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "completion.h"
#include "katepartdebug.h"

using namespace KateVi;

Completion::Completion(const QString &completedText, bool removeTail, CompletionType completionType)
    : m_completedText(completedText)
    , m_removeTail(removeTail)
    , m_completionType(completionType)
{
    if (m_completionType == FunctionWithArgs || m_completionType == FunctionWithoutArgs) {
        qCDebug(LOG_KTE) << "Completing a function while not removing tail currently unsupported; will remove tail instead";
        m_removeTail = true;
    }
}

QString Completion::completedText() const
{
    return m_completedText;
}

bool Completion::removeTail() const
{
    return m_removeTail;
}

Completion::CompletionType Completion::completionType() const
{
    return m_completionType;
}
