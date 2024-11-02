/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef JSON_H
#define JSON_H

#include <QTextStream>
#include <qjsonobject.h>
#include <QJsonDocument>
#include <QFile>
#include <string>
#include <syslog.h>

/**
* @brief json文件的类型
*/
enum class JsonType
{
    JSON_TYPE_SOUNDEFFECT = 0, // 开机音效类型的json文件
    JSON_TYPE_USERINFO,        // 用户信息类型的json文件
    JSON_TYPE_UNKNOWN          // 未知类型的json文件
};

class IJson
{
public:
    IJson(const QString& name, const JsonType& type) : m_filename(name), m_type(type)
    {
        load();
    }

    virtual ~IJson()
    {
        save();
    }

    /**
    * @brief  初始化json文件
    */
    virtual void init() = 0;

    /**
    * @brief  删除json文件中键为key的键值对
    *
    * @param  key 需要删除的键
    */
    virtual void remove(const QString& key) = 0;

    /**
    * @brief  在json文件插入键值对
    *
    * @param  key 插入的键
    *         value 插入的值
    */
    virtual void insert(const QString& key, const QJsonValue& value) = 0;

    /**
    * @brief  获取json文件指定键值
    *
    * @param  key 获取的键
    * @return
    */
    virtual QJsonValue getValue(QString key) const = 0;

    void update()
    {
        save();
    }

private:
    void load()
    {
        QFile file(m_filename);
        if (!file.open(QFile::ReadWrite)) {
            return;
        }

        QTextStream m_textStream(&file);
        QString str = m_textStream.readAll();
        file.close();

        QJsonParseError jsonError;
        QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &jsonError);

        if (jsonError.error != QJsonParseError::NoError) {
            return;
        }

        m_rootObj = doc.object();
    }

    void save() {
        QJsonDocument doc(m_rootObj);
        QFile file(m_filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            syslog(LOG_ERR, "open %s failed, %s", m_filename.toLatin1().data(), strerror(errno));
            return;
        }

        QTextStream m_textStream(&file);
        m_textStream.setCodec("UTF-8");
        m_textStream << doc.toJson();
        file.close();
    }

protected:
    QString m_filename;
    QJsonObject m_rootObj;
    JsonType m_type;
};


#endif // IJSON_H
