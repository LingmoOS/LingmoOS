/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2002 David Faure <david@mandrakesoft.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KREPLACETEST_H
#define KREPLACETEST_H

#include <QObject>
#include <QStringList>
#include <memory>

class KReplace;
class KReplaceTest : public QObject
{
    Q_OBJECT
public:
    KReplaceTest(const QStringList &text, const QString &buttonName);
    ~KReplaceTest() override;

    void replace(const QString &pattern, const QString &replacement, long options);
    void print();
    const QStringList &textLines() const
    {
        return m_text;
    }

public Q_SLOTS:
    void slotHighlight(const QString &, int, int);
    void slotReplaceNext();
    void slotReplace(const QString &text, int replacementIndex, int replacedLength, int matchedLength);

Q_SIGNALS:
    void exitLoop();

private:
    void enterLoop();

    QStringList::Iterator m_currentPos;
    QStringList m_text;
    std::unique_ptr<KReplace> m_replace;
    bool m_needEventLoop;
    QString m_buttonName;
};

#endif
