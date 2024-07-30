// SPDX-FileCopyrightText: 2014 Sven Brauch <svenbrauch@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef PLASMAPASS_ABBREVIATIONS_H
#define PLASMAPASS_ABBREVIATIONS_H

#include <QStringList>
#include <QVector>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QStringRef;
#endif
class QString;

namespace PlasmaPass
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
bool matchesAbbreviation(const QStringRef &word, const QStringRef &typed);
#else
bool matchesAbbreviation(const QStringView &word, const QStringView &typed);
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
bool matchesPath(const QStringRef &path, const QStringRef &typed);
#else
bool matchesPath(const QStringView &path, const QStringView &typed);
#endif

/**
 * @brief Matches a path against a list of search fragments.
 * @return -1 when no match is found, otherwise a positive integer, higher values mean lower quality
 */
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
int matchPathFilter(const QVector<QStringRef> &toFilter, const QVector<QStringRef> &text);
#else
int matchPathFilter(const QVector<QStringView> &toFilter, const QVector<QStringView> &text);
#endif
}

#endif
