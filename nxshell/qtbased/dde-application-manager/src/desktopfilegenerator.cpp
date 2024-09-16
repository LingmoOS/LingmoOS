// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <QStringBuilder>

#include "desktopfilegenerator.h"
#include "desktopfileparser.h"

bool DesktopFileGenerator::checkValidation(const QVariantMap &desktopFile, QString &err) noexcept
{
    if (!desktopFile.contains("Type") or !desktopFile.contains("Name")) {
        err = "required key doesn't exists.";
        return false;
    }

    auto type = qdbus_cast<QString>(desktopFile["Type"]);
    if (type.isEmpty()) {
        err = "Type's type is invalid";
        return false;
    }

    if (type == "Link" and !desktopFile.contains("URL")) {
        err = "URL must be set when Type is 'Link'";
        return false;
    }
    return true;
}

int DesktopFileGenerator::processMainGroupLocaleEntry(DesktopEntry::container_type::iterator mainEntry,
                                                      const QString &key,
                                                      const QVariant &value) noexcept
{
    if (key == "ActionName") {
        return 1;
    }

    if (key == "Name") {
        const auto &nameMap = qdbus_cast<QStringMap>(value);
        if (nameMap.isEmpty()) {
            qDebug() << "Name's type mismatch:" << nameMap;
            return -1;
        }

        mainEntry->insert("Name", QVariant::fromValue(nameMap));
        return 1;
    }

    if (key == "Icon") {
        const auto &iconMap = qdbus_cast<QStringMap>(value);
        if (auto icon = iconMap.constFind(DesktopFileDefaultKeyLocale); icon != iconMap.cend() and !icon->isEmpty()) {
            mainEntry->insert("Icon", *icon);
        }
        return 1;
    }

    if (key == "Exec") {
        const auto &execMap = qdbus_cast<QStringMap>(value);
        if (auto exec = execMap.constFind(DesktopFileDefaultKeyLocale); exec != execMap.cend() and !exec->isEmpty()) {
            mainEntry->insert("Exec", *exec);
        }
        return 1;
    }

    return 0;
}

bool DesktopFileGenerator::processMainGroup(DesktopEntry::container_type &content, const QVariantMap &rawValue) noexcept
{
    auto mainEntry = content.insert(DesktopFileEntryKey, {});
    for (auto it = rawValue.constKeyValueBegin(); it != rawValue.constKeyValueEnd(); ++it) {
        const auto &[key, value] = *it;

        if (mainEntry->contains(key)) {
            qDebug() << "duplicate key:" << key << ",skip";
            return false;
        }

        auto ret = processMainGroupLocaleEntry(mainEntry, key, value);
        if (ret == 1) {
            continue;
        }

        if (ret == -1) {
            return false;
        }

        mainEntry->insert(key, value);
    }

    mainEntry->insert("X-Deepin-CreateBy", QString{"dde-application-manager"});
    return true;
}

bool DesktopFileGenerator::processActionGroup(QStringList actions,
                                              DesktopEntry::container_type &content,
                                              const QVariantMap &rawValue) noexcept
{
    actions.removeDuplicates();
    if (actions.isEmpty()) {
        qDebug() << "empty actions";
        return false;
    }

    auto nameMap = qdbus_cast<QVariantMap>(rawValue["ActionName"]);
    if (nameMap.isEmpty()) {
        qDebug() << "ActionName's type mismatch.";
        return false;
    }

    QStringMap iconMap;
    if (auto actionIcon = rawValue.constFind("Icon"); actionIcon != rawValue.cend()) {
        iconMap = qdbus_cast<QStringMap>(*actionIcon);
        if (iconMap.isEmpty()) {
            qDebug() << "Icon's type mismatch.";
            return false;
        }
    }

    QStringMap execMap;
    if (auto actionExec = rawValue.constFind("Exec"); actionExec != rawValue.cend()) {
        execMap = qdbus_cast<QStringMap>(*actionExec);
        if (execMap.isEmpty()) {
            qDebug() << "Exec's type mismatch:" << actionExec->typeName();
            return false;
        }
    }

    for (const auto &action : actions) {
        if (action.isEmpty()) {
            qDebug() << "action's content is empty. skip";
            continue;
        }

        if (!nameMap.contains(action)) {
            qDebug() << "couldn't find actionName, current action:" << action;
            return false;
        }

        auto actionGroup = content.insert(DesktopFileActionKey % action, {});
        auto curVal = qdbus_cast<QStringMap>(nameMap[action]);
        if (curVal.isEmpty()) {
            qDebug() << "inner type of actionName is mismatched";
            return false;
        }
        actionGroup->insert("Name", QVariant::fromValue(curVal));

        if (auto actionIcon = iconMap.constFind(action); actionIcon != iconMap.cend() and !actionIcon->isEmpty()) {
            actionGroup->insert("Icon", iconMap[action]);
        }
        if (auto actionExec = execMap.constFind(action); actionExec != execMap.cend() and !actionExec->isEmpty()) {
            actionGroup->insert("Exec", execMap[action]);
        }
    };

    return true;
}

QString DesktopFileGenerator::generate(const QVariantMap &desktopFile, QString &err) noexcept
{
    DesktopEntry::container_type content{};
    if (auto actions = desktopFile.find("Actions"); actions != desktopFile.end()) {
        if (!desktopFile.contains("ActionName")) {
            err = "'ActionName' doesn't exists";
            return {};
        }

        if (!processActionGroup(actions->toStringList(), content, desktopFile)) {
            err = "please check action group";
            return {};
        }
    }

    if (!processMainGroup(content, desktopFile)) {
        err = "please check main group.";
        return {};
    }

    auto fileContent = toString(content);
    if (fileContent.isEmpty()) {
        err = "couldn't convert to desktop file.";
        return {};
    }

    return fileContent;
}
