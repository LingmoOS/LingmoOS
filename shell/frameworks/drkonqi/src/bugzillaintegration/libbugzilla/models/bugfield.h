/*
    SPDX-FileCopyrightText: 2019-2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BUGFIELD_H
#define BUGFIELD_H

#include <QObject>
#include <QPointer>
#include <QVariantHash>

namespace Bugzilla
{
/// https://bugzilla.readthedocs.io/en/5.0/api/core/v1/field.html
class BugFieldValue : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name MEMBER m_name NOTIFY changed)
    // not mapped because of lazyness and lack of need
    //    Q_PROPERTY(int sort_key READ sort_key WRITE setSort_key)
    //    // visibility_values not mapped
    //    Q_PROPERTY(bool is_active READ is_active WRITE setIs_active)
    //    Q_PROPERTY(QString description READ description WRITE setDescription)
    //    Q_PROPERTY(bool is_open READ is_open WRITE setIs_open)
    //    // can_change_to not mapped
public:
    explicit BugFieldValue(const QVariantHash &obj, QObject *parent = nullptr);

    QString name() const;

Q_SIGNALS:
    void changed();

private:
    QString m_name;
};

/// https://bugzilla.readthedocs.io/en/5.0/api/core/v1/field.html
class BugField : public QObject
{
    Q_OBJECT
    // Not mapped for lazyness and lack of need:
    //    Q_PROPERTY(int id READ id WRITE setId)
    //    Q_PROPERTY(Type type READ type WRITE setType)
    //    Q_PROPERTY(bool is_custom READ is_custom WRITE setIs_custom)
    //    Q_PROPERTY(QString name READ name WRITE setName)
    //    Q_PROPERTY(QString display_name READ display_name WRITE setDisplay_name)
    //    Q_PROPERTY(bool is_mandatory READ is_mandatory WRITE setIs_mandatory)
    //    Q_PROPERTY(bool is_on_bug_entry READ is_on_bug_entry WRITE setIs_on_bug_entry)
    //    Q_PROPERTY(QString visibility_field READ visibility_field WRITE setVisibility_field)
    //    Q_PROPERTY(QString value_field READ value_field WRITE setValue_field)
    Q_PROPERTY(QList<BugFieldValue *> values READ values MEMBER m_values NOTIFY changed)
public:
    enum class Type {
        Invalid = -1,
        Unknown = 0,
        SingleLineString = 1,
        SingleValue = 2,
        MultipleValue = 3,
        MultiLineText = 4,
        DateTime = 5,
        BugId = 6,
        SeeAlso = 7,
        Keywords = 8,
        Date = 9,
        Integer = 10,
    };

    using Ptr = QPointer<BugField>;

    explicit BugField(const QVariantHash &obj, QObject *parent = nullptr);

    QList<BugFieldValue *> values() const;

Q_SIGNALS:
    void changed();

private:
    static void registerVariantConverters();

    QList<BugFieldValue *> m_values;
};

} // namespace Bugzilla

#endif // BUGFIELD_H
