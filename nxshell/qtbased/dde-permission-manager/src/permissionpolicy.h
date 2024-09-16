// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PERMISSIONPOLICY_H
#define PERMISSIONPOLICY_H

#include <QStringList>
#include <QJsonObject>
#include <QMap>
#include <QLocale>
#include <QDebug>


struct Transactions
{
    QMap<QString, QString> titleMap;
    QMap<QString, QString> descriptionMap;
};

struct NameTransactions
{
    QMap<QString, QString> nameMap;
};

struct DialogContents
{
    QString dialogName;
    QString title;
    QString description;
    Transactions transactions;
};

class PermissionPolicy
{
public:
    PermissionPolicy(const QString& permissionGroup, const QString& permissionId);

    static QStringList getPolicyList(const QString& permissionGroup);

    inline bool isValid() const {
        return m_isValid;
    }

    inline QString group() const {
        return m_group;
    }

    inline QString id() const {
        return m_id;
    }

    inline bool skip_auto_allow() const {
        return m_skip_auto_allow;
    }

    inline QString name() const {
        return m_name;
    }

    inline QString title(const QString dialogName) const {
        QString language = QLocale::system().name();
        for (int i = 0; i < m_dialogContents.size(); ++i) {
            if (m_dialogContents[i].dialogName == dialogName) {
                if (m_dialogContents[i].transactions.titleMap.contains(language)) {
                    return m_dialogContents[i].transactions.titleMap[language];
                } else {
                    return m_dialogContents[i].title;
                }
            }
        }

        return "";
    }

    inline QString description(const QString dialogName) const {
        QString language = QLocale::system().name();
        for (int i = 0; i < m_dialogContents.size(); ++i) {
            if (m_dialogContents[i].dialogName == dialogName) {
                if (m_dialogContents[i].transactions.descriptionMap.contains(language)) {
                    return m_dialogContents[i].transactions.descriptionMap[language];
                } else {
                    return m_dialogContents[i].description;
                }
            }
        }

        return "";
    }

    inline QString prefer() const {
        return m_prefer;
    }

    inline QStringList options() const {
        return m_options;
    }

    inline QString type() const {
        return m_type;
    }

    inline QString registerMode() const {
        return m_registerMode;
    }

private:
    struct Transactions transactionsJsonParse(const QJsonObject& obj);
    void parseDialogContent(const QJsonObject& obj);
    void nameTransactionsJsonParse(const QJsonObject& obj);

private:
    bool m_isValid;
    bool m_skip_auto_allow;
    QString m_group;
    QString m_id;
    QString m_name;
    QString m_title;
    QString m_description;
    QString m_prefer;
    QString m_type; // system session。区分该权限是系统级还是用户级。
    QString m_registerMode; // limit、notlimit。limit指该权限需要被注册才能使用，notlimit无需注册。默认limit。
    QStringList m_options;
    QList<DialogContents> m_dialogContents;
    Transactions m_transactions;
    NameTransactions m_names;
};

#endif // PERMISSIONPOLICY_H
