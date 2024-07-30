/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "eventloggerregister.h"

#include "modeleventlogger.h"

EventLoggerRegister::~EventLoggerRegister()
{
}

EventLoggerRegister *EventLoggerRegister::s_instance = nullptr;
std::unique_ptr<EventLoggerRegister> EventLoggerRegister::s_destroyer;

EventLoggerRegister *EventLoggerRegister::instance(Behaviour behaviour)
{
    if (!s_instance) {
        s_instance = new EventLoggerRegister(behaviour);
        s_destroyer.reset(s_instance);
    }
    return s_instance;
}

void EventLoggerRegister::registerLogger(ModelEventLogger *logger)
{
    m_loggers.append(logger);
}

void EventLoggerRegister::unregisterLogger(ModelEventLogger *logger)
{
    m_loggers.remove(m_loggers.indexOf(logger));
}

void EventLoggerRegister::writeLogs()
{
    static bool asserting = false;
    if (!asserting) {
        // If logger->writeLog asserts, we don't segfault
        asserting = true;
        // The destructor writes the log.
        qDeleteAll(m_loggers);
        m_loggers.clear();
        asserting = false;
    }
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
#if QT_VERSION >= 0x050500
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
#endif
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}

EventLoggerRegister::EventLoggerRegister(Behaviour behaviour)
{
    if (behaviour == InstallMsgHandler) {
        qInstallMessageHandler(myMessageOutput);
    }
}
