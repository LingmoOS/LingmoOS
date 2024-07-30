/* vi: ts=8 sts=4 sw=4

    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>
*/

#ifndef __Handler_h_included__
#define __Handler_h_included__

#include <sys/types.h>

#include "secure.h"
#include <QByteArray>

/**
 * A ConnectionHandler handles a client. It is called from the main program
 * loop whenever there is data to read from a corresponding socket.
 * It keeps reading data until a newline is read. Then, a command is parsed
 * and executed.
 */

class ConnectionHandler : public SocketSecurity
{
public:
    ConnectionHandler(int fd);
    ~ConnectionHandler();

    ConnectionHandler(const ConnectionHandler &) = delete;
    ConnectionHandler &operator=(const ConnectionHandler &) = delete;

    /** Handle incoming data. */
    int handle();

    /* Send back exit code. */
    void sendExitCode();

private:
    enum Results {
        Res_OK,
        Res_NO,
    };

    int doCommand(QByteArray buf);
    void respond(int ok, const QByteArray &s = QByteArray());
    QByteArray makeKey(int namspace, const QByteArray &s1, const QByteArray &s2 = QByteArray(), const QByteArray &s3 = QByteArray()) const;

    int m_Fd, m_Timeout;
    int m_Priority, m_Scheduler;
    QByteArray m_Buf, m_Pass, m_Host;

public:
    int m_exitCode;
    bool m_hasExitCode;
    bool m_needExitCode;
    pid_t m_pid;
};

#endif
