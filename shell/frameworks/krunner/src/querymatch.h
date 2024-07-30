/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KRUNNER_QUERYMATCH_H
#define KRUNNER_QUERYMATCH_H

#include <QList>
#include <QSharedDataPointer>
#include <QUrl>

#include "krunner_export.h"

class QIcon;
class QVariant;

namespace KRunner
{
class Action;
class AbstractRunner;
class QueryMatchPrivate;

/**
 * @class QueryMatch querymatch.h <KRunner/QueryMatch>
 *
 * @short A match returned by an AbstractRunner in response to a given RunnerContext.
 */
class KRUNNER_EXPORT QueryMatch
{
public:
    /**
     * Constructs a PossibleMatch associated with a given RunnerContext
     * and runner.
     *
     * @param runner the runner this match belongs to
     */
    explicit QueryMatch(AbstractRunner *runner = nullptr);

    /**
     * Copy constructor
     */
    QueryMatch(const QueryMatch &other);

    ~QueryMatch();
    QueryMatch &operator=(const QueryMatch &other);
    bool operator==(const QueryMatch &other) const;
    bool operator!=(const QueryMatch &other) const;

    /**
     * @return the runner associated with this action
     */
    AbstractRunner *runner() const;

    /**
     * @return true if the match is valid and can therefore be run,
     *         an invalid match does not have an associated AbstractRunner
     */
    bool isValid() const;

    /**
     * Helper for reading standardized category relevance values
     */
    enum class CategoryRelevance {
        Lowest = 0,
        Low = 30,
        Moderate = 50,
        High = 70,
        Highest = 100,
    };

    /**
     * Relevance for matches in the category. The match with the highest relevance is respected for the entire category.
     * This value only affects the sorting of categories and not the sorting within the category. Use @ref setRelevance for this.
     * The value should be from 0 to 100.
     *
     * @since 6.0
     */
    void setCategoryRelevance(CategoryRelevance relevance)
    {
        setCategoryRelevance(qToUnderlying(relevance));
    }

    /**
     * @internal Internal for now, consumers should utilize CategoryRelevance enum
     *
     * @since 6.0
     */
    void setCategoryRelevance(qreal relevance);

    /**
     * Category relevance for this match
     *
     * @since 6.0
     */
    qreal categoryRelevance() const;

    /**
     * Sets information about the type of the match which is
     * used to group the matches.
     *
     * This string should be translated as it is displayed in an UI.
     * The default is @ref AbstractRunner::name
     */
    void setMatchCategory(const QString &category);

    /**
     * Extra information about the match which can be used
     * to categorize the type.
     *
     * The default is @ref AbstractRunner::name
     */
    QString matchCategory() const;

    /**
     * Sets the relevance of this action for the search
     * it was created for.
     *
     * @param relevance a number between 0 and 1.
     */
    void setRelevance(qreal relevance);

    /**
     * The relevance of this action to the search. By default,
     * the relevance is 1.
     *
     * @return a number between 0 and 1
     */
    qreal relevance() const;

    /**
     * Sets data to be used internally by the runner's @ref AbstractRunner::run implementation.
     *
     * When set, it is also used to form part of the @ref id for this match.
     * If that is inappropriate as an id, the runner may generate its own
     * id and set that with @ref setId
     */
    void setData(const QVariant &data);

    /**
     * @return the data associated with this match; usually runner-specific
     */
    QVariant data() const;

    /**
     * Sets the id for this match; useful if the id does not
     * match data().toString(). The id must be unique to all
     * matches from this runner, and should remain constant
     * for the same query for best results.
     *
     * If the "X-Plasma-Runner-Unique-Results" property from the metadata
     * is set to true, the runnerId will not be prepended to the ID.
     * This allows KRunner to de-duplicate results from different runners.
     * In case the runner's matches are less specific than ones from other runners, the
     * "X-Plasma-Runner-Weak-Results" property can be set so that duplicates from this
     * runner are removed.
     *
     * @param id the new identifying string to use to refer
     *           to this entry
     */
    void setId(const QString &id);

    /**
     * @return a string that can be used as an ID for this match,
     * even between different queries. It is based in part
     * on the source of the match (the AbstractRunner) and
     * distinguishing information provided by the runner,
     * ensuring global uniqueness as well as consistency
     * between query matches.
     */
    QString id() const;

    /**
     * Sets the main title text for this match; should be short
     * enough to fit nicely on one line in a user interface
     * For styled and multiline text, @ref setMultiLine should be set to true
     *
     * @param text the text to use as the title
     */
    void setText(const QString &text);

    /**
     * @return the title text for this match
     */
    QString text() const;

    /**
     * Sets the descriptive text for this match; can be longer
     * than the main title text
     *
     * @param text the text to use as the description
     */
    void setSubtext(const QString &text);

    /**
     * @return the descriptive text for this match
     */
    QString subtext() const;

    /**
     * Sets the icon associated with this match
     *
     * Prefer using setIconName.
     *
     * @param icon the icon to show along with the match
     */
    void setIcon(const QIcon &icon);

    /**
     * @return the icon for this match
     */
    QIcon icon() const;

    /**
     * Sets the icon name associated with this match
     *
     * @param icon the name of the icon to show along with the match
     * @since 5.24
     */
    void setIconName(const QString &iconName);

    /**
     * @return the name of the icon for this match
     * @since 5.24
     */
    QString iconName() const;

    /**
     * Sets the urls, if any, associated with this match
     */
    void setUrls(const QList<QUrl> &urls);

    /**
     * @return the urls for this match, empty list if none
     * These will be used in the default implementation of AbstractRunner::mimeDataForMatch
     */
    QList<QUrl> urls() const;

    /**
     * Sets whether or not this match can be activited
     *
     * @param enable true if the match is enabled and therefore runnable
     */
    void setEnabled(bool enable);

    /**
     * @return true if the match is enabled and therefore runnable, otherwise false
     */
    bool isEnabled() const;

    /**
     * Set the actions for this match.
     * This method allows you to set the actions inside of the AbstractRunner::match method
     * @see RunnerManager::actionsForMatch
     * @since 5.75
     */
    void setActions(const QList<KRunner::Action> &actions);

    /**
     * Adds an action to this match
     * @since 5.75
     * @see setActions
     */
    void addAction(const KRunner::Action &action);

    /**
     * List of actions set for this match
     * @return actions
     * @since 5.75
     */
    QList<KRunner::Action> actions() const;

    /**
     * The action that the user has selected when running the match.
     * This returns a nullptr if no action was selected.
     */
    KRunner::Action selectedAction() const;

    /**
     * Set if the text should be displayed as a multiLine string
     * @param multiLine
     * @since 5.82
     */
    void setMultiLine(bool multiLine);

    /**
     * If the text should be displayed as a multiLine string
     * If no explicit value is set set using setMultiline it will default to false
     * @return bool
     * @since 5.82
     */
    bool isMultiLine() const;

private:
    KRUNNER_NO_EXPORT void setSelectedAction(const KRunner::Action &action);
    friend class RunnerManager;
    QSharedDataPointer<QueryMatchPrivate> d;
};

/// @since 6.0
KRUNNER_EXPORT QDebug operator<<(QDebug debug, const KRunner::QueryMatch &match);
}
#endif
