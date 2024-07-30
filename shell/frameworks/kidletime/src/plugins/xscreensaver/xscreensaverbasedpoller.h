/* This file is part of the KDE libraries
 * SPDX-FileCopyrightText: 2009 Dario Freddi <drf at kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef XSCREENSAVERBASEDPOLLER_H
#define XSCREENSAVERBASEDPOLLER_H

#include "kwindowbasedidletimepoller_p.h"

#include "screensaver_interface.h"

class XScreensaverBasedPoller : public KWindowBasedIdleTimePoller
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID KAbstractIdleTimePoller_iid FILE "xcb.json")
    Q_INTERFACES(KAbstractIdleTimePoller)

public:
    explicit XScreensaverBasedPoller(QObject *parent = nullptr);
    ~XScreensaverBasedPoller() override;

public Q_SLOTS:
    void simulateUserActivity() override;

private:
    bool additionalSetUp() override;

private Q_SLOTS:
    void screensaverActivated(bool activated);
    int getIdleTime() override;

private:
    OrgFreedesktopScreenSaverInterface *m_screenSaverIface;
};

#endif /* XSCREENSAVERBASEDPOLLER_H_ */
