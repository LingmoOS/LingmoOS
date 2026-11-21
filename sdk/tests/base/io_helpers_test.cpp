// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <cstring>
#include <filesystem>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "base/byte_stream.h"
#include "base/event_loop.h"
#include "base/file.h"
#include "base/local_socket.h"
#include "base/lru_cache.h"
#include "base/timer.h"
#include "base/url.h"

using namespace Lingmo::Base;
using namespace std::chrono_literals;

TEST(ByteStreamTest, ReadsAndWritesBothOrders)
{
    ByteWriter little;
    little.write<uint32_t>(0x72756358).write<uint16_t>(0x0102).writeBytes("ab");
    EXPECT_EQ(little.bytes(), std::string("Xcur\x02\x01" "ab", 8));
    ByteWriter big(std::endian::big);
    big.write<uint16_t>(0x0102).write<int8_t>(-1);
    EXPECT_EQ(big.bytes(), std::string("\x01\x02\xff", 3));

    ByteReader reader(little.bytes());
    EXPECT_EQ(reader.read<uint32_t>(), 0x72756358u);
    EXPECT_EQ(reader.read<uint16_t>(), 0x0102);
    EXPECT_EQ(reader.readBytes(2), "ab");
    EXPECT_TRUE(reader.ok());
    EXPECT_TRUE(reader.atEnd());
    EXPECT_EQ(reader.read<uint8_t>(), 0);
    EXPECT_FALSE(reader.ok());
    // A failed stream stays failed.
    reader.seek(0);
    EXPECT_EQ(reader.read<uint8_t>(), 0);

    ByteReader bigReader(big.bytes(), std::endian::big);
    EXPECT_EQ(bigReader.read<uint16_t>(), 0x0102);
    EXPECT_EQ(bigReader.read<int8_t>(), -1);
    bigReader.seek(5);
    EXPECT_FALSE(bigReader.ok());
}

TEST(LruCacheTest, DropsTheLeastRecentlyUsed)
{
    LruCache<std::string, int> cache(3);
    cache.insert("a", std::make_unique<int>(1));
    cache.insert("b", std::make_unique<int>(2));
    cache.insert("c", std::make_unique<int>(3));
    ASSERT_NE(cache.find("a"), nullptr);
    cache.insert("d", std::make_unique<int>(4));
    EXPECT_FALSE(cache.contains("b"));
    EXPECT_EQ(*cache.find("a"), 1);
    EXPECT_EQ(cache.size(), 3u);
    EXPECT_TRUE(cache.insert("big", std::make_unique<int>(5), 2));
    EXPECT_EQ(cache.totalCost(), 3u);
    EXPECT_FALSE(cache.insert("huge", std::make_unique<int>(6), 4));
    EXPECT_FALSE(cache.contains("huge"));
    cache.insert("a", std::make_unique<int>(7));
    EXPECT_EQ(*cache.find("a"), 7);
    EXPECT_TRUE(cache.remove("a"));
    cache.setMaxCost(1);
    EXPECT_LE(cache.totalCost(), 1u);
    cache.clear();
    EXPECT_EQ(cache.size(), 0u);
}

TEST(UrlTest, LocalFiles)
{
    EXPECT_EQ(url::toLocalFile("file:///usr/share/a%20b.jpg"), "/usr/share/a b.jpg");
    EXPECT_EQ(url::toLocalFile("file://localhost/x"), "/x");
    EXPECT_EQ(url::toLocalFile("FILE:/x?y#z"), "/x");
    EXPECT_EQ(url::toLocalFile("file://server/x"), std::nullopt);
    EXPECT_EQ(url::toLocalFile("https://example.org/x"), std::nullopt);
    EXPECT_EQ(url::toLocalFile("/plain/path"), std::nullopt);
    EXPECT_TRUE(url::isLocalFile("file:///"));
    EXPECT_EQ(url::fromLocalFile("/a b/ü#"), "file:///a%20b/%C3%BC%23");
    EXPECT_EQ(url::toLocalFile(url::fromLocalFile("/a b/ü#")), "/a b/ü#");
    EXPECT_EQ(url::percentDecoded("100%"), "100%");
    EXPECT_EQ(url::percentDecoded("%zz%41"), "%zzA");
}

TEST(UrlTest, UriLists)
{
    EXPECT_EQ(url::parseUriList("file:///a.ics\r\n# a comment\r\n\r\n  file:///b%20c.ics  \r\nhttps://x/y"),
              (std::vector<std::string>{"file:///a.ics", "file:///b%20c.ics", "https://x/y"}));
    EXPECT_EQ(url::parseUriList("file:///a\n"), std::vector<std::string>{"file:///a"});
    EXPECT_TRUE(url::parseUriList("").empty());
    EXPECT_TRUE(url::parseUriList("\r\n \n").empty());
}

TEST(TemporaryTest, DirectoryAndFile)
{
    std::string directory;
    std::string file;
    {
        file::TemporaryDirectory temporary(std::filesystem::temp_directory_path().string(), "lingmo-test-");
        ASSERT_TRUE(temporary.isValid());
        directory = temporary.path();
        EXPECT_TRUE(std::filesystem::is_directory(directory));
        file::TemporaryFile temporaryFile(directory, "auth-");
        ASSERT_TRUE(temporaryFile.isValid());
        file = temporaryFile.path();
        EXPECT_TRUE(temporaryFile.write("contents"));
        EXPECT_EQ(file::read(file), "contents");
        EXPECT_EQ(std::filesystem::path(file).filename().string().substr(0, 5), "auth-");
    }
    EXPECT_FALSE(std::filesystem::exists(file));
    EXPECT_FALSE(std::filesystem::exists(directory));
    EXPECT_FALSE(file::TemporaryDirectory("/nonexistent/place").isValid());
}

TEST(LocalSocketTest, ExchangesAndDisconnects)
{
    EventLoop loop;
    int pair[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, pair), 0);
    LocalSocket socket(&loop);
    ASSERT_TRUE(socket.adopt(pair[0]));
    EXPECT_TRUE(socket.isConnected());
    std::string received;
    bool disconnected = false;
    socket.onReadyRead = [&] { received += socket.readAll(); };
    socket.onDisconnected = [&] {
        disconnected = true;
        loop.quit();
    };

    // A large write is queued and drained as the peer reads.
    const std::string big(1 << 20, 'x');
    EXPECT_TRUE(socket.write(big));
    std::string echoed;
    Timer reader(&loop);
    reader.onTimeout = [&] {
        char buffer[65536];
        const ssize_t n = ::recv(pair[1], buffer, sizeof(buffer), MSG_DONTWAIT);
        if (n > 0)
            echoed.append(buffer, size_t(n));
        if (echoed.size() == big.size()) {
            reader.stop();
            ::send(pair[1], "reply", 5, 0);
            ::close(pair[1]);
        }
    };
    reader.start(1ms);
    Timer timeout(&loop);
    timeout.setSingleShot(true);
    timeout.onTimeout = [&] { loop.quit(); };
    timeout.start(5s);
    loop.run();
    EXPECT_EQ(echoed.size(), big.size());
    EXPECT_EQ(received, "reply");
    EXPECT_TRUE(disconnected);
    EXPECT_FALSE(socket.isConnected());
    EXPECT_FALSE(socket.write("late"));
}

TEST(LocalSocketTest, ConnectsToAPath)
{
    EventLoop loop;
    file::TemporaryDirectory directory(std::filesystem::temp_directory_path().string(), "lingmo-socket-");
    const std::string path = directory.path() + "/socket";
    const int server = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    std::strcpy(address.sun_path, path.c_str());
    ASSERT_EQ(::bind(server, reinterpret_cast<sockaddr*>(&address), sizeof(address)), 0);
    ASSERT_EQ(::listen(server, 1), 0);

    LocalSocket socket(&loop);
    EXPECT_FALSE(socket.connectToPath(directory.path() + "/missing"));
    EXPECT_FALSE(socket.errorText().empty());
    ASSERT_TRUE(socket.connectToPath(path));
    const int peer = ::accept4(server, nullptr, nullptr, SOCK_CLOEXEC);
    ASSERT_GE(peer, 0);
    EXPECT_TRUE(socket.write("hello"));
    char buffer[8] = {};
    EXPECT_EQ(::recv(peer, buffer, sizeof(buffer), 0), 5);
    EXPECT_STREQ(buffer, "hello");
    socket.abort();
    EXPECT_FALSE(socket.isConnected());
    ::close(peer);
    ::close(server);
}

TEST(LocalServerTest, ListensOverAStaleSocketAndHandsOutClients)
{
    EventLoop loop;
    file::TemporaryDirectory directory(std::filesystem::temp_directory_path().string(), "lingmo-socket-");
    const std::string path = directory.path() + "/server";
    // A socket file left behind by a listener that is gone.
    {
        const int stale = ::socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
        sockaddr_un address{};
        address.sun_family = AF_UNIX;
        std::strcpy(address.sun_path, path.c_str());
        ASSERT_EQ(::bind(stale, reinterpret_cast<sockaddr*>(&address), sizeof(address)), 0);
        ::close(stale);
    }

    LocalServer server(&loop);
    EXPECT_FALSE(server.listen(directory.path() + "/missing/server"));
    EXPECT_FALSE(server.errorText().empty());
    ASSERT_TRUE(server.listen(path));
    EXPECT_TRUE(server.isListening());
    EXPECT_GE(server.fd(), 0);

    std::unique_ptr<LocalSocket> accepted;
    std::string received;
    server.onNewConnection = [&](int fd) {
        accepted = std::make_unique<LocalSocket>(&loop);
        ASSERT_TRUE(accepted->adopt(fd));
        accepted->onReadyRead = [&] {
            received += accepted->readAll();
            if (received == "hello")
                loop.quit();
        };
    };
    LocalSocket client(&loop);
    ASSERT_TRUE(client.connectToPath(path));
    EXPECT_TRUE(client.write("hello"));
    Timer timeout(&loop);
    timeout.setSingleShot(true);
    timeout.onTimeout = [&] { loop.quit(); };
    timeout.start(5s);
    loop.run();
    EXPECT_EQ(received, "hello");

    server.close();
    EXPECT_FALSE(server.isListening());
    EXPECT_FALSE(std::filesystem::exists(path));
}

TEST(LocalServerTest, LeavesAPathAnotherServerListensOn)
{
    EventLoop loop;
    file::TemporaryDirectory directory(std::filesystem::temp_directory_path().string(), "lingmo-socket-");
    const std::string path = directory.path() + "/server";
    LocalServer first(&loop);
    ASSERT_TRUE(first.listen(path));
    LocalServer second(&loop);
    EXPECT_FALSE(second.listen(path));
    EXPECT_FALSE(second.errorText().empty());
    EXPECT_FALSE(second.isListening());

    // The first one still gets its clients.
    bool connected = false;
    first.onNewConnection = [&](int fd) {
        connected = true;
        ::close(fd);
        loop.quit();
    };
    LocalSocket client(&loop);
    ASSERT_TRUE(client.connectToPath(path));
    Timer timeout(&loop);
    timeout.setSingleShot(true);
    timeout.onTimeout = [&] { loop.quit(); };
    timeout.start(5s);
    loop.run();
    EXPECT_TRUE(connected);
}
