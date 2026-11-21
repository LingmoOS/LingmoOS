// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DIALOG_H
#define DIALOG_H

#include <functional>
#include <memory>
#include <string>

namespace Lingmo::ui {
class DialogButton;
class Label;
class TextField;
class Window;
}

// The password dialog of one authentication request.
class Dialog
{
public:
    Dialog(const std::string &message, const std::string &identity);
    ~Dialog();

    Dialog(const Dialog &) = delete;
    Dialog &operator=(const Dialog &) = delete;

    void show();
    void hide();

    // PAM rejected the answer: the field empties for another try.
    void authenticationFailure();

    // PAM asks; an empty prompt shows the dialog's own placeholder.
    void setRequest(const std::string &prompt, bool echo);
    void setNotice(const std::string &text, bool error);

    const std::string &message() const { return m_message; }
    const std::string &identity() const { return m_identity; }
    const std::string &password() const { return m_password; }
    const std::string &prompt() const { return m_prompt; }
    bool echo() const { return m_echo; }
    const std::string &notice() const { return m_notice; }
    bool noticeIsError() const { return m_noticeIsError; }

    Lingmo::UI::Window *window() const { return m_window.get(); }

    // Authenticate or Return with a password typed.
    std::function<void()> onAccepted;
    // Cancel, Escape or the compositor closed it.
    std::function<void()> onCancel;

private:
    class Content;

    // An empty response would still count as an attempt, and pass on an
    // account without a password.
    void submit();
    void ready();
    void updateNotice();

    std::string m_message;
    std::string m_identity;
    std::string m_password;
    std::string m_prompt;
    bool m_echo = false;
    std::string m_notice;
    bool m_noticeIsError = false;

    std::unique_ptr<Lingmo::UI::Window> m_window;
    Lingmo::UI::TextField *m_passwordInput = nullptr;
    Lingmo::UI::Label *m_noticeLabel = nullptr;
    Lingmo::UI::DialogButton *m_doneButton = nullptr;
};

#endif // DIALOG_H
