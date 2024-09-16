// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semantichelper.h"
#include "global/searchhelper.h"

#include <QMap>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDateTime>

#include <unistd.h>

using namespace GrandSearch;

SemanticHelper::SemanticHelper()
{

}

bool SemanticHelper::entityFromJson(const QString &json, SemanticEntity &out)
{
    QJsonParseError er;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &er);
    if (er.error != QJsonParseError::NoError)
        return false;

    auto root = doc.array();
    if (root.isEmpty())
        return false;

    static QMap<QString, QString> typeTable = {
            {"text", DOCUMENT_GROUP},
            {"picture", PICTURE_GROUP},
            {"music", AUDIO_GROUP},
            {"movie", VIDEO_GROUP},
            {"file", FILE_GROUP},
            {"folder", FOLDER_GROUP},
            {"appliaction", APPLICATION_GROUP},
        };

    out = {};

    // 只取第一个
    const QJsonObject &obj = root.first().toObject();
    QSet<QString> typeOfKey;
    auto typeObj = obj.value("type").toArray();
    for(auto ro = typeObj.begin(); ro != typeObj.end(); ++ro) {
        if (ro->isObject()){
            auto type = ro->toObject().value("type");
            auto word = ro->toObject().value("word");
            if (type.isString()) {
                QString text = typeTable.value(type.toString().trimmed());
                if (!text.isEmpty() && !out.types.contains(text)) {
                    out.types.append(text);
                    if (word.isString()){
                        text = word.toString().trimmed();
                        if (!text.isEmpty())
                            typeOfKey.insert(text);
                    }
                }
            }

        }
    }

    auto entityObj = obj.value("entity").toArray();
    for (auto ro = entityObj.begin(); ro != entityObj.end(); ++ro) {
        if (ro->isString()){
            QString key = ro->toString().trimmed();
            if (!key.isEmpty()) {
                // 去除模型概率性带出的[SEP]...后缀
                int pos = key.indexOf("[");
                if (pos != -1) {
                    key = key.mid(0, pos);
                }
                out.keys.append(key);
            }
        }
    }

    QSet<QString> timeOfKey;
    auto timeObj = obj.value("time").toArray();
    for (auto ro = timeObj.begin(); ro != timeObj.end(); ++ro) {
        if (ro->isObject()){
            auto detail = ro->toObject().value("detail");
            if (detail.isObject()) {
                auto time = detail.toObject().value("time");
                if (time.isArray()) {
                    auto timeArray = time.toArray();
                    if (timeArray.size() == 2) {
                        QString t1 = timeArray.first().toString().trimmed();
                        QString t2 = timeArray.last().toString().trimmed();
                        QPair<qint64, qint64> value;
                        value.first = QDateTime::fromString(t1, "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch();
                        value.second = QDateTime::fromString(t2, "yyyy-MM-dd hh:mm:ss").toSecsSinceEpoch();
                        if (value.first > 0 || value.second > 0)
                            out.times.append(value);
                    }
                }
                auto text = ro->toObject().value("text");
                if (text.isString()) {
                    QString txt = text.toString().trimmed();
                    if (!txt.isEmpty())
                        timeOfKey.insert(txt);
                }
            }
        }
    }

    // remove text of type
    for (const QString &txt : typeOfKey)
        out.keys.removeOne(txt);

    // remove text of time;
    for (const QString &txt : timeOfKey)
        out.keys.removeOne(txt);

    // 组装log打印内容，方便查看
    QString types, keys;
    for (const QString &str : out.types) {
        types.append(str).append(", ");
    }
    for (const QString &str : out.keys) {
        keys.append(str).append(", ");
    }
    qInfo() << QString("types(%1) keys(%2)").arg(types).arg(keys);

    return true;
}

QString SemanticHelper::analyzeServiceName()
{
    return "org.deepin.ai.daemon.AnalyzeServer";
}

QString SemanticHelper::vectorServiceName()
{
    return "org.deepin.ai.daemon.VectorIndex";
}

bool SemanticHelper::isMatchTime(qint64 time, const QList<QPair<qint64, qint64> > &intervals)
{
    bool match = intervals.isEmpty();
    for (const QPair<qint64, qint64> &interval : intervals) {
        if (interval.first > 0 && time < interval.first)
            continue;
        if (interval.second > 0 && time > interval.second)
            continue;

        match = true;
        break;
    }

    return match;
}

QStringList SemanticHelper::typeTosuffix(const QStringList &type)
{
    QStringList ret;
    for (const QString &t: type) {
        auto s = SearchHelper::instance()->getSuffixByGroupName(t);
        if (!s.isEmpty())
            ret.append(s);
    }

    return ret;
}
