// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOOSTER_DECLARATIVE_H
#define BOOSTER_DECLARATIVE_H

#include <QScopedPointer>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "booster.h"

BEGIN_NAMESPACE
/*!
    \class DeclarativeBooster
    \brief DeclarativeBooster is a "booster" that only exec()'s the given binary.

    This can be used to launch any application.
 */
class DeclarativeBooster : public Booster
{
public:

    DeclarativeBooster() {}
    virtual ~DeclarativeBooster() {}

    //! \reimp
    virtual const string & boosterType() const override;

    //! \reimp
    void initialize(int initialArgc, char ** initialArgv, int boosterLauncherSocket,
                    int socketFd, SingleInstance * singleInstance,
                    bool bootMode) override;

    //! Disable copy-constructor
    DeclarativeBooster(const DeclarativeBooster & r) = delete;

    //! Disable assignment operator
    DeclarativeBooster & operator= (const DeclarativeBooster & r) = delete;

protected:

    //! \reimp
        int launchProcess() override;

    //! \reimp
    virtual bool preload() override;

private:
    static const string m_boosterType;

    //! wait for socket connection
    void accept();
};
END_NAMESPACE

#endif // BOOSTER_DECLARATIVE_H
