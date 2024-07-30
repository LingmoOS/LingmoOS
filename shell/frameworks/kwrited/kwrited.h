/*
    kwrited is a write(1) receiver for KDE.
    SPDX-FileCopyrightText: 1997, 1998 Lars Doelle <lars.doelle@on-line.de>
    SPDX-FileCopyrightText: 2008 George Kiagiadakis <gkiagia@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KWRITED_H
#define KWRITED_H

#include <QObject>
#include <config-kwrited.h>
class KPtyDevice;

class KWrited : public QObject
{
    Q_OBJECT
public:
    KWrited();
    ~KWrited() override;

private Q_SLOTS:
    void block_in();

private:
    KPtyDevice *pty;
};

#if !BUILD_AS_EXECUTABLE
#include <kdedmodule.h>

class KWritedModule : public KDEDModule
{
public:
    KWritedModule(QObject *parent, const QList<QVariant> &);
    ~KWritedModule() override;

private:
    KWrited *pro;
};

#endif

#endif
