/*
    This file is part of the KDE libraries
    Copyright (C) 2008 Marijn Kruisselbrink (m.kruisselbrink@student.tue.nl)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kwindowinfo_mac_p.h"
#include "kwindowinfo.h"
#include "kwindowsystem.h"

#include <QDebug>
#include <kxerrorhandler.h>
#include <netwm.h>
#include <QBitmap>
#include <QDesktopWidget>
#include <QDialog>

KWindowInfo::Private::Private()
    : ref(0), win(0), isLocal(false), loadedData(false), m_axWin(0), parent(), m_pid(-1)
{
}

void KWindowInfo::Private::setAxElement(const AXUIElementRef &axWin)
{
    m_axWin = axWin;
    CFRetain(m_axWin);
}

void KWindowInfo::Private::setProcessSerialNumber(const ProcessSerialNumber &psn)
{
    m_psn = psn;
    GetProcessPID(&psn, &m_pid);
}

KWindowInfo::Private::~Private()
{
    if (m_axWin) {
        CFRelease(m_axWin);
    }
}

void KWindowInfo::Private::updateData()
{
    ProcessInfoRec pinfo;
    char processName[512];
#ifdef Q_OS_MAC32
    FSSpec appSpec;
#else
    FSRef ref;
#endif
    pinfo.processInfoLength = sizeof pinfo;
    pinfo.processName = (unsigned char *) processName;
#ifdef Q_OS_MAC32
    pinfo.processAppSpec = &appSpec;
#else
    pinfo.processAppRef = &ref;
#endif
    GetProcessInformation(&m_psn, &pinfo);
    name = QString::fromLatin1(processName + 1, processName[0]);

    if (m_axWin) {
        CFStringRef title;
        if (AXUIElementCopyAttributeValue(m_axWin, kAXTitleAttribute, (CFTypeRef *)&title) == noErr) {
            CFStringGetCString(title, processName, sizeof processName, kCFStringEncodingUTF8);
            name = QString::fromUtf8(processName);
        }
    }

#ifdef Q_OS_MAC32
    iconSpec = appSpec;

    FSRef ref;
    FSpMakeFSRef(&appSpec, &ref);
#else
    iconSpec = ref;
#endif
    // check if it is in an application bundle (foo.app/Contents/MacOS/plasma)
    HFSUniStr255 name;
    FSRef parentRef;
    FSGetCatalogInfo(&ref, kFSCatInfoNone, 0, &name, 0, &parentRef);
    ref = parentRef;
    FSGetCatalogInfo(&ref, kFSCatInfoNone, 0, &name, 0, &parentRef);
    if (QString::fromUtf16(name.unicode, name.length) == "MacOS") {
        ref = parentRef;
        FSGetCatalogInfo(&ref, kFSCatInfoNone, 0, &name, 0, &parentRef);
        if (QString::fromUtf16(name.unicode, name.length) == "Contents") {
#ifdef Q_OS_MAC32
            FSSpec spec;
            ref = parentRef;
            FSGetCatalogInfo(&ref, kFSCatInfoNone, 0, &name, &spec, &parentRef);
            iconSpec = spec;
#else
            iconSpec = parentRef;
#endif
        }
    }

    loadedData = true;
}

KWindowInfo::KWindowInfo(WId win, unsigned long, unsigned long) : d(new Private)
{
    d->ref = 1;
    d->win = win;
    d->isLocal = true;
    if (!win) {
        d->win = (WId) d;
        d->isLocal = false;
    }
}

// this one is only to make QList<> or similar happy
KWindowInfo::KWindowInfo()
    : d(NULL)
{
}

KWindowInfo::~KWindowInfo()
{
    if (d != NULL) {
        if (--d->ref == 0) {
            delete d;
        }
    }
}

KWindowInfo::KWindowInfo(const KWindowInfo &wininfo)
    : d(wininfo.d)
{
    if (d != NULL) {
        ++d->ref;
    }
}

KWindowInfo &KWindowInfo::operator=(const KWindowInfo &wininfo)
{
    if (d != wininfo.d) {
        if (d != NULL)
            if (--d->ref == 0) {
                delete d;
            }
        d = wininfo.d;
        if (d != NULL) {
            ++d->ref;
        }
    }
    return *this;
}

bool KWindowInfo::valid(bool withdrawn_is_valid) const
{
    return d->pid() >= 0;
}

WId KWindowInfo::win() const
{
    return d->win;
}

unsigned long KWindowInfo::state() const
{
    return 0;
}

bool KWindowInfo::hasState(unsigned long s) const
{
    return false;
}

bool KWindowInfo::isMinimized() const
{
    if (d->axElement()) {
        CFBooleanRef val;
        if (AXUIElementCopyAttributeValue(d->axElement(), kAXMinimizedAttribute, (CFTypeRef *)&val) == noErr) {
            return CFBooleanGetValue(val);
        } else {
            return false;
        }
    } else {
        return false;
    }
}

NET::MappingState KWindowInfo::mappingState() const
{
    return (NET::MappingState) 0;
}

NETExtendedStrut KWindowInfo::extendedStrut() const
{
    NETExtendedStrut ext;
    return ext;
}

NET::WindowType KWindowInfo::windowType(int supported_types) const
{
    return (NET::WindowType) 0;
}

QString KWindowInfo::visibleNameWithState() const
{
    QString s = visibleName();
    if (isMinimized()) {
        s.prepend(QLatin1Char('('));
        s.append(QLatin1Char(')'));
    }
    return s;
}

QString KWindowInfo::visibleName() const
{
    return name();
}

QString KWindowInfo::name() const
{
    if (!d->loadedData) {
        d->updateData();
    }
    return d->name;
}

QString KWindowInfo::visibleIconNameWithState() const
{
    QString s = visibleIconName();
    if (isMinimized()) {
        s.prepend(QLatin1Char('('));
        s.append(QLatin1Char(')'));
    }
    return s;
}

QString KWindowInfo::visibleIconName() const
{
    return visibleName();
}

QString KWindowInfo::iconName() const
{
    return name();
}

bool KWindowInfo::isOnCurrentDesktop() const
{
    return isOnDesktop(KWindowSystem::currentDesktop());
}

bool KWindowInfo::isOnDesktop(int _desktop) const
{
    return true;
}

bool KWindowInfo::onAllDesktops() const
{
    return false;
}

int KWindowInfo::desktop() const
{
    return 0;
}

QRect KWindowInfo::geometry() const
{
    return QRect();
}

QRect KWindowInfo::frameGeometry() const
{
    return QRect();
}

bool KWindowInfo::actionSupported(NET::Action action) const
{
    return true; // no idea if it's supported or not -> pretend it is
}

#if 0
WId KWindowInfo::transientFor() const
{
    kWarning((d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2TransientFor) == 0, 176)
            << "Pass NET::WM2TransientFor to KWindowInfo";
    return d->info->transientFor();
}

WId KWindowInfo::groupLeader() const
{
    kWarning((d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2GroupLeader) == 0, 176)
            << "Pass NET::WM2GroupLeader to KWindowInfo";
    return d->info->groupLeader();
}

QByteArray KWindowInfo::windowClassClass() const
{
    kWarning((d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowClass) == 0, 176)
            << "Pass NET::WM2WindowClass to KWindowInfo";
    return d->info->windowClassClass();
}

QByteArray KWindowInfo::windowClassName() const
{
    kWarning((d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowClass) == 0, 176)
            << "Pass NET::WM2WindowClass to KWindowInfo";
    return d->info->windowClassName();
}

QByteArray KWindowInfo::windowRole() const
{
    kWarning((d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowRole) == 0, 176)
            << "Pass NET::WM2WindowRole to KWindowInfo";
    return d->info->windowRole();
}

QByteArray KWindowInfo::clientMachine() const
{
    kWarning((d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2ClientMachine) == 0, 176)
            << "Pass NET::WM2ClientMachine to KWindowInfo";
    return d->info->clientMachine();
}

bool KWindowInfo::actionSupported(NET::Action action) const
{
    kWarning((d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2AllowedActions) == 0, 176)
            << "Pass NET::WM2AllowedActions to KWindowInfo";
    if (KWindowSystem::allowedActionsSupported()) {
        return d->info->allowedActions() & action;
    } else {
        return true;    // no idea if it's supported or not -> pretend it is
    }
}

// see NETWM spec section 7.6
bool KWindowInfo::isMinimized() const
{
    if (mappingState() != NET::Iconic) {
        return false;
    }
    // NETWM 1.2 compliant WM - uses NET::Hidden for minimized windows
    if ((state() & NET::Hidden) != 0
            && (state() & NET::Shaded) == 0) {  // shaded may have NET::Hidden too
        return true;
    }
    // older WMs use WithdrawnState for other virtual desktops
    // and IconicState only for minimized
    return KWindowSystem::icccmCompliantMappingState() ? false : true;
}
#endif
