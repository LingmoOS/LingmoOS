// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dscheduletype.h"

#include "daccount.h"
#include "units.h"
#include "commondef.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

DScheduleType::DScheduleType()
    : DScheduleType("")
{
}

DScheduleType::DScheduleType(const QString &accountID)
    : m_accountID(accountID)
    , m_typeID("")
    , m_typeName("")
    , m_displayName("")
    , m_typePath("")
    , m_typeColor(DTypeColor())
    , m_description("")
    , m_privilege(None)
    , m_showState(Show)
    , m_deleted(0)
    , m_syncTag(0)
{
}

QString DScheduleType::accountID() const
{
    return m_accountID;
}

void DScheduleType::setAccountID(const QString &accountID)
{
    m_accountID = accountID;
}

DScheduleType::Privileges DScheduleType::privilege() const
{
    return m_privilege;
}

void DScheduleType::setPrivilege(const Privileges &privilege)
{
    m_privilege = privilege;
}

DTypeColor DScheduleType::typeColor() const
{
    return m_typeColor;
}

void DScheduleType::setTypeColor(const DTypeColor &typeColor)
{
    m_typeColor = typeColor;
}

void DScheduleType::setColorID(const QString &colorID)
{
    m_typeColor.setColorID(colorID);
}

QString DScheduleType::getColorID() const
{
    return m_typeColor.colorID();
}

void DScheduleType::setColorCode(const QString &colorCode)
{
    m_typeColor.setColorCode(colorCode);
}

QString DScheduleType::getColorCode() const
{
    return m_typeColor.colorCode();
}

QString DScheduleType::typeID() const
{
    return m_typeID;
}

void DScheduleType::setTypeID(const QString &typeID)
{
    m_typeID = typeID;
}

QString DScheduleType::displayName() const
{
    return m_displayName;
}

void DScheduleType::setDisplayName(const QString &displayName)
{
    m_displayName = displayName;
}

DScheduleType::ShowState DScheduleType::showState() const
{
    return m_showState;
}

void DScheduleType::setShowState(const ShowState &showState)
{
    m_showState = showState;
}

QString DScheduleType::typeName() const
{
    return m_typeName;
}

void DScheduleType::setTypeName(const QString &typeName)
{
    m_typeName = typeName;
}

QString DScheduleType::typePath() const
{
    return m_typePath;
}

void DScheduleType::setTypePath(const QString &typePath)
{
    m_typePath = typePath;
}

QString DScheduleType::description() const
{
    return m_description;
}

void DScheduleType::setDescription(const QString &description)
{
    m_description = description;
}

QDateTime DScheduleType::dtCreate() const
{
    return m_dtCreate;
}

void DScheduleType::setDtCreate(const QDateTime &dtCreate)
{
    m_dtCreate = dtCreate;
}

QDateTime DScheduleType::dtUpdate() const
{
    return m_dtUpdate;
}

void DScheduleType::setDtUpdate(const QDateTime &dtUpdate)
{
    m_dtUpdate = dtUpdate;
}

QDateTime DScheduleType::dtDelete() const
{
    return m_dtDelete;
}

void DScheduleType::setDtDelete(const QDateTime &dtDelete)
{
    m_dtDelete = dtDelete;
}

int DScheduleType::deleted() const
{
    return m_deleted;
}

void DScheduleType::setDeleted(int deleted)
{
    m_deleted = deleted;
}

bool DScheduleType::fromJsonString(DScheduleType::Ptr &scheduleType, const QString &jsonStr)
{
    if (scheduleType.isNull()) {
        scheduleType = DScheduleType::Ptr(new DScheduleType);
    }
    //反序列化
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "error:" << jsonError.errorString();
        return false;
    }
    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("accountID")) {
        scheduleType->setAccountID(rootObj.value("accountID").toString());
    }

    if (rootObj.contains("typeID")) {
        scheduleType->setTypeID(rootObj.value("typeID").toString());
    }

    if (rootObj.contains("typeName")) {
        scheduleType->setTypeName(rootObj.value("typeName").toString());
    }

    if (rootObj.contains("displayName")) {
        scheduleType->setDisplayName(rootObj.value("displayName").toString());
    }

    if (rootObj.contains("typePath")) {
        scheduleType->setTypePath(rootObj.value("typePath").toString());
    }

    if (rootObj.contains("TypeColor")) {
        QJsonObject colorObject = rootObj.value("TypeColor").toObject();
        DTypeColor typeColor;
        if (colorObject.contains("colorID")) {
            typeColor.setColorID(colorObject.value("colorID").toString());
        }
        if (colorObject.contains("colorCode")) {
            typeColor.setColorCode(colorObject.value("colorCode").toString());
        }
        if (colorObject.contains("privilege")) {
            typeColor.setPrivilege(static_cast<DTypeColor::Privilege>(colorObject.value("privilege").toInt()));
        }
        scheduleType->setTypeColor(typeColor);
    }

    if (rootObj.contains("description")) {
        scheduleType->setDescription(rootObj.value("description").toString());
    }

    if (rootObj.contains("privilege")) {
        scheduleType->setPrivilege(static_cast<Privilege>(rootObj.value("privilege").toInt()));
    }

    if (rootObj.contains("dtCreate")) {
        scheduleType->setDtCreate(QDateTime::fromString(rootObj.value("dtCreate").toString(), Qt::ISODate));
    }

    if (rootObj.contains("dtDelete")) {
        scheduleType->setDtDelete(QDateTime::fromString(rootObj.value("dtDelete").toString(), Qt::ISODate));
    }

    if (rootObj.contains("dtUpdate")) {
        scheduleType->setDtUpdate(QDateTime::fromString(rootObj.value("dtUpdate").toString(), Qt::ISODate));
    }

    if (rootObj.contains("showState")) {
        scheduleType->setShowState(static_cast<ShowState>(rootObj.value("showState").toInt()));
    }

    if (rootObj.contains("isDeleted")) {
        scheduleType->setDeleted(rootObj.value("isDeleted").toInt());
    }
    return true;
}

bool DScheduleType::toJsonString(const DScheduleType::Ptr &scheduleType, QString &jsonStr)
{
    if (scheduleType.isNull()) {
        qCWarning(CommonLogger) << "hold a reference to a null pointer.";
        return false;
    }
    //序列化
    QJsonObject rootObject;
    rootObject.insert("accountID", scheduleType->accountID());
    rootObject.insert("typeID", scheduleType->typeID());
    rootObject.insert("typeName", scheduleType->typeName());
    rootObject.insert("displayName", scheduleType->displayName());
    rootObject.insert("typePath", scheduleType->typePath());
    //类型颜色信息
    QJsonObject colorObject;
    colorObject.insert("colorID", scheduleType->typeColor().colorID());
    colorObject.insert("colorCode", scheduleType->typeColor().colorCode());
    colorObject.insert("privilege", scheduleType->typeColor().privilege());
    rootObject.insert("TypeColor", colorObject);

    rootObject.insert("description", scheduleType->description());
    rootObject.insert("privilege", int(scheduleType->privilege()));
    rootObject.insert("dtCreate", dtToString(scheduleType->dtCreate()));
    rootObject.insert("dtDelete", dtToString(scheduleType->dtDelete()));
    rootObject.insert("dtUpdate", dtToString(scheduleType->dtUpdate()));
    rootObject.insert("showState", scheduleType->showState());
    rootObject.insert("isDeleted", scheduleType->deleted());

    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObject);
    jsonStr = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
    return true;
}

bool DScheduleType::fromJsonListString(DScheduleType::List &stList, const QString &jsonStr)
{
    QJsonParseError jsonError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &jsonError));
    if (jsonError.error != QJsonParseError::NoError) {
        qCWarning(CommonLogger) << "error:" << jsonError.errorString();
        return false;
    }
    QJsonObject rootObj = jsonDoc.object();
    if (rootObj.contains("scheduleType")) {
        QJsonArray jsonArray = rootObj.value("scheduleType").toArray();
        for (auto ja : jsonArray) {
            QJsonObject typeObject = ja.toObject();
            DScheduleType::Ptr scheduleType = DScheduleType::Ptr(new DScheduleType);
            if (typeObject.contains("accountID")) {
                scheduleType->setAccountID(typeObject.value("accountID").toString());
            }

            if (typeObject.contains("typeID")) {
                scheduleType->setTypeID(typeObject.value("typeID").toString());
            }

            if (typeObject.contains("typeName")) {
                scheduleType->setTypeName(typeObject.value("typeName").toString());
            }

            if (typeObject.contains("displayName")) {
                scheduleType->setDisplayName(typeObject.value("displayName").toString());
            }

            if (typeObject.contains("typePath")) {
                scheduleType->setTypePath(typeObject.value("typePath").toString());
            }

            if (typeObject.contains("TypeColor")) {
                QJsonObject colorObject = typeObject.value("TypeColor").toObject();
                DTypeColor typeColor;
                if (colorObject.contains("colorID")) {
                    typeColor.setColorID(colorObject.value("colorID").toString());
                }
                if (colorObject.contains("colorCode")) {
                    typeColor.setColorCode(colorObject.value("colorCode").toString());
                }
                if (colorObject.contains("privilege")) {
                    typeColor.setPrivilege(static_cast<DTypeColor::Privilege>(colorObject.value("privilege").toInt()));
                }
                scheduleType->setTypeColor(typeColor);
            }

            if (typeObject.contains("description")) {
                scheduleType->setDescription(typeObject.value("description").toString());
            }

            if (typeObject.contains("privilege")) {
                scheduleType->setPrivilege(static_cast<Privilege>(typeObject.value("privilege").toInt()));
            }

            if (typeObject.contains("dtCreate")) {
                scheduleType->setDtCreate(QDateTime::fromString(typeObject.value("dtCreate").toString(), Qt::ISODate));
            }

            if (typeObject.contains("dtDelete")) {
                scheduleType->setDtDelete(QDateTime::fromString(typeObject.value("dtDelete").toString(), Qt::ISODate));
            }

            if (typeObject.contains("dtUpdate")) {
                scheduleType->setDtUpdate(QDateTime::fromString(typeObject.value("dtUpdate").toString(), Qt::ISODate));
            }

            if (typeObject.contains("showState")) {
                scheduleType->setShowState(static_cast<ShowState>(typeObject.value("showState").toInt()));
            }

            if (typeObject.contains("isDeleted")) {
                scheduleType->setDeleted(typeObject.value("isDeleted").toInt());
            }
            stList.append(scheduleType);
        }
    }
    return true;
}

bool DScheduleType::toJsonListString(const DScheduleType::List &stList, QString &jsonStr)
{
    //序列化
    QJsonObject rootObject;
    QJsonArray jsonArray;

    for (auto &scheduleType : stList) {
        QJsonObject typeObject;
        typeObject.insert("accountID", scheduleType->accountID());
        typeObject.insert("typeID", scheduleType->typeID());
        typeObject.insert("typeName", scheduleType->typeName());
        typeObject.insert("displayName", scheduleType->displayName());
        typeObject.insert("typePath", scheduleType->typePath());
        //类型颜色信息
        QJsonObject colorObject;
        colorObject.insert("colorID", scheduleType->typeColor().colorID());
        colorObject.insert("colorCode", scheduleType->typeColor().colorCode());
        colorObject.insert("privilege", scheduleType->typeColor().privilege());
        typeObject.insert("TypeColor", colorObject);

        typeObject.insert("description", scheduleType->description());
        typeObject.insert("privilege", int(scheduleType->privilege()));
        typeObject.insert("dtCreate", dtToString(scheduleType->dtCreate()));
        typeObject.insert("dtDelete", dtToString(scheduleType->dtDelete()));
        typeObject.insert("dtUpdate", dtToString(scheduleType->dtUpdate()));
        typeObject.insert("showState", scheduleType->showState());
        typeObject.insert("isDeleted", scheduleType->deleted());
        jsonArray.append(typeObject);
    }
    rootObject.insert("scheduleType", jsonArray);
    QJsonDocument jsonDoc;
    jsonDoc.setObject(rootObject);
    jsonStr = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
    return true;
}

int DScheduleType::syncTag() const
{
    return m_syncTag;
}

void DScheduleType::setSyncTag(int syncTag)
{
    m_syncTag = syncTag;
}

bool operator<(const DScheduleType::Ptr &st1, const DScheduleType::Ptr &st2)
{
    //权限不一致权限小的排在前面
    if (st1->privilege() != st2->privilege()) {
        return st1->privilege() < st2->privilege();
    }
    //权限一一致的创建时间早的排在前面
    if (st1->dtCreate() != st2->dtCreate()) {
        return st1->dtCreate() < st2->dtCreate();
    }
    return true;
}
