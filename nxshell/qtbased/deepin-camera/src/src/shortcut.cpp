// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcut.h"

Shortcut::Shortcut(QObject *parent)
    : QObject(parent)
{
    ShortcutGroup group1;
    ShortcutGroup group3;

    group1.groupName = tr("Camera");
    group3.groupName = tr("Settings");
    group1.groupItems << ShortcutItem(tr("Stop recording"), "space")
                      << ShortcutItem(tr("Stop taking photos"), "space")
                      << ShortcutItem(tr("Context menu"), "Alt + M")
                      << ShortcutItem(tr("Copy"), "Ctrl + C")
                      << ShortcutItem(tr("Delete"), "Delete")
                      << ShortcutItem(tr("Open"), "Ctrl + O")
                      << ShortcutItem(tr("Print"), "Ctrl + P");
    group3.groupItems << ShortcutItem(tr("Help"), "F1")
                      << ShortcutItem(tr("Display shortcuts"), "Ctrl + Shift + ?");
    m_shortcutGroups << group1 << group3;

    // convert to json object
    QJsonArray jsonGroups;
    for (auto scg : m_shortcutGroups) {
        QJsonObject jsonGroup;
        jsonGroup.insert("groupName", scg.groupName);
        QJsonArray jsonGroupItems;
        for (auto sci : scg.groupItems) {
            QJsonObject jsonItem;
            jsonItem.insert("name", sci.name);
            jsonItem.insert("value", sci.value);
            jsonGroupItems.append(jsonItem);
        }
        jsonGroup.insert("groupItems", jsonGroupItems);
        jsonGroups.append(jsonGroup);
    }

    m_shortcutObj.insert("shortcut", jsonGroups);
}

QString Shortcut::toStr()
{
    QJsonDocument doc(m_shortcutObj);
    return doc.toJson().data();
}
