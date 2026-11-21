// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TERMINALSESSION_H
#define TERMINALSESSION_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/timer.h"

#include "HistorySearch.h"
#include "KeyCodes.h"

namespace Konsole
{

class PtyProcess;
class ScreenWindow;
class Vt102Emulation;

// A shell or program on a pty with its emulation: what Konsole's Session and
// the QML plugin's KSession did together for one tab.
class TerminalSession
{
public:
    TerminalSession();
    ~TerminalSession();

    TerminalSession(const TerminalSession&) = delete;
    TerminalSession& operator=(const TerminalSession&) = delete;

    Vt102Emulation* emulation() const { return _emulation.get(); }

    // "$PWD" and "$HOME" and other variables are expanded; a directory that
    // does not exist falls back to the home directory.
    void setInitialWorkingDirectory(const std::string& directory);
    const std::string& initialWorkingDirectory() const { return _initialWorkingDirectory; }
    // The user's shell unless set.
    void setShellProgram(const std::string& program);
    const std::string& shellProgram() const { return _shellProgram; }
    void setShellProgramArguments(const std::vector<std::string>& arguments);
    const std::vector<std::string>& shellProgramArguments() const { return _shellArguments; }

    // Lines of scrollback; -1 keeps everything.
    void setHistorySize(int lines);
    int historySize() const;

    void startShellProgram();
    bool isRunning() const;
    int processId() const;
    int foregroundProcessId() const;

    // The program in front, or the working directory's name at the prompt.
    std::string title();
    bool hasActiveProcess() const;
    std::string foregroundProcessName();
    std::string currentDir();

    void sendText(const std::string& text);
    void sendKeyEvent(KeyEvent* event);

    void clearScrollback();
    // False on the alternate screen, which keeps the key for its program.
    bool clearScreenAndScrollback();
    void resetTerminal();

    // A view's size; the smallest of every view's is the terminal's.
    ScreenWindow* createWindow();
    void setViewSize(const void* view, int lines, int columns);
    void removeView(const void* view);

    std::optional<HistorySearch::Result> search(const std::u16string& pattern, bool caseSensitive, int startLine,
                                                int startColumn, bool forwards);

    // Asks the program to end (SIGHUP, then harder); finished follows.
    void close();

    Lingmo::base::Signal<> started;
    Lingmo::base::Signal<> finished;
    Lingmo::base::Signal<> titleChanged;
    Lingmo::base::Signal<> bellRequest;

private:
    void done(int exitCode, bool crashed);
    void setUserTitle(int what, const std::string& caption);
    bool sendSignal(int signal);
    void updateTerminalSize();
    void refresh();
    void updateForegroundProcessInfo();
    void checkForeground();
    void updateTitle();

    struct ViewSize
    {
        const void* view;
        int lines;
        int columns;
    };

    std::unique_ptr<Vt102Emulation> _emulation;
    std::unique_ptr<PtyProcess> _shellProcess;
    std::vector<Lingmo::base::Subscription> _connections;
    std::vector<ViewSize> _views;

    std::string _initialWorkingDirectory;
    std::string _workingDirectory;
    std::string _program;
    std::vector<std::string> _arguments;
    std::string _shellProgram;
    std::vector<std::string> _shellArguments;

    std::string _userTitle;
    std::string _iconText;
    std::string _iconName;
    std::string _nameTitle;
    bool _isTitleChanged = false;
    bool _wantedClose = false;
    bool _destroying = false;

    // The foreground process, as Konsole's ProcessInfo read it from procfs.
    int _foregroundPid = 0;
    bool _foregroundValid = false;
    std::string _foregroundName;
    std::string _foregroundDir;
    bool _foregroundHasDir = false;

    // The last OSC 0 or 2 title and the process group in front when it came;
    // it titles the tab while that group stays in front.
    std::string _programTitle;
    int _programTitleGroup = 0;
    int _foregroundGroup = 0;
    Lingmo::base::Timer _foregroundCheck;
    Lingmo::base::Timer _finishedLater;
};

}

#endif // TERMINALSESSION_H
