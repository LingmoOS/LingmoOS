// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <unistd.h>
#include <utility>
#include <vector>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/settings.h"
#include "pages/language/language.h"
#include "pages/language/preferredlanguagelist.h"
#include "ui/application.h"
#include "ui/keys.h"
#include "ui/layout.h"
#include "ui/test_input.h"
#include "ui/window.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using namespace settings;
using namespace std::chrono_literals;

namespace {

// Runs the loop until the condition holds or five seconds pass.
bool waitFor(const std::function<bool()>& condition)
{
    base::EventLoop& loop = Application::instance()->loop();
    const auto deadline = std::chrono::steady_clock::now() + 5s;
    while (!condition()) {
        if (std::chrono::steady_clock::now() > deadline)
            return false;
        const int timer = loop.startTimer(5ms, false, [&loop] { loop.quit(); });
        loop.run();
        loop.stopTimer(timer);
    }
    return true;
}

// lingmo-services' settings with the given languages, in a directory of its own.
class LanguageConfig
{
public:
    explicit LanguageConfig(const std::vector<std::string>& codes)
    {
        char pattern[] = "/tmp/settings-language-XXXXXX";
        m_directory = mkdtemp(pattern);
        path = m_directory + "/services.conf";
        base::Settings settings(path);
        settings.setValue("Language/language", codes.front());
        settings.setValue("Language/preferredLanguages", codes);
    }
    ~LanguageConfig() { std::filesystem::remove_all(m_directory); }

    std::string path;

private:
    std::string m_directory;
};

std::vector<std::string> codes(const std::vector<LanguageEntry>& entries)
{
    std::vector<std::string> result;
    for (const LanguageEntry& entry : entries)
        result.push_back(entry.code);
    return result;
}

} // namespace

TEST(Language, OpeningThePageDoesNotLoadTheCatalog)
{
    LanguageConfig config({"en_US", "zh_Hans_CN"});
    Language language(config.path);
    EXPECT_EQ(codes(language.preferredLanguages()), (std::vector<std::string>{"en_US", "zh_Hans_CN"}));
    EXPECT_TRUE(language.availableLanguages().empty());
    EXPECT_FALSE(language.isLoading());
}

TEST(Language, CatalogAndSearch)
{
    LanguageConfig config({"en_US", "zh_Hans_CN"});
    Language language(config.path);
    language.loadLanguages();
    ASSERT_TRUE(waitFor([&] { return !language.isLoading() && !language.availableLanguages().empty(); }));
    std::vector<std::string> available = codes(language.availableLanguages());
    ASSERT_EQ(available.size(), 40u);
    EXPECT_EQ(available.front(), "en_GB");
    EXPECT_EQ(available[available.size() - 2], "zh_Hant_HK");
    EXPECT_EQ(available.back(), "zh_Hant_TW");

    // Preferred languages are left out even when searched for.
    language.search("zh_Hans_CN");
    EXPECT_TRUE(language.availableLanguages().empty());
    // A language's own name, its code, ignoring case and the space around it.
    language.search("Deutsch");
    EXPECT_EQ(codes(language.availableLanguages()), std::vector<std::string>{"de_DE"});
    language.search("it_IT");
    EXPECT_EQ(language.availableLanguages().size(), 1u);
    language.search("no-such-language");
    EXPECT_TRUE(language.availableLanguages().empty());
    language.search("  ID_id  ");
    EXPECT_EQ(codes(language.availableLanguages()), std::vector<std::string>{"id_ID"});
    language.search("");
    EXPECT_EQ(language.availableLanguages().size(), 40u);
}

TEST(Language, ClosingWhileLoadingLeavesTheCatalogForTheNextPage)
{
    LanguageConfig config({"en_US", "zh_Hans_CN"});
    auto language = std::make_unique<Language>(config.path);
    language->loadLanguages();
    language.reset();
    Language reopened(config.path);
    reopened.loadLanguages();
    ASSERT_TRUE(waitFor([&] { return !reopened.isLoading() && reopened.availableLanguages().size() == 40; }));
}

TEST(Language, SavingWithoutTheServiceReportsAnError)
{
    // Never against the live session, whose languages this would change.
    const base::dbus::Proxy daemon(base::dbus::Connection::session(), "org.freedesktop.DBus",
                                   "/org/freedesktop/DBus", "org.freedesktop.DBus");
    if (daemon.call("NameHasOwner", std::string("org.lingmo.Services")).value<bool>().value_or(false))
        GTEST_SKIP() << "Needs a private session bus: run the tests in dbus-run-session";
    LanguageConfig config({"en_US", "de_DE"});
    Language language(config.path);
    language.moveLanguage(1, 0);
    ASSERT_TRUE(waitFor([&] { return !language.isSaving(); }));
    EXPECT_EQ(language.errorMessage(), "Could not save preferred languages. Please try again.");
    EXPECT_EQ(codes(language.preferredLanguages()), (std::vector<std::string>{"en_US", "de_DE"}));
}

namespace {

// The preferred language list in a window at 150%, its moves recorded.
struct DragFixture : ::testing::Test
{
    explicit DragFixture(std::vector<std::string> languages = {"en_US", "ja_JP", "de_DE"})
        : config(languages)
        , language(config.path)
    {
        window.setScale(1.5);
        auto root = std::make_unique<Column>();
        root->setMargins({20, 20, 20, 20});
        list = root->addChild<PreferredLanguageList>(language);
        list->onMoveRequested = [this](int from, int to) { moves.emplace_back(from, to); };
        window.setContentView(std::move(root));
        window.resize({520, 440});
        window.grab();
    }

    // The middle of a row's drag handle, in window coordinates.
    PointF handle(int index) const
    {
        const RectF rect = list->geometry();
        return {rect.right() - 16 + 9, rect.y + PreferredLanguageList::kRowHeight * (index + 0.5)};
    }
    double rowY(double rows) const { return list->geometry().y + PreferredLanguageList::kRowHeight * rows; }

    PointF startDrag(int index)
    {
        const PointF point = handle(index);
        TestInput::movePointer(window, point);
        TestInput::press(window, point);
        TestInput::movePointer(window, {point.x, point.y + 16});
        EXPECT_TRUE(list->isDragging());
        return point;
    }

    LanguageConfig config;
    Language language;
    Window window;
    PreferredLanguageList* list;
    std::vector<std::pair<int, int>> moves;
};

struct LongDragFixture : DragFixture
{
    LongDragFixture()
        : DragFixture({"en_US", "en_GB", "ja_JP", "ko_KR", "fr_FR", "de_DE", "es_ES", "pt_BR", "ru_RU", "it_IT",
                       "id_ID", "vi_VN"})
    {
    }
};

} // namespace

TEST_F(DragFixture, MovesDown)
{
    const PointF point = startDrag(0);
    TestInput::movePointer(window, {point.x, rowY(2.5)});
    EXPECT_EQ(list->targetIndex(), 2);
    EXPECT_TRUE(moves.empty());
    TestInput::release(window, {point.x, rowY(2.5)});
    EXPECT_EQ(moves, (std::vector<std::pair<int, int>>{{0, 2}}));
    EXPECT_FALSE(list->isDragging());
}

TEST_F(DragFixture, MovesUp)
{
    const PointF point = startDrag(2);
    TestInput::movePointer(window, {point.x, rowY(0.5)});
    TestInput::release(window, {point.x, rowY(0.5)});
    EXPECT_EQ(moves, (std::vector<std::pair<int, int>>{{2, 0}}));
}

TEST_F(DragFixture, EscapeCancels)
{
    const PointF point = startDrag(0);
    TestInput::movePointer(window, {point.x, rowY(2.5)});
    TestInput::typeKey(window, key::Escape);
    EXPECT_FALSE(list->isDragging());
    TestInput::release(window, {point.x, rowY(2.5)});
    EXPECT_TRUE(moves.empty());
}

TEST_F(DragFixture, ReleasingOutsideCancels)
{
    const PointF point = startDrag(0);
    TestInput::movePointer(window, {list->geometry().x - 10, rowY(2.5)});
    TestInput::release(window, {list->geometry().x - 10, rowY(2.5)});
    EXPECT_TRUE(moves.empty());
}

TEST_F(DragFixture, DroppingInPlaceSavesNothing)
{
    const PointF point = startDrag(0);
    TestInput::movePointer(window, point);
    TestInput::release(window, point);
    EXPECT_TRUE(moves.empty());
}

TEST_F(DragFixture, KeysMoveTheFocusedRow)
{
    TestInput::click(window, handle(1));
    TestInput::typeKey(window, key::Up);
    TestInput::typeKey(window, key::Down);
    EXPECT_EQ(moves, (std::vector<std::pair<int, int>>{{1, 0}, {1, 2}}));
}

TEST_F(LongDragFixture, ScrollsAtTheEdge)
{
    // Seven rows show; the rest scroll.
    EXPECT_EQ(list->geometry().height, PreferredLanguageList::kRowHeight * 7);
    const PointF point = startDrag(0);
    const PointF bottom{point.x, list->geometry().bottom() - 1};
    TestInput::movePointer(window, bottom);
    ASSERT_TRUE(waitFor([&] { return list->scrollY() > PreferredLanguageList::kRowHeight; }));
    TestInput::release(window, bottom);
    ASSERT_EQ(moves.size(), 1u);
    EXPECT_EQ(moves.front().first, 0);
    EXPECT_GT(moves.front().second, 0);
}

TEST_F(DragFixture, ReleasingAboveMovesToTheTop)
{
    const PointF point = startDrag(2);
    TestInput::movePointer(window, {point.x, rowY(0.5)});
    TestInput::movePointer(window, {point.x, list->geometry().y - 20});
    EXPECT_EQ(list->targetIndex(), 0);
    TestInput::release(window, {point.x, list->geometry().y - 20});
    EXPECT_EQ(moves, (std::vector<std::pair<int, int>>{{2, 0}}));
}

TEST_F(DragFixture, ReleasingBelowMovesToTheBottom)
{
    const PointF point = startDrag(0);
    const double below = list->geometry().bottom() + 20;
    TestInput::movePointer(window, {point.x, below});
    EXPECT_EQ(list->targetIndex(), list->count() - 1);
    TestInput::release(window, {point.x, below});
    EXPECT_EQ(moves, (std::vector<std::pair<int, int>>{{0, list->count() - 1}}));
}
