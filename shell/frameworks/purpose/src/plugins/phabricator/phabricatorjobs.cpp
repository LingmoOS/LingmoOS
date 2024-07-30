/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "phabricatorjobs.h"
#include "debug.h"

#include <QDir>
#include <QRegularExpression>
#include <QStandardPaths>

#include <KLocalizedString>

const char s_colourCodes[] = "\u001B[[0-9]*m";

using namespace Phabricator;

bool DifferentialRevision::buildArcCommand(const QString &workDir, const QString &patchFile, bool doBrowse)
{
    bool ret;
    QString arc = QStandardPaths::findExecutable(QStringLiteral("arc"));
    if (!arc.isEmpty()) {
        QStringList args;
        args << QStringLiteral("diff");
        if (m_id.isEmpty()) {
            // creating a new differential revision (review request)
            // the fact we skip "--create" means we'll be creating a new "differential diff"
            // which obliges the user to fill in the details we cannot provide through the plugin ATM.
            // TODO: grab the TARGET_GROUPS from .reviewboardrc and pass that via --reviewers
        } else {
            // updating an existing differential revision (review request)
            args << QStringLiteral("--update") << m_id;
        }
        args << QStringLiteral("--excuse") << QStringLiteral("patch submitted with the purpose/phabricator plugin");
        if (m_commit.isEmpty()) {
            args << QStringLiteral("--raw");
        } else {
            args << QStringLiteral("--allow-untracked") << QStringLiteral("--ignore-unsound-tests") << QStringLiteral("--nolint") << QStringLiteral("-nounit")
                 << QStringLiteral("--verbatim") << m_commit;
        }
        if (doBrowse) {
            args << QStringLiteral("--browse");
        }
        m_arcCmd.setProgram(arc);
        m_arcCmd.setArguments(args);
        if (!patchFile.isEmpty()) {
            m_arcCmd.setStandardInputFile(patchFile);
            m_arcInput = patchFile;
        }
        m_arcCmd.setWorkingDirectory(workDir);
        connect(&m_arcCmd, &QProcess::finished, this, &DifferentialRevision::done);
        setPercent(33);
        ret = true;
    } else {
        qCWarning(PLUGIN_PHABRICATOR) << "Could not find 'arc' in the path";
        setError(KJob::UserDefinedError + 3);
        setErrorText(i18n("Could not find the 'arc' command"));
        setErrorString(errorText());
        ret = false;
    }
    return ret;
}

void DifferentialRevision::start()
{
    if (!m_arcCmd.program().isEmpty()) {
        qCDebug(PLUGIN_PHABRICATOR) << "starting" << m_arcCmd.program() << m_arcCmd.arguments();
        qCDebug(PLUGIN_PHABRICATOR) << "\twordDir=" << m_arcCmd.workingDirectory() << "stdin=" << m_arcInput;
        m_arcCmd.start();
        if (m_arcCmd.waitForStarted(5000)) {
            setPercent(66);
        }
    }
}

void DifferentialRevision::setErrorString(const QString &msg)
{
    QRegularExpression unwanted(QString::fromUtf8(s_colourCodes));
    m_errorString = msg;
    m_errorString.replace(unwanted, QString());
}

QString DifferentialRevision::scrubbedResult()
{
    QString result = QString::fromUtf8(m_arcCmd.readAllStandardOutput());
    // the return string can contain terminal text colour codes: remove them.
    QRegularExpression unwanted(QString::fromUtf8(s_colourCodes));
    result.replace(unwanted, QString());
    return result;
}

QStringList DifferentialRevision::scrubbedResultList()
{
    QStringList result = QString::fromUtf8(m_arcCmd.readAllStandardOutput()).split(QChar::LineFeed);
    // the return string can contain terminal text colour codes: remove them.
    QRegularExpression unwanted(QString::fromUtf8(s_colourCodes));
    result.replaceInStrings(unwanted, QString());
    // remove all (now) empty strings
    result.removeAll(QString());
    return result;
}

NewDiffRev::NewDiffRev(const QUrl &patch, const QString &projectPath, bool doBrowse, QObject *parent)
    : DifferentialRevision(QString(), parent)
    , m_patch(patch)
    , m_project(projectPath)
{
    buildArcCommand(projectPath, patch.toLocalFile(), doBrowse);
}

void NewDiffRev::done(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::NormalExit || exitCode) {
        setError(KJob::UserDefinedError + exitCode);
        setErrorText(i18n("Could not create the new \"differential diff\""));
        setErrorString(QString::fromUtf8(m_arcCmd.readAllStandardError()));
        qCWarning(PLUGIN_PHABRICATOR) << "Could not create the new \"differential diff\":" << m_arcCmd.error() << ";" << errorString();
    } else {
        setPercent(99);
        const QString arcOutput = scrubbedResult();
        const char *diffOpCode = "Diff URI: ";
        int diffOffset = arcOutput.indexOf(QLatin1String(diffOpCode));
        if (diffOffset >= 0) {
            m_diffURI = arcOutput.mid(diffOffset + strlen(diffOpCode)).split(QChar::LineFeed).at(0);
        } else {
            m_diffURI = arcOutput;
        }
    }

    emitResult();
}

UpdateDiffRev::UpdateDiffRev(const QUrl &patch, const QString &basedir, const QString &id, const QString &updateComment, bool doBrowse, QObject *parent)
    : DifferentialRevision(id, parent)
    , m_patch(patch)
    , m_basedir(basedir)
{
    buildArcCommand(m_basedir, m_patch.toLocalFile(), doBrowse);
    QStringList args = m_arcCmd.arguments();
    if (updateComment.isEmpty()) {
        args << QStringLiteral("--message") << QStringLiteral("<placeholder: patch updated via the purpose/phabricator plugin>");
    } else {
        args << QStringLiteral("--message") << updateComment;
    }
    m_arcCmd.setArguments(args);
}

void UpdateDiffRev::done(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::NormalExit || exitCode) {
        setError(KJob::UserDefinedError + exitCode);
        setErrorText(i18n("Patch upload to Phabricator failed"));
        setErrorString(QString::fromUtf8(m_arcCmd.readAllStandardError()));
        qCWarning(PLUGIN_PHABRICATOR) << "Patch upload to Phabricator failed with exit code" << exitCode << ", error" << m_arcCmd.error() << ";"
                                      << errorString();
    } else {
        const QString arcOutput = scrubbedResult();
        const char *diffOpCode = "Revision URI: ";
        int diffOffset = arcOutput.indexOf(QLatin1String(diffOpCode));
        if (diffOffset >= 0) {
            m_diffURI = arcOutput.mid(diffOffset + strlen(diffOpCode)).split(QChar::LineFeed).at(0);
        } else {
            m_diffURI = arcOutput;
        }
    }
    emitResult();
}

DiffRevList::DiffRevList(const QString &projectDir, QObject *parent)
    : DifferentialRevision(QString(), parent)
    , m_projectDir(projectDir)
{
    buildArcCommand(m_projectDir);
}

bool Phabricator::DiffRevList::buildArcCommand(const QString &workDir, const QString &unused, bool)
{
    Q_UNUSED(unused)
    bool ret;
    QString arc = QStandardPaths::findExecutable(QStringLiteral("arc"));
    if (!arc.isEmpty()) {
        QStringList args;
        args << QStringLiteral("list");
        m_arcCmd.setProgram(arc);
        m_arcCmd.setArguments(args);
        m_arcCmd.setWorkingDirectory(workDir);
        connect(&m_arcCmd, &QProcess::finished, this, &DiffRevList::done);
        setPercent(33);
        ret = true;
    } else {
        qCWarning(PLUGIN_PHABRICATOR) << "Could not find 'arc' in the path";
        setError(KJob::UserDefinedError + 3);
        setErrorText(i18n("Could not find the 'arc' command"));
        setErrorString(errorText());
        ret = false;
    }
    return ret;
}

void DiffRevList::done(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::NormalExit || exitCode) {
        setError(KJob::UserDefinedError + exitCode);
        setErrorText(i18n("Could not get list of differential revisions in %1", QDir::currentPath()));
        setErrorString(QString::fromUtf8(m_arcCmd.readAllStandardError()));
        qCWarning(PLUGIN_PHABRICATOR) << "Could not get list of differential revisions" << m_arcCmd.error() << ";" << errorString();
    } else {
        setPercent(99);
        const QStringList reviews = scrubbedResultList();
        qCDebug(PLUGIN_PHABRICATOR) << "arc list returned:" << reviews;
        const QRegularExpression revIDExpr(QStringLiteral(" D[0-9][0-9]*: "));
        for (auto rev : reviews) {
            int idStart = rev.indexOf(revIDExpr);
            if (idStart >= 0) {
                QString revID = rev.mid(idStart + 1).split(QString::fromUtf8(": ")).at(0);
                QString revTitle = rev.section(revIDExpr, 1);
                if (rev.startsWith(QStringLiteral("* Accepted "))) {
                    // append a Unicode "Heavy Check Mark" to signal accepted revisions
                    revTitle += QStringLiteral(" ") + QString(QChar(0x2714));
                    m_statusMap[revTitle] = Accepted;
                } else if (rev.startsWith(QStringLiteral("* Needs Revision "))) {
                    // append a Unicode "Heavy Ballot X" for lack of a Unicode glyph
                    // resembling the icon used on the Phab site.
                    revTitle += QStringLiteral(" ") + QString(QChar(0x2718));
                    m_statusMap[revTitle] = NeedsRevision;
                } else if (rev.startsWith(QStringLiteral("* Needs Review "))) {
                    m_statusMap[revTitle] = NeedsReview;
                }
                m_reviews << qMakePair(revID, revTitle);
                m_revMap[revTitle] = revID;
            }
        }
    }
    emitResult();
}

#include "moc_phabricatorjobs.cpp"
