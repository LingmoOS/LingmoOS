/*
 *  SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "kscreen_export.h"

#include <QLoggingCategory>
#include <QObject>

namespace KScreen
{
void log(const QString &msg);

/** KScreen-internal file logging.
 *
 * The purpose of this class is to allow better debugging of kscreen. QDebug falls short here, since
 * we need to debug the concert of kscreen components from different processes.
 *
 * KScreen::Log manages access to kscreen's log file.
 *
 * The following environment variables are considered:
 * - disable logging by setting
 * KSCREEN_LOGGING=false
 * - set the log file to a custom path, the default is in ~/.local/share/kscreen/kscreen.log
 *
 * Please do not translate messages written to the logs, it's developer information and should be
 * english, independent from the user's locale preferences.
 *
 * @code
 *
 * Log::instance()->setContext("resume");
 * Log::log("Applying detected output configuration.");
 *
 * @endcode
 *
 * @since 5.8
 */
class KSCREEN_EXPORT Log
{
public:
    virtual ~Log();

    static Log *instance();

    /** Log a message to a file
     *
     * Call this static method to add a new line to the log.
     *
     * @arg msg The log message to write.
     */
    static void log(const QString &msg, const QString &category = QString());

    /** Context for the logs.
     *
     * The context can be used to indicate what is going on overall, it is used to be able
     * to group log entries into subsequential operations. For example the context can be
     * "handling resume", which is then added to the log messages.
     *
     * @arg msg The log message to write to the file.
     *
     * @see ontext()
     */
    QString context() const;

    /** Set the context for the logs.
     *
     * @see context()
     */
    void setContext(const QString &context);

    /** Logging to file is enabled by environmental var, is it?
     *
     * @arg msg The log message to write to the file.
     * @return Whether logging is enabled.
     */
    bool enabled() const;

    /** Path to the log file
     *
     * This is usually ~/.local/share/kscreen/kscreen.log, but can be changed by setting
     * KSCREEN_LOGFILE in the environment.
     *
     * @return The path to the log file.
     */
    QString logFile() const;

private:
    explicit Log();
    class Private;
    Private *const d;

    static Log *sInstance;
    explicit Log(Private *dd);
};

} // KSCreen namespace
