/* This file is part of the KDE libraries
 * SPDX-FileCopyrightText: 2009 Dario Freddi <drf at kde.org>
 * SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KIDLETIME_H
#define KIDLETIME_H

#include <QHash>
#include <QObject>
#include <kidletime_export.h>
#include <memory>

#if __has_include(<chrono>)
#include <chrono>
#endif

class KIdleTimePrivate;

/**
 * @class KIdleTime kidletime.h KIdleTime
 *
 * KIdleTime is a singleton reporting information on idle time. It is useful not
 * only for finding out about the current idle time of the PC, but also for getting
 * notified upon idle time events, such as custom timeouts, or user activity.
 *
 * @note All the intervals and times in this library are in milliseconds, unless
 *       specified otherwise
 *
 * @author Dario Freddi
 *
 * @since 4.4
 */
class KIDLETIME_EXPORT KIdleTime : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KIdleTime)
    Q_DISABLE_COPY(KIdleTime)

public:
    /**
     * Returns the singleton instance. Use this method to access KIdleTime
     *
     * @returns the instance of KIdleTime
     */
    static KIdleTime *instance();

    /**
     * The destructor
     */
    ~KIdleTime() override;

    /**
     * Retrieves the idle time of the system, in milliseconds
     *
     * @returns the idle time of the system
     */
    int idleTime() const;

    /**
     * Returns the list of timeout identifiers associated with their duration, in milliseconds,
     * the library is currently listening to.
     *
     * @see addIdleTimeout
     * @see removeIdleTimeout
     * @see timeoutReached
     */
    QHash<int, int> idleTimeouts() const;

    /**
     * Attempts to simulate user activity. This implies that after calling this
     * method, the idle time of the system will become 0 and eventually \link resumingFromIdle \endlink
     * will be triggered
     *
     * @see resumingFromIdle
     */
    void simulateUserActivity();

public Q_SLOTS:
    /**
     * Adds a new timeout to catch. When calling this method, after the system will be idle for
     * \c msec milliseconds, the signal \c timeoutReached will be triggered. Please note that until you will
     * call \c removeIdleTimeout or \c removeAllIdleTimeouts, the signal will be triggered every
     * time the system will be idle for \c msec milliseconds. This function also returns an unique
     * token for the timeout just added to allow easier identification.
     *
     * @param msec the time, in milliseconds, after which the signal will be triggered
     *
     * @returns an unique identifier for the timeout being added, that will be streamed by timeoutReached
     *
     * @see removeIdleTimeout
     * @see removeAllIdleTimeouts
     * @see timeoutReached
     *
     */
    int addIdleTimeout(int msec);

#if __has_include(<chrono>)
    /**
     * Convenience overload supporting C++ chrono types. May also be used with chrono literals.
     * @since 5.83
     */
    int addIdleTimeout(std::chrono::milliseconds msec)
    {
        return addIdleTimeout(int(msec.count()));
    }
#endif

    /**
     * Stops catching the idle timeout identified by the token \c identifier,
     * if it was registered earlier with addIdleTimeout.
     * Otherwise does nothing.
     *
     * @param identifier the token returned from addIdleTimeout of the timeout you want to stop listening to
     */
    void removeIdleTimeout(int identifier);

    /**
     * Stops catching every set timeout (if any). This means that after calling this method, the signal
     * \link timeoutReached \endlink won't be called again until you will add another timeout
     *
     * @see timeoutReached
     * @see addIdleTimeout
     */
    void removeAllIdleTimeouts();

    /**
     * Catches the next resume from idle event. This means that whenever user activity will be registered, or
     * \link simulateUserActivity \endlink is called, the signal \link resumingFromIdle \endlink will be triggered.
     * <p>
     * Please note that this method will trigger the signal just for the very first resume event after the call:
     * this means you explicitly have to request to track every single resume event you are interested in.
     *
     * @note This behavior is due to the fact that a resume event happens whenever the user sends an input to the
     *       system. This would lead to a massive amount of signals being delivered when the PC is being used.
     *       Moreover, you are usually interested in catching just significant resume events, such as the ones after
     *       a significant period of inactivity. For tracking user input, you can use the more efficient methods provided
     *       by Qt. The purpose of this library is just monitoring the activity of the user.
     *
     * @see resumingFromIdle
     * @see simulateUserActivity
     *
     */
    void catchNextResumeEvent();

    /**
     * Stops listening for resume event. This function serves for canceling \c catchNextResumeEvent, as it
     * will have effect just when \c catchNextResumeEvent has been called and \c resumingFromIdle not
     * yet triggered
     *
     * @see resumingFromIdle
     * @see catchNextResumeEvent
     *
     */
    void stopCatchingResumeEvent();

Q_SIGNALS:
    /**
     * Triggered, if KIdleTime is catching resume events, when the system resumes from an idle state. This means
     * that either \link simulateUserActivity \endlink was called or the user sent an input to the system.
     *
     * @see catchNextResumeEvent
     */
    void resumingFromIdle();

    /**
     * Triggered when the system has been idle for x milliseconds, identified by the previously set
     * timeout.
     *
     * This signal is triggered whenever each timeout previously registered with addIdleTimeout(int)
     * is reached. It is guaranteed that \p msec will exactly correspond to the identified timeout.
     *
     * @param identifier the identifier of the timeout the system has reached
     * @param msec the time, in milliseconds, the system has been idle for
     *
     * @see addIdleTimeout
     * @see removeIdleTimeout
     */
    void timeoutReached(int identifier, int msec); // clazy:exclude=overloaded-signal

private:
    KIDLETIME_NO_EXPORT KIdleTime();

    std::unique_ptr<KIdleTimePrivate> const d_ptr;
};

#endif /* KIDLETIME_H */
