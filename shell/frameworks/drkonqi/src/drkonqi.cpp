/*
    SPDX-FileCopyrightText: 2000-2003 Hans Petter Bieker <bieker@kde.org>
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "drkonqi.h"
#include "drkonqi_debug.h"

#include <chrono>

#include <QFileDialog>
#include <QPointer>
#include <QTemporaryFile>
#include <QTextStream>
#include <QTimerEvent>

#include <KCrash>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <QApplication>
#include <kio/filecopyjob.h>

#include "backtracegenerator.h"
#include "crashedapplication.h"
#include "debuggermanager.h"
#include "drkonqibackends.h"
#include "systeminformation.h"

#ifdef SYSTEMD_AVAILABLE
#include "coredumpbackend.h"
#endif

using namespace std::chrono_literals;

static AbstractDrKonqiBackend *factorizeBackend()
{
    // This is controlled by the environment because doing it as a cmdline option is supremely horrible because
    // DrKonqi is a singleton that gets created at a random point in time, while options are only set on it afterwards.
    // Since we don't want a nullptr backend we'll need the backend factorization to be independent of the cmdline.
    // This could maybe be changed but would require substantial rejiggering of the singleton to not have points in
    // time where there is no backend behind it.
#ifdef SYSTEMD_AVAILABLE
    if (qgetenv("DRKONQI_BACKEND") == QByteArrayLiteral("COREDUMPD")) {
        qunsetenv("DRKONQI_BACKEND");
        return new CoredumpBackend;
    }
#endif
    return new KCrashBackend();
}

DrKonqi::DrKonqi()
    : m_systemInformation(new SystemInformation())
    , m_backend(factorizeBackend())
    , m_signal(0)
    , m_pid(0)
    , m_kdeinit(false)
    , m_safer(false)
    , m_restarted(false)
    , m_keepRunning(false)
    , m_thread(0)
{
}

DrKonqi::~DrKonqi()
{
    delete m_systemInformation;
    delete m_backend;
}

// static
DrKonqi *DrKonqi::instance()
{
    static DrKonqi drKonqiInstance;
    return &drKonqiInstance;
}

// based on KCrashDelaySetHandler from kdeui/util/kcrash.cpp
class EnableCrashCatchingDelayed : public QObject
{
public:
    EnableCrashCatchingDelayed()
    {
        startTimer(10s);
    }

protected:
    void timerEvent(QTimerEvent *event) override
    {
        qCDebug(DRKONQI_LOG) << "Enabling drkonqi crash catching";
        KCrash::setDrKonqiEnabled(true);
        killTimer(event->timerId());
        this->deleteLater();
    }
};

bool DrKonqi::init()
{
    if (!instance()->m_backend->init()) {
        return false;
    } else { // all ok, continue initialization
        // Set drkonqi to handle its own crashes, but only if the crashed app
        // is not drkonqi already. If it is drkonqi, delay enabling crash catching
        // to prevent recursive crashes (in case it crashes at startup)
        if (crashedApplication()->fakeExecutableBaseName() != QLatin1String("drkonqi")) {
            qCDebug(DRKONQI_LOG) << "Enabling drkonqi crash catching";
            KCrash::setDrKonqiEnabled(true);
        } else {
            new EnableCrashCatchingDelayed;
        }
        return true;
    }
}

// static
SystemInformation *DrKonqi::systemInformation()
{
    return instance()->m_systemInformation;
}

// static
DebuggerManager *DrKonqi::debuggerManager()
{
    return instance()->m_backend->debuggerManager();
}

// static
CrashedApplication *DrKonqi::crashedApplication()
{
    return instance()->m_backend->crashedApplication();
}

// static
void DrKonqi::saveReport(const QString &reportText, QWidget *parent)
{
    if (isSafer()) {
        QTemporaryFile tf;
        tf.setFileTemplate(QStringLiteral("XXXXXX.kcrash"));
        tf.setAutoRemove(false);

        if (tf.open()) {
            QTextStream textStream(&tf);
            textStream << reportText;
            textStream.flush();
            KMessageBox::information(parent, xi18nc("@info", "Report saved to <filename>%1</filename>.", tf.fileName()));
        } else {
            KMessageBox::error(parent, i18nc("@info", "Could not create a file in which to save the report."));
        }
    } else {
        QString defname = getSuggestedKCrashFilename(crashedApplication());

        QPointer<QFileDialog> dlg(new QFileDialog(parent, defname));
        dlg->selectFile(defname);
        dlg->setWindowTitle(i18nc("@title:window", "Save Report"));
        dlg->setAcceptMode(QFileDialog::AcceptSave);
        dlg->setFileMode(QFileDialog::AnyFile);
        dlg->setOption(QFileDialog::DontResolveSymlinks, false);
        dlg->setDirectory(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).last());
        if (dlg->exec() != QDialog::Accepted) {
            return;
        }

        if (!dlg) {
            // Dialog is invalid, it was probably deleted (ex. via DBus call)
            // return and do not crash
            return;
        }

        QUrl fileUrl;
        if (!dlg->selectedUrls().isEmpty())
            fileUrl = dlg->selectedUrls().first();
        delete dlg;

        if (fileUrl.isValid()) {
            QTemporaryFile tf;
            if (tf.open()) {
                QTextStream ts(&tf);
                ts << reportText;
                ts.flush();
            } else {
                KMessageBox::error(parent,
                                   xi18nc("@info",
                                          "Cannot open file <filename>%1</filename> "
                                          "for writing.",
                                          tf.fileName()));
                return;
            }

            // QFileDialog was run with confirmOverwrite, so we can safely
            // overwrite as necessary.
            KIO::FileCopyJob *job = KIO::file_copy(QUrl::fromLocalFile(tf.fileName()), fileUrl, -1, KIO::DefaultFlags | KIO::Overwrite);
            KJobWidgets::setWindow(job, parent);
            if (!job->exec()) {
                KMessageBox::error(parent, job->errorString());
            }
        }
    }
}

void DrKonqi::setSignal(int signal)
{
    instance()->m_signal = signal;
}

void DrKonqi::setAppName(const QString &appName)
{
    instance()->m_appName = appName;
}

void DrKonqi::setAppPath(const QString &appPath)
{
    instance()->m_appPath = appPath;
}

void DrKonqi::setAppVersion(const QString &appVersion)
{
    instance()->m_appVersion = appVersion;
}

void DrKonqi::setBugAddress(const QString &bugAddress)
{
    instance()->m_bugAddress = bugAddress;
}

void DrKonqi::setProgramName(const QString &programName)
{
    instance()->m_programName = programName;
}

void DrKonqi::setProductName(const QString &productName)
{
    instance()->m_productName = productName;
}

void DrKonqi::setPid(int pid)
{
    instance()->m_pid = pid;
}

void DrKonqi::setKdeinit(bool kdeinit)
{
    instance()->m_kdeinit = kdeinit;
}

void DrKonqi::setSafer(bool safer)
{
    instance()->m_safer = safer;
}

void DrKonqi::setRestarted(bool restarted)
{
    instance()->m_restarted = restarted;
}

void DrKonqi::setKeepRunning(bool keepRunning)
{
    instance()->m_keepRunning = keepRunning;
}

void DrKonqi::setThread(int thread)
{
    instance()->m_thread = thread;
}

void DrKonqi::setStartupId(const QString &startupId)
{
    instance()->m_startupId = startupId;
}

int DrKonqi::signal()
{
    return instance()->m_signal;
}

const QString &DrKonqi::appName()
{
    return instance()->m_appName;
}

const QString &DrKonqi::appPath()
{
    return instance()->m_appPath;
}

const QString &DrKonqi::appVersion()
{
    return instance()->m_appVersion;
}

const QString &DrKonqi::bugAddress()
{
    return instance()->m_bugAddress;
}

const QString &DrKonqi::programName()
{
    return instance()->m_programName;
}

const QString &DrKonqi::productName()
{
    return instance()->m_productName;
}

int DrKonqi::pid()
{
    return instance()->m_pid;
}

bool DrKonqi::isKdeinit()
{
    return instance()->m_kdeinit;
}

bool DrKonqi::isSafer()
{
    return instance()->m_safer;
}

bool DrKonqi::isRestarted()
{
    return instance()->m_restarted;
}

bool DrKonqi::isKeepRunning()
{
    return instance()->m_keepRunning;
}

int DrKonqi::thread()
{
    return instance()->m_thread;
}

bool DrKonqi::ignoreQuality()
{
    static bool ignore = qEnvironmentVariableIsSet("DRKONQI_TEST_MODE") && qEnvironmentVariableIntValue("DRKONQI_IGNORE_QUALITY") == 1;
    return ignore;
}

bool DrKonqi::isTestingBugzilla()
{
    return kdeBugzillaURL().contains(QLatin1String("bugstest.kde.org"));
}

const QString &DrKonqi::kdeBugzillaURL()
{
    // WARNING: for practical reasons this cannot use the shared instance
    //   Initing the instances requires knowing the URL already, so we'd have
    //   an init loop. Use a local static instead. Otherwise we'd crash on
    //   initialization of global statics derived from our return value.
    //   Always copy into the local static and return that!
    static QString url;
    if (!url.isEmpty()) {
        return url;
    }

    url = QString::fromLocal8Bit(qgetenv("DRKONQI_KDE_BUGZILLA_URL"));
    if (!url.isEmpty()) {
        return url;
    }

    if (qEnvironmentVariableIsSet("DRKONQI_TEST_MODE")) {
        url = QStringLiteral("https://bugstest.kde.org/");
    } else {
        url = QStringLiteral("https://bugs.kde.org/");
    }

    return url;
}

const QString &DrKonqi::startupId()
{
    return instance()->m_startupId;
}

QString DrKonqi::backendClassName()
{
    return QString::fromLatin1(instance()->m_backend->metaObject()->className());
}

bool DrKonqi::isEphemeralCrash()
{
#ifdef SYSTEMD_AVAILABLE
    return qobject_cast<CoredumpBackend *>(instance()->m_backend) == nullptr; // not coredumpd backend => ephemeral
#else
    return true;
#endif
}

bool DrKonqi::minimalMode()
{
    return qEnvironmentVariableIntValue("DRKONQI_MINIMAL_MODE") > 0;
}

#include "drkonqi.moc"
