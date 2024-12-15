// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "applicationHooks.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

std::optional<ApplicationHook> ApplicationHook::loadFromFile(const QString &filePath) noexcept
{
    auto index = filePath.lastIndexOf(QDir::separator());
    auto fileName = filePath.last(filePath.size() - index - 1);

    QFile file{filePath};
    if (!file.open(QFile::Text | QFile::ReadOnly | QFile::ExistingOnly)) {
        qWarning() << "open hook file:" << filePath << "failed:" << file.errorString() << ", skip.";
        return std::nullopt;
    }

    auto content = file.readAll();
    QJsonParseError err;
    auto json = QJsonDocument::fromJson(content, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "parse hook failed:" << err.errorString();
        return std::nullopt;
    }
    if (json.isEmpty()) {
        qWarning() << "empty hook, skip.";
        return std::nullopt;
    }
    auto obj = json.object();

    auto it = obj.constFind("Exec");
    if (it == obj.constEnd()) {
        qWarning() << "invalid hook: lack of Exec.";
        return std::nullopt;
    }
    auto exec = it->toString();
    QFileInfo info{exec};
    if (!(info.exists() and info.isExecutable())) {
        qWarning() << "exec maybe doesn't exists or be executed.";
        return std::nullopt;
    }

    it = obj.constFind("Args");
    if (it == obj.constEnd()) {
        qWarning() << "invalid hook: lack of Args.";
        return std::nullopt;
    }
    auto args = it->toVariant().toStringList();

    return ApplicationHook{std::move(fileName), std::move(exec), std::move(args)};
}

QStringList generateHooks(const QList<ApplicationHook> &hooks) noexcept
{
    QStringList ret;
    for (const auto &hook : hooks) {
        ret.append(hook.execPath());
        ret.append(hook.args());
    }
    return ret;
}
