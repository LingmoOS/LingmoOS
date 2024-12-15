// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcut.h"

Shortcut::Shortcut(QObject *parent) : QObject(parent)
{
    ShortcutGroup group1;
    ShortcutGroup group2;
    ShortcutGroup group3;
    ShortcutGroup group4;
    ShortcutGroup group5;

    group1.groupName = tr("Files");
    group2.groupName = tr("Drawing");
    group3.groupName = tr("Shapes/Images");
    group4.groupName = tr("Settings");
    group5.groupName = tr("Align");



    group1.groupItems << ShortcutItem(tr("New"), "Ctrl+N")
                      << ShortcutItem(tr("Open"), "Ctrl+O")
                      << ShortcutItem(tr("Export"), "Ctrl+E")
                      << ShortcutItem(tr("Save"), "Ctrl+S")
                      << ShortcutItem(tr("Save as"), "Ctrl+Shift+S")
                      << ShortcutItem(tr("Print"), "Ctrl+P");

    group2.groupItems <<
                      ShortcutItem(tr("Select"),  "V") <<
                      ShortcutItem(tr("Import"),  "I") <<
                      ShortcutItem(tr("Rectangle"),        "R") <<
                      ShortcutItem(tr("Ellipse"),        "O") <<
                      ShortcutItem(tr("Triangle"),        "S") <<
                      ShortcutItem(tr("Star"),        "F") <<
                      ShortcutItem(tr("Polygon"),        "H") <<
                      ShortcutItem(tr("Line"),        "L") <<
                      ShortcutItem(tr("Text"),           "T") <<
                      ShortcutItem(tr("Pencil"),        "P") <<
                      ShortcutItem(tr("Eraser"),        "E") <<
                      ShortcutItem(tr("Blur"),           "B") <<
                      ShortcutItem(tr("Crop"),         "C") <<
                      ShortcutItem(tr("Expand canvas"),         "Ctrl + \"+\"") <<
                      ShortcutItem(tr("Shrink canvas"),         "Ctrl + \"-\"")
                      ;


    group3.groupItems <<
                      ShortcutItem(tr("Cut"), "Ctrl + X") <<
                      ShortcutItem(tr("Copy"), "Ctrl + C") <<
                      ShortcutItem(tr("Paste"), "Ctrl + V") <<
                      ShortcutItem(tr("Delete"), "Delete") <<
                      ShortcutItem(tr("Undo"), "Ctrl + Z") <<
                      ShortcutItem(tr("Redo"), "Ctrl + Y") <<
                      ShortcutItem(tr("Group"), "Ctrl+G") <<
                      ShortcutItem(tr("Ungroup"), "Ctrl+shift+G") <<
                      ShortcutItem(tr("Raise layer"), "Ctrl + ]") <<
                      ShortcutItem(tr("Lower layer"), "Ctrl + [") <<
                      ShortcutItem(tr("Layer to Top"), "Ctrl + Shift + ]") <<
                      ShortcutItem(tr("Layer to Bottom"), "Ctrl + Shift + [");


    group4.groupItems <<
                      ShortcutItem(tr("Help"), "F1") <<
                      ShortcutItem(tr("Display shortcuts"), "Ctrl + Shift + ?");

    group5.groupItems << ShortcutItem(tr("Align left"), "Ctrl+shift+L")
                      << ShortcutItem(tr("Align right"), "Ctrl+shift+R")
                      << ShortcutItem(tr("Align center horizontally"), "Ctrl+shift+H")
                      << ShortcutItem(tr("Align top"), "Ctrl+shift+T")
                      << ShortcutItem(tr("Align bottom"), "Ctrl+shift+B")
                      << ShortcutItem(tr("Align center vertically"), "Ctrl+shift+V");

    m_shortcutGroups << group1 << group2 << group3 << group4 << group5;

    //convert to json object
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
