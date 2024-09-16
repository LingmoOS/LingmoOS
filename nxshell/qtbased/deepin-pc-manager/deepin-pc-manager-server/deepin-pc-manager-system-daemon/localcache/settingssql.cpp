// Copyright (C) 2019 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingssql.h"

#include "../../deepin-pc-manager/src/window/modules/common/gsettingkey.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QXmlStreamAttribute>
#include <QXmlStreamReader>

#define DEFENDER_DATA_DIR_PATH "/usr/share/deepin-pc-manager/" // 数据库路径
#define LOCAL_CACHE_DB_NAME "localcache.db"                    // 数据库路径
#define TABLE_NAME_SETTINGS "Settings"
#define GSETTING_SCHEMA_XML_PATH \
    "/usr/share/glib-2.0/schemas/com.deepin.dde.deepin-pc-manager.gschema.xml"

SettingsSql::SettingsSql(const QString &connName, QObject *parent)
    : QObject(parent)
    , m_connName(connName)
    , m_sqlQuery(nullptr)
{
    initDb();

    setDefaultKeyValues();
}

SettingsSql::~SettingsSql() { }

bool SettingsSql::setValue(const QString &key, const QVariant &value, const bool isDefault)
{
    // 确认是否存在配置
    QVariant oldValue = getValue(key);

    // 将设置数据转换成json文本
    QJsonObject obj;
    obj.insert("value", value.toJsonValue());
    QJsonDocument doc(obj);
    QString jsonValue = doc.toJson();

    // 区分设置值还是默认值
    QString valueTypeStr;
    if (isDefault) {
        valueTypeStr = "defaultValue";
    } else {
        valueTypeStr = "value";
    }

    QString cmd;
    if (oldValue.isNull()) {
        // 新加配置
        cmd = QString("INSERT INTO %1 (key, %2) VALUES ('%3','%4');")
                  .arg(TABLE_NAME_SETTINGS)
                  .arg(valueTypeStr)
                  .arg(key)
                  .arg(jsonValue);
    } else {
        // 更新配置
        cmd = QString("UPDATE %1 SET %2='%3' WHERE key='%4';")
                  .arg(TABLE_NAME_SETTINGS)
                  .arg(valueTypeStr)
                  .arg(jsonValue)
                  .arg(key);
    }

    if (!m_sqlQuery.exec(cmd)) {
        qDebug() << Q_FUNC_INFO << m_sqlQuery.lastError();
        return false;
    }

    return true;
}

QVariant SettingsSql::getValue(const QString &key)
{
    QVariant retValue;

    QString cmd = QString("SELECT * FROM %1 WHERE key='%2';").arg(TABLE_NAME_SETTINGS).arg(key);
    if (!m_sqlQuery.exec(cmd)) {
        qDebug() << Q_FUNC_INFO << m_sqlQuery.lastError();
        return false;
    }

    int count = 0;
    QByteArray valueByteArray;
    while (m_sqlQuery.next()) {
        // 第一个数值为id
        valueByteArray = m_sqlQuery.value(2).toByteArray();
        // 如果该配置未设置过值，则取默认值
        if (valueByteArray.isEmpty()) {
            valueByteArray = m_sqlQuery.value(3).toByteArray();
        }
        count++;
    }
    if (1 < count) {
        // 正常情况下一个查询，数据库中只存在一个数据
        qDebug() << Q_FUNC_INFO << "queried more than 1 result!";
    }

    // 解析数据
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(valueByteArray, &error);
    if (doc.object().contains("value")) {
        retValue = doc.object().value("value").toVariant();
    }
    return retValue;
}

void SettingsSql::initDb()
{
    // 初始化数据库
    //  必须先判断待创建数据库所处目录是否存在，否则如果不存在，会导致数据库打开失败
    QDir defenderDataDir;
    if (!defenderDataDir.exists(DEFENDER_DATA_DIR_PATH)) {
        defenderDataDir.mkdir(DEFENDER_DATA_DIR_PATH);
    }
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connName);
    m_localCacheDbPath = QString("%1%2").arg(DEFENDER_DATA_DIR_PATH).arg(LOCAL_CACHE_DB_NAME);
    m_db.setDatabaseName(m_localCacheDbPath);

    if (!m_db.open()) {
        qDebug() << Q_FUNC_INFO << "local db open error!";
        return;
    }

    // 如果 localcache.db数据库已经存在，则服务启动时不需要刷新数据
    QStringList tableStrList = m_db.tables();
    m_sqlQuery = QSqlQuery(m_db);
    if (!tableStrList.contains(TABLE_NAME_SETTINGS)) {
        // 服务第一次启动启动时，创建包名解析表 flowDetail
        QString cmd =
            QString(
                "CREATE TABLE %1 (id Long primary key, key text, value text, defaultValue text);")
                .arg(TABLE_NAME_SETTINGS);
        bool bRet = m_sqlQuery.exec(cmd);
        if (!bRet) {
            qDebug() << Q_FUNC_INFO << m_sqlQuery.lastError();
            return;
        }
    }
}

// 解析gsetting xml文件，并根据文件内容设置默认值
void SettingsSql::setDefaultKeyValues()
{
    QFile file(GSETTING_SCHEMA_XML_PATH);
    if (!file.open(QIODevice::OpenModeFlag::ReadOnly)) {
        qDebug() << Q_FUNC_INFO << GSETTING_SCHEMA_XML_PATH << "open failed!";
        return;
    }

    // gsettings路径
    QString schemaPath;
    // gsettings id
    QString schemaId;
    bool isSchemaDataNeed = false;
    QString keyName;
    QString keyType;
    QXmlStreamReader xmlReader(&file);

    while (!xmlReader.atEnd()) {
        if (xmlReader.isStartElement() && "schema" == xmlReader.name()) {
            QXmlStreamAttributes attrs = xmlReader.attributes();
            if (attrs.hasAttribute("path")) {
                schemaPath = attrs.value("path").toString();
            }
            if (attrs.hasAttribute("id")) {
                schemaId = attrs.value("id").toString();
            }

            if ("/com/deepin/dde/deepin-pc-manager/" == schemaPath
                && DEEPIN_PC_MANAGER_GSETTING_PATH == schemaId) {
                isSchemaDataNeed = true;
            }

            xmlReader.readNext();
            continue;
        }

        if (!isSchemaDataNeed) {
            xmlReader.readNext();
            continue;
        }

        // 获取配置
        readAndSetKeyDefaultValueFromXml(&xmlReader, keyName, keyType);
        if (xmlReader.isEndElement() && "schema" == xmlReader.name()) {
            isSchemaDataNeed = false;
            xmlReader.readNext();
            continue;
        }
        xmlReader.readNext();
    }

    file.close();
}

// 从xml中读取默认值，并向数据库中写入
void SettingsSql::readAndSetKeyDefaultValueFromXml(QXmlStreamReader *xmlReader,
                                                   QString &keyName,
                                                   QString &keyType)
{
    if (xmlReader->isStartElement() && "key" == xmlReader->name()) {
        QXmlStreamAttributes attrs = xmlReader->attributes();
        if (attrs.hasAttribute("name")) {
            keyName = attrs.value("name").toString();
        }
        if (attrs.hasAttribute("type")) {
            keyType = attrs.value("type").toString();
        }
    }

    if (xmlReader->isStartElement() && "default" == xmlReader->name()) {
        if ("b" == keyType) {
            QString value = xmlReader->readElementText();
            bool bValue = "true" == value ? true : false;
            setValue(keyName, bValue, true);
        } else if ("i" == keyType) {
            setValue(keyName, xmlReader->readElementText().remove("\'").remove("\"").toInt(), true);
        } else if ("d" == keyType) {
            setValue(keyName,
                     xmlReader->readElementText().remove("\'").remove("\"").toDouble(),
                     true);
        } else if ("s" == keyType) {
            setValue(keyName, xmlReader->readElementText().remove("\'").remove("\""), true);
        } else {
            qDebug() << Q_FUNC_INFO << "unknown gsettings key type: " << keyType;
        }
    }
}
