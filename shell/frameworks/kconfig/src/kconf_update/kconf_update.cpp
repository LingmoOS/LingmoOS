/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kconfig_version.h"
#include <cstdlib>

#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextStream>
#include <QUrl>

#include <kconfig.h>
#include <kconfiggroup.h>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QStandardPaths>

#include "kconf_update_debug.h"

// Convenience wrapper around qCDebug to prefix the output with metadata of
// the file.
#define qCDebugFile(CATEGORY) qCDebug(CATEGORY) << m_currentFilename << ':' << m_lineCount << ":'" << m_line << "': "

class KonfUpdate
{
public:
    KonfUpdate(QCommandLineParser *parser);
    ~KonfUpdate();

    KonfUpdate(const KonfUpdate &) = delete;
    KonfUpdate &operator=(const KonfUpdate &) = delete;

    QStringList findUpdateFiles(bool dirtyOnly);

    bool updateFile(const QString &filename);

    void gotId(const QString &_id);
    void gotScript(const QString &_script);

protected:
    /** kconf_updaterc */
    KConfig *m_config;
    QString m_currentFilename;
    bool m_skip = false;
    bool m_bTestMode;
    bool m_bDebugOutput;
    QString m_id;

    bool m_bUseConfigInfo = false;
    QStringList m_arguments;
    QTextStream *m_textStream;
    QFile *m_file;
    QString m_line;
    int m_lineCount;
};

KonfUpdate::KonfUpdate(QCommandLineParser *parser)
    : m_textStream(nullptr)
    , m_file(nullptr)
    , m_lineCount(-1)
{
    bool updateAll = false;

    m_config = new KConfig(QStringLiteral("kconf_updaterc"));
    KConfigGroup cg(m_config, QString());

    QStringList updateFiles;

    m_bDebugOutput = parser->isSet(QStringLiteral("debug"));
    if (m_bDebugOutput) {
        // The only way to enable debug reliably is through a filter rule.
        // The category itself is const, so we can't just go around changing
        // its mode. This can however be overridden by the environment, so
        // we'll want to have a fallback warning if debug is not enabled
        // after setting the filter.
        QLoggingCategory::setFilterRules(QLatin1String("%1.debug=true").arg(QLatin1String{KCONF_UPDATE_LOG().categoryName()}));
        qDebug() << "Automatically enabled the debug logging category" << KCONF_UPDATE_LOG().categoryName();
        if (!KCONF_UPDATE_LOG().isDebugEnabled()) {
            qWarning("The debug logging category %s needs to be enabled manually to get debug output", KCONF_UPDATE_LOG().categoryName());
        }
    }

    m_bTestMode = parser->isSet(QStringLiteral("testmode"));
    if (m_bTestMode) {
        QStandardPaths::setTestModeEnabled(true);
    }

    if (parser->isSet(QStringLiteral("check"))) {
        m_bUseConfigInfo = true;
        const QString file =
            QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String{"kconf_update/"} + parser->value(QStringLiteral("check")));
        if (file.isEmpty()) {
            qWarning("File '%s' not found.", parser->value(QStringLiteral("check")).toLocal8Bit().data());
            qCDebug(KCONF_UPDATE_LOG) << "File" << parser->value(QStringLiteral("check")) << "passed on command line not found";
            return;
        }
        updateFiles.append(file);
    } else if (!parser->positionalArguments().isEmpty()) {
        updateFiles += parser->positionalArguments();
    } else if (m_bTestMode) {
        qWarning("Test mode enabled, but no files given.");
        return;
    } else {
        if (cg.readEntry("autoUpdateDisabled", false)) {
            return;
        }
        updateFiles = findUpdateFiles(true);
        updateAll = true;
    }

    for (const QString &file : std::as_const(updateFiles)) {
        updateFile(file);
    }

    if (updateAll && !cg.readEntry("updateInfoAdded", false)) {
        cg.writeEntry("updateInfoAdded", true);
        updateFiles = findUpdateFiles(false);
    }
}

KonfUpdate::~KonfUpdate()
{
    delete m_config;
    delete m_file;
    delete m_textStream;
}

QStringList KonfUpdate::findUpdateFiles(bool dirtyOnly)
{
    QStringList result;

    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("kconf_update"), QStandardPaths::LocateDirectory);
    for (const QString &d : dirs) {
        const QDir dir(d);

        const QStringList fileNames = dir.entryList(QStringList(QStringLiteral("*.upd")));
        for (const QString &fileName : fileNames) {
            const QString file = dir.filePath(fileName);
            QFileInfo info(file);

            KConfigGroup cg(m_config, fileName);
            const qint64 ctime = cg.readEntry("ctime", 0);
            const qint64 mtime = cg.readEntry("mtime", 0);
            const QString done = cg.readEntry("done", QString());
            if (!dirtyOnly //
                || (ctime != 0 && ctime != info.birthTime().toSecsSinceEpoch()) //
                || mtime != info.lastModified().toSecsSinceEpoch() //
                || (mtime != 0 && done.isEmpty())) {
                result.append(file);
            }
        }
    }
    return result;
}

/**
 * Syntax:
 * # Comment
 * Id=id
 * ScriptArguments=arguments
 * Script=scriptfile[,interpreter]
 **/
bool KonfUpdate::updateFile(const QString &filename)
{
    m_currentFilename = filename;
    const int i = m_currentFilename.lastIndexOf(QLatin1Char{'/'});
    if (i != -1) {
        m_currentFilename = m_currentFilename.mid(i + 1);
    }
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Could not open update-file '%s'.", qUtf8Printable(filename));
        return false;
    }

    qCDebug(KCONF_UPDATE_LOG) << "Checking update-file" << filename << "for new updates";

    QTextStream ts(&file);
    ts.setEncoding(QStringConverter::Encoding::Latin1);
    m_lineCount = 0;
    bool foundVersion = false;
    while (!ts.atEnd()) {
        m_line = ts.readLine().trimmed();
        const QLatin1String versionPrefix("Version=");
        if (m_line.startsWith(versionPrefix)) {
            if (m_line.mid(versionPrefix.length()) == QLatin1Char('6')) {
                foundVersion = true;
                continue;
            } else {
                qWarning(KCONF_UPDATE_LOG).noquote() << filename << "defined" << m_line << "but Version=6 was expected";
                return false;
            }
        }
        ++m_lineCount;
        if (m_line.isEmpty() || (m_line[0] == QLatin1Char('#'))) {
            continue;
        }
        if (m_line.startsWith(QLatin1String("Id="))) {
            if (!foundVersion) {
                qCDebug(KCONF_UPDATE_LOG, "Missing 'Version=6', file '%s' will be skipped.", qUtf8Printable(filename));
                break;
            }
            gotId(m_line.mid(3));
        } else if (m_skip) {
            continue;
        } else if (m_line.startsWith(QLatin1String("Script="))) {
            gotScript(m_line.mid(7));
            m_arguments.clear();
        } else if (m_line.startsWith(QLatin1String("ScriptArguments="))) {
            const QString argLine = m_line.mid(16);
            m_arguments = QProcess::splitCommand(argLine);
        } else {
            qCDebugFile(KCONF_UPDATE_LOG) << "Parse error";
        }
    }
    // Flush.
    gotId(QString());

    // Remember that this file was updated:
    if (!m_bTestMode) {
        QFileInfo info(filename);
        KConfigGroup cg(m_config, m_currentFilename);
        if (info.birthTime().isValid()) {
            cg.writeEntry("ctime", info.birthTime().toSecsSinceEpoch());
        }
        cg.writeEntry("mtime", info.lastModified().toSecsSinceEpoch());
        cg.sync();
    }

    return true;
}

void KonfUpdate::gotId(const QString &_id)
{
    // Remember that the last update group has been done:
    if (!m_id.isEmpty() && !m_skip && !m_bTestMode) {
        KConfigGroup cg(m_config, m_currentFilename);

        QStringList ids = cg.readEntry("done", QStringList());
        if (!ids.contains(m_id)) {
            ids.append(m_id);
            cg.writeEntry("done", ids);
            cg.sync();
        }
    }

    if (_id.isEmpty()) {
        return;
    }

    // Check whether this update group needs to be done:
    KConfigGroup cg(m_config, m_currentFilename);
    QStringList ids = cg.readEntry("done", QStringList());
    if (ids.contains(_id) && !m_bUseConfigInfo) {
        // qDebug("Id '%s' was already in done-list", _id.toLatin1().constData());
        m_skip = true;
        return;
    }
    m_skip = false;
    m_id = _id;
    if (m_bUseConfigInfo) {
        qCDebug(KCONF_UPDATE_LOG) << m_currentFilename << ": Checking update" << _id;
    } else {
        qCDebug(KCONF_UPDATE_LOG) << m_currentFilename << ": Found new update" << _id;
    }
}

void KonfUpdate::gotScript(const QString &_script)
{
    QString script;
    QString interpreter;
    const int i = _script.indexOf(QLatin1Char{','});
    if (i == -1) {
        script = _script.trimmed();
    } else {
        script = _script.left(i).trimmed();
        interpreter = _script.mid(i + 1).trimmed();
    }

    if (script.isEmpty()) {
        qCDebugFile(KCONF_UPDATE_LOG) << "Script fails to specify filename";
        m_skip = true;
        return;
    }

    QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("kconf_update/") + script);
    if (path.isEmpty()) {
        if (interpreter.isEmpty()) {
            path = QStringLiteral("%1/kconf_update_bin/%2").arg(QStringLiteral(CMAKE_INSTALL_FULL_LIBDIR), script);
            if (!QFile::exists(path)) {
                path = QStandardPaths::findExecutable(script);
            }
        }

        if (path.isEmpty()) {
            qCDebugFile(KCONF_UPDATE_LOG) << "Script" << script << "not found";
            m_skip = true;
            return;
        }
    }

    if (!m_arguments.isEmpty()) {
        qCDebug(KCONF_UPDATE_LOG) << m_currentFilename << ": Running script" << script << "with arguments" << m_arguments;
    } else {
        qCDebug(KCONF_UPDATE_LOG) << m_currentFilename << ": Running script" << script;
    }

    QStringList args;
    QString cmd;
    if (interpreter.isEmpty()) {
        cmd = path;
    } else {
        QString interpreterPath = QStandardPaths::findExecutable(interpreter);
        if (interpreterPath.isEmpty()) {
            qCDebugFile(KCONF_UPDATE_LOG) << "Cannot find interpreter" << interpreter;
            m_skip = true;
            return;
        }
        cmd = interpreterPath;
        args << path;
    }

    args += m_arguments;

    int result;
    qCDebug(KCONF_UPDATE_LOG) << "About to run" << cmd;
    if (m_bDebugOutput) {
        QFile scriptFile(path);
        if (scriptFile.open(QIODevice::ReadOnly)) {
            qCDebug(KCONF_UPDATE_LOG) << "Script contents is:\n" << scriptFile.readAll();
        }
    }
    QProcess proc;
    proc.start(cmd, args);
    if (!proc.waitForFinished(60000)) {
        qCDebugFile(KCONF_UPDATE_LOG) << "update script did not terminate within 60 seconds:" << cmd;
        m_skip = true;
        return;
    }
    result = proc.exitCode();
    proc.close();

    if (result != EXIT_SUCCESS) {
        qCDebug(KCONF_UPDATE_LOG) << m_currentFilename << ": !! An error occurred while running" << cmd;
        return;
    }

    qCDebug(KCONF_UPDATE_LOG) << "Successfully ran" << cmd;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setApplicationVersion(QStringLiteral(KCONFIG_VERSION_STRING));

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.setApplicationDescription(QCoreApplication::translate("main", "KDE Tool for updating user configuration files"));
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList{QStringLiteral("debug")}, QCoreApplication::translate("main", "Keep output results from scripts")));
    parser.addOption(QCommandLineOption(
        QStringList{QStringLiteral("testmode")},
        QCoreApplication::translate("main", "For unit tests only: do not write the done entries, so that with every re-run, the scripts are executed again")));
    parser.addOption(QCommandLineOption(QStringList{QStringLiteral("check")},
                                        QCoreApplication::translate("main", "Check whether config file itself requires updating"),
                                        QStringLiteral("update-file")));
    parser.addPositionalArgument(QStringLiteral("files"),
                                 QCoreApplication::translate("main", "File(s) to read update instructions from"),
                                 QStringLiteral("[files...]"));

    // TODO aboutData.addAuthor(ki18n("Waldo Bastian"), KLocalizedString(), "bastian@kde.org");

    parser.process(app);
    KonfUpdate konfUpdate(&parser);

    return 0;
}
