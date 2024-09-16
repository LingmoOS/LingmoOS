/*
    This file is part of the KDE libraries
    Copyright (C) 2007 Laurent Montel (montel@kde.org)
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

#include "kwindowsystem.h"
#include "kwindowinfo_mac_p.h"

#include <kxerrorhandler.h>
#include <QBitmap>
#include <QDesktopWidget>
#include <QDialog>
#include <QDebug>

#include <Carbon/Carbon.h>

// Uncomment the following line to enable the experimental (and not fully functional) window tracking code. Without this
// only the processes/applications are tracked, not the individual windows. This currently is quite broken as I can't
// seem to be able to convince the build system to generate a mov file from both the public header file, and also for this
// private class
// #define EXPERIMENTAL_WINDOW_TRACKING

static bool operator<(const ProcessSerialNumber &a, const ProcessSerialNumber &b)
{
    if (a.lowLongOfPSN != b.lowLongOfPSN) {
        return a.lowLongOfPSN < b.lowLongOfPSN;
    }
    return a.highLongOfPSN < b.highLongOfPSN;
}

class KWindowSystemPrivate : QObject
{
#ifdef EXPERIMENTAL_WINDOW_TRACKING
    Q_OBJECT
#endif
public:
    KWindowSystemPrivate();

    QMap<WId, KWindowInfo> windows;
    QList<WId> winids; // bah, because KWindowSystem::windows() returns a const reference, we need to keep this separate...
    QMap<pid_t, AXObserverRef> newWindowObservers;
    QMap<pid_t, AXObserverRef> windowClosedObservers;
    QMap<ProcessSerialNumber, WId> processes;
#ifdef EXPERIMENTAL_WINDOW_TRACKING
    QList<KWindowInfo> nonProcessedWindows;
#endif

    EventTargetRef m_eventTarget;
    EventHandlerUPP m_eventHandler;
    EventTypeSpec m_eventType[2];
    EventHandlerRef m_curHandler;

    void applicationLaunched(const ProcessSerialNumber &psn);
    void applicationTerminated(const ProcessSerialNumber &psn);

    bool m_noEmit;
    bool waitingForTimer;

#ifdef EXPERIMENTAL_WINDOW_TRACKING
    void newWindow(AXUIElementRef element, void *windowInfoPrivate);
    void windowClosed(AXUIElementRef element, void *windowInfoPrivate);
#endif

    static KWindowSystemPrivate *self()
    {
        return KWindowSystem::s_d_func();
    }
#ifdef EXPERIMENTAL_WINDOW_TRACKING
public Q_SLOTS:
    void tryRegisterProcess();
#endif
};

class KWindowSystemStaticContainer
{
public:
    KWindowSystemStaticContainer() : d(new KWindowSystemPrivate) { }
    KWindowSystem kwm;
    KWindowSystemPrivate *d;
};

KWINDOWSYSTEM_GLOBAL_STATIC(KWindowSystemStaticContainer, g_kwmInstanceContainer)

static OSStatus applicationEventHandler(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void *inUserData)
{
    KWindowSystemPrivate *d = (KWindowSystemPrivate *) inUserData;

    UInt32 kind;

    kind = GetEventKind(inEvent);
    ProcessSerialNumber psn;
    if (GetEventParameter(inEvent, kEventParamProcessID, typeProcessSerialNumber, NULL, sizeof psn, NULL, &psn) != noErr) {
        kWarning() << "Error getting event parameter in application event";
        return eventNotHandledErr;
    }

    if (kind == kEventAppLaunched) {
        d->applicationLaunched(psn);
    } else if (kind == kEventAppTerminated) {
        d->applicationTerminated(psn);
    }

    return noErr;
}

#ifdef EXPERIMENTAL_WINDOW_TRACKING
static void windowClosedObserver(AXObserverRef observer, AXUIElementRef element, CFStringRef notification, void *refcon)
{
    KWindowSystemPrivate::self()->windowClosed(element, refcon);
}

static void newWindowObserver(AXObserverRef observer, AXUIElementRef element, CFStringRef notification, void *refcon)
{
    KWindowSystemPrivate::self()->newWindow(element, refcon);
}
#endif

KWindowSystemPrivate::KWindowSystemPrivate()
    : QObject(0), m_noEmit(true), waitingForTimer(false)
{
    // find all existing windows
    ProcessSerialNumber psn = {0, kNoProcess};
    while (GetNextProcess(&psn) == noErr) {
        qDebug() << "calling appLaunched for " << psn.lowLongOfPSN << ":" << psn.highLongOfPSN;
        applicationLaunched(psn);
    }

    m_noEmit = false;

#ifdef Q_OS_MAC32
    // register callbacks for application launches/quits
    m_eventTarget = GetApplicationEventTarget();
    m_eventHandler = NewEventHandlerUPP(applicationEventHandler);
    m_eventType[0].eventClass = kEventClassApplication;
    m_eventType[0].eventKind = kEventAppLaunched;
    m_eventType[1].eventClass = kEventClassApplication;
    m_eventType[1].eventKind = kEventAppTerminated;
    if (InstallEventHandler(m_eventTarget, m_eventHandler, 2, m_eventType, this, &m_curHandler) != noErr) {
        qDebug() << "Installing event handler failed!\n";
    }
#else
#warning port me to Mac64
#endif
}

void KWindowSystemPrivate::applicationLaunched(const ProcessSerialNumber &psn)
{
#ifdef Q_OS_MAC32
    qDebug() << "new app: " << psn.lowLongOfPSN << ":" << psn.highLongOfPSN;
    ProcessInfoRec pinfo;
    FSSpec appSpec;
    pinfo.processInfoLength = sizeof pinfo;
    pinfo.processName = 0;
    pinfo.processAppSpec = &appSpec;
    GetProcessInformation(&psn, &pinfo);
    if ((pinfo.processMode & modeOnlyBackground) != 0) {
        return;
    }
    // found a process, create a pseudo-window for it

    KWindowInfo winfo(0, 0);
    windows[winfo.win()] = winfo;
    winids.append(winfo.win());
    winfo.d->setProcessSerialNumber(psn);
    pid_t pid = winfo.d->pid();
    processes[psn] = winfo.win();
    qDebug() << "  pid:" << pid;
    AXUIElementRef app = AXUIElementCreateApplication(pid);
    winfo.d->setAxElement(app);
    if (!m_noEmit) {
        emit KWindowSystem::self()->windowAdded(winfo.win());
    }

#ifdef EXPERIMENTAL_WINDOW_TRACKING
    // create an observer and listen for new window events
    AXObserverRef observer, newObserver;
    OSStatus err;
    if (AXObserverCreate(pid, windowClosedObserver, &observer) == noErr) {
        CFRunLoopAddSource(CFRunLoopGetCurrent(), AXObserverGetRunLoopSource(observer), kCFRunLoopCommonModes);
        windowClosedObservers[pid] = observer;
    }
    if ((err = AXObserverCreate(pid, newWindowObserver, &newObserver)) == noErr) {
        CFRunLoopAddSource(CFRunLoopGetCurrent(), AXObserverGetRunLoopSource(newObserver), kCFRunLoopCommonModes);
        newWindowObservers[pid] = newObserver;
        if ((err = AXObserverAddNotification(newObserver, app, kAXWindowCreatedNotification, winfo.d)) != noErr) {
            qDebug() << "Error " << err << " adding notification to observer";
            // adding notifier failed, apparently app isn't responding to accesability messages yet
            // try it one more time later, and for now just return
            QTimer::singleShot(500, this, SLOT(tryRegisterProcess()));
            nonProcessedWindows.append(winfo);
            return;
        } else {
            qDebug() << "Added notification and observer";
        }
    } else {
        qDebug() << "Error creating observer";
    }

    CFIndex windowsInApp;
    AXUIElementGetAttributeValueCount(app, kAXWindowsAttribute, &windowsInApp);
    CFArrayRef array;
    AXUIElementCopyAttributeValue(app, kAXWindowsAttribute, (CFTypeRef *)&array);
    for (CFIndex j = 0; j < windowsInApp; j++) {
        AXUIElementRef win = (AXUIElementRef) CFArrayGetValueAtIndex(array, j);
        newWindow(win, winfo.d);
    }
#endif
#else
#warning Port me to Mac64
#endif
}

#ifdef EXPERIMENTAL_WINDOW_TRACKING
void KWindowSystemPrivate::tryRegisterProcess()
{
    qDebug() << "Single-shot timer, trying to register processes";
    while (!nonProcessedWindows.empty()) {
        KWindowInfo winfo = nonProcessedWindows.takeLast();
        pid_t pid = winfo.d->pid();
        AXUIElementRef app = winfo.d->axElement();
        ProcessSerialNumber psn = winfo.d->psn();

        // create an observer and listen for new window events
        AXObserverRef observer;
        OSStatus err;
        observer = newWindowObservers[pid];
        if ((err = AXObserverAddNotification(observer, app, kAXWindowCreatedNotification, winfo.d)) != noErr) {
            qDebug() << "Error " << err << " adding notification to observer";
        } else {
            qDebug() << "Added notification and observer";
        }

        observer = windowClosedObservers[pid];

        CFIndex windowsInApp;
        AXUIElementGetAttributeValueCount(app, kAXWindowsAttribute, &windowsInApp);
        CFArrayRef array;
        AXUIElementCopyAttributeValue(app, kAXWindowsAttribute, (CFTypeRef *)&array);
        for (CFIndex j = 0; j < windowsInApp; j++) {
            AXUIElementRef win = (AXUIElementRef) CFArrayGetValueAtIndex(array, j);
            newWindow(win, winfo.d);
        }
    }
}
#endif

void KWindowSystemPrivate::applicationTerminated(const ProcessSerialNumber &psn)
{
    qDebug() << "Terminated PSN: " << psn.lowLongOfPSN << ":" << psn.highLongOfPSN;
    WId id = processes[psn];
    if (windows.contains(id)) {
        KWindowInfo winfo = windows[id];
        foreach (KWindowInfo::Private *wi, winfo.d->children) {
            winids.removeAll(wi->win);
            emit KWindowSystem::self()->windowRemoved(wi->win);
        }
        winids.removeAll(id);
        emit KWindowSystem::self()->windowRemoved(winfo.win());
    }
}

#ifdef EXPERIMENTAL_WINDOW_TRACKING
void KWindowSystemPrivate::windowClosed(AXUIElementRef element, void *refcon)
{
    qDebug() << "Received window closed notification";

    KWindowInfo::Private *wind = (KWindowInfo::Private *) refcon; // window being closed
    KWindowInfo::Private *parent = wind->parent;
    parent->children.removeAll(wind);
    winids.removeAll(wind->win);
    if (!m_noEmit) {
        emit KWindowSystem::self()->windowRemoved(wind->win);
    }
}

void KWindowSystemPrivate::newWindow(AXUIElementRef win, void *refcon)
{
    qDebug() << "Received new window notification";

    KWindowInfo::Private *winfod = (KWindowInfo::Private *) refcon;
    pid_t pid = winfod->pid();
    ProcessSerialNumber psn = winfod->psn();
    AXObserverRef observer = windowClosedObservers[pid];

    KWindowInfo win2(0, 0);
    // listen for closed events for this window
    if (AXObserverAddNotification(observer, win, kAXUIElementDestroyedNotification, win2.d) != noErr) {
        // when we can't receive close events, the window should not be added
        qDebug() "error adding closed observer to window.";
        return;
    }

    windows[win2.win()] = win2;
    winids.append(win2.win());
    win2.d->setProcessSerialNumber(psn);
    win2.d->setAxElement(win);
    winfod->children.append(win2.d);
    win2.d->parent = winfod;
    if (!m_noEmit) {
        emit KWindowSystem::self()->windowAdded(win2.win());
    }
}
#endif

KWindowSystem *KWindowSystem::self()
{
    return &(g_kwmInstanceContainer->kwm);
}

KWindowSystemPrivate *KWindowSystem::s_d_func()
{
    return g_kwmInstanceContainer->d;
}

const QList<WId> &KWindowSystem::windows()
{
    KWindowSystemPrivate *d = KWindowSystem::s_d_func();
    return d->winids;
}

bool KWindowSystem::hasWId(WId id)
{
    KWindowSystemPrivate *d = KWindowSystem::s_d_func();
    return d->windows.contains(id);
}

#ifndef KWINDOWSYSTEM_NO_DEPRECATED
KWindowInfo KWindowSystem::windowInfo(WId win, unsigned long properties, unsigned long properties2)
{
    KWindowSystemPrivate *d = KWindowSystem::s_d_func();
    if (d->windows.contains(win)) {
        return d->windows[win];
    } else {
        return KWindowInfo(win, properties, properties2);
    }
}
#endif

QList<WId> KWindowSystem::stackingOrder()
{
    //TODO
    QList<WId> lst;
    qDebug() << "QList<WId> KWindowSystem::stackingOrder() isn't yet implemented!";
    return lst;
}

WId KWindowSystem::activeWindow()
{
    //return something
    qDebug() << "WId KWindowSystem::activeWindow()   isn't yet implemented!";
    return 0;
}

void KWindowSystem::activateWindow(WId win, long time)
{
    //TODO
    qDebug() << "KWindowSystem::activateWindow( WId win, long time )isn't yet implemented!";
    KWindowSystemPrivate *d = KWindowSystem::s_d_func();
    if (d->windows.contains(win)) {
        ProcessSerialNumber psn = d->windows[win].d->psn();
        SetFrontProcess(&psn);
    }
}

void KWindowSystem::forceActiveWindow(WId win, long time)
{
    //TODO
    qDebug() << "KWindowSystem::forceActiveWindow( WId win, long time ) isn't yet implemented!";
    activateWindow(win, time);
}

void KWindowSystem::demandAttention(WId win, bool set)
{
    //TODO
    qDebug() << "KWindowSystem::demandAttention( WId win, bool set ) isn't yet implemented!";
}

bool KWindowSystem::compositingActive()
{
    return true;
}

int KWindowSystem::currentDesktop()
{
    return 1;
}

int KWindowSystem::numberOfDesktops()
{
    return 1;
}

void KWindowSystem::setCurrentDesktop(int desktop)
{
    qDebug() << "KWindowSystem::setCurrentDesktop( int desktop ) isn't yet implemented!";
    //TODO
}

void KWindowSystem::setOnAllDesktops(WId win, bool b)
{
    qDebug() << "KWindowSystem::setOnAllDesktops( WId win, bool b ) isn't yet implemented!";
    //TODO
}

void KWindowSystem::setOnDesktop(WId win, int desktop)
{
    //TODO
    qDebug() << "KWindowSystem::setOnDesktop( WId win, int desktop ) isn't yet implemented!";
}

void KWindowSystem::setMainWindow(QWidget *subwindow, WId id)
{
    qDebug() << "KWindowSystem::setMainWindow( QWidget*, WId ) isn't yet implemented!";
    //TODO
}

QPixmap KWindowSystem::icon(WId win, int width, int height, bool scale)
{
    if (hasWId(win)) {
        KWindowInfo info = windowInfo(win, 0);
        if (!info.d->loadedData) {
            info.d->updateData();
        }
        IconRef icon;
        SInt16 label;
#ifdef Q_OS_MAC32
        OSErr err = GetIconRefFromFile(&info.d->iconSpec, &icon, &label);
#else
        OSStatus err = GetIconRefFromFileInfo(&info.d->iconSpec, 0, 0,
                                              kIconServicesCatalogInfoMask, 0, kIconServicesNormalUsageFlag, &icon, &label);
#endif
        if (err != noErr) {
            qDebug() << "Error getting icon from application";
            return QPixmap();
        } else {
            QPixmap ret(width, height);
            ret.fill(QColor(0, 0, 0, 0));

            CGRect rect = CGRectMake(0, 0, width, height);

            CGContextRef ctx = qt_mac_cg_context(&ret);
            CGAffineTransform old_xform = CGContextGetCTM(ctx);
            CGContextConcatCTM(ctx, CGAffineTransformInvert(old_xform));
            CGContextConcatCTM(ctx, CGAffineTransformIdentity);

            ::RGBColor b;
            b.blue = b.green = b.red = 255 * 255;
            PlotIconRefInContext(ctx, &rect, kAlignNone, kTransformNone, &b, kPlotIconRefNormalFlags, icon);
            CGContextRelease(ctx);

            ReleaseIconRef(icon);
            return ret;
        }
    } else {
        qDebug() << "QPixmap KWindowSystem::icon( WId win, int width, int height, bool scale ) isn't yet implemented for local windows!";
        return QPixmap();
    }
}

QPixmap KWindowSystem::icon(WId win, int width, int height, bool scale, int flags)
{
    return icon(win, width, height, scale);
//    qDebug() << "QPixmap KWindowSystem::icon( WId win, int width, int height, bool scale, int flags ) isn't yet implemented!";
}

void KWindowSystem::setIcons(WId win, const QPixmap &icon, const QPixmap &miniIcon)
{
    //TODO
    qDebug() << "KWindowSystem::setIcons( WId win, const QPixmap& icon, const QPixmap& miniIcon ) isn't yet implemented!";
}

void KWindowSystem::setType(WId winid, NET::WindowType windowType)
{
#ifdef Q_OS_MAC32
    // not supported for 'global' windows; only for windows in the current process
    if (hasWId(winid)) {
        return;
    }

    static WindowGroupRef desktopGroup = 0;
    static WindowGroupRef dockGroup = 0;

    WindowRef win = HIViewGetWindow((HIViewRef) winid);
    //TODO: implement other types than Desktop and Dock
    if (windowType != NET::Desktop && windowType != NET::Dock) {
        qDebug() << "setType( WId win, NET::WindowType windowType ) isn't yet implemented for the type you requested!";
    }
    if (windowType == NET::Desktop) {
        if (!desktopGroup) {
            CreateWindowGroup(0, &desktopGroup);
            SetWindowGroupLevel(desktopGroup, kCGDesktopIconWindowLevel);
        }
        SetWindowGroup(win, desktopGroup);
    } else if (windowType == NET::Dock) {
        if (!dockGroup) {
            CreateWindowGroup(0, &dockGroup);
            SetWindowGroupLevel(dockGroup, kCGDockWindowLevel);
        }
        SetWindowGroup(win, dockGroup);
        ChangeWindowAttributes(win, kWindowNoTitleBarAttribute, kWindowNoAttributes);
    }
#else
#warning port me to Mac64
#endif
}

void KWindowSystem::setState(WId win, NET::States state)
{
    //TODO
    qDebug() << "KWindowSystem::setState( WId win, unsigned long state ) isn't yet implemented!";
}

void KWindowSystem::clearState(WId win, NET::States state)
{
    //TODO
    qDebug() << "KWindowSystem::clearState( WId win, unsigned long state ) isn't yet implemented!";
}

void KWindowSystem::minimizeWindow(WId win, bool animation)
{
    //TODO
    qDebug() << "KWindowSystem::minimizeWindow( WId win, bool animation) isn't yet implemented!";
}

void KWindowSystem::unminimizeWindow(WId win, bool animation)
{
    //TODO
    qDebug() << "KWindowSystem::unminimizeWindow( WId win, bool animation ) isn't yet implemented!";
}

void KWindowSystem::raiseWindow(WId win)
{
    //TODO
    qDebug() << "KWindowSystem::raiseWindow( WId win ) isn't yet implemented!";
}

void KWindowSystem::lowerWindow(WId win)
{
    //TODO
    qDebug() << "KWindowSystem::lowerWindow( WId win ) isn't yet implemented!";
}

bool KWindowSystem::icccmCompliantMappingState()
{
    return false;
}

QRect KWindowSystem::workArea(int desktop)
{
    //TODO
    qDebug() << "QRect KWindowSystem::workArea( int desktop ) isn't yet implemented!";
    return QRect();
}

QRect KWindowSystem::workArea(const QList<WId> &exclude, int desktop)
{
    //TODO
    qDebug() << "QRect KWindowSystem::workArea( const QList<WId>& exclude, int desktop ) isn't yet implemented!";
    return QRect();
}

QString KWindowSystem::desktopName(int desktop)
{
    return tr("Desktop %1").arg(desktop);
}

void KWindowSystem::setDesktopName(int desktop, const QString &name)
{
    qDebug() << "KWindowSystem::setDesktopName( int desktop, const QString& name ) isn't yet implemented!";
    //TODO
}

bool KWindowSystem::showingDesktop()
{
    return false;
}

void KWindowSystem::setUserTime(WId win, long time)
{
    qDebug() << "KWindowSystem::setUserTime( WId win, long time ) isn't yet implemented!";
    //TODO
}

void KWindowSystem::setExtendedStrut(WId win, int left_width, int left_start, int left_end,
                                     int right_width, int right_start, int right_end, int top_width, int top_start, int top_end,
                                     int bottom_width, int bottom_start, int bottom_end)
{
    qDebug() << "KWindowSystem::setExtendedStrut isn't yet implemented!";
    //TODO
}

void KWindowSystem::setStrut(WId win, int left, int right, int top, int bottom)
{
    qDebug() << "KWindowSystem::setStrut isn't yet implemented!";
    //TODO
}

bool KWindowSystem::allowedActionsSupported()
{
    return false;
}

QString KWindowSystem::readNameProperty(WId window, unsigned long atom)
{
    //TODO
    qDebug() << "QString KWindowSystem::readNameProperty( WId window, unsigned long atom ) isn't yet implemented!";
    return QString();
}

void KWindowSystem::connectNotify(const char *signal)
{
    qDebug() << "connectNotify( const char* signal )  isn't yet implemented!";
    //TODO
}

void KWindowSystem::allowExternalProcessWindowActivation(int pid)
{
    // Needed on mac ?
}

void KWindowSystem::setBlockingCompositing(WId window, bool active)
{
    //TODO
    qDebug() << "setBlockingCompositing( WId window, bool active ) isn't yet implemented!";
}

#include "moc_kwindowsystem.cpp"
