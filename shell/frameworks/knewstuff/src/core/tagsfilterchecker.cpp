/*
    SPDX-FileCopyrightText: 2018 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "tagsfilterchecker.h"

#include <knewstuffcore_debug.h>

#include <QMap>

namespace KNSCore
{
class TagsFilterCheckerPrivate
{
public:
    TagsFilterCheckerPrivate()
    {
    }
    ~TagsFilterCheckerPrivate()
    {
        qDeleteAll(validators);
    }
    class Validator;
    // If people start using a LOT of validators (>20ish), we can always change it, but
    // for now it seems reasonable that QMap is better than QHash here...
    QMap<QString, Validator *> validators;

    class Validator
    {
    public:
        Validator(const QString &tag, const QString &value)
            : m_tag(tag)
        {
            if (!value.isNull()) {
                m_acceptedValues << value;
            }
        }
        virtual ~Validator()
        {
        }
        virtual bool filterAccepts(const QString &tag, const QString &value) = 0;

    protected:
        friend class TagsFilterCheckerPrivate;
        QString m_tag;
        QStringList m_acceptedValues;
    };

    // Will only accept entries which have one of the accepted values set for the tag key
    class EqualityValidator : public Validator
    {
    public:
        EqualityValidator(const QString &tag, const QString &value)
            : Validator(tag, value)
        {
        }
        ~EqualityValidator() override
        {
        }
        bool filterAccepts(const QString &tag, const QString &value) override
        {
            bool result = true;
            if (tag == m_tag && !m_acceptedValues.contains(value)) {
                qCDebug(KNEWSTUFFCORE) << "Item excluded by filter on" << m_tag << "because" << value << "was not included in" << m_acceptedValues;
                result = false;
            }
            return result;
        }
    };

    // Will only accept entries which have none of the values set for the tag key
    class InequalityValidator : public Validator
    {
    public:
        InequalityValidator(const QString &tag, const QString &value)
            : Validator(tag, value)
        {
        }
        ~InequalityValidator() override
        {
        }
        bool filterAccepts(const QString &tag, const QString &value) override
        {
            bool result = true;
            if (tag == m_tag && m_acceptedValues.contains(value)) {
                qCDebug(KNEWSTUFFCORE) << "Item excluded by filter on" << m_tag << "because" << value << "was included in" << m_acceptedValues;
                result = false;
            }
            return result;
        }
    };

    void addValidator(const QString &filter)
    {
        int pos = 0;
        if ((pos = filter.indexOf(QLatin1String("=="))) > -1) {
            QString tag = filter.left(pos);
            QString value = filter.mid(tag.length() + 2);
            Validator *val = validators.value(tag, nullptr);
            if (!val) {
                val = new EqualityValidator(tag, QString());
                validators.insert(tag, val);
            }
            val->m_acceptedValues << value;
            qCDebug(KNEWSTUFFCORE) << "Created EqualityValidator for tag" << tag << "with value" << value;
        } else if ((pos = filter.indexOf(QLatin1String("!="))) > -1) {
            QString tag = filter.left(pos);
            QString value = filter.mid(tag.length() + 2);
            Validator *val = validators.value(tag, nullptr);
            if (!val) {
                val = new InequalityValidator(tag, QString());
                validators.insert(tag, val);
            }
            val->m_acceptedValues << value;
            qCDebug(KNEWSTUFFCORE) << "Created InequalityValidator for tag" << tag << "with value" << value;
        } else {
            qCDebug(KNEWSTUFFCORE) << "Critical error attempting to create tag filter validators. The filter is defined as" << filter
                                   << "which is not in the accepted formats key==value or key!=value";
        }
    }
};

TagsFilterChecker::TagsFilterChecker(const QStringList &tagFilter)
    : d(new TagsFilterCheckerPrivate)
{
    for (const QString &filter : tagFilter) {
        d->addValidator(filter);
    }
}

TagsFilterChecker::~TagsFilterChecker() = default;

bool TagsFilterChecker::filterAccepts(const QStringList &tags)
{
    // if any tag in the content matches any of the tag filters, skip this entry
    qCDebug(KNEWSTUFFCORE) << "Checking tags list" << tags << "against validators with keys" << d->validators.keys();
    for (const QString &tag : tags) {
        if (tag.isEmpty()) {
            // This happens when you do a split on an empty string (not an empty list, a list with one empty element... because reasons).
            // Also handy for other things, i guess, though, so let's just catch it here.
            continue;
        }
        QStringList current = tag.split(QLatin1Char('='));
        if (current.length() > 2) {
            qCDebug(KNEWSTUFFCORE) << "Critical error attempting to filter tags. Entry has tag defined as" << tag
                                   << "which is not in the format \"key=value\" or \"key\".";
            return false;
        } else if (current.length() == 1) {
            // If the tag is defined simply as a key, we give it the value "1", just to make our filtering work simpler
            current << QStringLiteral("1");
        }
        QMap<QString, TagsFilterCheckerPrivate::Validator *>::const_iterator i = d->validators.constBegin();
        while (i != d->validators.constEnd()) {
            if (!i.value()->filterAccepts(current.at(0), current.at(1))) {
                return false;
            }
            ++i;
        }
    }
    // If we have arrived here, nothing has filtered the entry
    // out (by being either incorrectly tagged or a filter rejecting
    // it), and consequently it is an acceptable entry.
    return true;
}

}
