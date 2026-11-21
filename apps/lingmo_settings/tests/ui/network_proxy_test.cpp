// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "input_test_support.h"
#include "pagetest.h"
#include "pages/network/common/textchecks.h"
#include "pages/network/proxy/proxysettings.h"
#include "ui/button.h"
#include "ui/test_input.h"
#include "ui/text_field.h"
#include "window/page.h"

using namespace Lingmo::ui;
using namespace settings::network;

namespace {

struct KeyFile : testing::Test
{
    void SetUp() override
    {
        char pattern[] = "/tmp/proxy-test-XXXXXX";
        directory = mkdtemp(pattern);
        path = directory + "/keyfile";
    }

    void TearDown() override { std::filesystem::remove_all(directory); }

    void write(const std::string& contents)
    {
        std::ofstream file(path);
        file << contents;
    }

    std::string contents() const
    {
        std::ifstream file(path);
        std::stringstream text;
        text << file.rdbuf();
        return text.str();
    }

    std::string directory;
    std::string path;
};

template<typename T>
void findAll(View* view, std::vector<T*>* found)
{
    if (auto* match = dynamic_cast<T*>(view))
        found->push_back(match);
    for (const auto& child : view->children())
        findAll<T>(child.get(), found);
}

PointF centre(View* view)
{
    const PointF origin = view->mapToWindow({0, 0});
    return {origin.x + view->geometry().width / 2, origin.y + view->geometry().height / 2};
}

} // namespace

TEST_F(KeyFile, ReadsGroupsAndKeys)
{
    write("# comment\n[system/proxy]\nmode='manual'\n\n[system/proxy/http]\nhost='proxy.example.com'\nport=3128\n");
    const auto groups = keyfile::read(path);
    EXPECT_EQ(groups.at("system/proxy").at("mode"), "'manual'");
    EXPECT_EQ(gsettings::toString(groups.at("system/proxy/http").at("host")), "proxy.example.com");
    EXPECT_EQ(gsettings::toInt(groups.at("system/proxy/http").at("port"), 0), 3128);
}

TEST_F(KeyFile, WritesOnlyWhatChanged)
{
    const std::string original = "[other]\nkey=1\n\n[system/proxy]\nmode='none'\nignore-hosts=['localhost']\n\n[last]\nx=2\n";
    write(original);
    // The same value again changes nothing.
    EXPECT_FALSE(keyfile::writeGroup(path, "system/proxy", {{"mode", "'none'"}}));
    EXPECT_EQ(contents(), original);

    // A changed key is replaced in place, a new one goes before the blank line ahead of the next group.
    EXPECT_TRUE(keyfile::writeGroup(path, "system/proxy", {{"mode", "'manual'"}, {"autoconfig-url", "''"}}));
    EXPECT_EQ(contents(), "[other]\nkey=1\n\n[system/proxy]\nmode='manual'\nignore-hosts=['localhost']\n"
                          "autoconfig-url=''\n\n[last]\nx=2\n");

    // A new group goes last, after a blank line.
    EXPECT_TRUE(keyfile::writeGroup(path, "system/proxy/socks", {{"host", "'s'"}, {"port", "1080"}}));
    EXPECT_TRUE(contents().ends_with("x=2\n\n[system/proxy/socks]\nhost='s'\nport=1080\n"));
}

TEST(GSettingsValues, ReadAsGLibWritesThem)
{
    EXPECT_EQ(gsettings::toString("'it\\'s'"), "it's");
    EXPECT_EQ(gsettings::toString("\"double\""), "double");
    EXPECT_EQ(gsettings::toString("'\\u00e9'"), "\xc3\xa9");
    EXPECT_EQ(gsettings::toString("unquoted", "fallback"), "fallback");
    EXPECT_EQ(gsettings::toInt("uint32 8080", 0), 8080);
    EXPECT_EQ(gsettings::toInt("'x'", 7), 7);
    EXPECT_TRUE(gsettings::toBool("true", false));
    EXPECT_EQ(gsettings::toStringList("@as []", {"x"}), std::vector<std::string>{});
    EXPECT_EQ(gsettings::toStringList("['localhost', '127.0.0.0/8']", {}),
              (std::vector<std::string>{"localhost", "127.0.0.0/8"}));
    EXPECT_EQ(gsettings::toStringList("['broken", {"x"}), std::vector<std::string>{"x"});
    EXPECT_EQ(gsettings::stringValue("it's\n"), "'it\\'s\\n'");
}

TEST(EditFieldText, PortsAndNumbers)
{
    EXPECT_TRUE(isPortText(""));
    EXPECT_TRUE(isPortText("1"));
    EXPECT_TRUE(isPortText("65535"));
    EXPECT_FALSE(isPortText("65536"));
    EXPECT_FALSE(isPortText("0"));
    EXPECT_FALSE(isPortText("08"));
    EXPECT_FALSE(isPortText("8a"));
    EXPECT_TRUE(isNumberText("", 10));
    EXPECT_TRUE(isNumberText("10", 10));
    EXPECT_FALSE(isNumberText("11", 10));
}

// A new server and its port saved together: the server going in first must
// not put the old port back into the field before it is read.
TEST_F(KeyFile, ANewServerKeepsTheTypedPort)
{
    write("[system/proxy]\nmode='manual'\n");
    gsettings::setPath(path);
    {
        PageFixture page("network", "proxy");
        Window& window = page.window->window();
        std::vector<TextField*> fields;
        findAll<TextField>(page.window->currentPage(), &fields);
        // The script URL, then HTTP, FTP and SOCKS, each with its port.
        ASSERT_EQ(fields.size(), 7u);
        const std::vector<std::string> typed = {"", "http.test", "3128", "", "", "socks.test", "1081"};
        for (size_t i = 0; i < fields.size(); ++i) {
            if (typed[i].empty())
                continue;
            window.grab();
            TestInput::click(window, centre(fields[i]));
            TestInput::typeText(window, typed[i]);
            ASSERT_EQ(fields[i]->text(), typed[i]);
        }

        std::vector<Button*> buttons;
        findAll<Button>(page.window->currentPage(), &buttons);
        ASSERT_FALSE(buttons.empty());
        buttons.back()->onClicked();
        EXPECT_EQ(fields[2]->text(), "3128");
        EXPECT_EQ(fields[6]->text(), "1081");
        EXPECT_TRUE(settings::testing::waitFor([&] { return contents().find("[system/proxy/socks]") != std::string::npos; }));
    }

    const ProxySettings saved = ProxySettings::load();
    gsettings::setPath({});
    EXPECT_EQ(saved.http, (ProxySettings::Server{"http.test", 3128}));
    EXPECT_EQ(saved.socks, (ProxySettings::Server{"socks.test", 1081}));
}
