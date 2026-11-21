// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TerminalSession.h"

#include <climits>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <pwd.h>
#include <unistd.h>

#include "base/file.h"
#include "base/paths.h"
#include "base/strings.h"

#include "History.h"
#include "ProxyEnvironment.h"
#include "PtyProcess.h"
#include "Screen.h"
#include "ScreenWindow.h"
#include "Vt102Emulation.h"
#include "tools.h"

namespace strings = Lingmo::base::strings;

namespace Konsole
{

static std::string currentPath()
{
    std::error_code error;
    const std::filesystem::path path = std::filesystem::current_path(error);
    return error ? std::string() : path.string();
}

// ShellCommand::expand(): "$PWD" alone is the real working directory; every
// other "$NAME" ended by '/', ' ' or the end and set in the environment is
// replaced by its value, as Konsole expands a command line.
static std::string expandVariables(const std::string& input)
{
    if (input == "$PWD")
        return currentPath();

    std::string text = input;
    size_t pos = 0;
    while ((pos = text.find('$', pos)) != std::string::npos) {
        if (pos > 0 && text[pos - 1] == '\\') {
            pos++;
            continue;
        }
        size_t end = text.find_first_of(" /", pos + 1);
        if (end == std::string::npos)
            end = text.size();
        const std::string key = text.substr(pos + 1, end - pos - 1);
        const std::string value = key == "HOME" ? Lingmo::base::paths::home()
                                  : key == "PWD" ? currentPath()
                                                 : std::string(key.empty() ? "" : (getenv(key.c_str()) ?: ""));
        if (!value.empty()) {
            text.replace(pos, end - pos, value);
            pos += value.size();
        } else {
            pos = end;
        }
    }
    return text;
}

static bool isDirectory(const std::string& path)
{
    std::error_code error;
    return std::filesystem::is_directory(path, error);
}

static bool exists(const std::string& path)
{
    std::error_code error;
    return std::filesystem::exists(path, error);
}

// The user's login shell, what a session's SHELL is set to.
static std::string userShell()
{
    if (const passwd* entry = getpwuid(getuid()); entry && entry->pw_shell && *entry->pw_shell)
        return entry->pw_shell;
    return std::string();
}

TerminalSession::TerminalSession()
    : _emulation(std::make_unique<Vt102Emulation>())
    , _shellProcess(std::make_unique<PtyProcess>())
{
    // KSession's defaults.
    const std::string shell = userShell();
    setShellProgram(shell.empty() ? "/bin/bash" : shell);
    setShellProgramArguments({std::string()});
    _emulation->setHistory(HistoryTypeBuffer(1000));
    _emulation->setKeyBindings(std::string());

    _connections.push_back(_emulation->titleChanged.connect([this](int what, const std::string& caption) {
        setUserTitle(what, caption);
    }));
    _connections.push_back(_emulation->stateSet.connect([this](int state) {
        if (state == NOTIFYBELL)
            bellRequest();
    }));
    _connections.push_back(_emulation->imageResizeRequest.connect([](const Lingmo::base::Size&) {
        // Session::setSize() asks the views to resize; no view takes the request.
    }));
    _connections.push_back(_emulation->imageSizeChanged.connect([this](int, int) { updateTerminalSize(); }));

    _shellProcess->setUtf8Mode(true);
    _connections.push_back(_shellProcess->receivedData.connect([this](const char* data, int length) {
        _emulation->receiveData(data, length);
        checkForeground();
    }));
    _connections.push_back(_emulation->sendData.connect([this](const char* data, int length) {
        _shellProcess->sendData(data, length);
    }));
    _connections.push_back(_emulation->useUtf8Request.connect([this](bool enable) {
        _shellProcess->setUtf8Mode(enable);
    }));
    _connections.push_back(_shellProcess->finished.connect([this](int exitCode, bool crashed) {
        done(exitCode, crashed);
    }));

    // Each update notes the process group in front, the one title() went by.
    _connections.push_back(_emulation->titleChanged.connect([this](int what, const std::string& title) {
        if (what != 0 && what != 2)
            return;
        _programTitle = title;
        _programTitleGroup = foregroundProcessId();
        updateTitle();
    }));

    // Nothing announces a program starting or ending, so the foreground is
    // looked at after output or a key, at most every 250 ms.
    _foregroundCheck.setSingleShot(true);
    _foregroundCheck.setInterval(std::chrono::milliseconds(250));
    _foregroundCheck.onTimeout = [this] {
        if (foregroundProcessId() != _foregroundGroup)
            updateTitle();
    };
    _connections.push_back(_emulation->outputFromKeypressEvent.connect([this] { checkForeground(); }));

    _finishedLater.setSingleShot(true);
    _finishedLater.onTimeout = [this] { finished(); };
}

TerminalSession::~TerminalSession()
{
    _destroying = true;
    close();
    _connections.clear();
}

void TerminalSession::updateTitle()
{
    _foregroundGroup = foregroundProcessId();
    titleChanged();
}

void TerminalSession::checkForeground()
{
    if (!_foregroundCheck.isActive())
        _foregroundCheck.start();
}

void TerminalSession::setInitialWorkingDirectory(const std::string& directory)
{
    if (_initialWorkingDirectory == directory)
        return;
    _initialWorkingDirectory = directory;

    // Session::validDirectory().
    std::string valid = expandVariables(directory);
    if (valid.empty())
        valid = currentPath();
    if (!isDirectory(valid))
        valid = Lingmo::base::paths::home();
    _workingDirectory = valid;
}

void TerminalSession::setShellProgram(const std::string& program)
{
    _shellProgram = program;
    _program = expandVariables(program);
}

void TerminalSession::setShellProgramArguments(const std::vector<std::string>& arguments)
{
    _shellArguments = arguments;
    _arguments.clear();
    for (const std::string& argument : arguments)
        _arguments.push_back(expandVariables(argument));
}

void TerminalSession::setHistorySize(int lines)
{
    if (historySize() == lines)
        return;
    if (lines < 0)
        _emulation->setHistory(HistoryTypeFile());
    else
        _emulation->setHistory(HistoryTypeBuffer(lines));
}

int TerminalSession::historySize() const
{
    const HistoryType& type = _emulation->history();
    if (type.isUnlimited())
        return -1;
    return type.maximumLineCount();
}

bool TerminalSession::isRunning() const
{
    return _shellProcess->isRunning();
}

int TerminalSession::processId() const
{
    return int(_shellProcess->processId());
}

int TerminalSession::foregroundProcessId() const
{
    return _shellProcess->foregroundProcessGroup();
}

void TerminalSession::startShellProgram()
{
    if (isRunning())
        return;

    std::string exec = _program;

    // A full path that does not exist, or none at all, falls back to the
    // user's shell, then /bin/sh; a bare name is looked up on PATH.
    if (exec.starts_with('/') || exec.empty()) {
        const std::string defaultShell = "/bin/sh";
        if (exec.empty() || !exists(exec))
            exec = userShell();
        if (exec.empty() || !exists(exec)) {
            LM_WARNING(qtermwidgetLogger) << "Neither default shell nor the user's shell is a correct path. Fallback to"
                                          << defaultShell;
            exec = defaultShell;
        }
    }

    // The arguments are sometimes just "", which counts as none.
    std::vector<std::string> arguments{exec};
    if (!std::string(strings::trimmed(strings::join(_arguments, " "))).empty())
        arguments.insert(arguments.end(), _arguments.begin(), _arguments.end());

    _shellProcess->setWorkingDirectory(_workingDirectory.empty() ? currentPath() : _workingDirectory);
    _shellProcess->setFlowControlEnabled(true);
    _shellProcess->setErase(_emulation->eraseChar());

    // Session::run() and Pty::start(): the background hint, the terminal type,
    // the session proxy, and the rest as Konsole sets them.
    std::vector<std::string> environment{"COLORFGBG=15;0", "TERM=xterm-256color"};
    std::vector<std::string> unset;
    const std::map<std::string, std::string> proxy = ProxyEnvironment::variables();
    for (const std::string& name : ProxyEnvironment::names()) {
        if (auto found = proxy.find(name); found != proxy.end())
            environment.push_back(name + "=" + found->second);
        else
            unset.push_back(name);
    }
    environment.push_back("WINDOWID=0");
    environment.push_back("COLORTERM=truecolor");

    if (!_shellProcess->start(exec, arguments, environment, unset, {"LANGUAGE="})) {
        LM_DEBUG(qtermwidgetLogger) << "CRASHED! result: -1";
        return;
    }

    started();
}

void TerminalSession::setUserTitle(int what, const std::string& caption)
{
    bool modified = false;

    // what=0 changes the user title and the icon, what=1 only the icon, what=2 only the title
    if ((what == 0) || (what == 2)) {
        _isTitleChanged = true;
        if (_userTitle != caption) {
            _userTitle = caption;
            modified = true;
        }
    }

    if ((what == 0) || (what == 1)) {
        _isTitleChanged = true;
        if (_iconText != caption) {
            _iconText = caption;
            modified = true;
        }
    }

    if (what == 30) {
        _isTitleChanged = true;
        if (_nameTitle != caption) {
            _nameTitle = caption;
            updateTitle();
            return;
        }
    }

    // change icon via \033]32;Icon\007
    if (what == 32) {
        _isTitleChanged = true;
        if (_iconName != caption) {
            _iconName = caption;
            modified = true;
        }
    }

    if (what == 50)
        return;

    if (modified)
        updateTitle();
}

// The cwd link, which only a process's own user can read.
static std::optional<std::string> workingDirectory(int pid)
{
    char path[PATH_MAX + 1];
    const ssize_t length = readlink(("/proc/" + std::to_string(pid) + "/cwd").c_str(), path, PATH_MAX);
    if (length < 0)
        return std::nullopt;
    path[length] = '\0';
    return strings::sanitizedUtf8(path);
}

void TerminalSession::updateForegroundProcessInfo()
{
    const int foregroundPid = _shellProcess->foregroundProcessGroup();
    if (foregroundPid != _foregroundPid) {
        _foregroundPid = foregroundPid;
        _foregroundValid = false;
        _foregroundName.clear();
        _foregroundDir.clear();
        _foregroundHasDir = false;
    }
    if (foregroundPid <= 0)
        return;

    // LinuxProcessInfo: the name from /proc/<pid>/stat, the directory from
    // its cwd link. What was read once stays until the process changes.
    const std::string base = "/proc/" + std::to_string(foregroundPid);
    if (!Lingmo::base::file::read(base + "/status"))
        return;
    const std::optional<std::string> stat = Lingmo::base::file::read(base + "/stat");
    if (!stat)
        return;

    int stack = 0;
    int field = 0;
    std::string name;
    std::string parent;
    for (const char c : *stat) {
        if (c == '(') {
            stack++;
        } else if (c == ')') {
            stack--;
        } else if (stack == 0 && c == ' ') {
            field++;
        } else if (field == 1) {
            name += c;
        } else if (field == 3) {
            parent += c;
        }
    }
    if (!name.empty())
        _foregroundName = strings::sanitizedUtf8(name);
    if (!strings::toNumber<int>(parent))
        return;
    _foregroundValid = true;

    if (std::optional<std::string> dir = workingDirectory(foregroundPid)) {
        _foregroundDir = *dir;
        _foregroundHasDir = true;
    }
}

std::string TerminalSession::foregroundProcessName()
{
    updateForegroundProcessInfo();
    return _foregroundValid ? _foregroundName : std::string();
}

std::string TerminalSession::currentDir()
{
    updateForegroundProcessInfo();
    if (_foregroundValid && _foregroundHasDir)
        return _foregroundDir;
    // A program in front that runs as another user, such as su, hides its
    // directory; the shell's stands in for it.
    if (processId() > 0 && processId() != _foregroundPid)
        return workingDirectory(processId()).value_or(std::string());
    return std::string();
}

bool TerminalSession::hasActiveProcess() const
{
    return processId() != foregroundProcessId();
}

std::string TerminalSession::title()
{
    // A program in front of the shell gives the title it set, or else its name.
    const int foreground = foregroundProcessId();
    if (foreground > 0 && foreground != processId()) {
        if (foreground == _programTitleGroup && !_programTitle.empty())
            return _programTitle;
        const std::string name = foregroundProcessName();
        if (!name.empty())
            return name;
    }

    std::string dir = currentDir();
    // At the prompt tabs are titled by the working directory; home and root keep their full path.
    std::string home = Lingmo::base::paths::home();
    while (home.size() > 1 && home.back() == '/')
        home.pop_back();
    if (dir == home || dir == "/")
        return dir;

    // QDir::dirName().
    while (dir.size() > 1 && dir.back() == '/')
        dir.pop_back();
    const size_t slash = dir.rfind('/');
    return slash == std::string::npos ? dir : dir.substr(slash + 1);
}

void TerminalSession::sendText(const std::string& text)
{
    _emulation->sendText(text);
}

void TerminalSession::sendKeyEvent(KeyEvent* event)
{
    _emulation->sendKeyEvent(event, false);
}

void TerminalSession::clearScrollback()
{
    _emulation->clearHistory();
}

bool TerminalSession::clearScreenAndScrollback()
{
    return _emulation->clearScreenAndHistory(!hasActiveProcess());
}

void TerminalSession::resetTerminal()
{
    // Resetting clears the screen into the history, so the history goes after it.
    _emulation->reset();
    _emulation->clearHistory();
    // A size change makes the shell, or a full-screen program, draw itself again;
    // Ctrl+L would clear the screen into the history once more.
    refresh();
}

void TerminalSession::refresh()
{
    const Lingmo::base::Size existingSize = _shellProcess->windowSize();
    _shellProcess->setWindowSize(existingSize.height(), existingSize.width() + 1);
    _shellProcess->setWindowSize(existingSize.height(), existingSize.width());
}

ScreenWindow* TerminalSession::createWindow()
{
    return _emulation->createWindow();
}

void TerminalSession::setViewSize(const void* view, int lines, int columns)
{
    bool found = false;
    for (ViewSize& size : _views) {
        if (size.view == view) {
            size.lines = lines;
            size.columns = columns;
            found = true;
        }
    }
    if (!found)
        _views.push_back({view, lines, columns});
    updateTerminalSize();
}

void TerminalSession::removeView(const void* view)
{
    std::erase_if(_views, [view](const ViewSize& size) { return size.view == view; });
    // A session closes with its last view.
    if (_views.empty())
        close();
}

void TerminalSession::updateTerminalSize()
{
    int minLines = -1;
    int minColumns = -1;

    // Views smaller than this have not been given their size yet.
    const int VIEW_LINES_THRESHOLD = 2;
    const int VIEW_COLUMNS_THRESHOLD = 2;

    for (const ViewSize& view : _views) {
        if (view.lines >= VIEW_LINES_THRESHOLD && view.columns >= VIEW_COLUMNS_THRESHOLD) {
            minLines = (minLines == -1) ? view.lines : std::min(minLines, view.lines);
            minColumns = (minColumns == -1) ? view.columns : std::min(minColumns, view.columns);
        }
    }

    // backend emulation must have a terminal of at least 1 column x 1 line in size
    if (minLines > 0 && minColumns > 0) {
        _emulation->setImageSize(minLines, minColumns);
        _shellProcess->setWindowSize(minLines, minColumns);
    }
}

std::optional<HistorySearch::Result> TerminalSession::search(const std::u16string& pattern, bool caseSensitive,
                                                             int startLine, int startColumn, bool forwards)
{
    HistorySearch history(_emulation.get(), RegExp(pattern, caseSensitive ? RegExp::NoOption : RegExp::CaseInsensitive),
                          forwards, startColumn, startLine);
    return history.search();
}

bool TerminalSession::sendSignal(int signal)
{
    if (processId() <= 0)
        return false;

    if (::kill(static_cast<pid_t>(_shellProcess->processId()), signal) == 0)
        return _shellProcess->waitForFinished(1000);
    return false;
}

void TerminalSession::close()
{
    _wantedClose = true;
    if (isRunning()) {
        // Try SIGHUP, and if unsuccessful, do a hard kill.
        // This is the sequence used by most other terminal emulators like xterm, gnome-terminal, ...
        if (sendSignal(SIGHUP))
            return;

        LM_WARNING(qtermwidgetLogger) << "Process " << processId() << " did not die with SIGHUP";
        _shellProcess->closePty();
        if (!_shellProcess->waitForFinished(1000)) {
            if (!sendSignal(SIGKILL)) {
                LM_WARNING(qtermwidgetLogger) << "Process " << processId() << " did not die with SIGKILL";
                if (!_destroying)
                    _finishedLater.start(std::chrono::milliseconds(1));
            }
        }
    } else if (!_destroying) {
        // terminal process has finished, just close the session
        _finishedLater.start(std::chrono::milliseconds(1));
    }
}

void TerminalSession::done(int, bool crashed)
{
    if (_destroying)
        return;

    // A shell that crashed by itself leaves its tab open, as Konsole's Session does.
    if (!_wantedClose && crashed)
        return;

    finished();
}

}
