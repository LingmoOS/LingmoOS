/*
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROCESS_H
#define PROCESS_H

#include <common/test.h>

#include <controller.h>

class QProcess;
class QTemporaryDir;

namespace Process
{
enum Action {
    Start,
    Stop,
    Kill,
    Crash,
};

class Modifier : public Test
{
    Q_OBJECT
public:
    Modifier(Action action);

private Q_SLOTS:
    void initTestCase();
    void testProcess();
    void cleanupTestCase();

private:
    Action m_action;
    static QProcess *s_process;
    static QTemporaryDir *s_tempDir;
};

Modifier *exec(Action action);

} // namespace Process

#endif /* PROCESS_H */
