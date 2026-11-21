// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dialogs/serverdialogs.h"
#include "model/places.h"
#include "views/style.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/strings.h"
#include "ui/check_box.h"
#include "ui/dialog_window.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/window.h"

namespace fm {

using ui::rounding::centeredStart;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trc;

// One recent server: a 32 px row, lit when the field holds it.
class RecentServer : public ui::View
{
public:
    RecentServer(std::string address, std::function<void(const std::string&, int)> pressed,
                 std::function<void(const std::string&)> removed)
        : m_address(std::move(address))
        , m_pressed(std::move(pressed))
        , m_removed(std::move(removed))
    {
        setTracksHover(true);
        m_icon = addChild<ui::ThemeIcon>("network-server", 16);
        m_icon->setThemeSize(16);
        m_label = addChild<ui::Label>(m_address);
        m_label->setElide(ui::Label::Elide::Middle);
        m_remove = addChild<ui::ThemeIcon>("edit-clear", 16);
        m_remove->setThemeSize(16);
    }

    void setCurrent(bool current)
    {
        m_current = current;
        requestPaint();
    }

    ui::SizeF sizeHint() const override { return {0, 32}; }
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        const ui::Theme& theme = ui::Theme::current();
        const double height = geometry().height;
        m_icon->setColor(theme.text);
        m_icon->setGeometry({8, centeredStart(height, 16), 16, 16});
        m_removeRect = {geometry().width - 4 - 24, centeredStart(height, 24), 24, 24};
        m_remove->setVisible(isHovered());
        m_remove->setColor(theme.text);
        m_remove->setOpacity(m_removeHovered ? 0.85 : 0.45);
        m_remove->setGeometry({m_removeRect.x + centeredStart(24, 14), m_removeRect.y + centeredStart(24, 14), 14, 14});
        const ui::SizeF hint = m_label->sizeHint();
        m_label->setGeometry({8 + 16 + 8, centeredStart(height, hint.height),
                              std::max(0.0, m_removeRect.x - 4 - 32), hint.height});
    }

    void paint(ui::Painter& painter) override
    {
        const ui::Color fill = m_current ? style::textAlpha(0.08) : isHovered() ? style::textAlpha(0.04)
                                                                                 : ui::Color {0, 0, 0, 0};
        if (fill.a)
            painter.drawRoundedRect(localRect(), {ui::Theme::current().smallRadius, 0, ui::Corners::Circular}, fill);
    }

    void hoverChanged() override
    {
        requestLayout();
        requestPaint();
    }

    void hoverMove(const ui::PointerEvent& event) override
    {
        const bool over = m_removeRect.contains(event.position);
        if (over != m_removeHovered) {
            m_removeHovered = over;
            requestLayout();
        }
    }

    void pointerPress(const ui::PointerEvent& event) override
    {
        if (isHovered() && m_removeRect.contains(event.position)) {
            m_removed(m_address);
            return;
        }
        m_pressed(m_address, event.clickCount);
    }

private:
    std::string m_address;
    std::function<void(const std::string&, int)> m_pressed;
    std::function<void(const std::string&)> m_removed;
    ui::ThemeIcon* m_icon;
    ui::Label* m_label;
    ui::ThemeIcon* m_remove;
    ui::RectF m_removeRect;
    bool m_current = false;
    bool m_removeHovered = false;
};

class ConnectServerDialog::Content : public ui::ColumnLayout
{
public:
    Content(ConnectServerDialog* dialog)
        : m_dialog(dialog)
    {
        setSpacing(12);
        // The dialog pads the sides and the bottom; the top takes 16 more.
        setPadding({0, 16, 0, 0});
        field = addChild<ui::TextField>();
        // TRANSLATORS: example addresses; keep the two addresses as they are.
        field->setPlaceholder(tr("smb://server/share or sftp://user@server"));
        field->setSelectByMouse(true);
        field->setInputPurpose(ui::TextPurpose::Url);
        field->setLayoutHints({.fillWidth = true});
        field->onAccepted = [this] { connectTo(field->text()); };
        field->onTextChanged = [this] { updateRows(); };

        recentTitle = addChild<ui::Label>(tr("Recent Servers"));
        recentTitle->setFontSize(12);
        recentTitle->setColor(ui::ColorId::DisabledText);
        ui::LayoutHints titleHints;
        titleHints.margins.top = 4;
        recentTitle->setLayoutHints(titleHints);
        recent = addChild<ui::Column>();
        recent->setLayoutHints({.fillWidth = true});

        auto* actions = addChild<ui::RowLayout>();
        actions->setSpacing(8);
        ui::LayoutHints actionHints {.horizontalAlignment = ui::Alignment::End};
        actionHints.margins.top = 4;
        actions->setLayoutHints(actionHints);
        auto* cancel = actions->addChild<ui::DialogButton>(tr("Cancel"));
        cancel->onClicked = [this] { m_dialog->m_dialog->close(); };
        connect = actions->addChild<ui::DialogButton>(tr("Connect"), true);
        connect->onClicked = [this] { connectTo(field->text()); };
        rebuild();
    }

    void rebuild()
    {
        // The rows go after the event, which may be one of theirs.
        std::vector<std::shared_ptr<View>> gone;
        while (!recent->children().empty())
            gone.push_back(recent->takeChild(recent->children().front().get()));
        base::EventLoop::current()->post([gone] {});
        const auto servers = m_dialog->m_places.recentServers();
        for (const std::string& server : servers) {
            recent->addChild<RecentServer>(
                    server,
                    [this](const std::string& address, int clicks) {
                        if (clicks == 2)
                            connectTo(address);
                        else
                            field->setText(address);
                    },
                    [this](const std::string& address) {
                        m_dialog->m_places.removeRecentServer(address);
                        rebuild();
                    });
        }
        recentTitle->setVisible(!servers.empty());
        recent->setVisible(!servers.empty());
        updateRows();
        requestLayout();
    }

    void updateRows()
    {
        for (const auto& child : recent->children())
            static_cast<RecentServer*>(child.get())->setCurrent(false);
        const auto servers = m_dialog->m_places.recentServers();
        for (size_t i = 0; i < servers.size() && i < recent->children().size(); ++i)
            static_cast<RecentServer*>(recent->children()[i].get())->setCurrent(servers[i] == field->text());
        connect->setEnabled(!base::strings::trimmed(field->text()).empty());
    }

    void connectTo(const std::string& address)
    {
        if (base::strings::trimmed(address).empty())
            return;
        m_dialog->m_dialog->close();
        m_dialog->m_places.connectToServer(address);
    }

    ui::TextField* field;
    ui::Label* recentTitle;
    ui::Column* recent;
    ui::DialogButton* connect;

private:
    ConnectServerDialog* m_dialog;
};

ConnectServerDialog::ConnectServerDialog(ui::Window* parent, PlacesModel& places)
    : m_places(places)
{
    m_dialog = std::make_unique<ui::DialogWindow>(parent, tr("Connect to Server"), 420);
    auto* content = m_dialog->setContent<Content>(this);
    m_dialog->onClosed = [this] {
        if (onDone)
            onDone();
    };
    m_dialog->window()->setBackgroundColor(ui::ColorId::SecondBackground);
    m_dialog->window()->addShortcut(ui::NoModifier, ui::key::Escape, [this] { m_dialog->close(); });
    m_field = content->field;
}

ConnectServerDialog::~ConnectServerDialog() = default;

void ConnectServerDialog::show()
{
    m_dialog->show();
    m_field->setFocus();
    m_field->selectAll();
}

void ConnectServerDialog::retranslate()
{
    // Closed: it goes once the event is over.
    if (!m_dialog->window())
        return;
    const std::string address = m_field->text();
    const bool focused = m_field->hasFocus();
    m_dialog->setTitle(tr("Connect to Server"));
    auto* content = m_dialog->setContent<Content>(this);
    m_field = content->field;
    m_field->setText(address);
    if (focused)
        m_field->setFocus();
}

class ServerPasswordDialog::Content : public ui::ColumnLayout
{
public:
    explicit Content(ServerPasswordDialog* dialog)
        : m_dialog(dialog)
    {
        const Request& request = dialog->m_request;
        setSpacing(12);
        // The dialog pads the sides and the bottom; the top takes 16 more.
        setPadding({0, 16, 0, 0});

        auto* message = addChild<ui::Label>(
                request.retry ? format(tr("The name or password for “{address}” is incorrect. Try again."),
                                       {{"address", request.address}})
                              : format(tr("Enter your name and password for “{address}”."),
                                       {{"address", request.address}}));
        message->setWrap(true);
        message->setColor(ui::ColorId::DisabledText);
        message->setLayoutHints({.fillWidth = true});

        auto* who = addChild<ui::RowLayout>();
        who->setSpacing(16);
        who->setVisible(request.anonymousAllowed);
        registered = who->addChild<ui::RadioButton>(tr("Registered user"));
        guest = who->addChild<ui::RadioButton>(tr("Guest"));
        registered->setGroup(&group);
        guest->setGroup(&group);
        registered->setChecked(true);
        guest->onToggled = [this](bool) { update(); };
        registered->onToggled = [this](bool) { update(); };

        fields = addChild<ui::ColumnLayout>();
        fields->setSpacing(8);
        fields->setLayoutHints({.fillWidth = true});
        user = fields->addChild<ui::TextField>(request.user);
        user->setPlaceholder(trc("user name", "Name"));
        user->setLayoutHints({.fillWidth = true});
        domain = fields->addChild<ui::TextField>(request.domain);
        domain->setPlaceholder(tr("Domain (optional)"));
        domain->setVisible(request.needsDomain);
        domain->setLayoutHints({.fillWidth = true});
        password = fields->addChild<ui::TextField>();
        password->setPlaceholder(tr("Password"));
        password->setEchoMode(ui::TextField::EchoMode::Password);
        password->setLayoutHints({.fillWidth = true});
        remember = fields->addChild<ui::CheckBox>(tr("Remember this password"));
        remember->setChecked(true);
        user->onAccepted = [this] { password->setFocus(); };
        domain->onAccepted = [this] { password->setFocus(); };
        password->onAccepted = [this] { accept(); };
        user->onTextChanged = [this] { update(); };
        password->onTextChanged = [this] { update(); };

        auto* actions = addChild<ui::RowLayout>();
        actions->setSpacing(8);
        ui::LayoutHints actionHints {.horizontalAlignment = ui::Alignment::End};
        actionHints.margins.top = 4;
        actions->setLayoutHints(actionHints);
        auto* cancel = actions->addChild<ui::DialogButton>(tr("Cancel"));
        cancel->onClicked = [this] { m_dialog->m_dialog->close(); };
        connect = actions->addChild<ui::DialogButton>(tr("Connect"), true);
        connect->onClicked = [this] { accept(); };
        update();
    }

    bool isGuest() const { return m_dialog->m_request.anonymousAllowed && guest->isChecked(); }
    bool complete() const { return isGuest() || (!user->text().empty() && !password->text().empty()); }

    void update()
    {
        fields->setEnabled(!isGuest());
        fields->setOpacity(isGuest() ? 0.5 : 1);
        connect->setEnabled(complete());
    }

    void accept()
    {
        if (!complete())
            return;
        m_dialog->m_answered = true;
        const bool guestLogin = isGuest();
        m_dialog->m_places.answerPassword(m_dialog->m_request.address, guestLogin ? std::string() : user->text(),
                                          guestLogin ? std::string() : domain->text(),
                                          guestLogin ? std::string() : password->text(), guestLogin,
                                          !guestLogin && remember->isChecked());
        m_dialog->m_dialog->close();
    }

    void focusFirst()
    {
        if (user->text().empty())
            user->setFocus();
        else
            password->setFocus();
    }

private:
    ServerPasswordDialog* m_dialog;
    ui::ButtonGroup group;

public:
    ui::RadioButton* registered;
    ui::RadioButton* guest;
    ui::ColumnLayout* fields;
    ui::TextField* user;
    ui::TextField* domain;
    ui::TextField* password;
    ui::CheckBox* remember;
    ui::DialogButton* connect;
};

ServerPasswordDialog::ServerPasswordDialog(ui::Window* parent, PlacesModel& places, Request request)
    : m_places(places)
    , m_request(std::move(request))
{
    m_dialog = std::make_unique<ui::DialogWindow>(parent, tr("Connect to Server"), 400);
    auto* content = m_dialog->setContent<Content>(this);
    m_content = content;
    m_dialog->onClosed = [this] {
        // Closed without an answer: the connection is given up.
        if (!m_answered)
            m_places.cancelConnection(m_request.address);
        if (onDone)
            onDone();
    };
    m_dialog->window()->setBackgroundColor(ui::ColorId::SecondBackground);
    m_dialog->window()->addShortcut(ui::NoModifier, ui::key::Escape, [this] { m_dialog->close(); });
    content->focusFirst();
}

ServerPasswordDialog::~ServerPasswordDialog() = default;

void ServerPasswordDialog::show()
{
    m_dialog->show();
}

void ServerPasswordDialog::retranslate()
{
    // Closed: it goes once the event is over.
    if (!m_dialog->window())
        return;
    const Content& old = *m_content;
    const bool guest = old.guest->isChecked();
    const bool remember = old.remember->isChecked();
    const std::string texts[] = {old.user->text(), old.domain->text(), old.password->text()};
    int focus = -1;
    const ui::TextField* oldFields[] = {old.user, old.domain, old.password};
    for (int i = 0; i < 3; ++i) {
        if (oldFields[i]->hasFocus())
            focus = i;
    }

    m_dialog->setTitle(tr("Connect to Server"));
    m_content = m_dialog->setContent<Content>(this);
    m_content->guest->setChecked(guest);
    m_content->registered->setChecked(!guest);
    m_content->remember->setChecked(remember);
    ui::TextField* fields[] = {m_content->user, m_content->domain, m_content->password};
    for (int i = 0; i < 3; ++i)
        fields[i]->setText(texts[i]);
    m_content->update();
    if (focus >= 0)
        fields[focus]->setFocus();
}

} // namespace fm
