// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <map>
#include <memory>
#include <thread>

#include "base/event_loop.h"
#include "lingmo/accounts/accounts_manager.h"
#include "lingmo/accounts/user_account.h"

using namespace Lingmo;
using namespace Lingmo::Accounts;
using namespace Lingmo::Base;
using namespace Lingmo::Base::dbus;
using namespace std::chrono_literals;

namespace {

constexpr const char* kService = "org.freedesktop.Accounts";
constexpr const char* kUserInterface = "org.freedesktop.Accounts.User";

// org.freedesktop.Accounts on the session bus, on a thread of its own.
class FakeAccountsService
{
public:
    FakeAccountsService()
    {
        std::promise<bool> ready;
        m_thread = std::thread([this, &ready] { run(ready); });
        m_ready = ready.get_future().get();
    }
    ~FakeAccountsService()
    {
        m_handle.post([this] { m_loop->quit(); });
        m_thread.join();
    }

    bool isReady() const { return m_ready; }
    // Read on the service's thread.
    template<typename F>
    auto inService(F f)
    {
        std::promise<decltype(f())> result;
        m_handle.post([&] {
            if constexpr (std::is_void_v<decltype(f())>) {
                f();
                result.set_value();
            } else {
                result.set_value(f());
            }
        });
        return result.get_future().get();
    }

    struct User
    {
        std::string name;
        std::string realName;
        int32_t accountType = 0;
        std::string password;
        std::unique_ptr<Object> object;
    };

    std::map<int64_t, User> users;
    bool failSetRealName = false;
    bool createAskedInteractively = false;
    bool setPasswordAskedInteractively = false;

private:
    static std::string path(int64_t uid) { return "/org/freedesktop/Accounts/User" + std::to_string(uid); }

    void addUser(int64_t uid, std::string name, std::string realName)
    {
        User& user = users[uid];
        user.name = std::move(name);
        user.realName = std::move(realName);
        user.object = std::make_unique<Object>(m_connection, path(uid), kUserInterface);
        Object& object = *user.object;
        object.addProperty("Uid", [uid] { return uint64_t(uid); });
        object.addProperty("UserName", [this, uid] { return users[uid].name; });
        object.addProperty("RealName", [this, uid] { return users[uid].realName; });
        object.addProperty("AccountType", [this, uid] { return users[uid].accountType; });
        object.addProperty("IconFile", [] { return std::string("/nonexistent/icon"); });
        object.addMethod("SetRealName", "s", "", [this, uid](MethodCall& call) {
            std::string name;
            call.read(name);
            // Answers a little later, as a real service waiting for polkit does.
            m_loop->startTimer(30ms, false, [this, uid, call, name]() mutable {
                if (failSetRealName) {
                    call.replyError("org.freedesktop.Accounts.Error.PermissionDenied", "Not allowed");
                    return;
                }
                users[uid].realName = name;
                users[uid].object->emitSignal("Changed");
                call.reply();
            });
        });
        object.addMethod("SetPassword", "ss", "", [this, uid](MethodCall& call) {
            std::string password, hint;
            call.read(password, hint);
            users[uid].password = password;
            setPasswordAskedInteractively = call.message().allowsInteractiveAuthorization();
            call.reply();
        });
        object.addSignal("Changed", "");
        object.publish();
    }

    void run(std::promise<bool>& ready)
    {
        EventLoop loop;
        m_loop = &loop;
        m_handle = loop.handle();
        std::unique_ptr<Connection> connection = Connection::open(Connection::Bus::Session, &loop);
        if (!connection || !connection->requestName(kService)) {
            ready.set_value(false);
            return;
        }
        m_connection = connection.get();
        addUser(1000, "lingmo", "Lingmo User");
        addUser(1001, "guest", "");

        Object manager(m_connection, "/org/freedesktop/Accounts", "org.freedesktop.Accounts");
        manager.addMethod("ListCachedUsers", [this] {
            std::vector<ObjectPath> paths;
            for (const auto& [uid, user] : users)
                paths.push_back({path(uid)});
            return paths;
        });
        manager.addMethod("FindUserById", "x", "o", [this](MethodCall& call) {
            int64_t uid = 0;
            call.read(uid);
            if (!users.contains(uid))
                call.replyError("org.freedesktop.Accounts.Error.Failed", "No such user");
            else
                call.reply(ObjectPath{path(uid)});
        });
        manager.addMethod("CreateUser", "ssi", "o", [this, &manager](MethodCall& call) {
            std::string name, realName;
            int32_t type = 0;
            call.read(name, realName, type);
            createAskedInteractively = call.message().allowsInteractiveAuthorization();
            if (name == "taken") {
                call.replyError("org.freedesktop.Accounts.Error.Failed", "A user with name 'taken' already exists");
                return;
            }
            const int64_t uid = 1000 + int64_t(users.size());
            addUser(uid, name, realName);
            users[uid].accountType = type;
            manager.emitSignal("UserAdded", ObjectPath{path(uid)});
            call.reply(ObjectPath{path(uid)});
        });
        manager.addMethod("DeleteUser", "xb", "", [this, &manager](MethodCall& call) {
            int64_t uid = 0;
            bool removeFiles = false;
            call.read(uid, removeFiles);
            users.erase(uid);
            manager.emitSignal("UserDeleted", ObjectPath{path(uid)});
            call.reply();
        });
        manager.addSignal("UserAdded", "o");
        manager.addSignal("UserDeleted", "o");
        ready.set_value(manager.publish());
        loop.run();
        users.clear();
        m_connection = nullptr;
    }

    std::thread m_thread;
    EventLoop* m_loop = nullptr;
    LoopHandle m_handle;
    Connection* m_connection = nullptr;
    bool m_ready = false;
};

class AccountsTest : public testing::Test
{
protected:
    void SetUp() override
    {
        if (!Connection::session(&m_loop))
            GTEST_SKIP() << "No session bus; run the tests in dbus-run-session";
        m_service = std::make_unique<FakeAccountsService>();
        ASSERT_TRUE(m_service->isReady());
    }

    // Runs the loop until the condition holds or two seconds pass.
    bool waitFor(const std::function<bool()>& condition)
    {
        const auto deadline = std::chrono::steady_clock::now() + 2s;
        while (!condition()) {
            if (std::chrono::steady_clock::now() > deadline)
                return false;
            runFor(5ms);
        }
        return true;
    }

    void runFor(std::chrono::milliseconds time)
    {
        const int timer = m_loop.startTimer(time, false, [this] { m_loop.quit(); });
        m_loop.run();
        m_loop.stopTimer(timer);
    }

    Connection* bus() { return Connection::session(&m_loop); }

    EventLoop m_loop;
    std::unique_ptr<FakeAccountsService> m_service;
};

} // namespace

TEST_F(AccountsTest, ListsAccountsAndReadsTheirValues)
{
    AccountsManager manager(bus());
    int changes = 0;
    Subscription subscription = manager.accountsChanged.connect([&] { ++changes; });
    ASSERT_TRUE(waitFor([&] { return manager.isLoaded(); }));
    EXPECT_EQ(changes, 1);
    ASSERT_EQ(manager.accounts().size(), 2u);
    const auto& reion = manager.accounts()[0];
    // Known from the path at once; the rest once the read answers.
    EXPECT_EQ(reion->userId(), 1000);
    ASSERT_TRUE(waitFor([&] { return reion->userName() == "reion"; }));
    EXPECT_EQ(reion->displayName(), "Lingmo User");
    const auto& guest = manager.accounts()[1];
    ASSERT_TRUE(waitFor([&] { return guest->userName() == "guest"; }));
    EXPECT_EQ(guest->displayName(), "guest");
    // A picture that is not there is no picture.
    EXPECT_EQ(guest->iconFileName(), "");
    EXPECT_EQ(manager.findUserById(1001), guest);
    EXPECT_FALSE(manager.findUserById(4242));
}

TEST_F(AccountsTest, ASetValueShowsUntilTheServiceAnswers)
{
    UserAccount account(1000, bus());
    ASSERT_TRUE(waitFor([&] { return account.realName() == "Lingmo User"; }));
    int changes = 0;
    Subscription subscription = account.changed.connect([&] { ++changes; });
    account.setRealName("Reion");
    EXPECT_EQ(account.realName(), "Reion");
    EXPECT_EQ(changes, 1);
    // The service's Changed re-reads, and the reply ends the pending value.
    ASSERT_TRUE(waitFor([&] { return m_service->inService([&] { return m_service->users[1000].realName; }) == "Reion"; }));
    runFor(100ms);
    EXPECT_EQ(account.realName(), "Reion");
}

TEST_F(AccountsTest, ARefusedValueGoesBack)
{
    m_service->inService([&] { m_service->failSetRealName = true; });
    UserAccount account(1000, bus());
    ASSERT_TRUE(waitFor([&] { return account.realName() == "Lingmo User"; }));
    account.setRealName("Someone");
    EXPECT_EQ(account.realName(), "Someone");
    EXPECT_TRUE(waitFor([&] { return account.realName() == "Lingmo User"; }));
}

TEST_F(AccountsTest, CreatesAndDeletesAccounts)
{
    AccountsManager manager(bus());
    ASSERT_TRUE(waitFor([&] { return manager.isLoaded(); }));
    int64_t deleted = -1;
    Subscription subscription = manager.userDeleted.connect([&](int64_t uid) { deleted = uid; });

    AccountsManager::AccountPtr created;
    std::string error = "unset";
    manager.createUser("alice", "", UserAccount::Administrator,
                       [&](const AccountsManager::AccountPtr& account, const std::string& message) {
                           created = account;
                           error = message;
                       });
    ASSERT_TRUE(waitFor([&] { return error != "unset"; }));
    EXPECT_EQ(error, "");
    ASSERT_TRUE(created);
    EXPECT_TRUE(m_service->inService([&] { return m_service->createAskedInteractively; }));
    EXPECT_EQ(manager.accounts().size(), 3u);
    EXPECT_EQ(manager.accounts().back(), created);
    ASSERT_TRUE(waitFor([&] { return created->userName() == "alice"; }));
    EXPECT_EQ(created->accountType(), UserAccount::Administrator);

    created->setPassword("$6$salt$hash");
    ASSERT_TRUE(waitFor([&] { return m_service->inService([&] { return m_service->users[1002].password; }) == "$6$salt$hash"; }));
    EXPECT_TRUE(m_service->inService([&] { return m_service->setPasswordAskedInteractively; }));

    std::string deleteError = "unset";
    manager.deleteUser(created->userId(), true, [&](const std::string& message) { deleteError = message; });
    ASSERT_TRUE(waitFor([&] { return deleteError != "unset" && deleted >= 0; }));
    EXPECT_EQ(deleteError, "");
    EXPECT_EQ(deleted, 1002);
    EXPECT_EQ(manager.accounts().size(), 2u);
}

TEST_F(AccountsTest, AFailedCreationReportsTheServicesMessage)
{
    AccountsManager manager(bus());
    std::string error;
    bool done = false;
    manager.createUser("taken", "", UserAccount::Standard,
                       [&](const AccountsManager::AccountPtr& account, const std::string& message) {
                           EXPECT_FALSE(account);
                           error = message;
                           done = true;
                       });
    ASSERT_TRUE(waitFor([&] { return done; }));
    EXPECT_EQ(error, "A user with name 'taken' already exists");
}
