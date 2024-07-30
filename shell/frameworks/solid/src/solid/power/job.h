/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_JOB_H
#define SOLID_JOB_H

#include <QObject>

#include "solid_export.h"

namespace Solid
{
class JobPrivate;
/**
 * This class represents an asynchronous job performed by Solid,
 * it is usually not used directly but instead it is inherited by some
 * other class, for example \See AcPluggedJob or \See StatesJob
 *
 * There are two ways of using this class, one is via exec() which will block
 * the thread until a result is fetched, the other is via connecting to the
 * signal result()
 *
 * Please, think twice before using exec(), it should be used only in either
 * unittest or cli apps.
 *
 * @note Job and its subclasses are meant to be used
 * in a fire-and-forget way. Jobs will delete themselves
 * when they finish using deleteLater()
 *
 * @note Even given their asynchronous nature, Jobs are still executed in the
 * main thread, so any blocking code executed in it will block the app calling it.
 */
class SOLID_EXPORT Job : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int error READ error NOTIFY result)
    Q_PROPERTY(QString errorText READ errorText NOTIFY result)

public:
    explicit Job(QObject *parent = nullptr);
    virtual ~Job();

    enum Error {
        /** Indicates there is no error */
        NoError = 0,
        /** Subclasses should define error codes starting at this value */
        UserDefinedError = 100,
    };
    Q_ENUM(Error)

    /**
     * Executes the job synchronously.
     *
     * This will start a nested QEventLoop internally. Nested event loop can be dangerous and
     * can have unintended side effects, you should avoid calling exec() whenever you can and use the
     * asynchronous interface of SolidJob instead.
     *
     * Should you indeed call this method, you need to make sure that all callers are reentrant,
     * so that events delivered by the inner event loop don't cause non-reentrant functions to be
     * called, which usually wreaks havoc.
     *
     * Note that the event loop started by this method does not process user input events, which means
     * your user interface will effectively be blocked. Other events like paint or network events are
     * still being processed. The advantage of not processing user input events is that the chance of
     * accidental reentrancy is greatly reduced. Still you should avoid calling this function.
     *
     * @return true if the job has been executed without error, false otherwise
     */
    bool exec();

    /**
     * Returns the error code, if there has been an error.
     *
     * Make sure to call this once result() has been emitted
     *
     * @return the error code for this job, 0 if no error.
     */
    int error() const;

    /**
     * Returns the error text if there has been an error.
     *
     * Only call if error is not 0.
     *
     * This is usually some extra data associated with the error,
     * such as a URL. Use errorString() to get a human-readable,
     * translated message.
     *
     * @return a string to help understand the error
     */
    QString errorText() const;

public Q_SLOTS:
    /**
     * Starts the job asynchronously.
     *
     * This method will schedule doStart() to be executed in the next
     * loop. This is done so this method returns as soon as possible.
     *
     * When the job is finished, result() is emitted.
     */
    void start();

private Q_SLOTS:
    /**
     * Implementation fo start() that will be executed in next loop
     *
     * This slot is always called in the next loop, triggered by start().
     *
     * When implementing this method it is important to remember that jobs
     * are not executed on a different thread (unless done that way), so any
     * blocking task has to be done in a different thread or process.
     */
    virtual void doStart() = 0;

protected:
    /**
     * Sets the error code.
     *
     * It should be called when an error
     * is encountered in the job, just before calling emitResult().
     *
     * You should define an enum of error codes,
     * with values starting at KJob::UserDefinedError, and use
     * those. For example,
     * @code
     * enum ExampleErrors{
     *   InvalidFoo = UserDefinedError,
     *   BarNotFound
     * };
     * @endcode
     *
     * @param errorCode the error code
     * @see emitResult()
     */
    void setError(int errorCode);

    /**
     * Sets the error text.
     *
     * It should be called when an error
     * is encountered in the job, just before calling emitResult().
     *
     * Provides extra information about the error that cannot be
     * determined directly from the error code.  For example, a
     * URL or filename.  This string is not normally translatable.
     *
     * @param errorText the error text
     * @see emitResult(), setError()
     */
    void setErrorText(const QString &errorText);

    /**
     * Utility function to emit the result signal, and suicide this job.
     * @note Deletes this job using deleteLater().
     * @see result()
     */
    void emitResult();

    JobPrivate *const d_ptr;
    SOLID_NO_EXPORT Job(JobPrivate &dd, QObject *parent);

private:
    Q_DECLARE_PRIVATE(Job)

Q_SIGNALS:
    void result(Solid::Job *job);
};
}

#endif // SOLID_JOB_H
