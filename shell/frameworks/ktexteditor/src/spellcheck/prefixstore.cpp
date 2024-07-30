/*
    SPDX-FileCopyrightText: 2009 Michel Ludwig <michel.ludwig@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "prefixstore.h"

#include "katepartdebug.h"

void KatePrefixStore::addPrefix(const QString &prefix)
{
    if (prefix.isEmpty()) {
        return;
    }
    if (m_prefixSet.contains(prefix)) {
        return;
    }
    unsigned long long state = 0;
    for (int i = 0; i < prefix.length(); ++i) {
        QChar c = prefix.at(i);

        CharToOccurrenceStateHash &hash = m_transitionFunction[state];
        CharToOccurrenceStateHash::iterator it = hash.find(c.unicode());
        if (it == hash.end()) {
            state = nextFreeState();
            hash[c.unicode()] = QPair<unsigned int, unsigned long long>(1, state);
            continue;
        }

        ++(*it).first;
        state = (*it).second;
    }
    // add the last state as accepting state
    m_acceptingStates.insert(state);

    m_prefixSet.insert(prefix);

    if (prefix.length() > m_longestPrefixLength) {
        m_longestPrefixLength = prefix.length();
    }
}

void KatePrefixStore::removePrefix(const QString &prefix)
{
    if (prefix.isEmpty()) {
        return;
    }
    if (!m_prefixSet.contains(prefix)) {
        return;
    }
    m_prefixSet.remove(prefix);

    unsigned long long state = 0;
    for (int i = 0; i < prefix.length(); ++i) {
        QChar c = prefix.at(i);

        CharToOccurrenceStateHash &hash = m_transitionFunction[state];
        CharToOccurrenceStateHash::iterator it = hash.find(c.unicode());
        if (it == hash.end()) {
            continue;
        }

        state = (*it).second;
        if (m_acceptingStates.contains(state) && i == prefix.length() - 1) {
            m_acceptingStates.remove(state);
        }

        if ((*it).first <= 1) {
            hash.erase(it);
            m_stateFreeList.push_back(state);
        } else {
            --(*it).first;
        }
    }

    if (prefix.length() == m_longestPrefixLength) {
        m_longestPrefixLength = computeLongestPrefixLength();
    }
}

void KatePrefixStore::dump()
{
    for (unsigned long long i = 0; i < m_lastAssignedState; ++i) {
        CharToOccurrenceStateHash &hash = m_transitionFunction[i];
        for (CharToOccurrenceStateHash::iterator it = hash.begin(); it != hash.end(); ++it) {
            qCDebug(LOG_KTE) << i << "x" << QChar(it.key()) << "->" << it.value().first << "x" << it.value().second;
        }
    }
    qCDebug(LOG_KTE) << "Accepting states" << m_acceptingStates;
}

QString KatePrefixStore::findPrefix(const QString &s, int start) const
{
    unsigned long long state = 0;
    for (int i = start; i < s.length(); ++i) {
        QChar c = s.at(i);
        const CharToOccurrenceStateHash &hash = m_transitionFunction[state];
        CharToOccurrenceStateHash::const_iterator it = hash.find(c.unicode());
        if (it == hash.end()) {
            return QString();
        }

        state = (*it).second;
        if (m_acceptingStates.contains(state)) {
            return s.mid(start, i + 1 - start);
        }
    }
    return QString();
}

QString KatePrefixStore::findPrefix(const Kate::TextLine &line, int start) const
{
    unsigned long long state = 0;
    for (int i = start; i < line.length(); ++i) {
        QChar c = line.at(i);
        const CharToOccurrenceStateHash &hash = m_transitionFunction[state];
        CharToOccurrenceStateHash::const_iterator it = hash.find(c.unicode());
        if (it == hash.end()) {
            return QString();
        }

        state = (*it).second;
        if (m_acceptingStates.contains(state)) {
            return line.string(start, i + 1 - start);
        }
    }
    return QString();
}

int KatePrefixStore::longestPrefixLength() const
{
    return m_longestPrefixLength;
}

void KatePrefixStore::clear()
{
    m_longestPrefixLength = 0;
    m_prefixSet.clear();
    m_transitionFunction.clear();
    m_acceptingStates.clear();
    m_stateFreeList.clear();
    m_lastAssignedState = 0;
}

int KatePrefixStore::computeLongestPrefixLength()
{
    int toReturn = 0;
    for (QSet<QString>::const_iterator i = m_prefixSet.cbegin(); i != m_prefixSet.cend(); ++i) {
        qCDebug(LOG_KTE) << "length" << (*i).length();
        toReturn = qMax(toReturn, (*i).length());
    }
    return toReturn;
}

unsigned long long KatePrefixStore::nextFreeState()
{
    if (!m_stateFreeList.isEmpty()) {
        return m_stateFreeList.takeFirst();
    }
    return ++m_lastAssignedState;
}
