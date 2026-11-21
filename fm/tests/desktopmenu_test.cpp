// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testutil.h"

#include "desktop/desktopmenuexporter.h"

#include "base/dbus.h"
#include "ui/application.h"

#include <gtest/gtest.h>

using namespace Lingmo;
using base::dbus::Variant;
using base::dbus::VariantMap;
using fm::DesktopMenuExporter;
using fmtest::runUntil;

extern ui::Application* s_app;

namespace {

// com.canonical.AppMenu.Registrar on a connection of its own, as the compositor serves it.
class Registrar
{
public:
    explicit Registrar(base::dbus::Connection* bus)
        : m_object(bus, "/com/canonical/AppMenu/Registrar", "com.canonical.AppMenu.Registrar")
    {
        m_object.addMethod("RegisterWindow", "uo", "", [this](base::dbus::MethodCall& call) {
            uint32_t window = 1;
            base::dbus::ObjectPath path;
            call.read(window, path);
            registrations.emplace_back(window, path.path);
            call.reply();
        });
        m_object.addMethod("UnregisterWindow", "u", "", [this](base::dbus::MethodCall& call) {
            ++unregistered;
            call.reply();
        });
        m_object.publish();
        bus->requestName("com.canonical.AppMenu.Registrar");
    }

    std::vector<std::pair<uint32_t, std::string>> registrations;
    int unregistered = 0;

private:
    base::dbus::Object m_object;
};

// A call answered by a service on this thread's loop: the loop runs while it waits.
template<typename... Args>
base::dbus::Reply callOnLoop(base::EventLoop& loop, const base::dbus::Proxy& proxy, const std::string& method,
                             const Args&... args)
{
    std::optional<base::dbus::Reply> result;
    const base::dbus::PendingCall pending
            = proxy.callAsync(method, [&](const base::dbus::Reply& reply) { result = reply; }, args...);
    runUntil(loop, [&] { return result.has_value(); }, 5000);
    return result.value_or(base::dbus::Reply());
}

} // namespace

TEST(DesktopMenuTest, ExportsRegistersAndTriggers)
{
    base::EventLoop& loop = s_app->loop();
    base::dbus::Connection* bus = base::dbus::Connection::session(&loop);
    ASSERT_NE(bus, nullptr);
    auto registrarBus = base::dbus::Connection::open(base::dbus::Connection::Bus::Session, &loop);
    ASSERT_TRUE(registrarBus);
    Registrar registrar(registrarBus.get());

    bool pasteEnabled = false;
    int refreshed = 0;
    int pasted = 0;
    auto exporter = std::make_unique<DesktopMenuExporter>(
            bus,
            [&] {
                DesktopMenuExporter::Entry paste;
                paste.label = "Paste";
                paste.enabled = pasteEnabled;
                paste.action = [&] { ++pasted; };
                DesktopMenuExporter::Entry separator;
                separator.separator = true;
                DesktopMenuExporter::Entry selectAll;
                selectAll.label = "Select_All";
                DesktopMenuExporter::Entry edit;
                edit.label = "Edit";
                edit.children = {paste, separator, selectAll};
                return std::vector<DesktopMenuExporter::Entry> {edit};
            },
            [&] {
                ++refreshed;
                pasteEnabled = true;
            });

    ASSERT_TRUE(runUntil(loop, [&] { return !registrar.registrations.empty(); }));
    EXPECT_EQ(registrar.registrations.front(), std::make_pair(uint32_t(0), std::string(DesktopMenuExporter::kPath)));

    base::dbus::Proxy menu(registrarBus.get(), bus->uniqueName(), DesktopMenuExporter::kPath, "com.canonical.dbusmenu");
    auto reply = callOnLoop(loop, menu, "GetLayout", int32_t(0), int32_t(-1), std::vector<std::string>());
    ASSERT_FALSE(reply.isError()) << reply.error().message;
    const Variant layout = reply.arguments().at(1);
    const Variant editItem = *layout.items().at(2).items().at(0).get<Variant>();
    const auto edit = editItem.items();
    const int32_t editId = edit.at(0).value<int32_t>();
    const auto children = edit.at(2).items();
    ASSERT_EQ(children.size(), 3u);
    const auto pasteItem = children[0].get<Variant>()->items();
    const VariantMap pasteProperties = pasteItem.at(1).value<VariantMap>();
    EXPECT_EQ(pasteProperties.at("label").value<std::string>(), "Paste");
    EXPECT_FALSE(pasteProperties.at("enabled").value<bool>(true));
    const VariantMap separator = children[1].get<Variant>()->items().at(1).value<VariantMap>();
    EXPECT_EQ(separator.at("type").value<std::string>(), "separator");
    const VariantMap selectAll = children[2].get<Variant>()->items().at(1).value<VariantMap>();
    EXPECT_EQ(selectAll.at("label").value<std::string>(), "Select__All");

    // AboutToShow brings the entries up to date and pushes what changed in place.
    const int32_t pasteId = pasteItem.at(0).value<int32_t>();
    bool updated = false;
    const base::dbus::Match match = menu.onSignal("ItemsPropertiesUpdated", [&](const base::dbus::Message& message) {
        std::vector<std::tuple<int32_t, VariantMap>> changed;
        std::vector<std::tuple<int32_t, std::vector<std::string>>> removed;
        ASSERT_TRUE(message.read(changed, removed));
        ASSERT_EQ(changed.size(), 1u);
        EXPECT_EQ(std::get<0>(changed[0]), pasteId);
        EXPECT_TRUE(std::get<1>(changed[0]).at("enabled").value<bool>(false));
        updated = true;
    });
    reply = callOnLoop(loop, menu, "AboutToShow", editId);
    ASSERT_FALSE(reply.isError()) << reply.error().message;
    // The reply asks the bar to refetch: it shows the submenu it fetched before otherwise.
    EXPECT_TRUE(reply.arguments().at(0).value<bool>());
    EXPECT_EQ(refreshed, 1);
    EXPECT_TRUE(runUntil(loop, [&] { return updated; }));
    // Also when nothing changed since: the bar's copy may predate an earlier update.
    reply = callOnLoop(loop, menu, "AboutToShow", editId);
    ASSERT_FALSE(reply.isError()) << reply.error().message;
    EXPECT_TRUE(reply.arguments().at(0).value<bool>());

    reply = callOnLoop(loop, menu, "Event", pasteId, std::string("clicked"), Variant(std::string()), uint32_t(0));
    ASSERT_FALSE(reply.isError());
    EXPECT_TRUE(runUntil(loop, [&] { return pasted == 1; }));

    reply = callOnLoop(loop, menu, "GetProperty", int32_t(999), std::string("label"));
    EXPECT_TRUE(reply.isError());

    exporter.reset();
    EXPECT_TRUE(runUntil(loop, [&] { return registrar.unregistered == 1; }));
}
