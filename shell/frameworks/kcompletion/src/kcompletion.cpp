/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcompletion.h"
#include "kcompletion_p.h"
#include <kcompletion_debug.h>

#include <QCollator>

void KCompletionPrivate::addWeightedItem(const QString &item)
{
    Q_Q(KCompletion);
    if (order != KCompletion::Weighted) {
        q->addItem(item, 0);
        return;
    }

    int len = item.length();
    uint weight = 0;

    // find out the weighting of this item (appended to the string as ":num")
    int index = item.lastIndexOf(QLatin1Char(':'));
    if (index > 0) {
        bool ok;
        weight = QStringView(item).mid(index + 1).toUInt(&ok);
        if (!ok) {
            weight = 0;
        }

        len = index; // only insert until the ':'
    }

    q->addItem(item.left(len), weight);
    return;
}

// tries to complete "string" from the tree-root
QString KCompletionPrivate::findCompletion(const QString &string)
{
    QString completion;
    const KCompTreeNode *node = m_treeRoot.get();

    // start at the tree-root and try to find the search-string
    for (const auto ch : string) {
        node = node->find(ch);

        if (node) {
            completion += ch;
        } else {
            return QString(); // no completion
        }
    }

    // Now we have the last node of the to be completed string.
    // Follow it as long as it has exactly one child (= longest possible
    // completion)

    while (node->childrenCount() == 1) {
        node = node->firstChild();
        if (!node->isNull()) {
            completion += *node;
        }
    }
    // if multiple matches and auto-completion mode
    // -> find the first complete match
    if (node && node->childrenCount() > 1) {
        hasMultipleMatches = true;

        if (completionMode == KCompletion::CompletionAuto) {
            rotationIndex = 1;
            if (order != KCompletion::Weighted) {
                while ((node = node->firstChild())) {
                    if (!node->isNull()) {
                        completion += *node;
                    } else {
                        break;
                    }
                }
            } else {
                // don't just find the "first" match, but the one with the
                // highest priority

                const KCompTreeNode *temp_node = nullptr;
                while (1) {
                    int count = node->childrenCount();
                    temp_node = node->firstChild();
                    uint weight = temp_node->weight();
                    const KCompTreeNode *hit = temp_node;
                    for (int i = 1; i < count; i++) {
                        temp_node = node->childAt(i);
                        if (temp_node->weight() > weight) {
                            hit = temp_node;
                            weight = hit->weight();
                        }
                    }
                    // 0x0 has the highest priority -> we have the best match
                    if (hit->isNull()) {
                        break;
                    }

                    node = hit;
                    completion += *node;
                }
            }
        }
    }

    return completion;
}

void KCompletionPrivate::defaultSort(QStringList &stringList)
{
    QCollator c;
    c.setCaseSensitivity(Qt::CaseSensitive);
    std::stable_sort(stringList.begin(), stringList.end(), c);
}

KCompletion::KCompletion()
    : d_ptr(new KCompletionPrivate(this))
{
    setOrder(Insertion);
}

KCompletion::~KCompletion()
{
}

void KCompletion::setOrder(CompOrder order)
{
    Q_D(KCompletion);
    d->order = order;
    d->matches.setSorting(order);
}

KCompletion::CompOrder KCompletion::order() const
{
    Q_D(const KCompletion);
    return d->order;
}

void KCompletion::setIgnoreCase(bool ignoreCase)
{
    Q_D(KCompletion);
    d->ignoreCase = ignoreCase;
}

bool KCompletion::ignoreCase() const
{
    Q_D(const KCompletion);
    return d->ignoreCase;
}

void KCompletion::setItems(const QStringList &itemList)
{
    clear();
    insertItems(itemList);
}

void KCompletion::insertItems(const QStringList &items)
{
    Q_D(KCompletion);
    for (const auto &str : items) {
        if (d->order == Weighted) {
            d->addWeightedItem(str);
        } else {
            addItem(str, 0);
        }
    }
}

QStringList KCompletion::items() const
{
    Q_D(const KCompletion);
    KCompletionMatchesWrapper list(d->sorterFunction); // unsorted
    list.extractStringsFromNode(d->m_treeRoot.get(), QString(), d->order == Weighted);
    return list.list();
}

bool KCompletion::isEmpty() const
{
    Q_D(const KCompletion);
    return (d->m_treeRoot->childrenCount() == 0);
}

void KCompletion::postProcessMatch(QString *) const
{
}

void KCompletion::postProcessMatches(QStringList *) const
{
}

void KCompletion::postProcessMatches(KCompletionMatches *) const
{
}

void KCompletion::addItem(const QString &item)
{
    Q_D(KCompletion);
    d->matches.clear();
    d->rotationIndex = 0;
    d->lastString.clear();

    addItem(item, 0);
}

void KCompletion::addItem(const QString &item, uint weight)
{
    Q_D(KCompletion);
    if (item.isEmpty()) {
        return;
    }

    KCompTreeNode *node = d->m_treeRoot.get();
    int len = item.length();

    bool sorted = (d->order == Sorted);
    bool weighted = ((d->order == Weighted) && weight > 1);

    // knowing the weight of an item, we simply add this weight to all of its
    // nodes.

    for (int i = 0; i < len; i++) {
        node = node->insert(item.at(i), sorted);
        if (weighted) {
            node->confirm(weight - 1); // node->insert() sets weighting to 1
        }
    }

    // add 0x0-item as delimiter with evtl. weight
    node = node->insert(QChar(0x0), true);
    if (weighted) {
        node->confirm(weight - 1);
    }
    //     qDebug("*** added: %s (%i)", item.toLatin1().constData(), node->weight());
}

void KCompletion::removeItem(const QString &item)
{
    Q_D(KCompletion);
    d->matches.clear();
    d->rotationIndex = 0;
    d->lastString.clear();

    d->m_treeRoot->remove(item);
}

void KCompletion::clear()
{
    Q_D(KCompletion);
    d->matches.clear();
    d->rotationIndex = 0;
    d->lastString.clear();

    d->m_treeRoot.reset(new KCompTreeNode);
}

QString KCompletion::makeCompletion(const QString &string)
{
    Q_D(KCompletion);
    if (d->completionMode == CompletionNone) {
        return QString();
    }

    // qDebug() << "KCompletion: completing: " << string;

    d->matches.clear();
    d->rotationIndex = 0;
    d->hasMultipleMatches = false;
    d->lastMatch = d->currentMatch;

    // in Shell-completion-mode, emit all matches when we get the same
    // complete-string twice
    if (d->completionMode == CompletionShell && string == d->lastString) {
        // Don't use d->matches since calling postProcessMatches()
        // on d->matches here would interfere with call to
        // postProcessMatch() during rotation

        d->matches.findAllCompletions(d->m_treeRoot.get(), string, d->ignoreCase, d->hasMultipleMatches);
        QStringList l = d->matches.list();
        postProcessMatches(&l);
        Q_EMIT matches(l);

        return QString();
    }

    QString completion;
    // in case-insensitive popup mode, we search all completions at once
    if (d->completionMode == CompletionPopup || d->completionMode == CompletionPopupAuto) {
        d->matches.findAllCompletions(d->m_treeRoot.get(), string, d->ignoreCase, d->hasMultipleMatches);
        if (!d->matches.isEmpty()) {
            completion = d->matches.first();
        }
    } else {
        completion = d->findCompletion(string);
    }

    if (d->hasMultipleMatches) {
        Q_EMIT multipleMatches();
    }

    d->lastString = string;
    d->currentMatch = completion;

    postProcessMatch(&completion);

    if (!string.isEmpty()) { // only emit match when string is not empty
        // qDebug() << "KCompletion: Match: " << completion;
        Q_EMIT match(completion);
    }

    return completion;
}

QStringList KCompletion::substringCompletion(const QString &string) const
{
    Q_D(const KCompletion);
    // get all items in the tree, eventually in sorted order
    KCompletionMatchesWrapper allItems(d->sorterFunction, d->order);
    allItems.extractStringsFromNode(d->m_treeRoot.get(), QString(), false);

    QStringList list = allItems.list();

    // subStringMatches is invoked manually, via a shortcut
    if (list.isEmpty()) {
        return list;
    }

    if (!string.isEmpty()) { // If it's empty, nothing to compare
        auto it = std::remove_if(list.begin(), list.end(), [&string](const QString &item) {
            return !item.contains(string, Qt::CaseInsensitive); // always case insensitive
        });
        list.erase(it, list.end());
    }

    postProcessMatches(&list);
    return list;
}

void KCompletion::setCompletionMode(CompletionMode mode)
{
    Q_D(KCompletion);
    d->completionMode = mode;
}

KCompletion::CompletionMode KCompletion::completionMode() const
{
    Q_D(const KCompletion);
    return d->completionMode;
}

void KCompletion::setShouldAutoSuggest(const bool shouldAutoSuggest)
{
    Q_D(KCompletion);
    d->shouldAutoSuggest = shouldAutoSuggest;
}

bool KCompletion::shouldAutoSuggest() const
{
    Q_D(const KCompletion);
    return d->shouldAutoSuggest;
}

void KCompletion::setSorterFunction(SorterFunction sortFunc)
{
    Q_D(KCompletion);
    d->sorterFunction = sortFunc ? sortFunc : KCompletionPrivate::defaultSort;
}

QStringList KCompletion::allMatches()
{
    Q_D(KCompletion);
    // Don't use d->matches since calling postProcessMatches()
    // on d->matches here would interfere with call to
    // postProcessMatch() during rotation
    KCompletionMatchesWrapper matches(d->sorterFunction, d->order);
    bool dummy;
    matches.findAllCompletions(d->m_treeRoot.get(), d->lastString, d->ignoreCase, dummy);
    QStringList l = matches.list();
    postProcessMatches(&l);
    return l;
}

KCompletionMatches KCompletion::allWeightedMatches()
{
    Q_D(KCompletion);
    // Don't use d->matches since calling postProcessMatches()
    // on d->matches here would interfere with call to
    // postProcessMatch() during rotation
    KCompletionMatchesWrapper matches(d->sorterFunction, d->order);
    bool dummy;
    matches.findAllCompletions(d->m_treeRoot.get(), d->lastString, d->ignoreCase, dummy);
    KCompletionMatches ret(matches);
    postProcessMatches(&ret);
    return ret;
}

QStringList KCompletion::allMatches(const QString &string)
{
    Q_D(KCompletion);
    KCompletionMatchesWrapper matches(d->sorterFunction, d->order);
    bool dummy;
    matches.findAllCompletions(d->m_treeRoot.get(), string, d->ignoreCase, dummy);
    QStringList l = matches.list();
    postProcessMatches(&l);
    return l;
}

KCompletionMatches KCompletion::allWeightedMatches(const QString &string)
{
    Q_D(KCompletion);
    KCompletionMatchesWrapper matches(d->sorterFunction, d->order);
    bool dummy;
    matches.findAllCompletions(d->m_treeRoot.get(), string, d->ignoreCase, dummy);
    KCompletionMatches ret(matches);
    postProcessMatches(&ret);
    return ret;
}

void KCompletion::setSoundsEnabled(bool enable)
{
    Q_D(KCompletion);
    d->beep = enable;
}

bool KCompletion::soundsEnabled() const
{
    Q_D(const KCompletion);
    return d->beep;
}

bool KCompletion::hasMultipleMatches() const
{
    Q_D(const KCompletion);
    return d->hasMultipleMatches;
}

/////////////////////////////////////////////////////
///////////////// tree operations ///////////////////

QString KCompletion::nextMatch()
{
    Q_D(KCompletion);
    QString completion;
    d->lastMatch = d->currentMatch;

    if (d->matches.isEmpty()) {
        d->matches.findAllCompletions(d->m_treeRoot.get(), d->lastString, d->ignoreCase, d->hasMultipleMatches);
        if (!d->matches.isEmpty()) {
            completion = d->matches.first();
        }
        d->currentMatch = completion;
        d->rotationIndex = 0;
        postProcessMatch(&completion);
        Q_EMIT match(completion);
        return completion;
    }

    QStringList matches = d->matches.list();
    d->lastMatch = matches[d->rotationIndex++];

    if (d->rotationIndex == matches.count()) {
        d->rotationIndex = 0;
    }

    completion = matches[d->rotationIndex];
    d->currentMatch = completion;
    postProcessMatch(&completion);
    Q_EMIT match(completion);
    return completion;
}

const QString &KCompletion::lastMatch() const
{
    Q_D(const KCompletion);
    return d->lastMatch;
}

QString KCompletion::previousMatch()
{
    Q_D(KCompletion);
    QString completion;
    d->lastMatch = d->currentMatch;

    if (d->matches.isEmpty()) {
        d->matches.findAllCompletions(d->m_treeRoot.get(), d->lastString, d->ignoreCase, d->hasMultipleMatches);
        if (!d->matches.isEmpty()) {
            completion = d->matches.last();
        }
        d->currentMatch = completion;
        d->rotationIndex = 0;
        postProcessMatch(&completion);
        Q_EMIT match(completion);
        return completion;
    }

    QStringList matches = d->matches.list();
    d->lastMatch = matches[d->rotationIndex];

    if (d->rotationIndex == 0) {
        d->rotationIndex = matches.count();
    }

    d->rotationIndex--;

    completion = matches[d->rotationIndex];
    d->currentMatch = completion;
    postProcessMatch(&completion);
    Q_EMIT match(completion);
    return completion;
}

QSharedPointer<KZoneAllocator> KCompTreeNode::m_alloc(new KZoneAllocator(8 * 1024));

#include "moc_kcompletion.cpp"
