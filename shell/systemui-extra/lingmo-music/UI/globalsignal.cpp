#include "globalsignal.h"
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnection>

GlobalUserSignal* GlobalUserSignal::instance = new GlobalUserSignal;

GlobalUserSignal *GlobalUserSignal::getInstance()
{
    return instance;
}

void GlobalUserSignal::exitApp()
{
    emit sigExit();
}

void GlobalUserSignal::deleteAllHistoryPlaylistMessageBox()
{
    emit sigDeleteAllHistoryPlaylistMessageBox();
}

void GlobalUserSignal::hideSearchResult()
{
    emit sigHideSearchResult();
}

void GlobalUserSignal::repaintLeftSidebarWidget()
{
    emit sigRepaintLeftSidebarWidget();
}

GlobalUserSignal::GlobalUserSignal(QObject *parent) : QObject(parent)
{
}
