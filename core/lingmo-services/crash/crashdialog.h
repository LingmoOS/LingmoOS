// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "coredump.h"

#include <functional>
#include <memory>
#include <string>

#include "base/dbus.h"
#include "base/signal.h"

namespace Lingmo::ui {
class DialogButton;
class Label;
class TextArea;
class View;
class Window;
}

// One open crash window. An application dialog offers to reopen it; the
// session restarts its own processes by itself. `name` is asked again when
// the session language changes.
class CrashDialog
{
public:
    enum class Kind { Application, SessionProcess };

    CrashDialog(Kind kind, const std::string &id, std::function<std::string()> name, const std::string &icon,
                const std::string &fixedFont);
    ~CrashDialog();

    CrashDialog(const CrashDialog &) = delete;
    CrashDialog &operator=(const CrashDialog &) = delete;

    bool sessionProcess() const { return m_kind == Kind::SessionProcess; }
    const std::string &name() const { return m_name; }
    std::string reason() const;
    std::string report() const;

    void setDump(const Coredump &dump);
    void show();

    void reopen();
    void close();

    // Reopen, Close, OK, Escape or the compositor closed it.
    std::function<void()> onFinished;

    Lingmo::UI::Window *window() const { return m_window.get(); }

private:
    class Content;

    void buildContent();
    void retranslate();
    void update();
    void setDetailsShown(bool shown);
    void accept();

    Kind m_kind;
    std::string m_id;
    std::function<std::string()> m_nameLookup;
    std::string m_name;
    std::string m_icon;
    std::string m_fixedFont;
    Coredump m_dump;
    bool m_hasDump = false;
    bool m_finished = false;
    bool m_detailsShown = false;
    bool m_reopening = false;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
    std::unique_ptr<Lingmo::UI::Window> m_window;
    Lingmo::UI::Label *m_text = nullptr;
    Lingmo::UI::View *m_reportFrame = nullptr;
    Lingmo::UI::TextArea *m_report = nullptr;
    Lingmo::UI::DialogButton *m_detailsButton = nullptr;
    Lingmo::Base::Subscription m_languageChanged;
};
