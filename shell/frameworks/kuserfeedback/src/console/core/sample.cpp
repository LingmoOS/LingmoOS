/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "sample.h"
#include <core/product.h>
#include <core/schemaentry.h>
#include <core/schemaentryelement.h>

#include <QDateTime>
#include <QDebug>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSharedData>
#include <QVariant>

using namespace KUserFeedback::Console;

class KUserFeedback::Console::SampleData : public QSharedData
{
public:
    QDateTime timestamp;
    QHash<QString, QVariant> data;
};

Sample::Sample() : d(new SampleData) {}
Sample::Sample(const Sample&) = default;
Sample::~Sample() = default;
Sample& Sample::operator=(const Sample&) = default;

QDateTime Sample::timestamp() const
{
    return d->timestamp;
}

QVariant Sample::value(const QString &name) const
{
    return d->data.value(name);
}

QVector<Sample> Sample::fromJson(const QByteArray &json, const Product &product)
{
    const auto array = QJsonDocument::fromJson(json).array();
    QVector<Sample> samples;
    samples.reserve(array.size());
    for (auto it = array.begin(); it != array.end(); ++it) {
        const auto obj = it->toObject();
        Sample s;
        s.d->timestamp = QDateTime::fromString(obj.value(QStringLiteral("timestamp")).toString(), Qt::ISODate);
        foreach (const auto &entry, product.schema()) {
            if (!obj.contains(entry.name()))
                continue;
            switch (entry.dataType()) {
                case SchemaEntry::Scalar:
                {
                    const auto entryData = obj.value(entry.name()).toObject();
                    foreach (const auto &elem, entry.elements()) {
                        if (!entryData.contains(elem.name()))
                            continue;
                        // TODO schema-dependent type conversion
                        s.d->data.insert(entry.name() + QLatin1Char('.') + elem.name(), entryData.value(elem.name()).toVariant());
                    }
                    break;
                }
                case SchemaEntry::List:
                {
                    const auto entryArray = obj.value(entry.name()).toArray();
                    QVariantList l;
                    l.reserve(entryArray.size());
                    foreach (const auto &entryDataValue, entryArray) {
                        const auto entryData = entryDataValue.toObject();
                        QVariantMap m;
                        foreach (const auto &elem, entry.elements()) {
                            if (!entryData.contains(elem.name()))
                                continue;
                            // TODO schema-dependent type conversion
                            m.insert(elem.name(), entryData.value(elem.name()).toVariant());
                        }
                        l.push_back(m);
                    }
                    s.d->data.insert(entry.name(), l);
                    break;
                }
                case SchemaEntry::Map:
                {
                    const auto entryMap = obj.value(entry.name()).toObject();
                    QVariantMap m;
                    for (auto it = entryMap.begin(); it != entryMap.end(); ++it) {
                        const auto entryData = it.value().toObject();
                        QVariantMap m2;
                        foreach (const auto &elem, entry.elements()) {
                            if (!entryData.contains(elem.name()))
                                continue;
                            // TODO schema-dependent type conversion
                            m2.insert(elem.name(), entryData.value(elem.name()).toVariant());
                        }
                        m.insert(it.key(), m2);
                    }
                    s.d->data.insert(entry.name(), m);
                    break;
                }
            }
        }
        samples.push_back(s);
    }
    return samples;
}

QByteArray Sample::toJson(const QVector<Sample> &samples, const Product &product)
{
    QJsonArray array;
    for (const auto &s : samples) {
        QJsonObject obj;
        obj[QLatin1String("timestamp")] = s.timestamp().toString(Qt::ISODate);
        foreach (const auto &entry, product.schema()) {
            switch (entry.dataType()) {
                case SchemaEntry::Scalar:
                {
                    QJsonObject subObj;
                    const auto entryData = obj.value(entry.name()).toObject();
                    foreach (const auto &elem, entry.elements()) {
                        const auto it = s.d->data.constFind(entry.name() + QLatin1Char('.') + elem.name());
                        if (it == s.d->data.constEnd())
                            continue;
                        subObj[elem.name()] = QJsonValue::fromVariant(it.value());
                    }
                    if (!subObj.isEmpty())
                        obj[entry.name()] = subObj;
                    break;
                }
                case SchemaEntry::List:
                {
                    QJsonArray a;
                    const auto it = s.d->data.constFind(entry.name());
                    if (it == s.d->data.constEnd())
                        continue;
                    const auto l = it.value().toList();
                    for (const auto &v : l) {
                        QJsonObject subObj;
                        const auto m = v.toMap();
                        for (auto it = m.begin(); it != m.end(); ++it) {
                            subObj[it.key()] = QJsonValue::fromVariant(it.value());
                        }
                        a.push_back(subObj);
                    }
                    obj[entry.name()] = a;
                    break;
                }
                case SchemaEntry::Map:
                {
                    QJsonObject map;
                    const auto it = s.d->data.constFind(entry.name());
                    if (it == s.d->data.constEnd())
                        continue;
                    const auto m = it.value().toMap();
                    for (auto it = m.begin(); it != m.end(); ++it) {
                        QJsonObject subObj;
                        const auto m = it.value().toMap();
                        for (auto it = m.begin(); it != m.end(); ++it) {
                            subObj[it.key()] = QJsonValue::fromVariant(it.value());
                        }
                        map[it.key()] = subObj;
                    }
                    obj[entry.name()] = map;
                    break;
                }
            }
        }
        array.push_back(obj);
    }

    QJsonDocument doc;
    doc.setArray(array);
    return doc.toJson();
}
