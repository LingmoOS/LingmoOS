/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "pushpulldialog.h"

#include <QProcess>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KTextEditor/MainWindow>
#include <KTextEditor/View>

#include <gitprocess.h>
#include <hostprocess.h>
#include <ktexteditor_utils.h>

PushPullDialog::PushPullDialog(KTextEditor::MainWindow *mainWindow, const QString &repoPath)
    : HUDDialog(nullptr, mainWindow->window())
    , m_repo(repoPath)
{
    m_lineEdit.setFont(Utils::editorFont());
    m_treeView.setFont(Utils::editorFont());
    setFilteringEnabled(false);
    loadLastExecutedCommands();
}

void PushPullDialog::openDialog(PushPullDialog::Mode m)
{
    // build the string
    QString builtString = m == Push ? buildPushString() : buildPullString();
    // find if we have a last executed push/pull command
    QString lastCmd = getLastPushPullCmd(m);

    QStringList lastExecCmds = m_lastExecutedCommands;

    if (!lastExecCmds.contains(builtString)) {
        lastExecCmds.push_front(builtString);
    }

    // if found, bring it up
    if (!lastCmd.isEmpty()) {
        lastExecCmds.removeAll(lastCmd);
        lastExecCmds.push_front(lastCmd);
    }

    setStringList(lastExecCmds);

    connect(m_treeView.selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex &current, const QModelIndex &) {
        m_lineEdit.setText(current.data().toString());
    });

    reselectFirst();

    exec();
}

QString PushPullDialog::getLastPushPullCmd(Mode m) const
{
    const QString cmdToFind = m == Push ? QStringLiteral("git push") : QStringLiteral("git pull");
    QString found;
    for (const auto &cmd : m_lastExecutedCommands) {
        if (cmd.startsWith(cmdToFind)) {
            found = cmd;
            break;
        }
    }
    return found;
}

void PushPullDialog::loadLastExecutedCommands()
{
    KConfigGroup config(KSharedConfig::openConfig(), "kategit");
    m_lastExecutedCommands = config.readEntry("lastExecutedGitCmds", QStringList());
}

void PushPullDialog::saveCommand(const QString &command)
{
    KConfigGroup config(KSharedConfig::openConfig(), "kategit");
    QStringList cmds = m_lastExecutedCommands;
    cmds.removeAll(command);
    cmds.push_front(command);
    while (cmds.size() > 8) {
        cmds.pop_back();
    }
    config.writeEntry("lastExecutedGitCmds", cmds);
}

/**
 * This is not for display, hence not reusing gitutils here
 */
static QString currentBranchName(const QString &repo)
{
    QProcess git;
    if (!setupGitProcess(git, repo, {QStringLiteral("symbolic-ref"), QStringLiteral("--short"), QStringLiteral("HEAD")})) {
        return {};
    }

    startHostProcess(git, QIODevice::ReadOnly);
    if (git.waitForStarted() && git.waitForFinished(-1)) {
        if (git.exitStatus() == QProcess::NormalExit && git.exitCode() == 0) {
            return QString::fromUtf8(git.readAllStandardOutput().trimmed());
        }
    }
    // give up
    return {};
}

static QStringList remotesList(const QString &repo)
{
    QProcess git;
    if (!setupGitProcess(git, repo, {QStringLiteral("remote")})) {
        return {};
    }

    startHostProcess(git, QIODevice::ReadOnly);
    if (git.waitForStarted() && git.waitForFinished(-1)) {
        if (git.exitStatus() == QProcess::NormalExit && git.exitCode() == 0) {
            return QString::fromUtf8(git.readAllStandardOutput()).split(QLatin1Char('\n'));
        }
    }
    return {};
}

QString PushPullDialog::buildPushString()
{
    auto br = currentBranchName(m_repo);
    if (br.isEmpty()) {
        return QStringLiteral("git push");
    }

    auto remotes = remotesList(m_repo);
    if (!remotes.contains(QStringLiteral("origin"))) {
        return QStringLiteral("git push");
    }

    return QStringLiteral("git push %1 %2").arg(QStringLiteral("origin")).arg(br);
}

QString PushPullDialog::buildPullString()
{
    auto br = currentBranchName(m_repo);
    if (br.isEmpty()) {
        return QStringLiteral("git pull");
    }

    auto remotes = remotesList(m_repo);
    if (!remotes.contains(QStringLiteral("origin"))) {
        return QStringLiteral("git pull");
    }

    return QStringLiteral("git pull %1 %2").arg(QStringLiteral("origin")).arg(br);
}

void PushPullDialog::slotReturnPressed(const QModelIndex &)
{
    if (!m_lineEdit.text().isEmpty()) {
        auto args = m_lineEdit.text().split(QLatin1Char(' '));
        if (args.first() == QStringLiteral("git")) {
            saveCommand(m_lineEdit.text());
            args.pop_front();
            Q_EMIT runGitCommand(args);
        }
    }

    hide();
}
