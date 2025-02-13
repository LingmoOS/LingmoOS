/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef GITUTILS_H
#define GITUTILS_H

#include <QString>
#include <functional>
#include <optional>

namespace GitUtils
{
enum RefType {
    Head = 0x1,
    Remote = 0x2,
    Tag = 0x4,
    All = 0x7,
};

/**
 * @brief Represents a branch
 */
struct Branch {
    /** Branch name */
    QString name;
    /** remote name, will be empty for local branches */
    QString remote;
    /** Ref type @see RefType */
    RefType type;
    /** last commit on this branch, may be empty **/
    QString lastCommit;
};

struct Result {
    QString error;
    int returnCode;
};

struct CheckoutResult : public Result {
    QString branch;
};

struct StatusEntry {
    QString file;
    char x;
    char y;
};

/**
 * @brief check if @p repo is a git repo
 * @param repo is path to the repo
 * @return
 */
bool isGitRepo(const QString &repo);

/**
 * @brief get name of current branch in @p repo
 */
QString getCurrentBranchName(const QString &repo);

/**
 * @brief checkout to @p branch in @p repo
 */
CheckoutResult checkoutBranch(const QString &repo, const QString &branch);

/**
 * @brief checkout to new @p branch in @p repo from @p fromBranch
 */
CheckoutResult checkoutNewBranch(const QString &repo, const QString &newBranch, const QString &fromBranch = QString());

/**
 * @brief get all local and remote branches
 */
QVector<Branch> getAllBranches(const QString &repo);

/**
 * @brief get all local and remote branches + tags
 */
QVector<Branch> getAllBranchesAndTags(const QString &repo, RefType ref = RefType::All);

/**
 * @brief get all local branches with last commit
 */
QVector<Branch> getAllLocalBranchesWithLastCommitSubject(const QString &repo);

std::pair<QString, QString> getLastCommitMessage(const QString &repo);

Result deleteBranches(const QStringList &branches, const QString &repo);
}

Q_DECLARE_TYPEINFO(GitUtils::Branch, Q_MOVABLE_TYPE);

#endif // GITUTILS_H
