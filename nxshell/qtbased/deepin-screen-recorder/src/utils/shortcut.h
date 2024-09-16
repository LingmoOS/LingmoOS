// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

struct ShortcutItem {
    QString name;
    QString value;
    ShortcutItem(QString n, QString v): name(n), value(v) {}
};

struct ShortcutGroup {
    QString groupName;
    QList<ShortcutItem> groupItems;
};

class Shortcut : public QObject
{
    Q_OBJECT
public:
    explicit Shortcut(QObject *parent = 0);
    QString toStr();
    QString getSysShortcuts(const QString type);
    QString getDefaultValue(const QString type);
private:
    QJsonObject m_shortcutObj;
    QList<ShortcutGroup> m_shortcutGroups;
};

#endif // SHORTCUT_H
