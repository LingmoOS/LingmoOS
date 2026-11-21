#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "ui/view.h"

namespace Lingmo::ui {
class DialogButton;
class Label;
class Window;
}

// One open prompt window. The content for the kind supplies the wording and
// the choice details.
class PermissionPrompt
{
public:
    // What a kind shows between the header and the buttons, and its wording.
    class Content : public Lingmo::UI::View
    {
    public:
        virtual std::string title() const = 0;
        virtual std::string description() const = 0;
        virtual std::string accept() const = 0;
        virtual bool ready() const = 0;
        virtual Lingmo::Base::DBus::VariantMap details() const = 0;
        // Sets its own text again, in the session's new language.
        virtual void retranslate() = 0;

        // A double click on a choice.
        std::function<void()> onAccepted;
    };

    // Known kinds have a content; an unknown kind is refused.
    static bool supports(const std::string &kind);

    PermissionPrompt(const std::string &kind, const std::string &appId, const std::string &parentWindow,
                     const Lingmo::Base::DBus::VariantMap &options, Lingmo::Base::DBus::MethodCall call);
    // Replies "failed" when nothing answered yet.
    ~PermissionPrompt();

    PermissionPrompt(const PermissionPrompt &) = delete;
    PermissionPrompt &operator=(const PermissionPrompt &) = delete;

    std::string caller() const { return m_call.message().sender(); }
    // Dismisses the prompt once the caller leaves the bus.
    void watchCaller(Lingmo::Base::DBus::Connection *bus);
    void show();
    void dismiss();

    void allow();
    void deny();

    Lingmo::UI::Window *window() const { return m_window.get(); }
    Content *content() const { return m_content; }

    // Answered or dismissed: the prompt can go.
    std::function<void()> onFinished;

    // The name and icon of a desktop entry; the id itself and no icon for an
    // application without one.
    struct Application {
        std::string id;
        std::string name;
        std::string icon;
    };
    static Application describeApplication(const std::string &appId);

private:
    void retranslate();
    void reply(uint32_t response, const Lingmo::Base::DBus::VariantMap &results);

    Lingmo::Base::DBus::MethodCall m_call;
    std::string m_kindIcon;
    std::string m_parentWindow;
    Application m_application;
    bool m_finished = false;
    Lingmo::Base::DBus::NameWatch m_callerWatch;
    std::unique_ptr<Lingmo::UI::Window> m_window;
    std::unique_ptr<Content> m_ownedContent;
    Content *m_content = nullptr;
    Lingmo::UI::Label *m_title = nullptr;
    Lingmo::UI::Label *m_description = nullptr;
    Lingmo::UI::DialogButton *m_cancel = nullptr;
    Lingmo::UI::DialogButton *m_accept = nullptr;
    Lingmo::Base::Subscription m_languageChanged;
};
