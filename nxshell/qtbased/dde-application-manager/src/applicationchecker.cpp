// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "constant.h"
#include "applicationchecker.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

bool ApplicationFilter::hiddenCheck(const DesktopEntry &entry) noexcept
{
    bool hidden{false};
    auto hiddenVal = entry.value(DesktopFileEntryKey, DesktopEntryHidden);

    if (hiddenVal.has_value()) {
        bool ok{false};
        hidden = toBoolean(hiddenVal.value(), ok);
        if (!ok) {
            qWarning() << "invalid hidden value:" << hiddenVal.value();
            return false;
        }
    }
    return hidden;
}

bool ApplicationFilter::tryExecCheck(const DesktopEntry &entry) noexcept
{
    auto tryExecVal = entry.value(DesktopFileEntryKey, "TryExec");
    if (tryExecVal.has_value()) {
        auto executable = toString(tryExecVal.value());
        if (executable.isEmpty()) {
            qWarning() << "invalid TryExec value:" << tryExecVal.value();
            return false;
        }

        if (executable.startsWith(QDir::separator())) {
            QFileInfo info{executable};
            return !(info.exists() and info.isExecutable());
        }
        return QStandardPaths::findExecutable(executable).isEmpty();
    }

    return false;
}

bool ApplicationFilter::showInCheck(const DesktopEntry &entry) noexcept
{
    auto desktops = QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP")).split(':', Qt::SkipEmptyParts);
    if (desktops.isEmpty()) {
        return true;
    }
    desktops.removeDuplicates();

    bool showInCurrentDE{true};
    auto onlyShowInVal = entry.value(DesktopFileEntryKey, "OnlyShowIn");
    while (onlyShowInVal.has_value()) {
        auto deStr = toString(onlyShowInVal.value());
        if (deStr.isEmpty()) {
            qWarning() << "invalid OnlyShowIn value:" << onlyShowInVal.value();
            break;
        }

        auto des = deStr.split(';', Qt::SkipEmptyParts);
        if (des.contains("DDE", Qt::CaseInsensitive)) {
            des.append("deepin");
        } else if (des.contains("deepin", Qt::CaseInsensitive)) {
            des.append("DDE");
        }
        des.removeDuplicates();

        showInCurrentDE = std::any_of(
            des.cbegin(), des.cend(), [&desktops](const QString &str) { return desktops.contains(str, Qt::CaseInsensitive); });
        break;
    }

    bool notShowInCurrentDE{false};
    auto notShowInVal = entry.value(DesktopFileEntryKey, "NotShowIn");
    while (notShowInVal.has_value()) {
        auto deStr = toString(notShowInVal.value());
        if (deStr.isEmpty()) {
            qWarning() << "invalid OnlyShowIn value:" << notShowInVal.value();
            break;
        }

        auto des = deStr.split(';', Qt::SkipEmptyParts);
        if (des.contains("DDE", Qt::CaseInsensitive)) {
            des.append("deepin");
        } else if (des.contains("deepin", Qt::CaseInsensitive)) {
            des.append("DDE");
        }
        des.removeDuplicates();

        notShowInCurrentDE = std::any_of(
            des.cbegin(), des.cend(), [&desktops](const QString &str) { return desktops.contains(str, Qt::CaseInsensitive); });
        break;
    }

    return !showInCurrentDE or notShowInCurrentDE;
}

bool ApplicationFilter::hiddenCheck(const std::unique_ptr<DesktopEntry> &entry) noexcept
{
    return hiddenCheck(*entry);
}

bool ApplicationFilter::tryExecCheck(const std::unique_ptr<DesktopEntry> &entry) noexcept
{
    return tryExecCheck(*entry);
}

bool ApplicationFilter::showInCheck(const std::unique_ptr<DesktopEntry> &entry) noexcept
{
    return showInCheck(*entry);
}
