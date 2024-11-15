#include "LingmoLogger.h"
#include "elog.h"

#include <cstdio>

LingmoLogger::LingmoLogger(QObject* parent)
    : QObject(parent)
    , _loggingTag("NONAME")
{
    /* close printf buffer */
    setbuf(stdout, NULL);
    /* initialize EasyLogger */
    elog_init();
    /* set EasyLogger log format */
    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_LVL | ELOG_FMT_TAG | ELOG_FMT_TIME);
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~ELOG_FMT_FUNC);
    /* start EasyLogger */
    elog_start();
}

LingmoLogger::~LingmoLogger()
{
    elog_stop();
    elog_deinit();
}

void LingmoLogger::lAssert(const QString& msg)
{
    elog_a(loggingTag().toStdString().c_str(), msg.toStdString().c_str());
}

void LingmoLogger::lError(const QString& msg)
{
    elog_e(loggingTag().toStdString().c_str(), msg.toStdString().c_str());
}

void LingmoLogger::lWarn(const QString& msg)
{
    elog_w(loggingTag().toStdString().c_str(), msg.toStdString().c_str());
}

void LingmoLogger::lInfo(const QString& msg)
{
    elog_i(loggingTag().toStdString().c_str(), msg.toStdString().c_str());
}

void LingmoLogger::lDebug(const QString& msg)
{
    elog_d(loggingTag().toStdString().c_str(), msg.toStdString().c_str());
}

void LingmoLogger::lVerbose(const QString& msg)
{
    elog_v(loggingTag().toStdString().c_str(), msg.toStdString().c_str());
}

void LingmoLogger::lAseertTag(const QString& tag, const QString& msg)
{
    elog_a(tag.toStdString().c_str(), msg.toStdString().c_str());
}

void LingmoLogger::lErrorTag(const QString& tag, const QString& msg)
{
    elog_e(tag.toStdString().c_str(), msg.toStdString().c_str());
}

void LingmoLogger::lWarnTag(const QString& tag, const QString& msg)
{
    elog_w(tag.toStdString().c_str(), msg.toStdString().c_str());
}

void LingmoLogger::lInfoTag(const QString& tag, const QString& msg)
{
    elog_i(tag.toStdString().c_str(), msg.toStdString().c_str());
}

void LingmoLogger::lDebugTag(const QString& tag, const QString& msg)
{
    elog_d(tag.toStdString().c_str(), msg.toStdString().c_str());
}

void LingmoLogger::lVerboseTag(const QString& tag, const QString& msg)
{
    elog_v(tag.toStdString().c_str(), msg.toStdString().c_str());
}