/*
    This file is part of the KDE libraries
    Copyright (C) 2008 Carlo Segato (brandon.ml@gmail.com)
    Copyright (C) 2011 Pau Garcia i Quiles (pgquiles@elpauer.org)

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

#include "kwindowinfo.h"
#include "kwindowsystem.h"
#include <windows.h>
#include <stdlib.h>
#include <QCoreApplication>

class Q_DECL_HIDDEN KWindowInfo::Private
{
public:
    Private()
        : properties(0), properties2(0)
    {}

    ~Private() { }

    HWND win_;
    int ref;
    unsigned long properties;
    unsigned long properties2;
private:
    Private(const Private &);
    void operator=(const Private &);
};

#include <QRect>

KWindowInfo::KWindowInfo(WId win, unsigned long properties, unsigned long properties2) : d(new Private)
{
    d->ref = 1;
    d->win_ = reinterpret_cast<HWND>(win);
    d->properties  = properties;
    d->properties2 = properties2;
}

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
    return true;
}

WId KWindowInfo::win() const
{
    return reinterpret_cast<WId>(d->win_);
}

unsigned long KWindowInfo::state() const
{
    unsigned long state = 0;
#ifndef _WIN32_WCE
    if (IsZoomed(d->win_)) {
        state |= NET::Max;
    }
#endif
    if (!IsWindowVisible(d->win_)) {
        state |= NET::Hidden;
    }

#ifndef _WIN32_WCE
    LONG_PTR lp = GetWindowLongPtr(d->win_, GWL_EXSTYLE);
    if (lp & WS_EX_TOOLWINDOW) {
        state |= NET::SkipTaskbar;
    }
#endif

    return state;
}

bool KWindowInfo::hasState(unsigned long s) const
{
    return (state() & s);
}

bool KWindowInfo::isMinimized() const
{
#ifndef _WIN32_WCE
    return IsIconic(d->win_);
#else
    return false;
#endif
}

NET::MappingState KWindowInfo::mappingState() const
{
#ifndef _WIN32_WCE
    if (IsIconic(d->win_)) {
        return NET::Iconic;
    }
#endif
    if (!IsWindowVisible(d->win_)) {
        return NET::Withdrawn;
    }
    return NET::Visible;
}

NETExtendedStrut KWindowInfo::extendedStrut() const
{
    return NETExtendedStrut();
}

NET::WindowType KWindowInfo::windowType(int supported_types) const
{
    NET::WindowType wt(NET::Normal);

    long windowStyle   = GetWindowLong(d->win_, GWL_STYLE);
    long windowStyleEx = GetWindowLong(d->win_, GWL_EXSTYLE);

    if (windowStyle & WS_POPUP && supported_types & NET::PopupMenuMask) {
        return NET::PopupMenu;
    } else if (windowStyleEx & WS_EX_TOOLWINDOW && supported_types & NET::TooltipMask) {
        return NET::Tooltip;
    } else if (!(windowStyle & WS_CHILD) && supported_types & NET::NormalMask) {
        return NET::Normal;
    }

    return wt;
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
    QByteArray windowText = QByteArray((GetWindowTextLength(d->win_) + 1) * sizeof(wchar_t), 0);
    GetWindowTextW(d->win_, (LPWSTR)windowText.data(), windowText.size());
    return QString::fromWCharArray((wchar_t *)windowText.data());
}

QString KWindowInfo::visibleIconNameWithState() const
{
    return QString();
}

QString KWindowInfo::visibleIconName() const
{
    return QString();
}

QString KWindowInfo::iconName() const
{
    return QString();
}

bool KWindowInfo::isOnCurrentDesktop() const
{
    return true;
}

bool KWindowInfo::isOnDesktop(int desk) const
{
    return desk == desktop();
}

bool KWindowInfo::onAllDesktops() const
{
    return false;
}

int KWindowInfo::desktop() const
{
    return 1;
}

QRect KWindowInfo::geometry() const
{
    RECT wndRect;
    memset(&wndRect, 0, sizeof(wndRect));

    //fetch the geometry INCLUDING the frames
    if (GetWindowRect(d->win_, &wndRect)) {
        QRect result;
        result.setCoords(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
        return result;
    }

    return QRect();
}

QRect KWindowInfo::frameGeometry() const
{
    RECT wndRect;
    memset(&wndRect, 0, sizeof(wndRect));

    //fetch only client area geometries ... i hope thats right
    if (GetClientRect(d->win_, &wndRect)) {
        QRect result;
        result.setCoords(wndRect.left, wndRect.top, wndRect.right, wndRect.bottom);
        return result;
    }

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
#endif

QByteArray KWindowInfo::windowClassClass() const
{
//    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowClass ) == 0, 176 )
//        << "Pass NET::WM2WindowClass to KWindowInfo";
//    return d->info->windowClassClass();

    // Implemented per http://tronche.com/gui/x/icccm/sec-4.html#WM_CLASS (but only 2nd and 3rd choices, -name ignored)
    char *resourcenamevar;
    resourcenamevar = getenv("RESOURCE_NAME");
    if (resourcenamevar != NULL) {
        return QByteArray(resourcenamevar);
    }

    return QCoreApplication::applicationName().toLocal8Bit();
}

QByteArray KWindowInfo::windowClassName() const
{
//    kWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowClass ) == 0, 176 )
//        << "Pass NET::WM2WindowClass to KWindowInfo";
//    return d->info->windowClassName();

    // Maybe should use RealGetWindowClass instead of GetClassName? See
    // http://blogs.msdn.com/b/oldnewthing/archive/2010/12/31/10110524.aspx

    const int max = 256; // truncate to 255 characters
    TCHAR name[max];
    int count = GetClassName(d->win_, name, max);
    return QString::fromUtf16((ushort *)name).toLocal8Bit();
}

#if 0
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
#endif
