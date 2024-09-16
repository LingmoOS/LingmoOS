// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "permissionoption.h"

#include <QObject>
#include <QDebug>

struct OptionData {
    int optionId;
    QString optionName;
    QString tsText;
    bool isNeedSaved;

};

PermissionOption::PermissionOption()
{
    m_data.insert("allow", {0, "allow", QObject::tr("Allow"), true});
    m_data.insert("deny", {1, "deny", QObject::tr("Deny"), true});
    m_data.insert("allow_once", {2, "allow_once", QObject::tr("Allow Once"), false});
    m_data.insert("deny_once", {3, "deny_once", QObject::tr("Deny Once"), false});
}

PermissionOption *PermissionOption::getInstance()
{
    static PermissionOption instance;
    return &instance;
}

QString PermissionOption::getTs(QString optionName)
{
    if (m_data.contains(optionName)) {
        return m_data.find(optionName).value().tsText;
    }
    return "";
}

int PermissionOption::getId(QString optionName)
{
    if (m_data.contains(optionName)) {
        return m_data.find(optionName).value().optionId;
    }
    return -1;
}

bool PermissionOption::isNeedSaved(QString optionName)
{
    if (m_data.contains(optionName)) {
        return m_data.find(optionName).value().isNeedSaved;
    }
    return false;
}

bool PermissionOption::isNeedSavedByIndex(int index)
{
    QMapIterator<QString, OptionData> data(m_data);
    while (data.hasNext()) {
        data.next();
        if (data.value().optionId == index) {
            return data.value().isNeedSaved;
        }
    }

    return false;
}
