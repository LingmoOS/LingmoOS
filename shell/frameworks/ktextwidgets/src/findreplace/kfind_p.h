/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001 S.R. Haque <srhaque@iee.org>.
    SPDX-FileCopyrightText: 2002 David Faure <david@mandrakesoft.com>
    SPDX-FileCopyrightText: 2004 Arend van Beelen jr. <arend@auton.nl>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KFIND_P_H
#define KFIND_P_H

#include "kfind.h"

#include <QDialog>
#include <QHash>
#include <QList>
#include <QPointer>
#include <QString>

class KFindPrivate
{
    Q_DECLARE_PUBLIC(KFind)

public:
    KFindPrivate(KFind *qq)
        : q_ptr(qq)
        , findDialog(nullptr)
        , currentId(0)
        , customIds(false)
        , patternChanged(false)
        , matchedPattern(QLatin1String(""))
        , emptyMatch(nullptr)
    {
    }

    virtual ~KFindPrivate()
    {
        if (dialog) {
            dialog->deleteLater();
        }
        dialog = nullptr;
        data.clear();
        delete emptyMatch;
        emptyMatch = nullptr;
    }

    struct Match {
        Match()
            : dataId(-1)
            , index(-1)
            , matchedLength(-1)
        {
        }
        bool isNull() const
        {
            return index == -1;
        }
        Match(int _dataId, int _index, int _matchedLength)
            : dataId(_dataId)
            , index(_index)
            , matchedLength(_matchedLength)
        {
            Q_ASSERT(index != -1);
        }

        int dataId;
        int index;
        int matchedLength;
    };

    struct Data {
        Data()
        {
        }
        Data(int id, const QString &text, bool dirty = false)
            : text(text)
            , id(id)
            , dirty(dirty)
        {
        }

        QString text;
        int id = -1;
        bool dirty = false;
    };

    void init(const QString &pattern);
    void startNewIncrementalSearch();

    void slotFindNext();
    void slotDialogClosed();

    KFind *const q_ptr;
    QPointer<QWidget> findDialog;
    int currentId;
    bool customIds : 1;
    bool patternChanged : 1;
    QString matchedPattern;
    QHash<QString, Match> incrementalPath;
    Match *emptyMatch;
    QList<Data> data; // used like a vector, not like a linked-list

    QString pattern;
    QDialog *dialog;
    long options;
    unsigned matches;

    QString text; // the text set by setData
    int index;
    int matchedLength;
    bool dialogClosed : 1;
    bool lastResult : 1;
};

#endif // KFIND_P_H
