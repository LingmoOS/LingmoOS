// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "permissionpolicy.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariant>

PermissionPolicy::PermissionPolicy(const QString& permissionGroup, const QString& permissionId)
    : m_isValid(false)
    , m_group(permissionGroup)
{
        QFile file("/usr/share/permission/policy/" + permissionGroup + "/" + permissionId + ".json");
        if (!file.open(QFile::Text | QFile::ReadOnly)) {
            return;
        }
        const QJsonObject& obj = QJsonDocument::fromJson(file.readAll()).object();
        m_id = obj["id"].toString();
        m_skip_auto_allow = obj["skip_auto_allow"].toBool();
        m_name = obj["name"].toString();
        m_prefer = obj["prefer"].toString();
        m_options = obj["options"].toVariant().toStringList();
        m_registerMode = obj["registerMode"].toString();
        m_type = obj["type"].toString();
        m_isValid = true;

        nameTransactionsJsonParse(obj);
        parseDialogContent(obj);
}

void PermissionPolicy::parseDialogContent(const QJsonObject& obj) {
    if (!obj.contains("dialogContent")) {
        qWarning() << "permission policy not contain dialog";
        return;
    }

    if (!obj["dialogContent"].isArray()) {
        qWarning() << "permission policy invalid format: is not an array";
        return;
    }

    const QJsonArray dialogArray = obj["dialogContent"].toArray();
    struct DialogContents dialogContents;
    for (int i = 0; i < dialogArray.size(); ++i) {
        if (dialogArray[i].isObject()) {
            QJsonObject dialogObj = dialogArray[i].toObject();
            dialogContents.dialogName = dialogObj["dialogName"].toString();
            dialogContents.title = dialogObj["title"].toString();
            dialogContents.description = dialogObj["description"].toString();
            dialogContents.transactions = transactionsJsonParse(dialogObj);
            m_dialogContents.append(dialogContents);
        }
    }
}

QStringList PermissionPolicy::getPolicyList(const QString& permissionGroup)
{
    QStringList ids;
    QDir dir("/usr/share/permission/policy/" + permissionGroup);
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (auto fileInfo : fileInfoList) {
        if (fileInfo.suffix() != "json") {
            continue;
        }
        QFile file(fileInfo.absoluteFilePath());
        if (!file.open(QFile::Text | QFile::ReadOnly)) {
            qWarning() << "open file failed";
            continue;
        }
        const QJsonObject& obj = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
        QString id = obj["id"].toString();
        if (!id.isEmpty()) {
            ids << id;
        }
    }
    return ids;
}

void PermissionPolicy::nameTransactionsJsonParse(const QJsonObject& obj) {
    if(obj.contains("translations")) {
        const QJsonObject &tsObj = obj["translations"].toObject();
        if(tsObj.contains("name")) {
            if(tsObj["name"].isArray()) {
                const QJsonArray &nameArray = tsObj["name"].toArray();
                for (int i = 0; i < nameArray.size(); i++) {
                    if (nameArray.at(i).isObject()) {
                        const QJsonObject &descObj = nameArray.at(i).toObject();
                        m_names.nameMap[descObj["language"].toString()] = descObj["text"].toString();
                    }
                }
            }
        }
    }
}

struct Transactions PermissionPolicy::transactionsJsonParse(const QJsonObject& obj)
{
    Transactions transactions;
    if(obj.contains("translations")) {
        const QJsonObject &tsObj = obj["translations"].toObject();
        if(tsObj.contains("title")) {
            if(tsObj["title"].isArray()) {
                const QJsonArray &titleArray = tsObj["title"].toArray();
                for (int i = 0; i < titleArray.size(); i++) {
                    if (titleArray.at(i).isObject()) {
                        const QJsonObject &titleObj = titleArray.at(i).toObject();
                        transactions.titleMap[titleObj["language"].toString()] = titleObj["text"].toString();
                    }
                }
            }
        }
        if(tsObj.contains("description")) {
            if(tsObj["description"].isArray()) {
                const QJsonArray &descriptionArray = tsObj["description"].toArray();
                for (int i = 0; i < descriptionArray.size(); i++) {
                    if (descriptionArray.at(i).isObject()) {
                        const QJsonObject &descObj = descriptionArray.at(i).toObject();
                        transactions.descriptionMap[descObj["language"].toString()] = descObj["text"].toString();
                    }
                }
            }
        }
    }

    return transactions;
}
