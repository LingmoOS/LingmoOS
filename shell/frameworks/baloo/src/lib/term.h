/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_TERM_H
#define BALOO_TERM_H

#include <QString>
#include <QVariant>
#include <QDebug>

#include <memory>

namespace Baloo {

class Term
{
public:
    enum Comparator {
        Auto,
        Equal,
        Contains,
        Greater,
        GreaterEqual,
        Less,
        LessEqual,
    };

    enum Operation {
        None,
        And,
        Or,
    };

    Term();
    Term(const Term& t);

    /**
     * The Item must contain the property \p property
     */
    explicit Term(const QString& property);

    /**
     * The Item must contain the property \p property with
     * value \value.
     *
     * The default comparator is Auto which has the following behavior
     * For Strings - Contains
     * For DateTime - Contains
     * For any other type - Equals
     */
    Term(const QString& property, const QVariant& value, Comparator c = Auto);

    /**
     * This term is a combination of other terms
     */
    explicit Term(Operation op);
    Term(Operation op, const Term& t);
    Term(Operation op, const QList<Term>& t);
    Term(const Term& lhs, Operation op, const Term& rhs);
    ~Term();

    bool isValid() const;

    /**
     * Negate this term. Negation only applies for Equal or Contains
     * For other Comparators you must invert it yourself
     */
    void setNegation(bool isNegated);

    bool negated() const;
    bool isNegated() const;

    void addSubTerm(const Term& term);
    void setSubTerms(const QList<Term>& terms);

    /**
     * Returns the first subTerm in the list of subTerms
     */
    Term subTerm() const;
    QList<Term> subTerms() const;

    void setOperation(Operation op);
    Operation operation() const;

    bool isEmpty() const;
    bool empty() const;

    /**
     * Return the property this term is targeting
     */
    QString property() const;
    void setProperty(const QString& property);

    QVariant value() const;
    void setValue(const QVariant& value);

    Comparator comparator() const;
    void setComparator(Comparator c);

    void setUserData(const QString& name, const QVariant& value);
    QVariant userData(const QString& name) const;

    QVariantMap toVariantMap() const;
    static Term fromVariantMap(const QVariantMap& map);

    bool operator == (const Term& rhs) const;

    Term& operator=(const Term& rhs);

private:
    class Private;
    std::unique_ptr<Private> const d;
};

inline Term operator &&(const Term& lhs, const Term& rhs)
{
    if (lhs.isEmpty())
        return rhs;
    else if (rhs.isEmpty())
        return lhs;

    return {lhs, Term::And, rhs};
}

inline Term operator ||(const Term& lhs, const Term& rhs)
{
    if (lhs.isEmpty())
        return rhs;
    else if (rhs.isEmpty())
        return lhs;

    return {lhs, Term::Or, rhs};
}

inline Term operator !(const Term& rhs)
{
    Term t(rhs);
    t.setNegation(!rhs.isNegated());
    return t;
}

/**
 * Helper for QTest
 * \sa QTest::toString
 *
 * @since: 5.70
 */
char *toString(const Term& term);

}

QDebug operator <<(QDebug d, const Baloo::Term& t);

Q_DECLARE_TYPEINFO(Baloo::Term, Q_RELOCATABLE_TYPE);

#endif
