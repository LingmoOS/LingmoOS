/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPLETIONMATCHESWRAPPER_P_H
#define KCOMPLETIONMATCHESWRAPPER_P_H

#include "kcompletion.h"
#include "kcomptreenode_p.h"

#include <kcompletionmatches.h>

class KCOMPLETION_EXPORT KCompletionMatchesWrapper
{
public:
    explicit KCompletionMatchesWrapper(KCompletion::SorterFunction const &sorterFunction, KCompletion::CompOrder compOrder = KCompletion::Insertion)
        : m_sortedListPtr(compOrder == KCompletion::Weighted ? new KCompletionMatchesList : nullptr)
        , m_dirty(false)
        , m_compOrder(compOrder)
        , m_sorterFunction(sorterFunction)
    {
    }

    KCompletionMatchesWrapper(const KCompletionMatchesWrapper &) = delete;
    KCompletionMatchesWrapper &operator=(const KCompletionMatchesWrapper &) = delete;

    void setSorting(KCompletion::CompOrder compOrder)
    {
        if (compOrder == KCompletion::Weighted && !m_sortedListPtr) {
            m_sortedListPtr = std::make_unique<KCompletionMatchesList>();
        } else if (compOrder != KCompletion::Weighted) {
            m_sortedListPtr.reset();
        }
        m_compOrder = compOrder;
        m_stringList.clear();
        m_dirty = false;
    }

    KCompletion::CompOrder sorting() const
    {
        return m_compOrder;
    }

    void append(int i, const QString &string)
    {
        if (m_sortedListPtr) {
            m_sortedListPtr->insert(i, string);
        } else {
            m_stringList.append(string);
        }
        m_dirty = true;
    }

    void clear()
    {
        if (m_sortedListPtr) {
            m_sortedListPtr->clear();
        }
        m_stringList.clear();
        m_dirty = false;
    }

    uint size() const
    {
        if (m_sortedListPtr) {
            return m_sortedListPtr->size();
        }
        return m_stringList.size();
    }

    bool isEmpty() const
    {
        return size() == 0;
    }

    QString first() const
    {
        return list().constFirst();
    }

    QString last() const
    {
        return list().constLast();
    }

    inline QStringList list() const;

    inline void findAllCompletions(const KCompTreeNode *, const QString &, bool ignoreCase, bool &hasMultipleMatches);

    inline void extractStringsFromNode(const KCompTreeNode *, const QString &beginning, bool addWeight = false);

    inline void extractStringsFromNodeCI(const KCompTreeNode *, const QString &beginning, const QString &restString);

    mutable QStringList m_stringList;
    std::unique_ptr<KCompletionMatchesList> m_sortedListPtr;
    mutable bool m_dirty;
    KCompletion::CompOrder m_compOrder;
    KCompletion::SorterFunction const &m_sorterFunction;
};

void KCompletionMatchesWrapper::findAllCompletions(const KCompTreeNode *treeRoot, const QString &string, bool ignoreCase, bool &hasMultipleMatches)
{
    // qDebug() << "*** finding all completions for " << string;

    if (string.isEmpty()) {
        return;
    }

    if (ignoreCase) { // case insensitive completion
        extractStringsFromNodeCI(treeRoot, QString(), string);
        hasMultipleMatches = (size() > 1);
        return;
    }

    QString completion;
    const KCompTreeNode *node = treeRoot;

    // start at the tree-root and try to find the search-string
    for (const QChar ch : string) {
        node = node->find(ch);

        if (node) {
            completion += ch;
        } else {
            return; // no completion -> return empty list
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
        // qDebug() << completion << node->latin1();
    }

    // there is just one single match)
    if (node->childrenCount() == 0) {
        append(node->weight(), completion);
    }

    else {
        // node has more than one child
        // -> recursively find all remaining completions
        hasMultipleMatches = true;
        extractStringsFromNode(node, completion);
    }
}

QStringList KCompletionMatchesWrapper::list() const
{
    if (m_sortedListPtr && m_dirty) {
        m_sortedListPtr->sort();
        m_dirty = false;

        m_stringList.clear();
        m_stringList.reserve(m_sortedListPtr->size());
        // high weight == sorted last -> reverse the sorting here
        std::transform(m_sortedListPtr->crbegin(), m_sortedListPtr->crend(), std::back_inserter(m_stringList), [](const KSortableItem<QString> &item) {
            return item.value();
        });
    } else if (m_compOrder == KCompletion::Sorted) {
        m_sorterFunction(m_stringList);
    }

    return m_stringList;
}

void KCompletionMatchesWrapper::extractStringsFromNode(const KCompTreeNode *node, const QString &beginning, bool addWeight)
{
    if (!node) {
        return;
    }

    // qDebug() << "Beginning: " << beginning;
    const KCompTreeChildren *list = node->children();
    QString string;
    QString w;

    // loop thru all children
    for (KCompTreeNode *cur = list->begin(); cur; cur = cur->m_next) {
        string = beginning;
        node = cur;
        if (!node->isNull()) {
            string += *node;
        }

        while (node && node->childrenCount() == 1) {
            node = node->firstChild();
            if (node->isNull()) {
                break;
            }
            string += *node;
        }

        if (node && node->isNull()) { // we found a leaf
            if (addWeight) {
                // add ":num" to the string to store the weighting
                string += QLatin1Char(':');
                w.setNum(node->weight());
                string.append(w);
            }
            append(node->weight(), string);
        }

        // recursively find all other strings.
        if (node && node->childrenCount() > 1) {
            extractStringsFromNode(node, string, addWeight);
        }
    }
}

void KCompletionMatchesWrapper::extractStringsFromNodeCI(const KCompTreeNode *node, const QString &beginning, const QString &restString)
{
    if (restString.isEmpty()) {
        extractStringsFromNode(node, beginning, false /*noweight*/);
        return;
    }

    QChar ch1 = restString.at(0);
    QString newRest = restString.mid(1);
    KCompTreeNode *child1;
    KCompTreeNode *child2;

    child1 = node->find(ch1); // the correct match
    if (child1) {
        extractStringsFromNodeCI(child1, beginning + QChar(*child1), newRest);
    }

    // append the case insensitive matches, if available
    if (ch1.isLetter()) {
        // find out if we have to lower or upper it. Is there a better way?
        QChar ch2 = ch1.toLower();
        if (ch1 == ch2) {
            ch2 = ch1.toUpper();
        }
        if (ch1 != ch2) {
            child2 = node->find(ch2);
            if (child2) {
                extractStringsFromNodeCI(child2, beginning + QChar(*child2), newRest);
            }
        }
    }
}

#endif // KCOMPLETIONMATCHESWRAPPER_P_H
