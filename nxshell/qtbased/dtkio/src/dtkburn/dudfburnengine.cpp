// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dudfburnengine.h"

#include <QDebug>
#include <QDir>
#include <QRegularExpression>
#include <QStandardPaths>

DBURN_BEGIN_NAMESPACE

namespace {
typedef struct
{
    long totalSize;
    long wroteSize;
    double progress;
} ProgressInfo;

// this is defined for callback register
std::function<void(const ProgressInfo *)> udfProgressCbBinder;
extern "C" void udProgressCbProxy(const ProgressInfo *info)
{
    udfProgressCbBinder(info);
}

extern "C" {
typedef void (*progress_cb)(const ProgressInfo *);
typedef void (*uburn_init)();
typedef int (*uburn_do_burn)(const char *dev, const char *file, const char *lable);
typedef void (*uburn_regi_cb)(progress_cb cb);
typedef char **(*uburn_get_errors)(int *);
typedef void (*uburn_show_verbose)();
typedef void (*uburn_redirect_output)(int redir_stdout, int redir_stderr);

static uburn_init ub_init { nullptr };
static uburn_do_burn ub_do_burn { nullptr };
static uburn_regi_cb ub_regi_cb { nullptr };
static uburn_get_errors ub_get_errors { nullptr };
static uburn_show_verbose ub_show_verbose { nullptr };
static uburn_redirect_output ub_redirect_output { nullptr };
}

}   // namespace

DUDFBurnEngine::DUDFBurnEngine(QObject *parent)
    : QObject(parent)
{
    initialize();
}

DUDFBurnEngine::~DUDFBurnEngine()
{
    if (m_libLoaded)
        m_lib.unload();
}

bool DUDFBurnEngine::doBurn(const QString &dev, const QPair<QString, QString> &files, const QString &volId)
{
    if (!canSafeUse())
        return false;

    Q_EMIT jobStatusChanged(JobStatus::Stalled, 0);

    udfProgressCbBinder = [this](const ProgressInfo *info) {
        Q_EMIT jobStatusChanged(JobStatus::Running, static_cast<int>(info->progress));
    };
    ub_init();
    ub_regi_cb(udProgressCbProxy);
    ub_show_verbose();
    ub_redirect_output(1, 0);

    int ret = ub_do_burn(dev.toStdString().c_str(), files.first.toStdString().c_str(),
                         volId.toStdString().c_str());

    // burn failed
    if (ret != 0) {
        int err_count = 0;
        char **errors = ub_get_errors(&err_count);
        if (errors != nullptr && err_count > 0) {
            QStringList errMsg;
            for (int i = err_count - 1; i >= 0; i--)
                errMsg.append(errors[i]);
            m_message = errMsg;
        }
        m_message.append(readErrorsFromLog());
        Q_EMIT jobStatusChanged(JobStatus::Failed, 100);
        return false;
    }

    Q_EMIT jobStatusChanged(JobStatus::Finished, 100);

    return true;
}

QStringList DUDFBurnEngine::lastErrorMessage() const
{
    return m_message;
}

void DUDFBurnEngine::initialize()
{
    m_lib.setFileName("udfburn");
    if (m_lib.isLoaded())
        return;
    m_libLoaded = m_lib.load();
    if (!m_libLoaded) {
        qWarning() << "[dtkburn] Cannot load lib: " << m_lib.fileName();
        return;
    }
    qDebug() << m_lib.fileName();
    ub_init = reinterpret_cast<uburn_init>(m_lib.resolve("burn_init"));
    m_funcsLoaded &= (ub_init != nullptr);

    ub_do_burn = reinterpret_cast<uburn_do_burn>(m_lib.resolve("burn_burn_to_disc"));
    m_funcsLoaded &= (ub_do_burn != nullptr);

    ub_regi_cb = reinterpret_cast<uburn_regi_cb>(m_lib.resolve("burn_register_progress_callback"));
    m_funcsLoaded &= (ub_regi_cb != nullptr);

    ub_show_verbose = reinterpret_cast<uburn_show_verbose>(m_lib.resolve("burn_show_verbose_information"));
    m_funcsLoaded &= (ub_show_verbose != nullptr);

    ub_redirect_output = reinterpret_cast<uburn_redirect_output>(m_lib.resolve("burn_redirect_output"));
    m_funcsLoaded &= (ub_redirect_output != nullptr);

    ub_get_errors = reinterpret_cast<uburn_get_errors>(m_lib.resolve("burn_get_last_errors"));
    m_funcsLoaded &= (ub_get_errors != nullptr);

    qInfo() << "[dtkburn] udf load lib " << (m_libLoaded ? "success" : "failed");
    qInfo() << "[dtkburn] udf load func " << (m_funcsLoaded ? "success" : "failed");
}

bool DUDFBurnEngine::canSafeUse() const
{
    return m_libLoaded && m_funcsLoaded;
}

QStringList DUDFBurnEngine::readErrorsFromLog() const
{
    auto &&homePaths { QStandardPaths::standardLocations(QStandardPaths::HomeLocation) };
    if (homePaths.isEmpty())
        return {};

    auto &&logPath { homePaths.at(0) + "/.cache/deepin/discburn/uburn/" };
    QDir logDir(logPath);
    if (!logDir.exists())
        return {};

    auto &&burns { logDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::SortFlag::Time) };
    if (burns.count() == 0)
        return {};

    auto &&lastBurn { burns.first() };
    auto &&logFilePath { logPath + lastBurn + "/log" };
    QFile logFile(logFilePath);
    if (!logFile.exists())
        return {};

    QStringList ret;
    logFile.open(QIODevice::ReadOnly | QIODevice::Text);
    while (!logFile.atEnd()) {
        QString &&line(logFile.readLine());
        if (line.startsWith("Warning") || line.startsWith("Error")) {
            line.remove(QRegularExpression("/home/.*/.cache/deepin/discburn/_dev_sr[0-9]*/"));
            ret << line;
        }
    }
    logFile.close();
    return ret;
}

DBURN_END_NAMESPACE
