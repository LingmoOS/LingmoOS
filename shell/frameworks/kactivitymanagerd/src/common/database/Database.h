/*
 * SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>
#include <QRegularExpression>
#include <QSqlQuery>
#include <memory>
#include <utils/d_ptr.h>

namespace Common
{
class Database : public QObject
{
    Q_OBJECT

public:
    typedef std::shared_ptr<Database> Ptr;

    enum Source {
        ResourcesDatabase,
    };

    enum OpenMode {
        ReadWrite,
        ReadOnly,
    };

    static Ptr instance(Source source, OpenMode openMode);

    QSqlQuery execQueries(const QStringList &queries) const;
    QSqlQuery execQuery(const QString &query, bool ignoreErrors = false) const;
    QSqlQuery createQuery() const;

    void setPragma(const QString &pragma);
    QVariant pragma(const QString &pragma) const;
    QVariant value(const QString &query) const;

    // For debugging purposes only
    QString lastQuery() const;

    ~Database() override;
    Database();

    friend class Locker;
    class Locker
    {
    public:
        explicit Locker(Database &database);
        ~Locker();

    private:
        QSqlDatabase &m_database;
    };

    void reportError(const QSqlError &error);

#define DATABASE_TRANSACTION(A)                                                                                                                                \
    /* enable this for debugging only: qCDebug(KAMD_LOG_RESOURCES) << "Location:" << __FILE__ << __LINE__; */                                                  \
    Common::Database::Locker lock(A)

Q_SIGNALS:
    void error(const QSqlError &error) const;

private:
    D_PTR;
};

template<typename EscapeFunction>
QString parseStarPattern(const QString &pattern, const QString &joker, EscapeFunction escape)
{
    const auto begin = pattern.constBegin();
    const auto end = pattern.constEnd();

    auto currentStart = pattern.constBegin();
    auto currentPosition = pattern.constBegin();

    bool isEscaped = false;

    // This should be available in the QString class...
    auto stringFromIterators = [&](const QString::const_iterator &currentStart, const QString::const_iterator &currentPosition) {
        return pattern.mid(std::distance(begin, currentStart), std::distance(currentStart, currentPosition));
    };

    // Escaping % and _ for sql like
    // auto escape = [] (QString str) {
    //     return str.replace("%", "\\%").replace("_", "\\_");
    // };

    QString resultPattern;
    resultPattern.reserve(pattern.size() * 1.5);

    for (; currentPosition != end; ++currentPosition) {
        if (isEscaped) {
            // Just skip the current character
            isEscaped = false;

        } else if (*currentPosition == QLatin1Char('\\')) {
            // Skip two characters
            isEscaped = true;

        } else if (*currentPosition == QLatin1Char('*')) {
            // Replacing the star with the sql like joker - %
            resultPattern.append(escape(stringFromIterators(currentStart, currentPosition)) + joker);
            currentStart = currentPosition + 1;

        } else {
            // This one is boring, nothing to do
        }
    }

    if (currentStart != currentPosition) {
        resultPattern.append(escape(stringFromIterators(currentStart, currentPosition)));
    }

    return resultPattern;
}

inline QString escapeSqliteLikePattern(QString pattern)
{
    return pattern.replace(QLatin1String("%"), QLatin1String("\\%")).replace(QLatin1String("_"), QLatin1String("\\_"));
}

inline QString starPatternToLike(const QString &pattern)
{
    return parseStarPattern(pattern, QStringLiteral("%"), escapeSqliteLikePattern);
}

inline QRegularExpression starPatternToRegex(const QString &pattern)
{
    return QRegularExpression(parseStarPattern(pattern, QStringLiteral(".*"), [](QString pattern) { return QRegularExpression::escape(QRegularExpression::anchoredPattern(pattern)); }));
}

} // namespace Common
