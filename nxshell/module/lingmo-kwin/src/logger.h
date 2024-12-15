#pragma once

#include <kwinglobals.h>
#include <QMutex>

#define LOG_EMERG   0   /* system is unusable */
#define LOG_ALERT   1   /* action must be taken immediately */
#define LOG_CRIT    2   /* critical conditions */
#define LOG_ERR     3   /* error conditions */
#define LOG_WARNING 4   /* warning conditions */
#define LOG_NOTICE  5   /* normal but significant condition */
#define LOG_INFO    6   /* informational */
#define LOG_DEBUG   7   /* debug-level messages */

namespace KWin
{

class KWIN_EXPORT Logger : public QObject
{
public:
    static Logger *instance();

    void installMessageHandler();

    void append(int level, const char *file, int line, const char *func, const char *category, const QString &msg);

    void setLoggingRules(const QString &rules);

private:
    Logger(QObject *parent = nullptr);
    ~Logger() = default;

    void appendRules(const QString &rules);

    QMutex m_mutex;
    QString m_rules;
};

}