// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOOSTERDESKTOP
#define BOOSTERDESKTOP

#include "booster.h"

class DesktopBooster : public Booster
{
public:
    DesktopBooster() {}
    virtual ~DesktopBooster() {}

    //! \reimp
    virtual const string & boosterType() const override;

    virtual int run(SocketManager * socketManager) override;
    void initialize(int initialArgc, char ** initialArgv, int boosterLauncherSocket,
                    int socketFd, SingleInstance * singleInstance,
                    bool bootMode) override;

protected:
    //! \reimp
    virtual bool preload() override;

private:
    //! Disable copy-constructor
    DesktopBooster(const DesktopBooster & r);

    //! Disable assignment operator
    DesktopBooster & operator= (const DesktopBooster & r);

    static const string m_boosterType;

    //! wait for socket connection
    void accept();
};

#endif
