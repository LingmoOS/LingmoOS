/*
    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "gitstatus.h"

#include <bytearraysplitter.h>
#include <gitprocess.h>

#include <KLocalizedString>
#include <QByteArray>
#include <QProcess>

#include <charconv>
#include <optional>

static void numStatForStatus(QVector<GitUtils::StatusItem> &list, const QString &workDir, bool modified)
{
    const auto args = modified ? QStringList{QStringLiteral("diff"), QStringLiteral("--numstat"), QStringLiteral("-z")}
                               : QStringList{QStringLiteral("diff"), QStringLiteral("--numstat"), QStringLiteral("--staged"), QStringLiteral("-z")};

    QProcess git;
    if (!setupGitProcess(git, workDir, args)) {
        return;
    }
    startHostProcess(git, QProcess::ReadOnly);
    if (git.waitForStarted() && git.waitForFinished(-1)) {
        if (git.exitStatus() != QProcess::NormalExit || git.exitCode() != 0) {
            return;
        }
    }

    GitUtils::parseDiffNumStat(list, git.readAllStandardOutput());
}

GitUtils::GitParsedStatus GitUtils::parseStatus(const QByteArray &raw, bool withNumStat, const QString &workingDir)
{
    QVector<GitUtils::StatusItem> untracked;
    QVector<GitUtils::StatusItem> unmerge;
    QVector<GitUtils::StatusItem> staged;
    QVector<GitUtils::StatusItem> changed;

    for (auto r : ByteArraySplitter(raw, '\0')) {
        if (r.length() < 3) {
            continue;
        }

        char x = r.at(0);
        char y = r.at(1);
        uint16_t xy = (((uint16_t)x) << 8) | y;
        using namespace GitUtils;

        r.remove_prefix(3);
        QByteArray file = r.toByteArray();

        switch (xy) {
        case StatusXY::QQ:
            untracked.append({file, GitStatus::Untracked, 'U', 0, 0});
            break;
        case StatusXY::II:
            untracked.append({file, GitStatus::Ignored, 'I', 0, 0});
            break;

        case StatusXY::DD:
            unmerge.append({file, GitStatus::Unmerge_BothDeleted, x, 0, 0});
            break;
        case StatusXY::AU:
            unmerge.append({file, GitStatus::Unmerge_AddedByUs, x, 0, 0});
            break;
        case StatusXY::UD:
            unmerge.append({file, GitStatus::Unmerge_DeletedByThem, x, 0, 0});
            break;
        case StatusXY::UA:
            unmerge.append({file, GitStatus::Unmerge_AddedByThem, x, 0, 0});
            break;
        case StatusXY::DU:
            unmerge.append({file, GitStatus::Unmerge_DeletedByUs, x, 0, 0});
            break;
        case StatusXY::AA:
            unmerge.append({file, GitStatus::Unmerge_BothAdded, x, 0, 0});
            break;
        case StatusXY::UU:
            unmerge.append({file, GitStatus::Unmerge_BothModified, x, 0, 0});
            break;
        }

        switch (x) {
        case 'M':
            staged.append({file, GitStatus::Index_Modified, x, 0, 0});
            break;
        case 'A':
            staged.append({file, GitStatus::Index_Added, x, 0, 0});
            break;
        case 'D':
            staged.append({file, GitStatus::Index_Deleted, x, 0, 0});
            break;
        case 'R':
            staged.append({file, GitStatus::Index_Renamed, x, 0, 0});
            break;
        case 'C':
            staged.append({file, GitStatus::Index_Copied, x, 0, 0});
            break;
        }

        switch (y) {
        case 'M':
            changed.append({file, GitStatus::WorkingTree_Modified, y, 0, 0});
            break;
        case 'D':
            changed.append({file, GitStatus::WorkingTree_Deleted, y, 0, 0});
            break;
        case 'A':
            changed.append({file, GitStatus::WorkingTree_IntentToAdd, y, 0, 0});
            break;
        }
    }

    if (withNumStat) {
        numStatForStatus(changed, workingDir, true);
        numStatForStatus(staged, workingDir, false);
    }

    return {untracked, unmerge, staged, changed};
}

QString GitUtils::statusString(GitUtils::GitStatus s)
{
    switch (s) {
    case WorkingTree_Modified:
    case Index_Modified:
        return i18n(" ‣ Modified");
    case Untracked:
        return i18n(" ‣ Untracked");
    case Index_Renamed:
        return i18n(" ‣ Renamed");
    case Index_Deleted:
    case WorkingTree_Deleted:
        return i18n(" ‣ Deleted");
    case Index_Added:
    case WorkingTree_IntentToAdd:
        return i18n(" ‣ Added");
    case Index_Copied:
        return i18n(" ‣ Copied");
    case Ignored:
        return i18n(" ‣ Ignored");
    case Unmerge_AddedByThem:
    case Unmerge_AddedByUs:
    case Unmerge_BothAdded:
    case Unmerge_BothDeleted:
    case Unmerge_BothModified:
    case Unmerge_DeletedByThem:
    case Unmerge_DeletedByUs:
        return i18n(" ‣ Conflicted");
    }
    return QString();
}

static void addNumStat(QVector<GitUtils::StatusItem> &items, int add, int sub, std::string_view file)
{
    // look in modified first, then staged
    auto item = std::find_if(items.begin(), items.end(), [file](const GitUtils::StatusItem &si) {
        return file.compare(0, si.file.size(), si.file.data()) == 0;
    });
    if (item != items.end()) {
        item->linesAdded = add;
        item->linesRemoved = sub;
        return;
    }
}

static std::optional<int> toInt(std::string_view s)
{
    int value{};
    auto res = std::from_chars(s.data(), s.data() + s.size(), value);
    if (res.ptr == (s.data() + s.size())) {
        return value;
    }
    return std::nullopt;
}

void GitUtils::parseDiffNumStat(QVector<GitUtils::StatusItem> &items, const QByteArray &raw)
{
    // format:
    // 12\t10\tFileName
    // 12 = add, 10 = sub, fileName at the end
    for (auto line : ByteArraySplitter(raw, '\0')) {
        size_t addEnd = line.find_first_of('\t');
        if (addEnd == std::string_view::npos) {
            continue;
        }

        size_t subStart = line.find_first_not_of('\t', addEnd);
        if (subStart == std::string_view::npos) {
            continue;
        }

        size_t subEnd = line.find_first_of('\t', subStart);
        if (subEnd == std::string_view::npos) {
            continue;
        }

        std::string_view addStr = line.substr(0, addEnd);
        std::string_view subStr = line.substr(subStart, subEnd - subStart);
        std::string_view fileStr = line.substr(subEnd + 1, line.size() - (subEnd + 1));

        auto add = toInt(addStr);
        auto sub = toInt(subStr);

        if (!add.has_value()) {
            continue;
        }
        if (!sub.has_value()) {
            continue;
        }

        addNumStat(items, add.value(), sub.value(), fileStr);
    }
}

QVector<GitUtils::StatusItem> GitUtils::parseDiffNameStatus(const QByteArray &raw)
{
    QVector<GitUtils::StatusItem> out;
    for (auto l : ByteArraySplitter(raw, '\n')) {
        ByteArraySplitter splitter(l, '\t');
        if (splitter.empty()) {
            continue;
        }
        auto it = splitter.begin();
        GitUtils::StatusItem i;
        i.statusChar = (*it).at(0);

        ++it;
        if (it == splitter.end()) {
            continue;
        }
        i.file = (*it).toByteArray();
        out.append(i);
    }
    return out;
}
