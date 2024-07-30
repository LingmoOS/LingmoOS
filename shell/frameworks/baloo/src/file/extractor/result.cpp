/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "result.h"
#include "propertydata.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <QDateTime>
#include <KFileMetaData/PropertyInfo>
#include <KFileMetaData/TypeInfo>

// In order to use it in a vector
Result::Result()
    : ExtractionResult(QString(), QString())
    , m_termGen(m_doc)
    , m_termGenForText(m_doc)
{
}

Result::Result(const QString& url, const QString& mimetype, const Flags& flags)
    : KFileMetaData::ExtractionResult(url, mimetype, flags)
    , m_termGen(m_doc)
    , m_termGenForText(m_doc)
{
}

void Result::add(KFileMetaData::Property::Property property, const QVariant& value)
{
    if (value.typeId() == QMetaType::QStringList) {
        const auto valueList = value.toStringList();
        for (const auto& val : valueList) {
            m_map.insert(property, val);
        }
    } else {
        m_map.insert(property, value);
    }

    int propNum = static_cast<int>(property);
    QByteArray prefix = 'X' + QByteArray::number(propNum) + '-';

    if (value.typeId() == QMetaType::Bool) {
        m_doc.addTerm(prefix);
    } else if (value.typeId() == QMetaType::Int || value.typeId() == QMetaType::UInt) {
        const QByteArray term = prefix + value.toString().toUtf8();
        m_doc.addTerm(term);
    } else if (value.typeId() == QMetaType::QDate) {
        const QByteArray term = prefix + value.toDate().toString(Qt::ISODate).toUtf8();
        m_doc.addTerm(term);
    } else if (value.typeId() == QMetaType::QDateTime) {
        const QByteArray term = prefix + value.toDateTime().toString(Qt::ISODate).toUtf8();
        m_doc.addTerm(term);
    } else if (value.typeId() == QMetaType::QStringList) {
        bool shouldBeIndexed = KFileMetaData::PropertyInfo(property).shouldBeIndexed();
        const auto valueList = value.toStringList();
        for (const auto& val : valueList)
        {
            if (val.isEmpty()) {
                continue;
            }
            m_termGen.indexText(val, prefix);
            if (shouldBeIndexed) {
                m_termGen.indexText(val);
            }
        }
    } else {
        const QString val = value.toString();
        if (val.isEmpty()) {
            return;
        }

        m_termGen.indexText(val, prefix);
        KFileMetaData::PropertyInfo pi(property);
        if (pi.shouldBeIndexed()) {
            m_termGen.indexText(val);
        }
    }
}

void Result::append(const QString& text)
{
    m_termGenForText.indexText(text);
}

void Result::addType(KFileMetaData::Type::Type type)
{
    QByteArray num = QByteArray::number(static_cast<int>(type));
    m_doc.addTerm(QByteArray("T") + num);
}

void Result::finish()
{
    if (m_map.isEmpty()) {
        m_doc.setData(QByteArray());
        return;
    }
    QJsonObject jo = Baloo::propertyMapToJson(m_map);
    QJsonDocument jdoc;
    jdoc.setObject(jo);
    m_doc.setData(jdoc.toJson(QJsonDocument::JsonFormat::Compact));
}

void Result::setDocument(const Baloo::Document& doc)
{
    m_doc = doc;
    // All document metadata are indexed from position 1000
    m_termGen.setDocument(m_doc);
    m_termGen.setPosition(1000);

    // All document plain text starts from 10000. This is done to avoid
    // clashes with the term positions
    m_termGenForText.setDocument(m_doc);
    m_termGenForText.setPosition(10000);
}
