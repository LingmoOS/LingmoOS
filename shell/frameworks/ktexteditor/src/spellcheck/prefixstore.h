/*
    SPDX-FileCopyrightText: 2008-2009 Michel Ludwig <michel.ludwig@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PREFIXSTORE_H
#define PREFIXSTORE_H

#include <QHash>
#include <QList>
#include <QPair>
#include <QSet>
#include <QString>

#include "katetextline.h"

/**
 * This class can be used to efficiently search for occurrences of strings in
 * a given string. Theoretically speaking, a finite deterministic automaton is
 * constructed which exactly accepts the strings that are to be recognized. In
 * order to check whether a given string contains one of the strings that are being
 * searched for the constructed automaton has to applied on each position in the
 * given string.
 **/
class KatePrefixStore
{
public:
    typedef QPair<bool, bool> BooleanPair;

    virtual ~KatePrefixStore() = default;

    void addPrefix(const QString &prefix);
    void removePrefix(const QString &prefix);

    /**
     * Returns the shortest prefix of the given string that is contained in
     * this prefix store starting at position 'start'.
     **/
    QString findPrefix(const QString &s, int start = 0) const;

    /**
     * Returns the shortest prefix of the given string that is contained in
     * this prefix store starting at position 'start'.
     **/
    QString findPrefix(const Kate::TextLine &line, int start = 0) const;

    int longestPrefixLength() const;

    void clear();

    void dump();

protected:
    int m_longestPrefixLength = 0;
    QSet<QString> m_prefixSet;

    // State x Char -> Nr. of char occurrences in prefixes x State
    typedef QHash<unsigned short, QPair<unsigned int, unsigned long long>> CharToOccurrenceStateHash;
    typedef QHash<unsigned long long, CharToOccurrenceStateHash> TransitionFunction;
    TransitionFunction m_transitionFunction;
    QSet<unsigned long long> m_acceptingStates;
    QList<unsigned long long> m_stateFreeList;
    unsigned long long m_lastAssignedState = 0;

    int computeLongestPrefixLength();
    unsigned long long nextFreeState();
    //     bool containsPrefixOfLengthEndingWith(int length, const QChar& c);
};

#endif
