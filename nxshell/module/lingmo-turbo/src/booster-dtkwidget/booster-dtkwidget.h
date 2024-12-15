/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of applauncherd
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#ifndef BOOSTER_DTKWIDGET_H
#define BOOSTER_DTKWIDGET_H

#include "booster.h"

BEGIN_NAMESPACE
/*!
    \class QWBooster
    \brief QWBooster is a "booster" that only exec()'s the given binary.

    This can be used to launch any application.
 */
class QWBooster : public Booster
{
public:

    QWBooster() {}
    virtual ~QWBooster() {}

    //! \reimp
    virtual const string & boosterType() const override;

    //! \reimp
    void initialize(int initialArgc, char ** initialArgv, int boosterLauncherSocket,
                    int socketFd, SingleInstance * singleInstance,
                    bool bootMode) override;

protected:

    //! \reimp
    virtual bool preload() override;

private:

    //! Disable copy-constructor
    QWBooster(const QWBooster & r);

    //! Disable assignment operator
    QWBooster & operator= (const QWBooster & r);

    static const string m_boosterType;

    //! wait for socket connection
    void accept();

#ifdef UNIT_TEST
    friend class Ut_QWBooster;
#endif
};
END_NAMESPACE

#endif // QWBooster_H
