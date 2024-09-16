// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "semanticanalysistask.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "../data/createjsondata.h"
#include "../data/queryjsondata.h"
#include "../data/canceljsondata.h"
#include "../data/changejsondata.h"
#include "../globaldef.h"

semanticAnalysisTask::semanticAnalysisTask(QObject *parent)
    : QObject(parent)
{
}

semanticAnalysisTask::~semanticAnalysisTask()
{
    deleteJsonData();
}

bool semanticAnalysisTask::resolveTaskJson(const QString &semantic)
{
    setIntent("");
    deleteJsonData();

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(semantic.toUtf8(), &jsonError);

    if (!doc.isNull() && (jsonError.error == QJsonParseError::NoError)) {
        auto rootObject = doc.object();
        if (!(rootObject.contains("intent")
              && rootObject["intent"].isObject())) {
            return false;
        }

        auto intentObject = rootObject["intent"].toObject();
        if (intentObject.empty()) {
            return false;
        }
        if (intentObject.contains("semantic") && intentObject["semantic"].isArray()) {
            auto semanticObjArr = intentObject["semantic"].toArray();
            for (int i = 0; i < semanticObjArr.size(); ++i) {
                auto semanticObj = semanticObjArr[i].toObject();
                if (semanticObj.contains("intent") && semanticObj["intent"].isString()) {
                    setIntent(semanticObj["intent"].toString());
                }
                m_JsonData = createJsonDataFactory(Intent());
                if (m_JsonData != nullptr) {
                    m_JsonData->JosnResolve(semanticObj);
                }
            }
        }
        if (intentObject.contains("voice_answer") && intentObject["voice_answer"].isArray()) {
            auto voiceAnsObjArr = intentObject["voice_answer"].toArray();
            for (int i = 0; i < voiceAnsObjArr.size(); ++i) {
                auto voiceAnsObj = voiceAnsObjArr[i].toObject();
                if (voiceAnsObj.contains("type") && voiceAnsObj["type"].isString()) {
                    if (voiceAnsObj["type"] != "TTS") {
                        continue;
                    }
                    if (voiceAnsObj.contains("content") && voiceAnsObj["content"].isString()) {
                        //语音播报的文本
                        if (m_JsonData != nullptr) {
                            m_JsonData->setSuggestMsg(voiceAnsObj["content"].toString());
                        }
                        break;
                    }
                }
            }
        }
        if (intentObject.contains("shouldEndSession") && intentObject["shouldEndSession"].isBool()) {
            setShouldEndSession(intentObject["shouldEndSession"].toBool());
        } else {
            setShouldEndSession(true);
        }
    } else {
        return false;
    }
    return true;
}

QString semanticAnalysisTask::Intent() const
{
    return m_Intent;
}

void semanticAnalysisTask::setIntent(const QString &Intent)
{
    m_Intent = Intent;
}

JsonData *semanticAnalysisTask::getJsonData() const
{
    return m_JsonData;
}

void semanticAnalysisTask::deleteJsonData()
{
    if (m_JsonData != nullptr) {
        delete m_JsonData;
        m_JsonData = nullptr;
    }
}

JsonData *semanticAnalysisTask::createJsonDataFactory(const QString &Intent)
{
    if (Intent == JSON_CREATE) {
        //创建
        return new CreateJsonData();
    } else if (Intent == JSON_VIEW) {
        //查询
        return new QueryJsonData();

    } else if (Intent == JSON_CANCEL) {
        //取消
        return new cancelJsonData();

    } else if (Intent == JSON_CHANGE) {
        //改变
        return new changejsondata();
    }
    return nullptr;
}

void semanticAnalysisTask::setShouldEndSession(bool isEnd)
{
    if (m_JsonData != nullptr) {
        m_JsonData->setShouldEndSession(isEnd);
    }
}

void semanticAnalysisTask::setJsonData(JsonData *JsonData)
{
    m_JsonData = JsonData;
}
