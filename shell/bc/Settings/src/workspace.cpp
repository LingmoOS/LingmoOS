#include "workspace.h"
#include <QProcess>
#include <QSettings>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

Workspace::Workspace(QObject *parent)
    : QObject(parent)
{
}

void Workspace::openKwinscreenedges()
{
    QProcess::startDetached("kcmshell5", QStringList() << "kcm_kwinscreenedges");
}

void Workspace::openDisplayeffect()
{
    QProcess::startDetached("kcmshell5", QStringList() << "kwincompositing");
}

void Workspace::openTaskSwitcher()
{
    QProcess::startDetached("kcmshell5", QStringList() << "kcm_kwintabbox");
}

void Workspace::openFontmanager()
{
    QProcess::startDetached("kcmshell5", QStringList() << "kcm_fontinst");
}

void Workspace::openFileAssociations()
{
    QProcess::startDetached("kcmshell5", QStringList() << "kcm_filetypes");
}

void Workspace::openFilesearch()
{
    QProcess::startDetached("kcmshell5", QStringList() << "kcm_baloofile");
}

void Workspace::openWebsearch()
{
    QProcess::startDetached("kcmshell5", QStringList() << "webshortcuts");
}

void Workspace::openKeyboard()
{
    QProcess::startDetached("kcmshell5", QStringList() << "kcm_keyboard");
}