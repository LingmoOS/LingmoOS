// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <set>
#include <thread>

#include "base/auto_reset.h"
#include "base/command_line.h"
#include "base/elapsed_timer.h"
#include "base/uuid.h"
#include "base/weak_ptr.h"

using namespace Lingmo::Base;
using namespace std::chrono_literals;

namespace {

struct Base
{
    virtual ~Base() = default;
    int value = 7;
};

struct Owner : Base
{
    WeakPtrFactory<Owner> weakFactory{this};
};

} // namespace

TEST(WeakPtrTest, NullsWhenTheOwnerGoes)
{
    WeakPtr<Owner> weak;
    WeakPtr<Base> base;
    EXPECT_FALSE(weak);
    {
        Owner owner;
        EXPECT_FALSE(owner.weakFactory.hasWeakPtrs());
        weak = owner.weakFactory.getWeakPtr();
        base = weak;
        EXPECT_TRUE(owner.weakFactory.hasWeakPtrs());
        EXPECT_EQ(weak.get(), &owner);
        EXPECT_EQ(base->value, 7);
        EXPECT_EQ(weak, owner.weakFactory.getWeakPtr());
    }
    EXPECT_FALSE(weak);
    EXPECT_EQ(weak.get(), nullptr);
    EXPECT_EQ(base.get(), nullptr);
    EXPECT_TRUE(weak == nullptr);
}

TEST(WeakPtrTest, Invalidate)
{
    Owner owner;
    WeakPtr<Owner> first = owner.weakFactory.getWeakPtr();
    owner.weakFactory.invalidateWeakPtrs();
    EXPECT_FALSE(first);
    WeakPtr<Owner> second = owner.weakFactory.getWeakPtr();
    EXPECT_TRUE(second);
    second.reset();
    EXPECT_FALSE(second);
}

TEST(AutoResetTest, RestoresTheValue)
{
    int value = 1;
    {
        AutoReset reset(value, 5);
        EXPECT_EQ(value, 5);
        value = 9;
    }
    EXPECT_EQ(value, 1);
    std::string text = "a";
    {
        AutoReset<std::string> saved(text);
        text = "b";
    }
    EXPECT_EQ(text, "a");

    int runs = 0;
    {
        ScopeGuard guard([&] { ++runs; });
    }
    {
        ScopeGuard guard([&] { ++runs; });
        guard.dismiss();
    }
    EXPECT_EQ(runs, 1);
}

TEST(UuidTest, GenerateParseFormat)
{
    std::set<std::string> seen;
    for (int i = 0; i < 100; ++i) {
        const Uuid uuid = Uuid::generate();
        EXPECT_EQ(uuid.version(), 4);
        EXPECT_EQ(uuid.bytes()[8] & 0xC0, 0x80);
        const std::string text = uuid.toString();
        ASSERT_EQ(text.size(), 36u);
        EXPECT_EQ(text[14], '4');
        EXPECT_EQ(Uuid::parse(text), uuid);
        seen.insert(text);
    }
    EXPECT_EQ(seen.size(), 100u);

    const auto parsed = Uuid::parse("{123E4567-E89B-12D3-A456-426614174000}");
    ASSERT_TRUE(parsed);
    EXPECT_EQ(parsed->toString(), "123e4567-e89b-12d3-a456-426614174000");
    EXPECT_FALSE(Uuid::parse("123e4567e89b12d3a456426614174000"));
    EXPECT_FALSE(Uuid::parse("123e4567-e89b-12d3-a456-42661417400g"));
    EXPECT_TRUE(Uuid().isNil());
    EXPECT_EQ(Uuid().toString(), "00000000-0000-0000-0000-000000000000");
}

TEST(ElapsedTimerTest, Measures)
{
    ElapsedTimer timer;
    EXPECT_FALSE(timer.isValid());
    timer.start();
    EXPECT_TRUE(timer.isValid());
    std::this_thread::sleep_for(5ms);
    EXPECT_GE(timer.elapsed(), 5ms);
    EXPECT_GE(timer.elapsedMs(), 5);
    EXPECT_TRUE(timer.hasExpired(1ms));
    EXPECT_FALSE(timer.hasExpired(1h));
    EXPECT_GE(timer.restart(), 5ms);
    EXPECT_LT(timer.elapsed(), 5ms);
    timer.invalidate();
    EXPECT_FALSE(timer.isValid());
}

TEST(CommandLineTest, OptionsAndPositionals)
{
    CommandLineParser parser;
    parser.addHelpOption();
    const CommandLineOption socket("socket", "Socket name.", "name");
    const CommandLineOption scale("scale", "Scale.", "factor", "1.0");
    const CommandLineOption client("clipboard-client", "Repeatable.", "path");
    const CommandLineOption noDrm("no-drm", "Flag.");
    const CommandLineOption verbose(std::vector<std::string>{"V", "verbose"}, "Two names.");
    ASSERT_TRUE(parser.addOption(socket));
    ASSERT_TRUE(parser.addOption(scale));
    ASSERT_TRUE(parser.addOption(client));
    ASSERT_TRUE(parser.addOption(noDrm));
    ASSERT_TRUE(parser.addOption(verbose));
    EXPECT_FALSE(parser.addOption(CommandLineOption("socket")));
    parser.addPositionalArgument("command", "What to do.");

    ASSERT_TRUE(parser.parse({"/usr/bin/tool", "--socket", "wayland-1", "list", "--clipboard-client=/a",
                              "--clipboard-client", "/b", "-V", "--no-drm", "--", "--not-an-option"}));
    EXPECT_EQ(parser.programName(), "tool");
    EXPECT_TRUE(parser.isSet(socket));
    EXPECT_EQ(parser.value(socket), "wayland-1");
    EXPECT_FALSE(parser.isSet("scale"));
    EXPECT_EQ(parser.value(scale), "1.0");
    EXPECT_EQ(parser.values(client), (std::vector<std::string>{"/a", "/b"}));
    EXPECT_TRUE(parser.isSet("no-drm"));
    EXPECT_TRUE(parser.isSet("verbose"));
    EXPECT_FALSE(parser.isSet("help"));
    EXPECT_EQ(parser.positionalArguments(), (std::vector<std::string>{"list", "--not-an-option"}));
    EXPECT_EQ(parser.value("unknown"), "");
}

TEST(CommandLineTest, Errors)
{
    CommandLineParser parser;
    parser.addOption({"socket", "Socket name.", "name"});
    parser.addOption(CommandLineOption("x"));
    parser.addOption(CommandLineOption("y"));
    EXPECT_FALSE(parser.parse({"tool", "--bogus"}));
    EXPECT_EQ(parser.errorText(), "Unknown option 'bogus'.");
    EXPECT_FALSE(parser.parse({"tool", "--socket"}));
    EXPECT_EQ(parser.errorText(), "Missing value after '--socket'.");
    EXPECT_FALSE(parser.parse({"tool", "--x=1"}));
    EXPECT_TRUE(parser.parse({"tool", "-xy", "-"}));
    EXPECT_TRUE(parser.isSet("x"));
    EXPECT_TRUE(parser.isSet("y"));
    EXPECT_EQ(parser.positionalArguments(), (std::vector<std::string>{"-"}));
}

TEST(CommandLineTest, HelpText)
{
    CommandLineParser parser;
    parser.setApplicationDescription("Capture an output.");
    parser.addHelpOption();
    parser.addOption({"output", "Output to capture.", "name"});
    parser.addPositionalArgument("file", "Destination PNG file.", "[file]");
    parser.parse({"screenshot"});
    EXPECT_EQ(parser.helpText(),
              "Usage: screenshot [options] [file]\n"
              "Capture an output.\n"
              "\n"
              "Options:\n"
              "  -h, --help       Displays help on command line options.\n"
              "  --output <name>  Output to capture.\n"
              "\n"
              "Arguments:\n"
              "  file             Destination PNG file.\n");
}

TEST(CommandLineDeathTest, HelpExits)
{
    CommandLineParser parser;
    parser.addHelpOption();
    const char* argv[] = {"tool", "--help"};
    EXPECT_EXIT(parser.process(2, const_cast<char**>(argv)), testing::ExitedWithCode(0), "");
    const char* bad[] = {"tool", "--nope"};
    EXPECT_EXIT(parser.process(2, const_cast<char**>(bad)), testing::ExitedWithCode(1), "Unknown option 'nope'");
}
