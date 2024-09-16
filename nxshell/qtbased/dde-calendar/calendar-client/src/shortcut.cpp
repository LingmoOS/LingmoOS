// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "shortcut.h"

Shortcut::Shortcut(QObject *parent)
    : QObject(parent)
{
    ShortcutGroup group1;
    ShortcutGroup group2;
    ShortcutGroup group3;
    ShortcutGroup group4;

    group1.groupName = "";
    group1.groupItems << ShortcutItem(tr("Help"),  "F1") <<
                      ShortcutItem(tr("Delete event"),       "Delete");

    group2.groupName = "";
    group2.groupItems << ShortcutItem(tr("Copy"), "Ctrl+C") <<
                      ShortcutItem(tr("Cut"),  "Ctrl+X") <<
                      ShortcutItem(tr("Paste"), "Ctrl+V") ;

    group3.groupName = "";
    group3.groupItems << ShortcutItem(tr("Delete"), "Delete") <<
                      ShortcutItem(tr("Select all"), "Ctrl+A");
    m_shortcutGroups << group1 << group2 << group3;
    //convert to json object
    QJsonArray jsonGroups;
    for (auto scg : m_shortcutGroups) {
        QJsonObject jsonGroup;
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
