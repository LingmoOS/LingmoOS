/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2004 Arend van Beelen jr. <arend@auton.nl>
    SPDX-FileCopyrightText: 2010 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KFINDTEST_H
#define KFINDTEST_H

#include <QObject>
#include <QStringList>
#include <ktextwidgets_export.h>
#include <memory>
class KFind;
class KFindRecorder : public QObject
{
    Q_OBJECT

public:
    explicit KFindRecorder(const QStringList &text);

    ~KFindRecorder() override;
    void find(const QString &pattern, long options = 0);
    bool findNext(const QString &pattern = QString());

    void changeText(int line, const QString &text);

    const QStringList &hits() const
    {
        return m_hits;
    }
    void clearHits()
    {
        m_hits.clear();
    }

public Q_SLOTS:
    void slotHighlight(const QString &text, int index, int matchedLength);
    void slotHighlight(int id, int index, int matchedLengthlength);

private:
    std::unique_ptr<KFind> m_find;
    QStringList m_text;
    int m_line;
    QStringList m_hits;
};

class TestKFind : public QObject
{
    Q_OBJECT

public:
    TestKFind();

private Q_SLOTS:

    void testStaticFindRegexp_data();
    void testStaticFindRegexp();

    void testRegexpUnicode_data();
    void testRegexpUnicode();

    void testSimpleSearch();
    void testSimpleRegexp();

    void testLineBeginRegularExpression();
    void testFindIncremental();
    void testFindIncrementalDynamic();

private:
    QString m_text;
};

#endif
