/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef JOBLOGGINGWEAVER_H
#define JOBLOGGINGWEAVER_H

#include "JobLoggingDecorator.h"
#include "src/jobpointer.h"
#include "src/weaver.h"

class JobLoggingWeaver : public ThreadWeaver::Weaver
{
    Q_OBJECT
public:
    explicit JobLoggingWeaver(QObject *parent = nullptr);
    void enqueue(const QList<ThreadWeaver::JobPointer> &jobs) override;

private:
    JobLoggingDecoratorCollector collector_;
};

#endif // JOBLOGGINGWEAVER_H
