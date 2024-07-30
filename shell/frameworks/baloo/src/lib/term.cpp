/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "term.h"

#include <QDateTime>

namespace Baloo {

class Baloo::Term::Private {
public:
    Operation m_op = None;
    Comparator m_comp = Auto;

    QString m_property;
    QVariant m_value;

    bool m_isNegated = false;

    QList<Term> m_subTerms;
    QVariantHash m_userData;
};

Term::Term()
    : d(new Private)
{
}

Term::Term(const Term& t)
    : d(new Private(*t.d))
{
}

Term::Term(const QString& property)
    : d(new Private)
{
    d->m_property = property;
}

Term::Term(const QString& property, const QVariant& value, Term::Comparator c)
    : d(new Private)
{
    d->m_property = property;
    d->m_value = value;

    if (c == Auto) {
        if (value.typeId() == QMetaType::QString) {
            d->m_comp = Contains;
        } else if (value.typeId() == QMetaType::QDateTime) {
            d->m_comp = Contains;
        } else {
            d->m_comp = Equal;
        }
    }
    else {
        d->m_comp = c;
    }
}

/*
Term::Term(const QString& property, const QVariant& start, const QVariant& end)
    : d(new Private)
{
    d->m_property = property;
    d->m_op = Range;

    // FIXME: How to save range queries?
}
*/

Term::Term(Term::Operation op)
    : d(new Private)
{
    d->m_op = op;
}

Term::Term(Term::Operation op, const Term& t)
    : d(new Private)
{
    d->m_op = op;
    d->m_subTerms << t;
}

Term::Term(Term::Operation op, const QList<Term>& t)
    : d(new Private)
{
    d->m_op = op;
    d->m_subTerms = t;
}

Term::Term(const Term& lhs, Term::Operation op, const Term& rhs)
    : d(new Private)
{
    d->m_op = op;
    if (lhs.isEmpty()) {
	*d = *(rhs.d);
	return;
    }
    if (rhs.isEmpty()) {
	*d = *(lhs.d);
	return;
    }

    if (lhs.operation() == op) {
        d->m_subTerms << lhs.subTerms();
    } else {
        d->m_subTerms << lhs;
    }

    if (rhs.operation() == op) {
        d->m_subTerms << rhs.subTerms();
    } else {
        d->m_subTerms << rhs;
    }
}

Term::~Term() = default;

bool Term::isValid() const
{
    // Terms with an operator but no subterms are still valid
    if (d->m_op != Term::None) {
        return true;
    }

    if (d->m_comp == Term::Auto) {
        return false;
    }

    return true;
}

void Term::setNegation(bool isNegated)
{
    d->m_isNegated = isNegated;
}

bool Term::isNegated() const
{
    return d->m_isNegated;
}

bool Term::negated() const
{
    return d->m_isNegated;
}

void Term::addSubTerm(const Term& term)
{
    d->m_subTerms << term;
}

void Term::setSubTerms(const QList<Term>& terms)
{
    d->m_subTerms = terms;
}

Term Term::subTerm() const
{
    if (!d->m_subTerms.isEmpty()) {
        return d->m_subTerms.first();
    }

    return Term();
}

QList<Term> Term::subTerms() const
{
    return d->m_subTerms;
}

void Term::setOperation(Term::Operation op)
{
    d->m_op = op;
}

Term::Operation Term::operation() const
{
    return d->m_op;
}

bool Term::empty() const
{
    return isEmpty();
}

bool Term::isEmpty() const
{
    return d->m_property.isEmpty() && d->m_value.isNull() && d->m_subTerms.isEmpty();
}

QString Term::property() const
{
    return d->m_property;
}

void Term::setProperty(const QString& property)
{
    d->m_property = property;
}

void Term::setValue(const QVariant& value)
{
    d->m_value = value;
}

QVariant Term::value() const
{
    return d->m_value;
}

Term::Comparator Term::comparator() const
{
    return d->m_comp;
}

void Term::setComparator(Term::Comparator c)
{
    d->m_comp = c;
}

void Term::setUserData(const QString& name, const QVariant& value)
{
    d->m_userData.insert(name, value);
}

QVariant Term::userData(const QString& name) const
{
    return d->m_userData.value(name);
}

QVariantMap Term::toVariantMap() const
{
    QVariantMap map;
    if (d->m_op != None) {
        QVariantList variantList;
        for (const Term& term : std::as_const(d->m_subTerms)) {
            variantList << QVariant(term.toVariantMap());
        }

        if (d->m_op == And) {
            map[QStringLiteral("$and")] = variantList;
        } else {
            map[QStringLiteral("$or")] = variantList;
        }

        return map;
    }

    QString op;
    switch (d->m_comp) {
    case Equal:
        map[d->m_property] = d->m_value;
        return map;

    case Contains:
        op = QStringLiteral("$ct");
        break;

    case Greater:
        op = QStringLiteral("$gt");
        break;

    case GreaterEqual:
        op = QStringLiteral("$gte");
        break;

    case Less:
        op = QStringLiteral("$lt");
        break;

    case LessEqual:
        op = QStringLiteral("$lte");
        break;

    case Auto:
        Q_ASSERT(0);
    }

    QVariantMap m;
    m[op] = d->m_value;
    map[d->m_property] = QVariant(m);

    return map;
}

namespace {
    // QJson does not recognize QDate/QDateTime parameters. We try to guess
    // and see if they can be converted into date/datetime.
    QVariant tryConvert(const QVariant& var) {
    if (var.canConvert<QDateTime>()) {
        QDateTime dt = var.toDateTime();
        if (!dt.isValid()) {
            return var;
        }

        if (!var.toString().contains(QLatin1Char('T'))) {
            return QVariant(var.toDate());
        }
        return dt;
    }
        return var;
    }
}

Term Term::fromVariantMap(const QVariantMap& map)
{
    if (map.size() != 1) {
        return Term();
    }

    Term term;

    QString andOrString;
    if (map.contains(QLatin1String("$and"))) {
        andOrString = QStringLiteral("$and");
        term.setOperation(And);
    }
    else if (map.contains(QLatin1String("$or"))) {
        andOrString = QStringLiteral("$or");
        term.setOperation(Or);
    }

    if (!andOrString.isEmpty()) {
        QList<Term> subTerms;

        const QVariantList list = map[andOrString].toList();
        for (const QVariant &var : list) {
            subTerms << Term::fromVariantMap(var.toMap());
        }

        term.setSubTerms(subTerms);
        return term;
    }

    QString prop = map.cbegin().key();
    term.setProperty(prop);

    QVariant value = map.value(prop);
    if (value.typeId() == QMetaType::QVariantMap) {
        QVariantMap mapVal = value.toMap();
        if (mapVal.size() != 1) {
            return term;
        }

        QString op = mapVal.cbegin().key();
        Term::Comparator com;
        if (op == QLatin1String("$ct")) {
            com = Contains;
        } else if (op == QLatin1String("$gt")) {
            com = Greater;
        } else if (op == QLatin1String("$gte")) {
            com = GreaterEqual;
        } else if (op == QLatin1String("$lt")) {
            com = Less;
        } else if (op == QLatin1String("$lte")) {
            com = LessEqual;
        } else {
            return term;
        }

        term.setComparator(com);
        term.setValue(tryConvert(mapVal.value(op)));

        return term;
    }

    term.setComparator(Equal);
    term.setValue(tryConvert(value));

    return term;
}

bool Term::operator==(const Term& rhs) const
{
    if (d->m_op != rhs.d->m_op || d->m_comp != rhs.d->m_comp ||
        d->m_isNegated != rhs.d->m_isNegated || d->m_property != rhs.d->m_property ||
        d->m_value != rhs.d->m_value)
    {
        return false;
    }

    if (d->m_subTerms.size() != rhs.d->m_subTerms.size()) {
        return false;
    }

    if (d->m_subTerms.isEmpty()) {
        return true;
    }

    for (const Term& t : std::as_const(d->m_subTerms)) {
        if (!rhs.d->m_subTerms.contains(t)) {
            return false;
        }
    }

    return true;
}

Term& Term::operator=(const Term& rhs)
{
    *d = *rhs.d;
    return *this;
}

char *toString(const Term& term)
{
    QString buffer;
    QDebug stream(&buffer);
    stream << term;
    return qstrdup(buffer.toUtf8().constData());
}

} // namespace Baloo

namespace {
    QString comparatorToString(Baloo::Term::Comparator c) {
        switch (c) {
        case Baloo::Term::Auto:
            return QStringLiteral("Auto");
        case Baloo::Term::Equal:
            return QStringLiteral("=");
        case Baloo::Term::Contains:
            return QStringLiteral(":");
        case Baloo::Term::Less:
            return QStringLiteral("<");
        case Baloo::Term::LessEqual:
            return QStringLiteral("<=");
        case Baloo::Term::Greater:
            return QStringLiteral(">");
        case Baloo::Term::GreaterEqual:
            return QStringLiteral(">=");
        }

        return QString();
    }

    QString operationToString(Baloo::Term::Operation op) {
        switch (op) {
        case Baloo::Term::None:
            return QStringLiteral("NONE");
        case Baloo::Term::And:
            return QStringLiteral("AND");
        case Baloo::Term::Or:
            return QStringLiteral("OR");
        }

        return QString();
    }
}

QDebug operator <<(QDebug d, const Baloo::Term& t)
{
    QDebugStateSaver saver(d);
    d.noquote().nospace();
    if (t.subTerms().isEmpty()) {
        if (!t.property().isEmpty()) {
            d << t.property();
        }
	d << comparatorToString(t.comparator());
    if (t.value().typeId() == QMetaType::QString) {
            d << QLatin1Char('"') << t.value().toString() << QLatin1Char('"');
    } else {
            d << t.value().typeName() << QLatin1Char('(')
              << t.value().toString() << QLatin1Char(')');
    }
    }
    else {
        d << "[" << operationToString(t.operation());
        const auto subTerms = t.subTerms();
        for (const Baloo::Term& term : subTerms) {
            d << QLatin1Char(' ') << term;
        }
        d << "]";
    }
    return d;
}
