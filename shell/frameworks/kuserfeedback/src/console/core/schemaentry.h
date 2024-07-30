/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_SCHEMAENTRY_H
#define KUSERFEEDBACK_CONSOLE_SCHEMAENTRY_H

#include <QObject>
#include <QSharedDataPointer>
#include <QVector>

class QJsonArray;
class QJsonObject;

namespace KUserFeedback {
namespace Console {

class SchemaEntryData;
class SchemaEntryElement;

/** Represents one schema entry.
 *  A schema entry can be a scalar, a list or a map, and consists of one or
 *  more named entries. A schema entry can also have higher-level aggregation
 *  hints, used for visualizing the recorded data. The following examples
 *  list common configurations.
 *
 *  \li Product version, Qt version, framework version:
 *  scalar, one string element containing the version, category aggregation
 *  \li Usage time, start count:
 *  scalar, one integer element containing the data, numeric aggregation
 *  \li Platform type/version:
 *  scalar, two string elements containing platform type/sub-type, hierarchical category aggregation
 *  \li View usage/feature usage ratios:
 *  Option 1 (fixed set): scalar, multiple numeric elements per feature, ratio set aggregation
 *  Option 2 (variable set): map for feature to single numeric element, ratio set aggregation
 *  \li Screen size:
 *  list (one entry per screen), two numeric elements for height/width, x/y aggregation
 *  \li Screen resolution, screen count:
 *  scalar of single numeric element, numeric aggregation
 *  \li event counts:
 *  scalar, multiple numeric elements, numeric aggregation
 *  \li language settings:
 *  scalar, one string element, category aggregation
 */
class SchemaEntry
{
    Q_GADGET
public:
    enum DataType {
        Scalar,
        List,
        Map
    };
    Q_ENUM(DataType)

    SchemaEntry();
    SchemaEntry(const SchemaEntry &entry);
    ~SchemaEntry();
    SchemaEntry& operator=(const SchemaEntry &entry);

    bool operator==(const SchemaEntry &other) const;

    int internalId() const;
    void setInternalId(int internalId);

    QString name() const;
    void setName(const QString& name);

    DataType dataType() const;
    void setDataType(DataType type);

    QVector<SchemaEntryElement> elements() const;
    void setElements(const QVector<SchemaEntryElement> &elements);
    SchemaEntryElement element(const QString &name) const;

    QJsonObject toJsonObject() const;
    static QVector<SchemaEntry> fromJson(const QJsonArray &array);

private:
    QSharedDataPointer<SchemaEntryData> d;
};

}
}

Q_DECLARE_METATYPE(KUserFeedback::Console::SchemaEntry)
Q_DECLARE_TYPEINFO(KUserFeedback::Console::SchemaEntry, Q_MOVABLE_TYPE);

#endif // KUSERFEEDBACK_CONSOLE_PRODUCTSCHEMAENTRY_H
