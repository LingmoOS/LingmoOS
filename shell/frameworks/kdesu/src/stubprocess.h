/*
    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>

    SPDX-License-Identifier: GPL-2.0-only
*/

#ifndef KDESUSTUBPROCESS_H
#define KDESUSTUBPROCESS_H

#include "ptyprocess.h"

#include <kdesu/kdesu_export.h>

#include <QByteArray>
#include <QList>

namespace KDESu
{
namespace KDESuPrivate
{
class KCookie;
}
class StubProcessPrivate;

/** \class StubProcess stubprocess.h KDESu/StubProcess
 * Chat with kdesu_stub.
 *
 * StubProcess extends PtyProcess with functionality to chat with kdesu_stub.
 */

class KDESU_EXPORT StubProcess : public PtyProcess
{
public:
    /**
     * Different schedulers. SchedNormal is the normal Unix timesharing
     * scheduler, while SchedRealtime is a POSIX.1b realtime scheduler.
     */
    enum Scheduler {
        SchedNormal,
        SchedRealtime,
    };

    StubProcess();
    ~StubProcess() override;

    /**
     * Set the command.
     */
    void setCommand(const QByteArray &command);

    /**
     * Set the target user.
     */
    void setUser(const QByteArray &user);

    /**
     * Set to "X only mode": Sycoca is not built and kdeinit is not launched.
     */
    void setXOnly(bool xonly);

    /**
     * Set the priority of the process. The priority value must be between 0
     * and 100, 0 being the lowest priority. This value is mapped to the
     * scheduler and system dependent priority range of the OS.
     */
    void setPriority(int prio);

    /**
     * Set the scheduler type.
     */
    void setScheduler(int sched);

protected:
    void virtual_hook(int id, void *data) override;

    /**
     * Exchange all parameters with kdesu_stub.
     */
    int converseStub(int check);

    /**
     * This virtual function can be overloaded when special behavior is
     * desired. By default, it returns the value returned by KCookie.
     */
    virtual QByteArray display();

    /**
     * See display.
     */
    virtual QByteArray displayAuth();

    // KF6 TODO: move to StubProcessPrivate
    bool m_XOnly;
    int m_priority;
    int m_scheduler;
    QByteArray m_command;
    QByteArray m_user;
    KDESuPrivate::KCookie *m_cookie;

private:
    KDESU_NO_EXPORT void writeString(const QByteArray &str);

protected:
    KDESU_NO_EXPORT explicit StubProcess(StubProcessPrivate &dd);

private:
    Q_DECLARE_PRIVATE(StubProcess)
};

}

#endif // KDESUSTUBPROCESS_H
