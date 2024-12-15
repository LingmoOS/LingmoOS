// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "compatibilitymanager.h"
#include "constant.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

std::optional<CompatibilityDesktopEntry::Entry> CompatibilityDesktopEntry::group(const QString &key)  const noexcept
{
    auto iter = m_entrys.find(key);
    if(iter == m_entrys.end()){
        return std::nullopt;
    }

    return *iter;
}

std::any CompatibilityDesktopEntry::value(const QString &key, const CompatibilityDesktopEntry::ValueKey &valueKey) const noexcept
{
    auto iter = m_entrys.find(key);
    if(iter == m_entrys.end()){
        return std::nullopt;
    }

    if(valueKey == CompatibilityDesktopEntry::Exec){
        return get<0>(*iter);
    }

    if(valueKey == CompatibilityDesktopEntry::Env){
        return get<1>(*iter);
    }

    return std::nullopt;
}

void CompatibilityDesktopEntry::insert(const QString &groupKey, const Entry &entry) noexcept
{
    auto iter = m_entrys.find(groupKey);
    if(iter == m_entrys.end()){
        m_entrys.insert(groupKey,entry);
    }else{
        qWarning()<<"insert : "<<groupKey<<"fail";
    }
}

CompatibilityManager::CompatibilityManager()
{
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &CompatibilityManager::loadCompatibilityConfig);

    if(!m_watcher.addPath(CompatibilityConfigFilePath)){
        qWarning() << "add path : "<< CompatibilityConfigFilePath << "to watch fail";
    }
    loadCompatibilityConfig();
}

std::optional<QString> CompatibilityManager::getExec(const QString &desktopId, const QString &groupId)
{
    auto iter = m_compatibilityConfig.find(desktopId);
    if(iter == m_compatibilityConfig.end()){
        return std::nullopt;
    }

    auto value = iter->value(groupId, CompatibilityDesktopEntry::Exec);
    if(!value.has_value()){
        return std::nullopt;
    }

    if (value.type() != typeid(QString)){
        return std::nullopt;
    }

    return std::any_cast<QString>(value);
}

QStringList CompatibilityManager::getEnv(const QString &desktopId, const QString &groupId)
{
    auto iter = m_compatibilityConfig.find(desktopId);
    if(iter == m_compatibilityConfig.end()){
        return QStringList();
    }

    auto value = iter->value(groupId, CompatibilityDesktopEntry::Env);
    if(!value.has_value()){
        return QStringList();
    }

    if (value.type() != typeid(QStringList)){
        return QStringList();
    }

    return std::any_cast<QStringList>(value);
}

void CompatibilityManager::loadCompatibilityConfig(){
    qInfo() << "loadCompatibilityConfig";
    m_compatibilityConfig.clear();
    QFile file{CompatibilityConfigFilePath};

    auto err = parse(file);
    if (err != ParserError::NoError) {
        qWarning() << "parse file :" << file.fileName() << ", err";
    }
}

ParserError CompatibilityManager::parse(QFile &file) noexcept{
    if (!file.open(QFile::Text | QFile::ReadOnly | QFile::ExistingOnly)) {
        qWarning() << "open file:" << file.fileName() << "failed:" << file.errorString() << ", skip.";
        return ParserError::NotFound;
    }

    auto content = file.readAll();
    QJsonParseError err;
    auto json = QJsonDocument::fromJson(content, &err);
    if (err.error != QJsonParseError::NoError) {
        qWarning() << "parse compatibility config failed:" << err.errorString();
        return ParserError::MissingInfo;
    }
    if (json.isEmpty()) {
        qWarning() << "file : "<<file.fileName()<<" empty";
        return ParserError::MissingInfo;
    }

    auto obj = json.object();
    for (const QString &desktopId : obj.keys()) {
        auto existDesktopId = m_compatibilityConfig.find(desktopId);
        if(existDesktopId != m_compatibilityConfig.end()){
            qWarning() << "the desktop id : "<<desktopId<<"exist skip";
            continue;
        }
        auto desktopEntry = obj[desktopId].toObject();

        // 处理 Desktop Entry
        CompatibilityDesktopEntry compatibilityEntry;
        for(auto entryId : desktopEntry.keys()){
            if(!entryId.startsWith(DesktopFileEntryKey) && !entryId.startsWith(DesktopFileActionKey)){
                qWarning() << "parse entry : "<<entryId<<" fail";
                return ParserError::MissingInfo;
            }

            auto entry = desktopEntry[entryId].toObject();
            auto exec = entry[DesktopEntryExec].toString();
            auto envArray = entry[DesktopEntryEnv].toArray();

            QStringList env;
            for (const QJsonValue &envJson : envArray) {
                env.push_back(envJson.toString());
            }

            //auto tub = std::make_tuple(exec,std::move(env.join("; ") + ";"));
            auto tub = std::make_tuple(exec,env);
            qWarning()<<get<0>(tub);
            qWarning()<<get<1>(tub);
            compatibilityEntry.insert(entryId,std::move(tub));
        }

        m_compatibilityConfig.insert(desktopId,compatibilityEntry);
    }

    for(auto iter:m_compatibilityConfig.keys()){
        qWarning()<<"--------";
        qWarning()<<iter;
        for(auto entry: m_compatibilityConfig[iter].m_entrys.keys()){
            qWarning()<<entry;
            qWarning()<<get<0>(m_compatibilityConfig[iter].m_entrys[entry]);
            qWarning()<<get<1>(m_compatibilityConfig[iter].m_entrys[entry]);
        }
    }
    return ParserError::NoError;
}
