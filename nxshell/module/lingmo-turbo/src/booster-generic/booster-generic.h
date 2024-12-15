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

#ifndef BOOSTER_GENERIC_H
#define BOOSTER_GENERIC_H

#include "booster.h"

/*!
    \class EBooster
    \brief EBooster is a "booster" that only exec()'s the given binary.

    This can be used to launch any application.
 */
class EBooster : public Booster
{
public:

    EBooster() {}
    virtual ~EBooster() {}

    //! \reimp
    virtual const string & boosterType() const;

protected:

    //! \reimp
    virtual int launchProcess();

    //! \reimp
    virtual bool preload();

private:

    //! Disable copy-constructor
    EBooster(const EBooster & r);

    //! Disable assignment operator
    EBooster & operator= (const EBooster & r);

    static const string m_boosterType;

    //! wait for socket connection
    void accept();

#ifdef UNIT_TEST
    friend class Ut_EBooster;
#endif
};

#endif // EBooster_H
