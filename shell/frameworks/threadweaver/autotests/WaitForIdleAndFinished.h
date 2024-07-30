/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WAITFORIDLEANDFINISHED_H
#define WAITFORIDLEANDFINISHED_H

namespace ThreadWeaver
{
class Queue;
}

/** @brief Helper class for tests to ensure that after the scope if left, the queue is idle and resumed.
 * Upon destruction, ensure the weaver is idle and suspended.
 */
class WaitForIdleAndFinished
{
public:
    explicit WaitForIdleAndFinished(ThreadWeaver::Queue *weaver);
    ~WaitForIdleAndFinished();
    WaitForIdleAndFinished(const WaitForIdleAndFinished &) = delete;
    WaitForIdleAndFinished &operator=(const WaitForIdleAndFinished &) = delete;
    void finish();

private:
    ThreadWeaver::Queue *weaver_;
};

/** @brief Create a WaitForIdleAndFinished, and suppress the IDE unused warning. */
/* clang-format off */
#define WAITFORIDLEANDFINISHED(queue) \
    const WaitForIdleAndFinished waitForIdleAndFinished##__LINE__(queue); \
    Q_UNUSED(waitForIdleAndFinished##__LINE__);
/* clang-format on */

#endif // WAITFORIDLEANDFINISHED_H
