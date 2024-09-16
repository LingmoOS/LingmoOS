// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef APPLICATIONHOOKS_H
#define APPLICATIONHOOKS_H

#include <optional>
#include <QStringList>

class ApplicationHook
{
public:
    static std::optional<ApplicationHook> loadFromFile(const QString &filePath) noexcept;

    [[nodiscard]] const QString &hookName() const noexcept { return m_hookName; }
    [[nodiscard]] const QString &execPath() const noexcept { return m_execPath; }
    [[nodiscard]] const QStringList &args() const noexcept { return m_args; }

    friend bool operator>(const ApplicationHook &lhs, const ApplicationHook &rhs) { return lhs.m_hookName > rhs.m_hookName; };
    friend bool operator<(const ApplicationHook &lhs, const ApplicationHook &rhs) { return lhs.m_hookName < rhs.m_hookName; };
    friend bool operator==(const ApplicationHook &lhs, const ApplicationHook &rhs) { return lhs.m_hookName == rhs.m_hookName; };
    friend bool operator!=(const ApplicationHook &lhs, const ApplicationHook &rhs) { return !(lhs == rhs); };

private:
    ApplicationHook(QString &&hookName, QString &&execPath, QStringList &&args)
        : m_hookName(hookName)
        , m_execPath(std::move(execPath))
        , m_args(std::move(args))
    {
    }
    QString m_hookName;
    QString m_execPath;
    QStringList m_args;
};

QStringList generateHooks(const QList<ApplicationHook> &hooks) noexcept;

#endif
