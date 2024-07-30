/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_SCHEMAENTRYELEMENT_H
#define KUSERFEEDBACK_CONSOLE_SCHEMAENTRYELEMENT_H

#include <qobjectdefs.h>
#include <QMetaType>
#include <QSharedDataPointer>
#include <QVector>

QT_BEGIN_NAMESPACE
class QJsonArray;
class QJsonObject;
QT_END_NAMESPACE

namespace KUserFeedback {
namespace Console {

class SchemaEntryElementData;

/** One element in a SchemaEntry. */
class SchemaEntryElement
{
    Q_GADGET
public:
    enum Type {
        Integer,
        Number,
        String,
        Boolean
    };
    Q_ENUM(Type)

    SchemaEntryElement();
    SchemaEntryElement(const SchemaEntryElement &other);
    ~SchemaEntryElement();
    SchemaEntryElement& operator=(const SchemaEntryElement &other);

    bool operator==(const SchemaEntryElement &other) const;

    QString name() const;
    void setName(const QString& name);

    Type type() const;
    void setType(Type type);

    QJsonObject toJsonObject() const;
    static QVector<SchemaEntryElement> fromJson(const QJsonArray &array);

private:
    QSharedDataPointer<SchemaEntryElementData> d;
};
}
}

Q_DECLARE_METATYPE(KUserFeedback::Console::SchemaEntryElement)
Q_DECLARE_METATYPE(KUserFeedback::Console::SchemaEntryElement::Type)
Q_DECLARE_TYPEINFO(KUserFeedback::Console::SchemaEntryElement, Q_MOVABLE_TYPE);

#endif // KUSERFEEDBACK_CONSOLE_SCHEMAENTRYELEMENT_H
