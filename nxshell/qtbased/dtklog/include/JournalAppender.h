// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef JOURNALAPPENDER_H
#define JOURNALAPPENDER_H

#include "dlog_global.h"
#include "AbstractAppender.h"

DLOG_CORE_BEGIN_NAMESPACE

class LIBDLOG_SHARED_EXPORT JournalAppender : public AbstractAppender
{
public:
    JournalAppender()
        : AbstractAppender()
    {
    }
    ~JournalAppender() override = default;
    using JournalPriority = int;

protected:
    virtual void append(const QDateTime &time,
                        Logger::LogLevel level,
                        const char *file,
                        int line,
                        const char *func,
                        const QString &category,
                        const QString &msg) override;

private:
    bool m_ignoreEnvPattern;
};

DLOG_CORE_END_NAMESPACE

#endif // JOURNALAPPENDER_H
