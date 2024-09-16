// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QtCore>
#include "LogManager.h"
#include "dconfig.h"
#include <DSGApplication>
#include <Logger.h>
#include <ConsoleAppender.h>
#include <RollingFileAppender.h>
#include <JournalAppender.h>
#include "dstandardpaths.h"

DCORE_BEGIN_NAMESPACE

#define RULES_KEY ("rules")
// Courtesy qstandardpaths_unix.cpp
static void appendOrganizationAndApp(QString &path)
{
#ifndef QT_BOOTSTRAPPED
    const QString org = QCoreApplication::organizationName();
    if (!org.isEmpty())
        path += QLatin1Char('/') + org;
    const QString appName = QCoreApplication::applicationName();
    if (!appName.isEmpty())
        path += QLatin1Char('/') + appName;
#else
    Q_UNUSED(path);
#endif
}

#define DEFAULT_FMT "%{time}{yyyy-MM-dd, HH:mm:ss.zzz} [%{type:-7}] [%{file:-20} %{function:-35} %{line}] %{message}"

class DLogManagerPrivate {
public:
    explicit DLogManagerPrivate(DLogManager *q)
        : m_format(DEFAULT_FMT)
        , q_ptr(q)
    {
    }

    DConfig *createDConfig(const QString &appId);
    void initLoggingRules();
    void updateLoggingRules();

    QString m_format;
    QString m_logPath;
    ConsoleAppender* m_consoleAppender = nullptr;
    RollingFileAppender* m_rollingFileAppender = nullptr;
    JournalAppender* m_journalAppender = nullptr;
    QScopedPointer<DConfig> m_dsgConfig;
    QScopedPointer<DConfig> m_fallbackConfig;

    DLogManager *q_ptr = nullptr;
    Q_DECLARE_PUBLIC(DLogManager)

};

DConfig *DLogManagerPrivate::createDConfig(const QString &appId)
{
    if (appId.isEmpty())
        return nullptr;

    DConfig *config = DConfig::create(appId, "org.deepin.dtk.preference");
    if (!config->isValid()) {
        qWarning() << "Logging rules config is invalid, please check `appId` [" << appId << "]arg is correct";
        delete config;
        config = nullptr;
        return nullptr;
    }

    QObject::connect(config, &DConfig::valueChanged, config, [this](const QString &key) {
        if (key != RULES_KEY)
            return;

        updateLoggingRules();
    });

    return config;
}

void DLogManagerPrivate::initLoggingRules()
{
    if (qEnvironmentVariableIsSet("DTK_DISABLED_LOGGING_RULES"))
        return;

    // 1. 未指定 fallbackId 时，以 dsgAppId 为准
    QString dsgAppId = DSGApplication::id();
    m_dsgConfig.reset(createDConfig(dsgAppId));

    QString fallbackId = qgetenv("DTK_LOGGING_FALLBACK_APPID");
    // 2. fallbackId 和 dsgAppId 非空且不等时，都创建和监听变化
    if (!fallbackId.isEmpty() && fallbackId != dsgAppId)
        m_fallbackConfig.reset(createDConfig(fallbackId));

    // 3. 默认值和非默认值时，非默认值优先
    updateLoggingRules();
}

void DLogManagerPrivate::updateLoggingRules()
{
    QVariant var;
    // 4. 优先看 dsgConfig 是否默认值，其次 fallback 是否默认值
    if (m_dsgConfig && !m_dsgConfig->isDefaultValue(RULES_KEY)) {
        var = m_dsgConfig->value(RULES_KEY);
    } else if (m_fallbackConfig && !m_fallbackConfig->isDefaultValue(RULES_KEY)) {
        var = m_fallbackConfig->value(RULES_KEY);
    } else if (m_dsgConfig) {
        var = m_dsgConfig->value(RULES_KEY);
    }

    if (var.isValid())
        QLoggingCategory::setFilterRules(var.toString().replace(";", "\n"));
}
/*!
@~english
  \class Dtk::Core::DLogManager
  \inmodule dtkcore

  \brief DLogManager is the deepin user application log manager.
 */

DLogManager::DLogManager()
    :d_ptr(new DLogManagerPrivate(this))
{
    d_ptr->initLoggingRules();
}

void DLogManager::initConsoleAppender(){
    Q_D(DLogManager);
    d->m_consoleAppender = new ConsoleAppender;
    d->m_consoleAppender->setFormat(d->m_format);
    dlogger->registerAppender(d->m_consoleAppender);
}

void DLogManager::initRollingFileAppender(){
    Q_D(DLogManager);
    d->m_rollingFileAppender = new RollingFileAppender(getlogFilePath());
    d->m_rollingFileAppender->setFormat(d->m_format);
    d->m_rollingFileAppender->setLogFilesLimit(5);
    d->m_rollingFileAppender->setDatePattern(RollingFileAppender::DailyRollover);
    dlogger->registerAppender(d->m_rollingFileAppender);
}

void DLogManager::initJournalAppender()
{
#if (defined BUILD_WITH_SYSTEMD && defined Q_OS_LINUX)
    Q_D(DLogManager);
    d->m_journalAppender = new JournalAppender();
    dlogger->registerAppender(d->m_journalAppender);
#endif
}

/*!
@~english
  \brief Registers the appender to write the log records to the Console.

  \sa registerFileAppender
 */
void DLogManager::registerConsoleAppender(){
    DLogManager::instance()->initConsoleAppender();
}

/*!
@~english
  \brief Registers the appender to write the log records to the file.

  \sa getlogFilePath
  \sa registerConsoleAppender
 */
void DLogManager::registerFileAppender() {
    DLogManager::instance()->initRollingFileAppender();
}

void DLogManager::registerJournalAppender()
{
    DLogManager::instance()->initJournalAppender();
}

/*!
@~english
  \brief Return the path file log storage.

  \brief DLogManager::getlogFilePath Get the log file path
  \brief The default log path is ~/.cache/<OrganizationName>/<ApplicationName>.log
  \brief If the environment variable $HOME cannot be acquired, DLogManager will not log anything
  \sa registerFileAppender
 */
QString DLogManager::getlogFilePath()
{
    //No longer set the default log path (and mkdir) when constructing now, instead set the default value if it's empty when getlogFilePath is called.
    //Fix the problem that the log file is still created in the default path when the log path is set manually.
    if (DLogManager::instance()->d_func()->m_logPath.isEmpty()) {
        if (DStandardPaths::homePath().isEmpty()) {
            qWarning() << "Unable to locate the cache directory, cannot acquire home directory, and the log will not be written to file..";
            return QString();
        }

        QString cachePath(DStandardPaths::path(DStandardPaths::XDG::CacheHome));
        appendOrganizationAndApp(cachePath);

        if (!QDir(cachePath).exists()) {
            QDir(cachePath).mkpath(cachePath);
        }
        instance()->d_func()->m_logPath = instance()->joinPath(cachePath, QString("%1.log").arg(qApp->applicationName()));
    }

    return QDir::toNativeSeparators(DLogManager::instance()->d_func()->m_logPath);
}

/*!
@~english
  \brief DLogManager::setlogFilePath Set the log file path
  \a logFilePath Log file path
  \brief If the file path set is not the file path, nothing will do, and an output warning
 */
void DLogManager::setlogFilePath(const QString &logFilePath)
{
    QFileInfo info(logFilePath);
    if (info.exists() && !info.isFile())
        qWarning() << "invalid file path:" << logFilePath << " is not a file";
    else
        DLogManager::instance()->d_func()->m_logPath = logFilePath;
}

void DLogManager::setLogFormat(const QString &format)
{
    //m_format = "%{time}{yyyy-MM-dd, HH:mm:ss.zzz} [%{type:-7}] [%{file:-20} %{function:-35} %{line}] %{message}\n";
    DLogManager::instance()->d_func()->m_format = format;
}

QString DLogManager::joinPath(const QString &path, const QString &fileName){
    QString separator(QDir::separator());
    return QString("%1%2%3").arg(path, separator, fileName);
}

DLogManager::~DLogManager()
{
}

DCORE_END_NAMESPACE
