/*
    This file is part of the KDE libraries
    Copyright (C) 2007 Laurent Montel (montel@kde.org)
    Copyright (C) 2007 Christian Ehrlicher (ch.ehrlicher@gmx.de)

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

#include <QDesktopWidget>
#include <QIcon>
#include <QBitmap>
#include <QPixmap>
#include <QLibrary>
#include <QDebug>
#include <QApplication>
#include <QtWin>
#include <QMetaMethod>

#include <windows.h>
#include <windowsx.h>

#ifdef _WIN64
#define GCL_HICON GCLP_HICON
#define GCL_HICONSM GCLP_HICONSM
#endif

//function to register us as taskmanager
#define RSH_UNREGISTER  0
#define RSH_REGISTER    1
#define RSH_TASKMGR     3
typedef bool (WINAPI *PtrRegisterShellHook)(HWND hWnd, DWORD method);

static PtrRegisterShellHook pRegisterShellHook = 0;
static int WM_SHELLHOOK = -1;

class KWindowSystemStaticContainer
{
public:
    KWindowSystemStaticContainer() : d(0) {}
    KWindowSystem kwm;
    KWindowSystemPrivate *d;
};

Q_GLOBAL_STATIC(KWindowSystemStaticContainer, g_kwmInstanceContainer)

struct InternalWindowInfo {
    InternalWindowInfo() {}
    QPixmap bigIcon;
    QPixmap smallIcon;
    QString windowName;
};

class KWindowSystemPrivate : public QWidget
{
    friend class KWindowSystem;
public:
    KWindowSystemPrivate(int what);
    ~KWindowSystemPrivate();

    static bool CALLBACK EnumWindProc(HWND hwnd, LPARAM lparam);
    static void readWindowInfo(HWND hwnd, InternalWindowInfo *winfo);

    void windowAdded(WId wid);
    void windowRemoved(WId wid);
    void windowActivated(WId wid);
    void windowRedraw(WId wid);
    void windowFlash(WId wid);
    void windowStateChanged(WId wid);
    void reloadStackList();
    void activate();

protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;

private:
    bool activated;
    int what;
    WId fakeHwnd;
    QList<WId> stackingOrder;
    QMap<WId, InternalWindowInfo> winInfos;
};

static HBITMAP QPixmapMask2HBitmap(const QPixmap &pix)
{
    QBitmap bm = pix.mask();
    if (bm.isNull()) {
        bm = QBitmap(pix.size());
        bm.fill(Qt::color1);
    }
    QImage im = bm.toImage().convertToFormat(QImage::Format_Mono);
    im.invertPixels();                  // funny blank'n'white games on windows
    int w = im.width();
    int h = im.height();
    int bpl = ((w + 15) / 16) * 2;      // bpl, 16 bit alignment
    QByteArray bits(bpl * h, '\0');
    for (int y = 0; y < h; y++) {
        memcpy(bits.data() + y * bpl, im.scanLine(y), bpl);
    }
    return CreateBitmap(w, h, 1, 1, bits.constData());
}

KWindowSystemPrivate::KWindowSystemPrivate(int what) : QWidget(0), activated(false)
{
    //i think there is no difference in windows we always load everything
    what = KWindowSystem::INFO_WINDOWS;
    setVisible(false);
}

void KWindowSystemPrivate::activate()
{
#if 0
    //prevent us from doing the same over and over again
    if (activated) {
        return;
    }
    activated = true;

    //resolve winapi stuff
    if (!pRegisterShellHook) {
        pRegisterShellHook = (PtrRegisterShellHook)QLibrary::resolve("shell32", (LPCSTR)0xb5);
    }

    //get the id for the shellhook message
    if (WM_SHELLHOOK == -1) {
        WM_SHELLHOOK = RegisterWindowMessage(TEXT("SHELLHOOK"));
        //qDebug() << "WM_SHELLHOOK:" << WM_SHELLHOOK << winId();
    }

    bool shellHookRegistered = false;
    if (pRegisterShellHook) {
        shellHookRegistered = pRegisterShellHook(winId(), RSH_TASKMGR);
    }

    if (!shellHookRegistered)
        //use a timer and poll the windows ?
    {
        qDebug() << "Could not create shellhook to receive WindowManager Events";
    }

    //fetch window infos
    reloadStackList();
#endif
}

KWindowSystemPrivate::~KWindowSystemPrivate()
{
    if (pRegisterShellHook) {
        pRegisterShellHook(reinterpret_cast<HWND>(winId()), RSH_UNREGISTER);
    }
}

/**
 *the callback procedure for the invisible ShellHook window
 */
bool KWindowSystemPrivate::nativeEvent(const QByteArray &eventType, void *message_, long *result)
{
    if (eventType != QByteArrayLiteral("windows_generic_MSG")) {
        return QWidget::nativeEvent(eventType, message_, result);
    }

    MSG *message = static_cast<MSG *>(message_);

    /*
        check winuser.h for the following codes
        HSHELL_WINDOWCREATED        1
        HSHELL_WINDOWDESTROYED      2
        HSHELL_ACTIVATESHELLWINDOW  3
        HSHELL_WINDOWACTIVATED      4
        HSHELL_GETMINRECT           5
        HSHELL_RUDEAPPACTIVATED     32768 + 4 = 32772
        HSHELL_REDRAW               6
        HSHELL_FLASH                32768 + 6 = 32774
        HSHELL_TASKMAN              7
        HSHELL_LANGUAGE             8
        HSHELL_SYSMENU              9
        HSHELL_ENDTASK              10
        HSHELL_ACCESSIBILITYSTATE   11
        HSHELL_APPCOMMAND           12
        HSHELL_WINDOWREPLACED       13
        HSHELL_WINDOWREPLACING      14
       */
    if (message->message == WM_SHELLHOOK) {
//         qDebug() << "what has happened?:" << message->wParam << message->message;

        switch (message->wParam) {
        case HSHELL_WINDOWCREATED:
            KWindowSystem::s_d_func()->windowAdded(static_cast<WId>(message->lParam));
            break;
        case HSHELL_WINDOWDESTROYED:
            KWindowSystem::s_d_func()->windowRemoved(static_cast<WId>(message->lParam));
            break;
        case HSHELL_WINDOWACTIVATED:
#ifndef _WIN32_WCE
        case HSHELL_RUDEAPPACTIVATED:
#endif
            KWindowSystem::s_d_func()->windowActivated(static_cast<WId>(message->lParam));
            break;
#ifndef _WIN32_WCE
        case HSHELL_GETMINRECT:
            KWindowSystem::s_d_func()->windowStateChanged(static_cast<WId>(message->lParam));
            break;
        case HSHELL_REDRAW: //the caption has changed
            KWindowSystem::s_d_func()->windowRedraw(static_cast<WId>(message->lParam));
            break;
        case HSHELL_FLASH:
            KWindowSystem::s_d_func()->windowFlash(static_cast<WId>(message->lParam));
            break;
#endif
        }
    }
    return QWidget::nativeEvent(eventType, message_, result);
}

bool CALLBACK KWindowSystemPrivate::EnumWindProc(HWND hWnd, LPARAM lparam)
{
    WId win = reinterpret_cast<WId>(hWnd);
    QByteArray windowText = QByteArray((GetWindowTextLength(hWnd) + 1) * sizeof(wchar_t), 0);
    GetWindowTextW(hWnd, (LPWSTR)windowText.data(), windowText.size());
    DWORD ex_style = GetWindowExStyle(hWnd);
    KWindowSystemPrivate *p = KWindowSystem::s_d_func();

    QString add;
    if (!QString::fromWCharArray((wchar_t *)windowText.data()).trimmed().isEmpty() && IsWindowVisible(hWnd) && !(ex_style & WS_EX_TOOLWINDOW)
            && !GetParent(hWnd) && !GetWindow(hWnd, GW_OWNER) && !p->winInfos.contains(win)) {

//        qDebug()<<"Adding window to windowList " << add + QString(windowText).trimmed();

        InternalWindowInfo winfo;
        KWindowSystemPrivate::readWindowInfo(hWnd, &winfo);

        p->stackingOrder.append(win);
        p->winInfos.insert(win, winfo);
    }
    return true;
}

void KWindowSystemPrivate::readWindowInfo(HWND hWnd, InternalWindowInfo *winfo)
{
    QByteArray windowText = QByteArray((GetWindowTextLength(hWnd) + 1) * sizeof(wchar_t), 0);
    GetWindowTextW(hWnd, (LPWSTR)windowText.data(), windowText.size());
    //maybe use SendMessageTimout here?
    QPixmap smallIcon;
    HICON hSmallIcon = (HICON)SendMessage(hWnd, WM_GETICON, ICON_SMALL, 0);
    //if(!hSmallIcon) hSmallIcon = (HICON)SendMessage(hWnd, WM_GETICON, ICON_SMALL2, 0);
    if (!hSmallIcon) {
        hSmallIcon = (HICON)SendMessage(hWnd, WM_GETICON, ICON_BIG, 0);
    }
#ifndef _WIN32_WCE
    if (!hSmallIcon) {
        hSmallIcon = (HICON)GetClassLong(hWnd, GCL_HICONSM);
    }
    if (!hSmallIcon) {
        hSmallIcon = (HICON)GetClassLong(hWnd, GCL_HICON);
    }
#endif
    if (!hSmallIcon) {
        hSmallIcon = (HICON)SendMessage(hWnd, WM_QUERYDRAGICON, 0, 0);
    }
    if (hSmallIcon) {
        smallIcon  = QtWin::fromHICON(hSmallIcon);
    }

    QPixmap bigIcon;
    HICON hBigIcon = (HICON)SendMessage(hWnd, WM_GETICON, ICON_BIG, 0);
    //if(!hBigIcon) hBigIcon = (HICON)SendMessage(hWnd, WM_GETICON, ICON_SMALL2, 0);
    if (!hBigIcon) {
        hBigIcon = (HICON)SendMessage(hWnd, WM_GETICON, ICON_SMALL, 0);
    }
#ifndef _WIN32_WCE
    if (!hBigIcon) {
        hBigIcon = (HICON)GetClassLong(hWnd, GCL_HICON);
    }
    if (!hBigIcon) {
        hBigIcon = (HICON)GetClassLong(hWnd, GCL_HICONSM);
    }
#endif
    if (!hBigIcon) {
        hBigIcon = (HICON)SendMessage(hWnd, WM_QUERYDRAGICON, 0, 0);
    }
    if (hBigIcon) {
        bigIcon  = QtWin::fromHICON(hBigIcon);
    }

    winfo->bigIcon    = bigIcon;
    winfo->smallIcon  = smallIcon;
    winfo->windowName = QString::fromWCharArray((wchar_t *)windowText.data()).trimmed();
}

void KWindowSystemPrivate::windowAdded(WId wid)
{
//     qDebug() << "window added!";
    KWindowSystem::s_d_func()->reloadStackList();
    emit KWindowSystem::self()->windowAdded(wid);
    emit KWindowSystem::self()->activeWindowChanged(wid);
    emit KWindowSystem::self()->stackingOrderChanged();
}

void KWindowSystemPrivate::windowRemoved(WId wid)
{
//     qDebug() << "window removed!";
    KWindowSystem::s_d_func()->reloadStackList();
    emit KWindowSystem::self()->windowRemoved(wid);
    emit KWindowSystem::self()->stackingOrderChanged();
}

void KWindowSystemPrivate::windowActivated(WId wid)
{
//     qDebug() << "window activated!";
    if (!wid) {
        return;
    }

    KWindowSystem::s_d_func()->reloadStackList();
    emit KWindowSystem::self()->activeWindowChanged(wid);
    emit KWindowSystem::self()->stackingOrderChanged();
}

void KWindowSystemPrivate::windowRedraw(WId wid)
{
    KWindowSystem::s_d_func()->reloadStackList();
}

void KWindowSystemPrivate::windowFlash(WId wid)
{
    //emit KWindowSystem::self()->demandAttention( wid );
}

void KWindowSystemPrivate::windowStateChanged(WId wid)
{
    emit KWindowSystem::self()->windowChanged(wid);
}

void KWindowSystemPrivate::reloadStackList()
{
    KWindowSystem::s_d_func()->stackingOrder.clear();
    KWindowSystem::s_d_func()->winInfos.clear();
//    EnumWindows((WNDENUMPROC)EnumWindProc, 0 );
}

KWindowSystem *KWindowSystem::self()
{
    return &(g_kwmInstanceContainer()->kwm);
}

KWindowSystemPrivate *KWindowSystem::s_d_func()
{
    return g_kwmInstanceContainer()->d;
}

void KWindowSystem::init(int what)
{
    KWindowSystemPrivate *const s_d = s_d_func();

    if (what >= INFO_WINDOWS) {
        what = INFO_WINDOWS;
    } else {
        what = INFO_BASIC;
    }

    if (!s_d) {
        g_kwmInstanceContainer()->d = new KWindowSystemPrivate(what); // invalidates s_d
        g_kwmInstanceContainer()->d->activate();
    } else if (s_d->what < what) {
        delete s_d;
        g_kwmInstanceContainer()->d = new KWindowSystemPrivate(what); // invalidates s_d
        g_kwmInstanceContainer()->d->activate();
    }

}

bool KWindowSystem::allowedActionsSupported()
{
    return false;
}

int KWindowSystem::currentDesktop()
{
    return 1;
}

int KWindowSystem::numberOfDesktops()
{
    return 1;
}

void KWindowSystem::setMainWindow(QWidget *subwindow, WId mainwindow)
{
    SetForegroundWindow(reinterpret_cast<HWND>(subwindow->winId()));
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

WId KWindowSystem::activeWindow()
{
    return reinterpret_cast<WId>(GetActiveWindow());
}

void KWindowSystem::activateWindow(WId win, long)
{
    SetActiveWindow(reinterpret_cast<HWND>(win));
}

void KWindowSystem::forceActiveWindow(WId win, long time)
{
    HWND hwnd = reinterpret_cast<HWND>(win);
    // FIXME restoring a hidden window doesn't work: the window contents just appear white.
    // But the mouse cursor still acts as if the widgets were there (e.g. button clicking works),
    // which indicates the issue is at the window/backingstore level.
    // This is probably a side effect of bypassing Qt's internal window state handling.
#ifndef _WIN32_WCE
    if (IsIconic(hwnd) /*|| !IsWindowVisible( win ) */) {
        // Do not activate the window as we restore it,
        // otherwise the window appears see-through (contents not updated).
        ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    }
#endif
    // Puts the window in front and activates it.
    //to bring a window to the front while the user is active in a different apllication we
    //have to atach our self to the current active window
    HWND hwndActiveWin = GetForegroundWindow();
    int  idActive      = GetWindowThreadProcessId(hwndActiveWin, NULL);
    if (AttachThreadInput(GetCurrentThreadId(), idActive, TRUE)) {
        SetForegroundWindow(hwnd);
        SetFocus(hwnd);
        AttachThreadInput(GetCurrentThreadId(), idActive, FALSE);
    }

}

void KWindowSystem::demandAttention(WId win, bool set)
{
// One can not flash a windows in wince
#ifndef _WIN32_WCE
    FLASHWINFO fi;
    fi.cbSize = sizeof(FLASHWINFO);
    fi.hwnd = reinterpret_cast<HWND>(win);
    fi.dwFlags = set ? FLASHW_ALL : FLASHW_STOP;
    fi.uCount = 5;
    fi.dwTimeout = 0;

    FlashWindowEx(&fi);
#endif
}

QPixmap KWindowSystem::icon(WId win, int width, int height, bool scale)
{
    KWindowSystem::init(INFO_WINDOWS);

    QPixmap pm;
    if (KWindowSystem::s_d_func()->winInfos.contains(win)) {
        if (width < 24 || height < 24) {
            pm = KWindowSystem::s_d_func()->winInfos[win].smallIcon;
        } else {
            pm = KWindowSystem::s_d_func()->winInfos[win].bigIcon;
        }
    } else {
        qDebug() << "KWindowSystem::icon winid not in winInfos";
        UINT size = ICON_BIG;
        if (width < 24 || height < 24) {
            size = ICON_SMALL;
        }
        HICON hIcon = (HICON)SendMessage(reinterpret_cast<HWND>(win), WM_GETICON, size, 0);
        if (hIcon != NULL) {
            pm = QtWin::fromHICON(hIcon);
        }
    }
    if (scale) {
        pm = pm.scaled(width, height);
    }
    return pm;
}

QPixmap KWindowSystem::icon(WId win, int width, int height, bool scale, int)
{
    return icon(win, width, height, scale);
}

void KWindowSystem::setIcons(WId win, const QPixmap &icon, const QPixmap &miniIcon)
{
    KWindowSystem::init(INFO_WINDOWS);
    KWindowSystemPrivate *s_d = s_d_func();

    if (s_d->winInfos.contains(win)) {
        // is this safe enough or do i have to refresh() the window infos
        s_d->winInfos[win].smallIcon = miniIcon;
        s_d->winInfos[win].bigIcon   = icon;
    }

    HICON hIconBig =  QtWin::toHICON(icon);
    HICON hIconSmall = QtWin::toHICON(miniIcon);

    HWND hwnd = reinterpret_cast<HWND>(win);
    hIconBig = (HICON)SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
    hIconSmall = (HICON)SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);

}

void KWindowSystem::setState(WId win, NET::States state)
{
    HWND hwnd = reinterpret_cast<HWND>(win);
    bool got = false;
#ifndef _WIN32_WCE
    if (state & NET::SkipTaskbar) {
        got = true;
        LONG_PTR lp = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, lp | WS_EX_TOOLWINDOW);
    }
#endif
    if (state & NET::KeepAbove) {
        got = true;
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    if (state & NET::KeepBelow) {
        got = true;
        SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    if (state & NET::Max) {
        got = true;
        ShowWindow(hwnd, SW_MAXIMIZE);
    }
    if (!got) {
        qDebug() << "KWindowSystem::setState( WId win, unsigned long state ) isn't yet implemented for the state you requested!";
    }
}

void KWindowSystem::clearState(WId win, NET::States state)
{
    bool got = false;
    HWND hwnd = reinterpret_cast<HWND>(win);

#ifndef _WIN32_WCE
    if (state & NET::SkipTaskbar) {
        got = true;
        LONG_PTR lp = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, lp & ~WS_EX_TOOLWINDOW);
    }
#endif
    if (state & NET::KeepAbove) {
        got = true;
        //lets hope this remove the topmost
        SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    if (state & NET::Max) {
        got = true;
        ShowWindow(hwnd, SW_RESTORE);
    }
    if (!got) {
        qDebug() << "KWindowSystem::clearState( WId win, unsigned long state ) isn't yet implemented!";
    }
}

void KWindowSystem::minimizeWindow(WId win, bool animation)
{
    Q_UNUSED(animation);
    ShowWindow(reinterpret_cast<HWND>(win), SW_MINIMIZE);
}

void KWindowSystem::unminimizeWindow(WId win, bool animation)
{
    Q_UNUSED(animation);
    ShowWindow(reinterpret_cast<HWND>(win), SW_RESTORE);
}

void KWindowSystem::raiseWindow(WId win)
{

    //to bring a window to the front while the user is active in a different apllication we
    //have to atach our self to the current active window
    HWND hwndActiveWin = GetForegroundWindow();
    int  idActive      = GetWindowThreadProcessId(hwndActiveWin, NULL);
    if (AttachThreadInput(GetCurrentThreadId(), idActive, TRUE)) {
        SetForegroundWindow(reinterpret_cast<HWND>(win));
        AttachThreadInput(GetCurrentThreadId(), idActive, FALSE);
    }
}

void KWindowSystem::lowerWindow(WId win)
{
    SetWindowPos(reinterpret_cast<HWND>(win), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);   // mhhh?
}

bool KWindowSystem::compositingActive()
{
    return true;
}

QRect KWindowSystem::workArea(int desktop)
{
    return qApp->desktop()->availableGeometry(desktop);
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

bool KWindowSystem::icccmCompliantMappingState()
{
    return false;
}

// optimalization - create KWindowSystemPrivate only when needed and only for what is needed
void KWindowSystem::connectNotify(const QMetaMethod &method)
{
    int what = INFO_BASIC;
    if (method == QMetaMethod::fromSignal(&KWindowSystem::workAreaChanged)) {
        what = INFO_WINDOWS;
    } else if (method == QMetaMethod::fromSignal(&KWindowSystem::strutChanged)) {
        what = INFO_WINDOWS;
    } else if (method == QMetaMethod::fromSignal(static_cast<void(KWindowSystem::*)(WId, const ulong *)>(&KWindowSystem::windowChanged))) {
        what = INFO_WINDOWS;
    } else if (method == QMetaMethod::fromSignal(static_cast<void(KWindowSystem::*)(WId, uint)>(&KWindowSystem::windowChanged))) {
        what = INFO_WINDOWS;
    } else if (method == QMetaMethod::fromSignal(static_cast<void(KWindowSystem::*)(WId)>(&KWindowSystem::windowChanged))) {
        what = INFO_WINDOWS;
    }

    init(what);
    QObject::connectNotify(method);
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

QString KWindowSystem::readNameProperty(WId window, unsigned long atom)
{
    //TODO
    qDebug() << "QString KWindowSystem::readNameProperty( WId window, unsigned long atom ) isn't yet implemented!";
    return QString();
}

QList<WId> KWindowSystem::stackingOrder()
{
    KWindowSystem::init(INFO_WINDOWS);
    return KWindowSystem::s_d_func()->stackingOrder;
}

const QList<WId> &KWindowSystem::windows()
{
    KWindowSystem::init(INFO_WINDOWS);
    return KWindowSystem::s_d_func()->stackingOrder;
}

void KWindowSystem::setType(WId win, NET::WindowType windowType)
{
//TODO
    qDebug() << "setType( WId win, NET::WindowType windowType ) isn't yet implemented!";
}

#ifndef KWINDOWSYSTEM_NO_DEPRECATED
KWindowInfo KWindowSystem::windowInfo(WId win, unsigned long properties, unsigned long properties2)
{
    KWindowSystem::init(INFO_WINDOWS);
    return KWindowInfo(win, properties, properties2);
}
#endif

bool KWindowSystem::hasWId(WId w)
{
    KWindowSystem::init(INFO_WINDOWS);
    return KWindowSystem::s_d_func()->winInfos.contains(w);
}

void KWindowSystem::allowExternalProcessWindowActivation(int pid)
{
#ifndef _WIN32_WCE
    AllowSetForegroundWindow(pid == -1 ? ASFW_ANY : pid);
#endif
}

void KWindowSystem::setBlockingCompositing(WId window, bool active)
{
    //TODO
    qDebug() << "setBlockingCompositing( WId window, bool active ) isn't yet implemented!";
}

#include "moc_kwindowsystem.cpp"
