/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "querymatch.h"
#include "action.h"

#include <QIcon>
#include <QPointer>
#include <QReadWriteLock>
#include <QSharedData>
#include <QVariant>

#include "abstractrunner.h"
#include "abstractrunner_p.h"

namespace KRunner
{
class QueryMatchPrivate : public QSharedData
{
public:
    explicit QueryMatchPrivate(AbstractRunner *r)
        : QSharedData()
        , runner(r)
    {
    }

    QueryMatchPrivate(const QueryMatchPrivate &other)
        : QSharedData(other)
    {
        QReadLocker l(&other.lock);
        runner = other.runner;
        categoryRelevance = other.categoryRelevance;
        relevance = other.relevance;
        selAction = other.selAction;
        enabled = other.enabled;
        idSetByData = other.idSetByData;
        matchCategory = other.matchCategory;
        id = other.id;
        text = other.text;
        subtext = other.subtext;
        icon = other.icon;
        iconName = other.iconName;
        data = other.data;
        urls = other.urls;
        actions = other.actions;
        multiLine = other.multiLine;
    }

    void setId(const QString &newId)
    {
        if (runner && runner->d->hasUniqueResults) {
            id = newId;
        } else {
            if (runner) {
                id = runner.data()->id();
            }
            if (!id.isEmpty()) {
                id.append(QLatin1Char('_')).append(newId);
            }
        }
        idSetByData = false;
    }

    mutable QReadWriteLock lock;
    QPointer<AbstractRunner> runner;
    QString matchCategory;
    QString id;
    QString text;
    QString subtext;
    QString mimeType;
    QList<QUrl> urls;
    QIcon icon;
    QString iconName;
    QVariant data;
    qreal categoryRelevance = 50;
    qreal relevance = .7;
    KRunner::Action selAction;
    KRunner::Actions actions;
    bool enabled = true;
    bool idSetByData = false;
    bool multiLine = false;
};

QueryMatch::QueryMatch(AbstractRunner *runner)
    : d(new QueryMatchPrivate(runner))
{
}

QueryMatch::QueryMatch(const QueryMatch &other)
    : d(other.d)
{
}

QueryMatch::~QueryMatch() = default;

bool QueryMatch::isValid() const
{
    return d->runner != nullptr;
}

QString QueryMatch::id() const
{
    if (d->id.isEmpty() && d->runner) {
        return d->runner.data()->id();
    }

    return d->id;
}

void QueryMatch::setCategoryRelevance(qreal relevance)
{
    d->categoryRelevance = qBound(0.0, relevance, 100.0);
}

qreal QueryMatch::categoryRelevance() const
{
    return d->categoryRelevance;
}

void QueryMatch::setMatchCategory(const QString &category)
{
    d->matchCategory = category;
}

QString QueryMatch::matchCategory() const
{
    if (d->matchCategory.isEmpty() && d->runner) {
        return d->runner->name();
    }
    return d->matchCategory;
}

void QueryMatch::setRelevance(qreal relevance)
{
    d->relevance = qMax(qreal(0.0), relevance);
}

qreal QueryMatch::relevance() const
{
    return d->relevance;
}

AbstractRunner *QueryMatch::runner() const
{
    return d->runner.data();
}

void QueryMatch::setText(const QString &text)
{
    QWriteLocker locker(&d->lock);
    d->text = text;
}

void QueryMatch::setSubtext(const QString &subtext)
{
    QWriteLocker locker(&d->lock);
    d->subtext = subtext;
}

void QueryMatch::setData(const QVariant &data)
{
    QWriteLocker locker(&d->lock);
    d->data = data;

    if (d->id.isEmpty() || d->idSetByData) {
        const QString matchId = data.toString();
        if (!matchId.isEmpty()) {
            d->setId(matchId);
            d->idSetByData = true;
        }
    }
}

void QueryMatch::setId(const QString &id)
{
    QWriteLocker locker(&d->lock);
    d->setId(id);
}

void QueryMatch::setIcon(const QIcon &icon)
{
    QWriteLocker locker(&d->lock);
    d->icon = icon;
}

void QueryMatch::setIconName(const QString &iconName)
{
    QWriteLocker locker(&d->lock);
    d->iconName = iconName;
}

QVariant QueryMatch::data() const
{
    QReadLocker locker(&d->lock);
    return d->data;
}

QString QueryMatch::text() const
{
    QReadLocker locker(&d->lock);
    return d->text;
}

QString QueryMatch::subtext() const
{
    QReadLocker locker(&d->lock);
    return d->subtext;
}

QIcon QueryMatch::icon() const
{
    QReadLocker locker(&d->lock);
    return d->icon;
}

QString QueryMatch::iconName() const
{
    QReadLocker locker(&d->lock);
    return d->iconName;
}

void QueryMatch::setUrls(const QList<QUrl> &urls)
{
    QWriteLocker locker(&d->lock);
    d->urls = urls;
}

QList<QUrl> QueryMatch::urls() const
{
    QReadLocker locker(&d->lock);
    return d->urls;
}

void QueryMatch::setEnabled(bool enabled)
{
    d->enabled = enabled;
}

bool QueryMatch::isEnabled() const
{
    return d->enabled && d->runner;
}

KRunner::Action QueryMatch::selectedAction() const
{
    return d->selAction;
}

void QueryMatch::setSelectedAction(const KRunner::Action &action)
{
    d->selAction = action;
}

void QueryMatch::setMultiLine(bool multiLine)
{
    d->multiLine = multiLine;
}

bool QueryMatch::isMultiLine() const
{
    return d->multiLine;
}

QueryMatch &QueryMatch::operator=(const QueryMatch &other)
{
    if (d != other.d) {
        d = other.d;
    }

    return *this;
}

bool QueryMatch::operator==(const QueryMatch &other) const
{
    return (d == other.d);
}

bool QueryMatch::operator!=(const QueryMatch &other) const
{
    return (d != other.d);
}

void QueryMatch::setActions(const QList<KRunner::Action> &actions)
{
    QWriteLocker locker(&d->lock);
    d->actions = actions;
}

void QueryMatch::addAction(const KRunner::Action &action)
{
    QWriteLocker locker(&d->lock);
    d->actions << action;
}

KRunner::Actions QueryMatch::actions() const
{
    QReadLocker locker(&d->lock);
    return d->actions;
}

QDebug operator<<(QDebug debug, const KRunner::QueryMatch &match)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "QueryMatch(category: " << match.matchCategory() << " text:" << match.text() << ")";
    return debug;
}

} // KRunner namespace
