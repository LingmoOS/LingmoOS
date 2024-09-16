// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <pwd.h>
#include <vector>
#include <cstring>
#include "global.h"
#include "launchoptions.h"

QStringList generateCommand(const QVariantMap &props) noexcept
{
    std::vector<std::unique_ptr<LaunchOption>> options;
    for (auto it = props.constKeyValueBegin(); it != props.constKeyValueEnd(); ++it) {
        const auto &[key, value] = *it;
        if (key == setUserLaunchOption::key()) {
            options.emplace_back(std::make_unique<setUserLaunchOption>(value));
        } else if (key == setEnvLaunchOption::key()) {
            options.emplace_back(std::make_unique<setEnvLaunchOption>(value));
        } else if (key == unsetEnvLaunchOption::key()) {
            options.emplace_back(std::make_unique<unsetEnvLaunchOption>(value));
        } else if (key == hookLaunchOption::key()) {
            options.emplace_back(std::make_unique<hookLaunchOption>(value));
        } else if (key == setWorkingPathLaunchOption::key()) {
            options.emplace_back(std::make_unique<setWorkingPathLaunchOption>(value));
        } else if (key == builtInSearchExecOption::key()) {
            options.emplace_back(std::make_unique<builtInSearchExecOption>(value));
        } else {
            qWarning() << "unsupported options" << key;
        }
    }

    options.push_back(std::make_unique<splitLaunchOption>());

    std::sort(options.begin(),
              options.end(),
              [](const std::unique_ptr<LaunchOption> &lOption, const std::unique_ptr<LaunchOption> &rOption) {
                  if (lOption->type() == rOption->type()) {
                      return lOption->m_priority >= rOption->m_priority;
                  }

                  if (lOption->type() == AppExecOption and rOption->type() == systemdOption) {
                      return false;
                  }

                  if (lOption->type() == AppExecOption and rOption->type() == splitOption) {
                      return false;
                  }

                  if (lOption->type() == splitOption and rOption->type() == systemdOption) {
                      return false;
                  }

                  return true;
              });

    QStringList ret;
    std::for_each(options.begin(), options.end(), [&ret](const std::unique_ptr<LaunchOption> &option) {
        auto str = option->generateCommandLine();
        if (!str.isEmpty()) {
            ret.append(std::move(str));
        }
    });

    return ret;
}

QStringList setUserLaunchOption::generateCommandLine() const noexcept
{
    QStringList ret;

    bool ok{false};
    auto uid = m_val.toUInt(&ok);
    if (!ok) {
        return ret;
    }

    auto curUID = getCurrentUID();

    if (uid == curUID) {
        return ret;
    }

    ret.append("pkexec");
    ret.append("--user");

    struct passwd *destUser = getpwuid(uid);
    QString userName = destUser->pw_name;
    ret.append(userName);

    ret.append("env");
    ret.append("DISPLAY=:0");

    auto authFile = qgetenv("XAUTHORITY");
    ret.append(QString{"XAUTHORITY=%1"}.arg(authFile));

    return ret;
}

QStringList splitLaunchOption::generateCommandLine() const noexcept
{
    return QStringList{m_val.toString()};
}

QStringList setWorkingPathLaunchOption::generateCommandLine() const noexcept
{
    auto str = m_val.toString();
    if (str.isEmpty()) {
        return {};
    }

    return QStringList{QString{"--WorkingDirectory=%1"}.arg(str)};
}

QStringList StringListLaunchOption::generateCommandLine() const noexcept
{
    auto list = m_val.toStringList();
    if (list.isEmpty()) {
        return {};
    }

    QStringList ret;
    const QString ok = optionKey();
    for (const auto &ov : list) {
        ret << QString{"%1=%2"}.arg(ok).arg(ov);
    }

    return ret;
}
