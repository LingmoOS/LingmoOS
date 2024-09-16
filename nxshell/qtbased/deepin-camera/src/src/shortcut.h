// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
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
    ShortcutItem(const QString& n,const QString& v): name(n), value(v) {}
};

struct ShortcutGroup {
    QString groupName;
    QList<ShortcutItem> groupItems;
};

/**
 * @brief setPic 取消image带来的警告
 * space停止拍照或录像，Alt + M调用菜单，Ctrl + C复制，Ctrl + O打开文件夹，Ctrl + P打印，F1帮助，Delete删除，Ctrl + Shift + ?列出快捷键
 */
class Shortcut : public QObject
{
    Q_OBJECT
public:
    explicit Shortcut(QObject *parent = 0);

    /**
    * @brief toStr 数据类型转换
    */
    QString toStr();

private:
    QJsonObject          m_shortcutObj;
    QList<ShortcutGroup> m_shortcutGroups;
};

#endif // SHORTCUT_H
