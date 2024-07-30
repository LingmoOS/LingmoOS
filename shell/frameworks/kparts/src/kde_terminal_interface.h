/*
    interface.h
    SPDX-FileCopyrightText: 2002 Dominique Devriese <devriese@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KDELIBS_KDE_TERMINAL_INTERFACE_H
#define KDELIBS_KDE_TERMINAL_INTERFACE_H

#include <QObject>
#include <QStringList>

/**
 * TerminalInterface is an interface implemented by KonsolePart to
 * allow developers access to the KonsolePart in ways that are not
 * possible through the normal KPart interface.
 *
 * Note that besides the functions below here, KonsolePart also has
 * some signals you can connect to.  They aren't in this class cause
 * we can't have signals without having a QObject, which
 * TerminalInterface is not.
 *
 * These are some signals you can connect to:
 *  void currentDirectoryChanged(const QString& dir);
 *
 * See the example code below for how to connect to these.
 *
 * Use it like this:
 * \code
 *  // Query the .desktop file to get service information about konsolepart
 *  KService::Ptr service = KService::serviceByDesktopName("konsolepart");
 *
 *  if (!service) {
 *      QMessageBox::critical(this, tr("Konsole not installed"), tr("Please install the kde konsole and try again!"), QMessageBox::Ok);
 *      return ;
 *  }
 *
 *  // Create one instance of konsolepart
 *  KParts::ReadOnlyPart *part = service->createInstance<KParts::ReadOnlyPart>(parent, parentWidget, QVariantList());
 *
 *  if (!part) {
 *      return;
 *  }
 *
 *  // Cast the konsolepart to the TerminalInterface..
 *  TerminalInterface *terminalIface = qobject_cast<TerminalInterface *>(part);
 *
 *  if (!terminalIface) {
 *      return;
 *  }
 *
 *  // Now use the interface in all sorts of ways, e.g.
 *  //    terminalIface->showShellInDir(QDir::home().path());
 *  // Or:
 *  //    QStringList list;
 *  //    list.append("python");
 *  //    terminalIface->startProgram( QString::fromUtf8( "/usr/bin/python" ), list);
 *  // Or connect to one of the signals. Connect to the Part object,
 *  // not to the TerminalInterface, since the latter is not a QObject,
 *  // and as such does not have signals..:
 *  connect(part, &KParts::ReadOnlyPart::currentDirectoryChanged, this, [this](const QString &dirPath) {
 *      currentDirectoryChanged(dirPath);
 *  });
 *  // etc.
 *
 * \endcode
 *
 * @author Dominique Devriese <devriese@kde.org>
 */
class TerminalInterface
{
public:
    virtual ~TerminalInterface()
    {
    }
    /**
     * This starts @p program, with arguments @p args
     */
    virtual void startProgram(const QString &program, const QStringList &args) = 0;
    /**
     * If no shell is running, this starts a shell with the
     * @dir as the starting directory.
     * If a shell is already running, nothing is done.
     */
    virtual void showShellInDir(const QString &dir) = 0;

    /**
     * This sends @param text as input to the currently running
     * program..
     */
    virtual void sendInput(const QString &text) = 0;

    /**
     * Return terminal PID.  If no process is currently running, returns 0.
     */
    virtual int terminalProcessId() = 0;

    /**
     * Return foregound PID, If there is no foreground process running, returns -1
     */
    virtual int foregroundProcessId() = 0;

    /**
     * Returns sub process name. If there is no sub process running, returns empty QString
     */
    virtual QString foregroundProcessName() = 0;

    /**
     * Returns the current working directory
     */
    virtual QString currentWorkingDirectory() const = 0;

    /**
     * Returns the names of available profiles.
     */
    virtual QStringList availableProfiles() const = 0;

    /**
     * Returns the name of the currently active profile.
     */
    virtual QString currentProfileName() const = 0;

    /**
     * Changes the currently active profile to @p profileName.
     * @returns Returns true if setting the profile was successful
     */
    virtual bool setCurrentProfile(const QString &profileName) = 0;

    /**
     * Returns the property @p profileProperty of the currently active profile.
     */
    virtual QVariant profileProperty(const QString &profileProperty) const = 0;
};

Q_DECLARE_INTERFACE(TerminalInterface, "org.kde.TerminalInterface")

#endif
