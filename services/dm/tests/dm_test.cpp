#include <gtest/gtest.h>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <functional>
#include <optional>
#include <poll.h>
#include <pwd.h>
#include <sys/stat.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/file.h"
#include "base/local_socket.h"
#include "base/settings.h"

#include "common/wire.h"
#include "daemon/backgrounds.h"
#include "daemon/config.h"
#include "daemon/daemon.h"
#include "daemon/greeterserver.h"
#include "daemon/helperprocess.h"
#include "daemon/sessions.h"
#include "daemon/state.h"

using namespace Lingmo;
using namespace std::chrono_literals;
using dm::Json;

// When set, the test binary is the helper: it accepts "right" and its session
// runs until terminated, or, set to "stubborn", ignores SIGTERM, or, set to
// "linger", ends half a second after it. User "slow" is asked for a password
// only after a second; a session command holding "--fail-open" fails to open
// after 300 ms, as when pam_open_session does.
static constexpr char kFakeHelper[] = "DM_TEST_FAKE_HELPER";

static base::EventLoop *g_loop = nullptr;

static volatile sig_atomic_t g_terminated = 0;

static std::string readLine()
{
    std::string line;
    char c = 0;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != '\n')
        line += c;
    return line;
}

static void writeLine(const Json &message)
{
    const std::string bytes = dm::encodeLine(message);
    if (write(STDOUT_FILENO, bytes.data(), bytes.size()) != ssize_t(bytes.size()))
        _exit(3);
}

static int fakeHelper(const char *mode)
{
    const auto login = dm::decodeLine(readLine());
    if (!login)
        return 2;

    if ((*login)["authenticate"].toBool()) {
        const std::string user = (*login)["user"].toString();
        if (user != "alice" && user != "slow")
            return 2;
        if (user == "slow")
            usleep(1000 * 1000);

        writeLine(Json::object({{"type", "question"}, {"style", "secret"}, {"text", "Password:"}}));
        const auto answer = dm::decodeLine(readLine());
        if (!answer || (*answer)["response"].toString() != "right") {
            writeLine(Json::object({{"type", "error"}, {"auth", true}, {"message", "Authentication failure"}}));
            return 1;
        }
    }

    writeLine(Json::object({{"type", "authenticated"}}));

    const auto start = dm::decodeLine(readLine());
    if (!start || !(*start)["cmd"].isArray() || (*start)["cmd"].toArray().empty())
        return 0;

    for (const Json &argument : (*start)["cmd"].toArray()) {
        if (argument.toString() == "--fail-open") {
            usleep(300 * 1000);
            writeLine(Json::object({{"type", "error"}, {"auth", false}, {"message", "Cannot open session"}}));
            return 1;
        }
    }

    if (std::strcmp(mode, "stubborn") == 0)
        signal(SIGTERM, SIG_IGN);
    if (std::strcmp(mode, "linger") == 0)
        signal(SIGTERM, [](int) { g_terminated = 1; });

    writeLine(Json::object({{"type", "started"}, {"pid", int64_t(getpid())}, {"session", "42"}}));

    while (!g_terminated)
        pause();
    usleep(500 * 1000);
    return 0;
}

// Runs the loop until done() holds or the timeout passes; returns done().
static bool runUntil(const std::function<bool()> &done, std::chrono::milliseconds timeout = 5s)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    g_loop->dispatch();
    while (!done()) {
        if (std::chrono::steady_clock::now() >= deadline)
            return done();
        pollfd ready {g_loop->fd(), POLLIN, 0};
        poll(&ready, 1, 5);
        g_loop->dispatch();
    }
    return true;
}

static void runFor(std::chrono::milliseconds duration)
{
    runUntil([] { return false; }, duration);
}

static void writeFile(const std::string &path, const std::string &contents)
{
    ASSERT_TRUE(base::file::writeAtomically(path, contents));
}

static std::string selfPath()
{
    char path[4096] = {};
    const ssize_t length = readlink("/proc/self/exe", path, sizeof(path) - 1);
    return length > 0 ? std::string(path, size_t(length)) : std::string();
}

class DisplayManagerTest : public ::testing::Test
{
protected:
    base::file::TemporaryDirectory directory {"/tmp", "dm-test"};
};

TEST(WireTest, FramesSurviveSplitsAndBatches)
{
    const std::string first = dm::encodeFrame(Json::object({{"type", "success"}}));
    const std::string second = dm::encodeFrame(Json::object({{"type", "cancel_session"}}));
    dm::FrameReader reader;
    reader.append(first.substr(0, 3));
    EXPECT_FALSE(reader.next());
    reader.append(first.substr(3) + second);
    const auto a = reader.next();
    const auto b = reader.next();
    ASSERT_TRUE(a && b);
    EXPECT_EQ((*a)["type"].toString(), "success");
    EXPECT_EQ((*b)["type"].toString(), "cancel_session");
    EXPECT_FALSE(reader.next());
    EXPECT_FALSE(reader.failed());
}

TEST(WireTest, OversizedOrMalformedFramesFailTheReader)
{
    dm::FrameReader oversized;
    const uint32_t length = dm::kMaxFrameSize + 1;
    oversized.append(std::string(reinterpret_cast<const char *>(&length), sizeof(length)));
    EXPECT_FALSE(oversized.next());
    EXPECT_TRUE(oversized.failed());

    dm::FrameReader malformed;
    const std::string payload("[1,2]");
    const uint32_t size = uint32_t(payload.size());
    malformed.append(std::string(reinterpret_cast<const char *>(&size), sizeof(size)) + payload);
    EXPECT_FALSE(malformed.next());
    EXPECT_TRUE(malformed.failed());
}

TEST(WireTest, LinesRoundTrip)
{
    const Json message = Json::object({{"type", "answer"}, {"response", nullptr}});
    const std::string line = dm::encodeLine(message);
    EXPECT_EQ(line.back(), '\n');
    EXPECT_EQ(std::count(line.begin(), line.end(), '\n'), 1);
    EXPECT_EQ(*dm::decodeLine(line), message);
    EXPECT_FALSE(dm::decodeLine("not json"));
}

TEST_F(DisplayManagerTest, SessionFilesParse)
{
    writeFile(directory.path() + "/plasma.desktop",
              "[Desktop Entry]\n"
              "# a comment\n"
              "Name=Plasma (Wayland)\n"
              "Comment=Plasma by KDE\n"
              "Exec=/usr/bin/startplasma-wayland --arg \"two words\" %f\n"
              "DesktopNames=KDE;Plasma;\n"
              "[Other Group]\n"
              "Exec=ignored\n");
    const auto entry = dm::readSessionFile(directory.path() + "/plasma.desktop");
    ASSERT_TRUE(entry);
    EXPECT_EQ(entry->id, "plasma");
    EXPECT_EQ(entry->name, "Plasma (Wayland)");
    EXPECT_EQ(entry->exec, (std::vector<std::string> {"/usr/bin/startplasma-wayland", "--arg", "two words"}));
    EXPECT_EQ(entry->desktopNames, (std::vector<std::string> {"KDE", "Plasma"}));
}

TEST_F(DisplayManagerTest, HiddenMissingOrCommandlessSessionsAreNotOffered)
{
    writeFile(directory.path() + "/hidden.desktop", "[Desktop Entry]\nName=Hidden\nExec=/bin/true\nHidden=true\n");
    writeFile(directory.path() + "/nodisplay.desktop",
              "[Desktop Entry]\nName=NoDisplay\nExec=/bin/true\nNoDisplay=true\n");
    writeFile(directory.path() + "/missing.desktop",
              "[Desktop Entry]\nName=Missing\nExec=/bin/true\nTryExec=/nonexistent/program\n");
    writeFile(directory.path() + "/noexec.desktop", "[Desktop Entry]\nName=NoExec\n");
    writeFile(directory.path() + "/good.desktop", "[Desktop Entry]\nName=Good\nExec=/bin/true\nTryExec=true\n");
    const auto sessions = dm::findSessions({directory.path()});
    ASSERT_EQ(sessions.size(), 1u);
    EXPECT_EQ(sessions.front().id, "good");
}

TEST_F(DisplayManagerTest, EarlierDirectoriesWinAndIdsNameFilesOnly)
{
    base::file::TemporaryDirectory system("/tmp", "dm-test");
    writeFile(directory.path() + "/lingmo.desktop", "[Desktop Entry]\nName=Local\nExec=/bin/true\n");
    writeFile(system.path() + "/lingmo.desktop", "[Desktop Entry]\nName=System\nExec=/bin/false\n");
    writeFile(directory.path() + "/gnome.desktop", "[Desktop Entry]\nName=GNOME\nExec=/bin/true\nHidden=true\n");
    writeFile(system.path() + "/gnome.desktop", "[Desktop Entry]\nName=GNOME\nExec=/bin/true\n");
    const std::vector<std::string> directories {directory.path(), system.path()};
    EXPECT_EQ(dm::findSession(directories, "lingmo")->name, "Local");
    // A hidden local entry masks the system one.
    EXPECT_FALSE(dm::findSession(directories, "gnome"));
    EXPECT_FALSE(dm::findSession(directories, "../lingmo"));
    EXPECT_FALSE(dm::findSession(directories, ".hidden"));
    EXPECT_FALSE(dm::findSession(directories, ""));
}

TEST_F(DisplayManagerTest, RemoveSessionDesktopFilesOnlyRemovesTheRequestedSession)
{
    base::file::TemporaryDirectory system("/tmp", "dm-test");
    writeFile(directory.path() + "/lingmo-initial-setup.desktop",
              "[Desktop Entry]\nName=Setup\nExec=/bin/true\n");
    writeFile(system.path() + "/lingmo-initial-setup.desktop",
              "[Desktop Entry]\nName=Setup\nExec=/bin/true\n");
    writeFile(system.path() + "/lingmo-wayland.desktop",
              "[Desktop Entry]\nName=Lingmo\nExec=/bin/true\n");

    ASSERT_TRUE(dm::removeSessionDesktopFiles({directory.path(), system.path()}, "lingmo-initial-setup"));
    EXPECT_FALSE(std::filesystem::exists(directory.path() + "/lingmo-initial-setup.desktop"));
    EXPECT_FALSE(std::filesystem::exists(system.path() + "/lingmo-initial-setup.desktop"));
    EXPECT_TRUE(std::filesystem::exists(system.path() + "/lingmo-wayland.desktop"));
    EXPECT_FALSE(dm::removeSessionDesktopFiles({directory.path()}, "../lingmo-wayland"));
}

TEST(SessionTest, EnvironmentsCarryWhatPamSystemdReads)
{
    const auto contains = [](const std::vector<std::string> &list, const std::string &value) {
        return std::find(list.begin(), list.end(), value) != list.end();
    };

    dm::SessionEntry gnome;
    gnome.id = "gnome";
    gnome.desktopNames = {"GNOME"};
    const auto session = dm::sessionEnvironment(gnome, 2);
    EXPECT_TRUE(contains(session, "XDG_SESSION_TYPE=wayland"));
    EXPECT_TRUE(contains(session, "XDG_SESSION_CLASS=user"));
    EXPECT_TRUE(contains(session, "XDG_SEAT=seat0"));
    EXPECT_TRUE(contains(session, "XDG_VTNR=2"));
    EXPECT_TRUE(contains(session, "DESKTOP_SESSION=gnome"));
    EXPECT_TRUE(contains(session, "XDG_SESSION_DESKTOP=GNOME"));
    EXPECT_TRUE(contains(session, "XDG_CURRENT_DESKTOP=GNOME"));

    const auto bare = dm::sessionEnvironment(dm::SessionEntry {}, 2);
    EXPECT_TRUE(contains(bare, "XDG_SESSION_TYPE=wayland"));
    for (const std::string &variable : bare)
        EXPECT_NE(variable.rfind("DESKTOP_SESSION=", 0), 0u);

    const auto greeter = dm::greeterEnvironment(1, "/run/x.sock");
    EXPECT_TRUE(contains(greeter, "XDG_SESSION_CLASS=greeter"));
    EXPECT_TRUE(contains(greeter, "XDG_VTNR=1"));
    EXPECT_TRUE(contains(greeter, "GREETD_SOCK=/run/x.sock"));
}

TEST_F(DisplayManagerTest, StateRemembersTheLastLogin)
{
    const std::string path = directory.path() + "/state/state.conf";
    dm::State state(path);
    EXPECT_NE(access(path.c_str(), F_OK), 0);
    state.remember("alice", "gnome");
    state.remember("bob", "lingmo-wayland");
    // The keys the login screen reads.
    const base::Settings reread(path);
    EXPECT_EQ(reread.stringValue("Last/User"), "bob");
    EXPECT_EQ(reread.stringValue("Sessions/alice"), "gnome");
    EXPECT_EQ(reread.stringValue("Sessions/bob"), "lingmo-wayland");
    EXPECT_EQ(base::file::read(path).value_or(""),
              "[Last]\nUser=bob\n\n[Sessions]\nalice=gnome\nbob=lingmo-wayland\n");
    // The login screen reads it as its own user.
    struct stat info = {};
    ASSERT_EQ(stat(path.c_str(), &info), 0);
    EXPECT_TRUE(info.st_mode & S_IROTH);
}

TEST_F(DisplayManagerTest, ConfigFallsBackToDefaults)
{
    const dm::Config defaults = dm::Config::load("/nonexistent/dm.conf");
    EXPECT_EQ(defaults.greeterVt, 1);
    EXPECT_EQ(defaults.sessionVt, 2);
    EXPECT_EQ(defaults.greeterUser, "lingmo-greeter");
    EXPECT_EQ(defaults.greeterCommand, (std::vector<std::string> {"/usr/bin/lingmo-aurora", "--login"}));

    EXPECT_TRUE(defaults.autologinUser.empty());

    writeFile(directory.path() + "/dm.conf",
              "[Greeter]\nVt=8\nCommand=/usr/bin/lingmo-aurora --login --no-xwayland\n"
              "[Session]\nVt=9\nDirectories=/a::/b\n[Daemon]\nRuntimeDirectory=/run/test\n"
              "[Autologin]\nUser=live\nSession=gnome\n");
    const dm::Config config = dm::Config::load(directory.path() + "/dm.conf");
    EXPECT_EQ(config.greeterVt, 8);
    EXPECT_EQ(config.sessionVt, 9);
    EXPECT_EQ(config.greeterCommand.back(), "--no-xwayland");
    EXPECT_EQ(config.sessionDirectories, (std::vector<std::string> {"/a", "/b"}));
    EXPECT_EQ(config.runtimeDirectory, "/run/test");
    EXPECT_EQ(config.greeterUser, "lingmo-greeter");
    EXPECT_EQ(config.autologinUser, "live");
    EXPECT_EQ(config.autologinSession, "gnome");
}

static std::string currentUser()
{
    const passwd *entry = getpwuid(getuid());
    return entry && entry->pw_name ? entry->pw_name : "";
}

static base::dbus::UnixFd openForReading(const std::string &path)
{
    return base::dbus::UnixFd::adopt(open(path.c_str(), O_RDONLY | O_CLOEXEC));
}

// Runs the loop until the copy answers; its answer.
static std::optional<bool> copyPicture(dm::Backgrounds &backgrounds, const std::string &user,
                                       base::dbus::UnixFd file)
{
    std::optional<bool> answer;
    backgrounds.setPicture(user, std::move(file), [&answer](bool copied) { answer = copied; });
    runUntil([&answer] { return answer.has_value(); });
    return answer;
}

TEST_F(DisplayManagerTest, BackgroundsKeepTheUsersPictureAndColour)
{
    const std::string user = currentUser();
    ASSERT_FALSE(user.empty());
    dm::Backgrounds backgrounds(directory.path() + "/backgrounds");

    writeFile(directory.path() + "/sky.jpg", "picture bytes");
    base::dbus::UnixFd file = openForReading(directory.path() + "/sky.jpg");
    // The caller's offset is not where the copy starts.
    ASSERT_EQ(lseek(file.fd(), 4, SEEK_SET), 4);
    EXPECT_EQ(copyPicture(backgrounds, user, file), true);
    EXPECT_EQ(lseek(file.fd(), 0, SEEK_CUR), 4);
    EXPECT_EQ(base::file::read(backgrounds.picturePath(user)).value_or(""), "picture bytes");
    // The keys the login screen reads, as services.conf has them.
    {
        const base::Settings settings(backgrounds.settingsPath(user));
        EXPECT_EQ(settings.intValue("Appearance/BackgroundType", -1), 0);
        EXPECT_EQ(settings.stringValue("Appearance/Wallpaper"), backgrounds.picturePath(user));
    }
    // The login screen reads both as its own user.
    struct stat info = {};
    ASSERT_EQ(stat(backgrounds.picturePath(user).c_str(), &info), 0);
    EXPECT_TRUE(info.st_mode & S_IROTH);
    ASSERT_EQ(stat(backgrounds.settingsPath(user).c_str(), &info), 0);
    EXPECT_TRUE(info.st_mode & S_IROTH);

    EXPECT_TRUE(backgrounds.setColor(user, "#2B8ADA"));
    {
        const base::Settings settings(backgrounds.settingsPath(user));
        EXPECT_EQ(settings.intValue("Appearance/BackgroundType", -1), 1);
        EXPECT_EQ(settings.stringValue("Appearance/BackgroundColor"), "#2B8ADA");
    }
    EXPECT_FALSE(backgrounds.setColor(user, "red"));
    EXPECT_FALSE(backgrounds.setColor(user, "#12345"));
    EXPECT_FALSE(backgrounds.setColor(user, "#12x456"));
}

TEST_F(DisplayManagerTest, BackgroundsRefuseWhatIsNotARegularFile)
{
    const std::string user = currentUser();
    dm::Backgrounds backgrounds(directory.path() + "/backgrounds");

    EXPECT_EQ(copyPicture(backgrounds, user, openForReading(directory.path())), false);
    int pipeFds[2];
    ASSERT_EQ(pipe(pipeFds), 0);
    close(pipeFds[1]);
    EXPECT_EQ(copyPicture(backgrounds, user, base::dbus::UnixFd::adopt(pipeFds[0])), false);
    writeFile(directory.path() + "/empty.jpg", "");
    EXPECT_EQ(copyPicture(backgrounds, user, openForReading(directory.path() + "/empty.jpg")), false);
    EXPECT_NE(access(backgrounds.settingsPath(user).c_str(), F_OK), 0);
    EXPECT_NE(access(backgrounds.picturePath(user).c_str(), F_OK), 0);
}

TEST_F(DisplayManagerTest, BackgroundsKeepTheLatestRequest)
{
    const std::string user = currentUser();
    dm::Backgrounds backgrounds(directory.path() + "/backgrounds");

    // A colour chosen while a picture is still being copied wins.
    writeFile(directory.path() + "/sky.jpg", "picture bytes");
    std::optional<bool> answer;
    backgrounds.setPicture(user, openForReading(directory.path() + "/sky.jpg"),
                           [&answer](bool copied) { answer = copied; });
    EXPECT_TRUE(backgrounds.setColor(user, "#000"));
    runUntil([&answer] { return answer.has_value(); });
    EXPECT_EQ(answer, true);
    const base::Settings settings(backgrounds.settingsPath(user));
    EXPECT_EQ(settings.intValue("Appearance/BackgroundType", -1), 1);
    EXPECT_NE(access(backgrounds.picturePath(user).c_str(), F_OK), 0);
    // No copy is left behind.
    std::error_code error;
    for (const auto &entry : std::filesystem::directory_iterator(directory.path() + "/backgrounds", error))
        EXPECT_EQ(entry.path().filename().string(), user + ".conf");
}

TEST_F(DisplayManagerTest, BackgroundsOfRemovedUsersAreDropped)
{
    const std::string user = currentUser();
    const std::string path = directory.path() + "/backgrounds";
    dm::Backgrounds backgrounds(path);
    EXPECT_TRUE(backgrounds.setColor(user, "#000"));
    writeFile(path + "/no-such-user-dm-test.conf", "");
    writeFile(path + "/no-such-user-dm-test.picture", "");
    writeFile(path + "/" + user + ".picture.7", "");

    backgrounds.removeStale();
    EXPECT_EQ(access(backgrounds.settingsPath(user).c_str(), F_OK), 0);
    EXPECT_NE(access((path + "/no-such-user-dm-test.conf").c_str(), F_OK), 0);
    EXPECT_NE(access((path + "/no-such-user-dm-test.picture").c_str(), F_OK), 0);
    EXPECT_NE(access((path + "/" + user + ".picture.7").c_str(), F_OK), 0);
}

TEST(ConfigTest, CommandsSplitAsTheConfigurationAlwaysHas)
{
    EXPECT_EQ(dm::splitCommand("a  b\tc"), (std::vector<std::string> {"a", "b", "c"}));
    EXPECT_EQ(dm::splitCommand("a \"b c\" d"), (std::vector<std::string> {"a", "b c", "d"}));
    EXPECT_EQ(dm::splitCommand("say \"\"\"hi\"\"\""), (std::vector<std::string> {"say", "\"hi\""}));
    EXPECT_EQ(dm::splitCommand("x\"y z\"w"), (std::vector<std::string> {"xy zw"}));
    EXPECT_EQ(dm::splitCommand("\"\" a"), (std::vector<std::string> {"a"}));
    EXPECT_TRUE(dm::splitCommand("   ").empty());
}

TEST_F(DisplayManagerTest, GreeterServerSpeaksGreetd)
{
    const std::string path = directory.path() + "/greeter.sock";
    dm::GreeterServer server;
    ASSERT_TRUE(server.listen(path, getuid()));
    struct stat info = {};
    ASSERT_EQ(stat(path.c_str(), &info), 0);
    EXPECT_EQ(info.st_mode & (S_IRGRP | S_IROTH | S_IWGRP | S_IWOTH), 0u);

    std::vector<std::string> created;
    std::vector<std::tuple<std::vector<std::string>, std::vector<std::string>, std::string>> started;
    server.onCreateSession = [&](const std::string &username) { created.push_back(username); };
    server.onStartSession = [&](const std::vector<std::string> &command, const std::vector<std::string> &environment,
                                const std::string &session) { started.emplace_back(command, environment, session); };

    base::LocalSocket client(g_loop);
    ASSERT_TRUE(client.connectToPath(path));

    // Two requests at once: the second waits for the first's answer.
    client.write(dm::encodeFrame(Json::object({{"type", "create_session"}, {"username", "alice"}})));
    client.write(dm::encodeFrame(Json::object({{"type", "start_session"},
                                               {"cmd", Json::Array {}},
                                               {"env", Json::Array {Json("LANG=C")}},
                                               {"session", "gnome"}})));
    ASSERT_TRUE(runUntil([&] { return created.size() == 1; }));
    EXPECT_EQ(created.front(), "alice");
    runFor(50ms);
    EXPECT_TRUE(started.empty());

    server.respond(dm::GreeterServer::authMessage("secret", "Password:"));
    ASSERT_TRUE(runUntil([&] { return started.size() == 1; }));
    EXPECT_EQ(std::get<1>(started.front()), std::vector<std::string> {"LANG=C"});
    EXPECT_EQ(std::get<2>(started.front()), "gnome");
    server.respond(dm::GreeterServer::success());

    dm::FrameReader reader;
    std::vector<Json> responses;
    ASSERT_TRUE(runUntil([&] {
        reader.append(client.readAll());
        while (auto response = reader.next())
            responses.push_back(*response);
        return responses.size() == 2;
    }));
    EXPECT_EQ(responses[0]["type"].toString(), "auth_message");
    EXPECT_EQ(responses[0]["auth_message_type"].toString(), "secret");
    EXPECT_EQ(responses[1]["type"].toString(), "success");
}

TEST_F(DisplayManagerTest, GreeterServerDropsAMalformedClient)
{
    const std::string path = directory.path() + "/greeter.sock";
    dm::GreeterServer server;
    ASSERT_TRUE(server.listen(path, getuid()));
    int disconnected = 0;
    server.onDisconnected = [&] { ++disconnected; };

    base::LocalSocket client(g_loop);
    ASSERT_TRUE(client.connectToPath(path));
    const uint32_t length = dm::kMaxFrameSize + 1;
    client.write(std::string(reinterpret_cast<const char *>(&length), sizeof(length)));
    EXPECT_TRUE(runUntil([&] { return disconnected == 1; }));
}

TEST_F(DisplayManagerTest, AReplacedGreeterClientCountsAsDisconnected)
{
    const std::string path = directory.path() + "/greeter.sock";
    dm::GreeterServer server;
    ASSERT_TRUE(server.listen(path, getuid()));
    int disconnected = 0;
    std::vector<std::string> created;
    server.onDisconnected = [&] { ++disconnected; };
    server.onCreateSession = [&](const std::string &username) { created.push_back(username); };

    base::LocalSocket first(g_loop);
    ASSERT_TRUE(first.connectToPath(path));
    first.write(dm::encodeFrame(Json::object({{"type", "create_session"}, {"username", "alice"}})));
    ASSERT_TRUE(runUntil([&] { return created.size() == 1; }));

    base::LocalSocket second(g_loop);
    ASSERT_TRUE(second.connectToPath(path));
    EXPECT_TRUE(runUntil([&] { return disconnected == 1; }));

    // The newcomer's request is served although the first one's was never answered.
    second.write(dm::encodeFrame(Json::object({{"type", "create_session"}, {"username", "bob"}})));
    ASSERT_TRUE(runUntil([&] { return created.size() == 2; }));
    EXPECT_EQ(created.back(), "bob");
}

class HelperTest : public ::testing::Test
{
protected:
    void SetUp() override { dm::HelperProcess::setProgram(selfPath()); }
    void TearDown() override { unsetenv(kFakeHelper); }
};

TEST_F(HelperTest, HelperProtocolRunsALogin)
{
    setenv(kFakeHelper, "1", 1);
    dm::HelperProcess helper;
    std::vector<std::string> questions;
    int authenticated = 0;
    int started = 0;
    int finished = 0;
    helper.onQuestion = [&](const std::string &style, const std::string &) { questions.push_back(style); };
    helper.onAuthenticated = [&] { ++authenticated; };
    helper.onStarted = [&](const std::string &) { ++started; };
    helper.onFinished = [&] { ++finished; };

    helper.login("lingmo-dm", "alice", true, 2);
    ASSERT_TRUE(runUntil([&] { return questions.size() == 1; }));
    EXPECT_EQ(questions.front(), "secret");
    helper.answer(Json("right"));
    ASSERT_TRUE(runUntil([&] { return authenticated == 1; }));
    EXPECT_TRUE(helper.isAuthenticated());
    helper.start({"/bin/true"}, {"XDG_VTNR=2"});
    ASSERT_TRUE(runUntil([&] { return started == 1; }));
    EXPECT_EQ(helper.sessionId(), "42");
    EXPECT_TRUE(helper.hasStarted());
    helper.terminate();
    EXPECT_TRUE(runUntil([&] { return finished == 1; }));
}

TEST_F(HelperTest, AHelperIgnoringTerminationIsKilledAtTheDeadline)
{
    setenv(kFakeHelper, "stubborn", 1);
    dm::HelperProcess helper;
    helper.setStopTimeout(300ms);
    int questions = 0;
    int authenticated = 0;
    int started = 0;
    int finished = 0;
    helper.onQuestion = [&](const std::string &, const std::string &) { ++questions; };
    helper.onAuthenticated = [&] { ++authenticated; };
    helper.onStarted = [&](const std::string &) { ++started; };
    helper.onFinished = [&] { ++finished; };

    helper.login("lingmo-dm", "alice", true, 2);
    ASSERT_TRUE(runUntil([&] { return questions == 1; }));
    helper.answer(Json("right"));
    ASSERT_TRUE(runUntil([&] { return authenticated == 1; }));
    helper.start({"/bin/true"}, {"XDG_VTNR=2"});
    ASSERT_TRUE(runUntil([&] { return started == 1; }));
    helper.terminate();
    runFor(150ms);
    EXPECT_EQ(finished, 0);
    EXPECT_TRUE(runUntil([&] { return finished == 1; }, 2s));
}

TEST_F(HelperTest, HelperReportsAWrongPassword)
{
    setenv(kFakeHelper, "1", 1);
    dm::HelperProcess helper;
    std::vector<bool> failed;
    int finished = 0;
    int questions = 0;
    helper.onFailed = [&](bool authentication, const std::string &) { failed.push_back(authentication); };
    helper.onFinished = [&] { ++finished; };
    helper.onQuestion = [&](const std::string &, const std::string &) { ++questions; };

    helper.login("lingmo-dm", "alice", true, 2);
    ASSERT_TRUE(runUntil([&] { return questions == 1; }));
    helper.answer(Json("wrong"));
    ASSERT_TRUE(runUntil([&] { return finished == 1; }));
    ASSERT_EQ(failed.size(), 1u);
    EXPECT_TRUE(failed.front());
    EXPECT_FALSE(helper.isAuthenticated());
}

TEST_F(HelperTest, AHelperThatCannotStartFailsAndFinishes)
{
    dm::HelperProcess::setProgram("/nonexistent/lingmo-dm-helper");
    dm::HelperProcess helper;
    std::vector<std::string> failures;
    int finished = 0;
    helper.onFailed = [&](bool, const std::string &message) { failures.push_back(message); };
    helper.onFinished = [&] { ++finished; };
    helper.login("lingmo-dm", "alice", true, 2);
    EXPECT_TRUE(failures.empty());
    ASSERT_TRUE(runUntil([&] { return finished == 1; }));
    EXPECT_EQ(failures, std::vector<std::string> {"The login helper did not start"});
}

// The daemon with fake helpers, the test as its login screen.
class DaemonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        dm::HelperProcess::setProgram(selfPath());
        setenv(kFakeHelper, "linger", 1);
        // The daemon's own D-Bus service is not under test.
        setenv("DBUS_SYSTEM_BUS_ADDRESS", "unix:path=/nonexistent/system_bus_socket", 1);

        config.greeterUser = getpwuid(getuid())->pw_name;
        config.greeterCommand = {"/bin/true"};
        config.sessionDirectories = {directory.path()};
        config.statePath = directory.path() + "/state.conf";
        config.runtimeDirectory = directory.path() + "/run";
        daemon = std::make_unique<dm::Daemon>(config);
        ASSERT_TRUE(daemon->start());
        ASSERT_TRUE(greeter.connectToPath(config.runtimeDirectory + "/greeter.sock"));
    }

    void TearDown() override
    {
        daemon.reset();
        unsetenv(kFakeHelper);
        unsetenv("DBUS_SYSTEM_BUS_ADDRESS");
    }

    // A new daemon with `config`, as after a restart in the same boot.
    void restart()
    {
        daemon.reset();
        daemon = std::make_unique<dm::Daemon>(config);
        ASSERT_TRUE(daemon->start());
    }

    void request(const Json &message) { greeter.write(dm::encodeFrame(message)); }

    // The next response, or null when none comes in time.
    Json response(std::chrono::milliseconds timeout = 5s)
    {
        std::optional<Json> next;
        runUntil(
                [&] {
                    reader.append(greeter.readAll());
                    next = reader.next();
                    return next.has_value();
                },
                timeout);
        return next.value_or(Json());
    }

    // create_session and the password, up to the success response.
    void logIn(const std::string &user)
    {
        request(Json::object({{"type", "create_session"}, {"username", user}}));
        ASSERT_EQ(response()["type"].toString(), "auth_message");
        request(Json::object({{"type", "post_auth_message_response"}, {"response", "right"}}));
        ASSERT_EQ(response()["type"].toString(), "success");
    }

    void startSession(const Json::Array &command)
    {
        request(Json::object({{"type", "start_session"}, {"cmd", command}, {"env", Json::Array {}}}));
        ASSERT_EQ(response()["type"].toString(), "success");
    }

    base::file::TemporaryDirectory directory {"/tmp", "dm-test"};
    dm::Config config;
    std::unique_ptr<dm::Daemon> daemon;
    base::LocalSocket greeter {g_loop};
    dm::FrameReader reader;
};

TEST_F(DaemonTest, AnAnswerAfterAuthenticationIsRefusedNotLeftUnanswered)
{
    logIn("alice");
    request(Json::object({{"type", "post_auth_message_response"}, {"response", "right"}}));
    const Json answer = response(2s);
    EXPECT_EQ(answer["type"].toString(), "error");

    // The login survives the stray answer.
    startSession(Json::Array {Json("/bin/sh")});
}

TEST_F(DaemonTest, ANewLoginDropsTheStartWaitingForThePreviousSession)
{
    logIn("alice");
    startSession(Json::Array {Json("/bin/sh")});

    // The first session takes half a second to end after the second login asks it to.
    logIn("alice");
    startSession(Json::Array {Json("/bin/sh")});

    // A third login replaces the second before the first session is gone.
    request(Json::object({{"type", "create_session"}, {"username", "alice"}}));
    ASSERT_EQ(response()["type"].toString(), "auth_message");
    runFor(1500ms);

    request(Json::object({{"type", "post_auth_message_response"}, {"response", "right"}}));
    EXPECT_EQ(response()["type"].toString(), "success");
}

TEST_F(DaemonTest, ASessionThatFailsToOpenAnswersNoLaterRequest)
{
    logIn("alice");
    startSession(Json::Array {Json("/bin/sh"), Json("--fail-open")});

    // In flight while the session fails; its answer is the password prompt a second later.
    request(Json::object({{"type", "create_session"}, {"username", "slow"}}));
    EXPECT_EQ(response()["type"].toString(), "auth_message");
}

TEST_F(DaemonTest, AutologinStartsTheSessionOncePerBoot)
{
    writeFile(directory.path() + "/lingmo-wayland.desktop", "[Desktop Entry]\nName=Lingmo\nExec=/bin/sh\n");
    config.autologinUser = "alice";
    restart();
    const auto loggedIn = [&] {
        return base::file::read(config.statePath).value_or("").find("alice=lingmo-wayland") != std::string::npos;
    };
    EXPECT_TRUE(runUntil(loggedIn));

    // The daemon came back in the same boot: the login screen, not another login.
    std::filesystem::remove(config.statePath);
    restart();
    runFor(1s);
    EXPECT_FALSE(loggedIn());
}

TEST_F(DaemonTest, AutologinToAMissingSessionLeavesTheLoginScreen)
{
    config.autologinUser = "alice";
    config.autologinSession = "missing";
    restart();
    runFor(500ms);
    EXPECT_FALSE(std::filesystem::exists(config.statePath));

    ASSERT_TRUE(greeter.connectToPath(config.runtimeDirectory + "/greeter.sock"));
    logIn("alice");
}

int main(int argc, char **argv)
{
    if (const char *mode = getenv(kFakeHelper))
        return fakeHelper(mode);

    base::EventLoop loop;
    g_loop = &loop;
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
