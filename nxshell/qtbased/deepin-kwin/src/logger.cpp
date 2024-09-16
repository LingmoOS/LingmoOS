#include "logger.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QLoggingCategory>

#include <systemd/sd-journal.h>

#define CONFIGMANAGER_SERVICE           "org.desktopspec.ConfigManager"
#define CONFIGMANAGER_INTERFACE         "org.desktopspec.ConfigManager"
#define CONFIGMANAGER_MANGER_INTERFACE  "org.desktopspec.ConfigManager.Manager"

namespace KWin
{

static void qtLoggerMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString &msg)
{
    int logLevel = LOG_INFO;
    switch (type) {
    case QtDebugMsg:
        logLevel = LOG_DEBUG;
        break;
    case QtInfoMsg:
        logLevel = LOG_INFO;
        break;
    case QtWarningMsg:
        logLevel = LOG_WARNING;
        break;
    case QtCriticalMsg:
        logLevel = LOG_ERR;
        break;
    case QtFatalMsg:
        logLevel = LOG_CRIT;
        break;
    default:
        logLevel = LOG_INFO;
        break;
    }

    Logger::instance()->append(logLevel, context.file, context.line, context.function, context.category, msg);
}

Logger::Logger(QObject *parent)
    : QObject(parent)
{
    QByteArray logRules = qgetenv("QT_LOGGING_RULES");
    qunsetenv("QT_LOGGING_RULES");

    // set env
    setLoggingRules(logRules);

    // set dconfig
    QDBusInterface interfaceRequire(CONFIGMANAGER_SERVICE, "/", CONFIGMANAGER_INTERFACE, QDBusConnection::systemBus());
    QDBusReply<QDBusObjectPath> reply = interfaceRequire.call("acquireManager", "org.kde.kwin", "org.kde.kwin.logging", "");
    if (reply.isValid()) {
        QDBusInterface interfaceValue(CONFIGMANAGER_SERVICE, reply.value().path(), CONFIGMANAGER_MANGER_INTERFACE, QDBusConnection::systemBus());
        QDBusReply<QVariant> replyValue = interfaceValue.call("value", "log_rules");
        appendRules(replyValue.value().toString());
        setLoggingRules(m_rules);
    }
}

Logger *Logger::instance()
{
    static Logger logger;
    return &logger;
}

void Logger::installMessageHandler()
{
    qInstallMessageHandler(qtLoggerMessageHandler);
}

void Logger::append(int level, const char *file, int line, const char *func, const char *category, const QString &msg)
{
    sd_journal_send("MESSAGE=%s",
                    msg.toStdString().c_str(),
                    "PRIORITY=%d",
                    level,
                    "CODE_FILE=%s",
                    file,
                    "CODE_LINE=%d",
                    line,
                    "CODE_FUNC=%s",
                    func,
                    "CODE_CATEGORY=%s",
                    category,
                    NULL);
}

void Logger::setLoggingRules(const QString &rules)
{
    QString tmpRules = rules;
    m_rules = tmpRules.replace(";", "\n");
    QLoggingCategory::setFilterRules(m_rules);
}

void Logger::appendRules(const QString &rules)
{
    QString tmpRules = rules;
    tmpRules.replace(";", "\n");
    QStringList tmplist = tmpRules.split('\n');
    for (int i = 0; i < tmplist.count(); ++i) {
        if (m_rules.contains(tmplist.at(i))) {
            tmplist.removeAt(i);
            --i;
        }
    }
    if (tmplist.isEmpty())
        return;

    if (m_rules.isEmpty()) {
        m_rules = tmplist.join("\n");
    } else {
        m_rules += "\n" + tmplist.join("\n");
    }
}

}