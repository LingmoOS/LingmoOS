/*
    This file is part of the KContacts framework.

    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KCONTACTS_TESTLOCK_H
#define KCONTACTS_TESTLOCK_H

#include <QLabel>
#include <QWidget>

#include "kcontacts/lock.h"

class QLabel;
class QTreeWidget;

class LockWidget : public QWidget
{
    Q_OBJECT
public:
    LockWidget(const QString &identifier);
    ~LockWidget();

protected Q_SLOTS:
    void lock();
    void unlock();

    void updateLockView();

private:
    KContacts::Lock *mLock = nullptr;

    QLabel *mStatus = nullptr;
    QTreeWidget *mLockView = nullptr;
};

#endif
