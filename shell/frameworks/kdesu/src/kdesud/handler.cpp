/*
    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>

    handler.cpp: A connection handler for kdesud.
*/

#include "handler.h"

#include <ksud_debug.h>

#include <assert.h>
#include <cerrno>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>

#include <sshprocess.h>
#include <suprocess.h>

#include "lexer.h"
#include "repo.h"

using namespace KDESu;

#define BUF_SIZE 1024

// Global repository
extern Repository *repo;
void kdesud_cleanup();

ConnectionHandler::ConnectionHandler(int fd)
    : SocketSecurity(fd)
    , m_exitCode(0)
    , m_hasExitCode(false)
    , m_needExitCode(false)
    , m_pid(0)
{
    m_Fd = fd;
    m_Priority = 50;
    m_Scheduler = SuProcess::SchedNormal;
}

ConnectionHandler::~ConnectionHandler()
{
    m_Buf.fill('x');
    m_Pass.fill('x');
    close(m_Fd);
}

/*
 * Handle a connection: make sure we don't block
 */

int ConnectionHandler::handle()
{
    int ret;
    int nbytes;

    m_Buf.reserve(BUF_SIZE);
    nbytes = recv(m_Fd, m_Buf.data() + m_Buf.size(), BUF_SIZE - 1 - m_Buf.size(), 0);

    if (nbytes < 0) {
        if (errno == EINTR) {
            return 0;
        }
        // read error
        return -1;
    } else if (nbytes == 0) {
        // eof
        return -1;
    }

    m_Buf.resize(m_Buf.size() + nbytes);
    if (m_Buf.size() == BUF_SIZE - 1) {
        qCWarning(KSUD_LOG) << "line too long";
        return -1;
    }

    // Do we have a complete command yet?
    int n;
    while ((n = m_Buf.indexOf('\n')) != -1) {
        n++;
        QByteArray newbuf = QByteArray(m_Buf.data(), n); // ensure new detached buffer for simplicity
        int nsize = m_Buf.size() - n;
        ::memmove(m_Buf.data(), m_Buf.data() + n, nsize);
        ::memset(m_Buf.data() + nsize, 'x', n);
        m_Buf.resize(nsize);
        ret = doCommand(newbuf);
        if (newbuf.isDetached()) { // otherwise somebody else will clear it
            newbuf.fill('x');
        }
        if (ret < 0) {
            return ret;
        }
    }

    return 0;
}

QByteArray ConnectionHandler::makeKey(int _namespace, const QByteArray &s1, const QByteArray &s2, const QByteArray &s3) const
{
    QByteArray res;
    res.setNum(_namespace);
    res += '*';
    res += s1 + '*' + s2 + '*' + s3;
    return res;
}

void ConnectionHandler::sendExitCode()
{
    if (!m_needExitCode) {
        return;
    }
    QByteArray buf;
    buf.setNum(m_exitCode);
    buf.prepend("OK ");
    buf.append("\n");

    send(m_Fd, buf.data(), buf.length(), 0);
}

void ConnectionHandler::respond(int ok, const QByteArray &s)
{
    QByteArray buf;

    switch (ok) {
    case Res_OK:
        buf = "OK";
        break;
    case Res_NO:
    default:
        buf = "NO";
        break;
    }

    if (!s.isEmpty()) {
        buf += ' ';
        buf += s;
    }

    buf += '\n';

    send(m_Fd, buf.data(), buf.length(), 0);
}

/*
 * Parse and do one command. On a parse error, return -1. This will
 * close the socket in the main accept loop.
 */

int ConnectionHandler::doCommand(QByteArray buf)
{
    if ((uid_t)peerUid() != getuid()) {
        qCWarning(KSUD_LOG) << "Peer uid not equal to me\n";
        qCWarning(KSUD_LOG) << "Peer: " << peerUid() << " Me: " << getuid();
        return -1;
    }

    QByteArray key;
    QByteArray command;
    QByteArray pass;
    QByteArray name;
    QByteArray user;
    QByteArray value;
    QByteArray env_check;
    Data_entry data;

    Lexer *l = new Lexer(buf);
    int tok = l->lex();
    switch (tok) {
    case Lexer::Tok_pass: // "PASS password:string timeout:int\n"
        tok = l->lex();
        if (tok != Lexer::Tok_str) {
            goto parse_error;
        }
        m_Pass.fill('x');
        m_Pass = l->lval();
        tok = l->lex();
        if (tok != Lexer::Tok_num) {
            goto parse_error;
        }
        m_Timeout = l->lval().toInt();
        if (l->lex() != '\n') {
            goto parse_error;
        }
        if (m_Pass.isNull()) {
            m_Pass = "";
        }
        qCDebug(KSUD_LOG) << "Password set!\n";
        respond(Res_OK);
        break;

    case Lexer::Tok_host: // "HOST host:string\n"
        tok = l->lex();
        if (tok != Lexer::Tok_str) {
            goto parse_error;
        }
        m_Host = l->lval();
        if (l->lex() != '\n') {
            goto parse_error;
        }
        qCDebug(KSUD_LOG) << "Host set to " << m_Host;
        respond(Res_OK);
        break;

    case Lexer::Tok_prio: // "PRIO priority:int\n"
        tok = l->lex();
        if (tok != Lexer::Tok_num) {
            goto parse_error;
        }
        m_Priority = l->lval().toInt();
        if (l->lex() != '\n') {
            goto parse_error;
        }
        qCDebug(KSUD_LOG) << "priority set to " << m_Priority;
        respond(Res_OK);
        break;

    case Lexer::Tok_sched: // "SCHD scheduler:int\n"
        tok = l->lex();
        if (tok != Lexer::Tok_num) {
            goto parse_error;
        }
        m_Scheduler = l->lval().toInt();
        if (l->lex() != '\n') {
            goto parse_error;
        }
        qCDebug(KSUD_LOG) << "Scheduler set to " << m_Scheduler;
        respond(Res_OK);
        break;

    case Lexer::Tok_exec: // "EXEC command:string user:string [options:string (env:string)*]\n"
    {
        QByteArray options;
        QList<QByteArray> env;
        tok = l->lex();
        if (tok != Lexer::Tok_str) {
            goto parse_error;
        }
        command = l->lval();
        tok = l->lex();
        if (tok != Lexer::Tok_str) {
            goto parse_error;
        }
        user = l->lval();
        tok = l->lex();
        if (tok != '\n') {
            if (tok != Lexer::Tok_str) {
                goto parse_error;
            }
            options = l->lval();
            tok = l->lex();
            while (tok != '\n') {
                if (tok != Lexer::Tok_str) {
                    goto parse_error;
                }
                QByteArray env_str = l->lval();
                env.append(env_str);
                if (strncmp(env_str.constData(), "DESKTOP_STARTUP_ID=", strlen("DESKTOP_STARTUP_ID=")) != 0) {
                    env_check += '*' + env_str;
                }
                tok = l->lex();
            }
        }

        QByteArray auth_user;
        if ((m_Scheduler != SuProcess::SchedNormal) || (m_Priority > 50)) {
            auth_user = "root";
        } else {
            auth_user = user;
        }
        key = makeKey(2, m_Host, auth_user, command);
        // We only use the command if the environment is the same.
        if (repo->find(key) == env_check) {
            key = makeKey(0, m_Host, auth_user, command);
            pass = repo->find(key);
        }
        if (pass.isNull()) // isNull() means no password, isEmpty() can mean empty password
        {
            if (m_Pass.isNull()) {
                respond(Res_NO);
                break;
            }
            data.value = env_check;
            data.timeout = m_Timeout;
            key = makeKey(2, m_Host, auth_user, command);
            repo->add(key, data);
            data.value = m_Pass;
            data.timeout = m_Timeout;
            key = makeKey(0, m_Host, auth_user, command);
            repo->add(key, data);
            pass = m_Pass;
        }

        // Execute the command asynchronously
        qCDebug(KSUD_LOG) << "Executing command: " << command;
        pid_t pid = fork();
        if (pid < 0) {
            qCDebug(KSUD_LOG) << "fork(): " << strerror(errno);
            respond(Res_NO);
            break;
        } else if (pid > 0) {
            m_pid = pid;
            respond(Res_OK);
            break;
        }

        // Ignore SIGCHLD because "class SuProcess" needs waitpid()
        signal(SIGCHLD, SIG_DFL);

        int ret;
        if (m_Host.isEmpty()) {
            SuProcess proc;
            proc.setCommand(command);
            proc.setUser(user);
            if (options.contains('x')) {
                proc.setXOnly(true);
            }
            proc.setPriority(m_Priority);
            proc.setScheduler(m_Scheduler);
            proc.setEnvironment(env);
            ret = proc.exec(pass.data());
        } else {
            SshProcess proc;
            proc.setCommand(command);
            proc.setUser(user);
            proc.setHost(m_Host);
            ret = proc.exec(pass.data());
        }

        qCDebug(KSUD_LOG) << "Command completed: " << command;
        _exit(ret);
    }

    case Lexer::Tok_delCmd: // "DEL command:string user:string\n"
        tok = l->lex();
        if (tok != Lexer::Tok_str) {
            goto parse_error;
        }
        command = l->lval();
        tok = l->lex();
        if (tok != Lexer::Tok_str) {
            goto parse_error;
        }
        user = l->lval();
        if (l->lex() != '\n') {
            goto parse_error;
        }
        key = makeKey(0, m_Host, user, command);
        if (repo->remove(key) < 0) {
            qCDebug(KSUD_LOG) << "Unknown command: " << command;
            respond(Res_NO);
        } else {
            qCDebug(KSUD_LOG) << "Deleted command: " << command << ", user = " << user;
            respond(Res_OK);
        }
        break;

    case Lexer::Tok_delVar: // "DELV name:string \n"
    {
        tok = l->lex();
        if (tok != Lexer::Tok_str) {
            goto parse_error;
        }
        name = l->lval();
        tok = l->lex();
        if (tok != '\n') {
            goto parse_error;
        }
        key = makeKey(1, name);
        if (repo->remove(key) < 0) {
            qCDebug(KSUD_LOG) << "Unknown name: " << name;
            respond(Res_NO);
        } else {
            qCDebug(KSUD_LOG) << "Deleted name: " << name;
            respond(Res_OK);
        }
        break;
    }

    case Lexer::Tok_delGroup: // "DELG group:string\n"
        tok = l->lex();
        if (tok != Lexer::Tok_str) {
            goto parse_error;
        }
        name = l->lval();
        if (repo->removeGroup(name) < 0) {
            qCDebug(KSUD_LOG) << "No keys found under group: " << name;
            respond(Res_NO);
        } else {
            qCDebug(KSUD_LOG) << "Removed all keys under group: " << name;
            respond(Res_OK);
        }
        break;

    case Lexer::Tok_delSpecialKey: // "DELS special_key:string\n"
        tok = l->lex();
        if (tok != Lexer::Tok_str) {
            goto parse_error;
        }
        name = l->lval();
        if (repo->removeSpecialKey(name) < 0) {
            respond(Res_NO);
        } else {
            respond(Res_OK);
        }
        break;

    case Lexer::Tok_set: // "SET name:string value:string group:string timeout:int\n"
        tok = l->lex();
        if (tok != Lexer::Tok_str) {
            goto parse_error;
        }
        name = l->lval();
        tok = l->lex();
        if (tok != Lexer::Tok_str) {
            goto parse_error;
        }
        data.value = l->lval();
        tok = l->lex();
        if (tok != Lexer::Tok_str) {
            goto parse_error;
        }
        data.group = l->lval();
        tok = l->lex();
        if (tok != Lexer::Tok_num) {
            goto parse_error;
        }
        data.timeout = l->lval().toInt();
        if (l->lex() != '\n') {
            goto parse_error;
        }
        key = makeKey(1, name);
        repo->add(key, data);
        qCDebug(KSUD_LOG) << "Stored key: " << key;
        respond(Res_OK);
        break;

    case Lexer::Tok_get: // "GET name:string\n"
        tok = l->lex();
        if (tok != Lexer::Tok_str) {
            goto parse_error;
        }
        name = l->lval();
        if (l->lex() != '\n') {
            goto parse_error;
        }
        key = makeKey(1, name);
        qCDebug(KSUD_LOG) << "Request for key: " << key;
        value = repo->find(key);
        if (!value.isEmpty()) {
            respond(Res_OK, value);
        } else {
            respond(Res_NO);
        }
        break;

    case Lexer::Tok_getKeys: // "GETK groupname:string\n"
        tok = l->lex();
        if (tok != Lexer::Tok_str) {
            goto parse_error;
        }
        name = l->lval();
        if (l->lex() != '\n') {
            goto parse_error;
        }
        qCDebug(KSUD_LOG) << "Request for group key: " << name;
        value = repo->findKeys(name);
        if (!value.isEmpty()) {
            respond(Res_OK, value);
        } else {
            respond(Res_NO);
        }
        break;

    case Lexer::Tok_chkGroup: // "CHKG groupname:string\n"
        tok = l->lex();
        if (tok != Lexer::Tok_str) {
            goto parse_error;
        }
        name = l->lval();
        if (l->lex() != '\n') {
            goto parse_error;
        }
        qCDebug(KSUD_LOG) << "Checking for group key: " << name;
        if (repo->hasGroup(name) < 0) {
            respond(Res_NO);
        } else {
            respond(Res_OK);
        }
        break;

    case Lexer::Tok_ping: // "PING\n"
        tok = l->lex();
        if (tok != '\n') {
            goto parse_error;
        }
        respond(Res_OK);
        break;

    case Lexer::Tok_exit: // "EXIT\n"
        tok = l->lex();
        if (tok != '\n') {
            goto parse_error;
        }
        m_needExitCode = true;
        if (m_hasExitCode) {
            sendExitCode();
        }
        break;

    case Lexer::Tok_stop: // "STOP\n"
        tok = l->lex();
        if (tok != '\n') {
            goto parse_error;
        }
        qCDebug(KSUD_LOG) << "Stopping by command";
        respond(Res_OK);
        kdesud_cleanup();
        exit(0);

    default:
        qCWarning(KSUD_LOG) << "Unknown command: " << l->lval();
        respond(Res_NO);
        goto parse_error;
    }

    delete l;
    return 0;

parse_error:
    qCWarning(KSUD_LOG) << "Parse error";
    delete l;
    return -1;
}
