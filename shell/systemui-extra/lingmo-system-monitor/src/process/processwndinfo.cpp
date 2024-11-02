#include "processwndinfo.h"

#include <QtX11Extras/QX11Info>
#include "lingmosdk/applications/windowmanager/windowmanager.h"
using namespace kdk;

ProcessWndInfo::ProcessWndInfo(QObject* parent)
    : QObject(parent)
{
    kdk::WindowManager::self();
}

ProcessWndInfo::~ProcessWndInfo()
{

}

void ProcessWndInfo::updateWindowInfos()
{
    m_mapProcWndInfos.clear();
    for(kdk::WindowId w : kdk::WindowManager::windows()) {
        ProcWindowInfo wndInfo;
        if (w.isValid()) {
            wndInfo.procId = kdk::WindowManager::getPid(w);
            wndInfo.wndName = kdk::WindowManager::getWindowTitle(w);
            wndInfo.wndIcon = kdk::WindowManager::getWindowIcon(w);
            m_mapProcWndInfos[wndInfo.procId] = wndInfo;
        }
    }
}

bool ProcessWndInfo::acceptWindow(WId window)
{
    QFlags<NET::WindowTypeMask> ignoreList;
    ignoreList |= NET::DesktopMask;
    ignoreList |= NET::DockMask;
    ignoreList |= NET::SplashMask;
    ignoreList |= NET::ToolbarMask;
    ignoreList |= NET::MenuMask;
    ignoreList |= NET::PopupMenuMask;
    ignoreList |= NET::NotificationMask;

    KWindowInfo info(window, NET::WMWindowType | NET::WMState, NET::WM2TransientFor);
    if (!info.valid())
        return false;

    if (NET::typeMatchesMask(info.windowType(NET::AllTypesMask), ignoreList))
        return false;

    if (info.state() & NET::SkipTaskbar)
        return false;

    // WM_TRANSIENT_FOR hint not set - normal window
    WId transFor = info.transientFor();
    if (transFor == 0 || transFor == window || transFor == (WId) QX11Info::appRootWindow())
        return true;

    info = KWindowInfo(transFor, NET::WMWindowType);

    QFlags<NET::WindowTypeMask> normalFlag;
    normalFlag |= NET::NormalMask;
    normalFlag |= NET::DialogMask;
    normalFlag |= NET::UtilityMask;

    return !NET::typeMatchesMask(info.windowType(NET::AllTypesMask), normalFlag);
}

QList<pid_t> ProcessWndInfo::getWindowPids()
{
    return m_mapProcWndInfos.keys();
}

WId ProcessWndInfo::getWndIdByPid(pid_t pid)
{
    if (m_mapProcWndInfos.contains(pid)) {
        return m_mapProcWndInfos[pid].wndId;
    }
    return 0;
}

QString ProcessWndInfo::getWndNameByPid(pid_t pid)
{
    if (m_mapProcWndInfos.contains(pid)) {
        return m_mapProcWndInfos[pid].wndName;
    }
    return "";
}

QIcon ProcessWndInfo::getWndIconByPid(pid_t pid)
{
    if (m_mapProcWndInfos.contains(pid)) {
        return m_mapProcWndInfos[pid].wndIcon;
    }
    return QIcon();
}
