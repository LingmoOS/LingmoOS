// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <future>
#include <systemd/sd-bus.h>
#include <memory>
#include <thread>

#include "base/event_loop.h"
#include "lingmo/files/volume_manager.h"
#include "testing.h"

using namespace Lingmo;
using namespace Lingmo::Base;
using namespace Lingmo::Base::dbus;
using namespace Lingmo::Files;
using namespace std::chrono_literals;
using Interfaces = VolumeManager::Interfaces;

namespace {

const std::string service = "org.freedesktop.UDisks2";
const std::string root = "/org/freedesktop/UDisks2";
const std::string disk = root + "/block_devices/sdb1";
const std::string drive = root + "/drives/usb";
const std::string encryptedDisk = root + "/block_devices/sdc1";
const std::string cleartextDisk = root + "/block_devices/dm_2d0";
const std::string encryptedDrive = root + "/drives/usb_encrypted";

std::vector<uint8_t> bytes(const std::string& text)
{
    return std::vector<uint8_t>(text.begin(), text.end());
}

// UDisks2 on the session bus, on a thread of its own.
class FakeUDisks
{
public:
    FakeUDisks()
    {
        objects[ObjectPath{drive}][service + ".Drive"] = {
            {"Removable", Variant(true)}, {"Ejectable", Variant(true)}, {"Optical", Variant(true)}};
        objects[ObjectPath{disk}][service + ".Block"] = {
            {"Drive", Variant(ObjectPath{drive})},
            {"PreferredDevice", Variant(bytes(std::string("/dev/sdb1\0", 10)))},
            {"IdLabel", Variant(std::string("USB"))},
            {"Size", Variant(uint64_t(4096))}};
        objects[ObjectPath{disk}][service + ".Filesystem"] = {
            {"MountPoints", Variant(std::vector<std::vector<uint8_t>>())}};

        std::promise<bool> ready;
        m_thread = std::thread([this, &ready] {
            run(ready);
        });
        m_ready = ready.get_future().get();
    }

    ~FakeUDisks()
    {
        m_handle.post([this] {
            m_loop->quit();
        });
        m_thread.join();
    }

    bool isReady() const { return m_ready; }

    // A locked LUKS stick of its own drive; "secret" unlocks it.
    void addEncrypted()
    {
        objects[ObjectPath{encryptedDrive}][service + ".Drive"] = {
            {"Removable", Variant(true)}, {"Ejectable", Variant(true)}};
        objects[ObjectPath{encryptedDisk}][service + ".Block"] = {
            {"Drive", Variant(ObjectPath{encryptedDrive})},
            {"PreferredDevice", Variant(bytes(std::string("/dev/sdc1\0", 10)))},
            {"Size", Variant(uint64_t(4096))}};
        objects[ObjectPath{encryptedDisk}][service + ".Encrypted"] = {{"CleartextDevice", Variant(ObjectPath{"/"})}};
    }

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

    VolumeManager::Objects objects;
    std::vector<std::string> calls;
    // Whether every call so far let the service ask for authorization.
    bool interactive = true;
    bool fail = false;
    std::unique_ptr<Connection> connection;

private:
    void answer(MethodCall& call, const std::string& member, bool mount)
    {
        calls.push_back(member);
        interactive = interactive && call.message().allowsInteractiveAuthorization();

        if (call.message().signature() != "a{sv}")
            call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Expected options map");
        else if (fail)
            call.replyError("org.freedesktop.UDisks2.Error.NotAuthorized", "Permission denied");
        else if (mount)
            call.reply(std::string("/media/USB Café"));
        else
            call.reply();
    }

    void run(std::promise<bool>& ready)
    {
        EventLoop loop;
        m_loop = &loop;
        m_handle = loop.handle();
        connection = Connection::open(Connection::Bus::Session, &loop);
        if (!connection || !connection->requestName(service)) {
            ready.set_value(false);
            return;
        }

        // sd-bus keeps ObjectManager to itself; a callback for the path answers first.
        sd_bus_slot* managerSlot = nullptr;
        const auto getManagedObjects = [](sd_bus_message* raw, void* data, sd_bus_error*) -> int {
            const Message message(raw);
            if (message.interface() != "org.freedesktop.DBus.ObjectManager" || message.member() != "GetManagedObjects")
                return 0;
            MethodCall call(message);
            call.reply(static_cast<FakeUDisks*>(data)->objects);
            return 1;
        };
        const bool managed = sd_bus_add_object(connection->bus(), &managerSlot, root.c_str(), getManagedObjects, this) >= 0;
        Slot manager(managerSlot);

        auto filesystem = std::make_unique<Object>(connection.get(), disk, service + ".Filesystem");
        filesystem->addMethod("Mount", "a{sv}", "s", [this](MethodCall& call) {
            answer(call, "Mount", true);
        });
        filesystem->addMethod("Unmount", "a{sv}", "", [this](MethodCall& call) {
            answer(call, "Unmount", false);
        });

        auto driveObject = std::make_unique<Object>(connection.get(), drive, service + ".Drive");
        driveObject->addMethod("Eject", "a{sv}", "", [this](MethodCall& call) {
            answer(call, "Eject", false);
        });
        driveObject->addMethod("PowerOff", "a{sv}", "", [this](MethodCall& call) {
            answer(call, "PowerOff", false);
        });

        auto encrypted = std::make_unique<Object>(connection.get(), encryptedDisk, service + ".Encrypted");
        encrypted->addMethod("Unlock", "sa{sv}", "o", [this](MethodCall& call) {
            calls.push_back("Unlock");
            interactive = interactive && call.message().allowsInteractiveAuthorization();
            std::string passphrase;
            if (!call.read(passphrase) || passphrase != "secret") {
                call.replyError("org.freedesktop.UDisks2.Error.Failed",
                                "Error unlocking /dev/sdc1: Failed to activate device: Operation not permitted");
                return;
            }
            Interfaces cleartext;
            cleartext[service + ".Block"] = {
                {"CryptoBackingDevice", Variant(ObjectPath{encryptedDisk})},
                {"PreferredDevice", Variant(bytes(std::string("/dev/dm-0\0", 10)))},
                {"IdLabel", Variant(std::string("Secret"))}};
            cleartext[service + ".Filesystem"] = {{"MountPoints", Variant(std::vector<std::vector<uint8_t>>())}};
            objects[ObjectPath{cleartextDisk}] = cleartext;
            objects[ObjectPath{encryptedDisk}][service + ".Encrypted"]["CleartextDevice"] =
                Variant(ObjectPath{cleartextDisk});
            connection->emitSignal(root, "org.freedesktop.DBus.ObjectManager", "InterfacesAdded",
                                   ObjectPath{cleartextDisk}, cleartext);
            call.reply(ObjectPath{cleartextDisk});
        });
        encrypted->addMethod("Lock", "a{sv}", "", [this](MethodCall& call) {
            answer(call, "Lock", false);
            objects.erase(ObjectPath{cleartextDisk});
            objects[ObjectPath{encryptedDisk}][service + ".Encrypted"]["CleartextDevice"] = Variant(ObjectPath{"/"});
            connection->emitSignal(root, "org.freedesktop.DBus.ObjectManager", "InterfacesRemoved",
                                   ObjectPath{cleartextDisk},
                                   std::vector<std::string>{service + ".Block", service + ".Filesystem"});
        });

        auto cleartext = std::make_unique<Object>(connection.get(), cleartextDisk, service + ".Filesystem");
        cleartext->addMethod("Mount", "a{sv}", "s", [this](MethodCall& call) {
            calls.push_back("Mount");
            interactive = interactive && call.message().allowsInteractiveAuthorization();
            auto& filesystem = objects[ObjectPath{cleartextDisk}][service + ".Filesystem"];
            filesystem["MountPoints"] =
                Variant(std::vector<std::vector<uint8_t>>{bytes(std::string("/media/Secret") + '\0')});
            connection->emitSignal(cleartextDisk, "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                   service + ".Filesystem", filesystem, std::vector<std::string>());
            call.reply(std::string("/media/Secret"));
        });
        cleartext->addMethod("Unmount", "a{sv}", "", [this](MethodCall& call) {
            answer(call, "Unmount", false);
            auto& filesystem = objects[ObjectPath{cleartextDisk}][service + ".Filesystem"];
            filesystem["MountPoints"] = Variant(std::vector<std::vector<uint8_t>>());
            connection->emitSignal(cleartextDisk, "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                   service + ".Filesystem", filesystem, std::vector<std::string>());
        });

        auto encryptedDriveObject = std::make_unique<Object>(connection.get(), encryptedDrive, service + ".Drive");
        encryptedDriveObject->addMethod("Eject", "a{sv}", "", [this](MethodCall& call) {
            answer(call, "Eject", false);
        });

        ready.set_value(managed && filesystem->publish() && driveObject->publish() && encrypted->publish()
                        && cleartext->publish() && encryptedDriveObject->publish());
        loop.run();
        manager.reset();
        filesystem.reset();
        driveObject.reset();
        encrypted.reset();
        cleartext.reset();
        encryptedDriveObject.reset();
        connection.reset();
    }

    std::thread m_thread;
    EventLoop* m_loop = nullptr;
    LoopHandle m_handle;
    bool m_ready = false;
};

} // namespace

TEST(VolumeManagerTest, LifecycleAndOperations)
{
    EventLoop loop;
    Connection* bus = Connection::session(&loop);
    if (!bus)
        GTEST_SKIP() << "No session bus; run the tests in dbus-run-session";

    FakeUDisks fake;
    ASSERT_TRUE(fake.isReady());

    VolumeManager manager(bus);
    std::vector<std::string> added, changed, removed;
    std::vector<std::tuple<std::string, std::string, std::string>> done, errors;
    const Subscription s1 = manager.volumeAdded.connect([&](const std::string& id) { added.push_back(id); });
    const Subscription s2 = manager.volumeChanged.connect([&](const std::string& id) { changed.push_back(id); });
    const Subscription s3 = manager.volumeRemoved.connect([&](const std::string& id) { removed.push_back(id); });
    const Subscription s4 = manager.operationFinished.connect(
        [&](const std::string& id, const std::string& operation, const std::string& point) {
            done.emplace_back(id, operation, point);
        });
    const Subscription s5 = manager.operationFailed.connect(
        [&](const std::string& id, const std::string& operation, const std::string& message) {
            errors.emplace_back(id, operation, message);
        });

    using files::testing::waitFor;

    ASSERT_TRUE(waitFor(loop, [&] { return added.size() == 1; }));
    EXPECT_EQ(manager.volume(disk).device, "/dev/sdb1");
    EXPECT_EQ(manager.volume(disk).name, "USB");
    EXPECT_TRUE(manager.volume(disk).optical);

    manager.mount(disk);
    ASSERT_TRUE(waitFor(loop, [&] { return done.size() == 1; }));
    EXPECT_EQ(std::get<2>(done.back()), "/media/USB Café");

    fake.inService([&] { fake.fail = true; });
    manager.unmount(disk);
    ASSERT_TRUE(waitFor(loop, [&] { return errors.size() == 1; }));
    EXPECT_EQ(done.size(), 1u);

    fake.inService([&] {
        fake.fail = false;
        auto& filesystem = fake.objects[ObjectPath{disk}][service + ".Filesystem"];
        filesystem["MountPoints"] = Variant(std::vector<std::vector<uint8_t>>{bytes(std::string("/media/USB Café") + '\0')});
        fake.connection->emitSignal(disk, "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                    service + ".Filesystem", filesystem, std::vector<std::string>());
    });
    ASSERT_TRUE(waitFor(loop, [&] { return !changed.empty(); }));
    EXPECT_EQ(manager.volume(disk).mountPoints, std::vector<std::string>{"/media/USB Café"});

    fake.inService([&] { fake.calls.clear(); });
    manager.eject(disk);
    ASSERT_TRUE(waitFor(loop, [&] { return done.size() == 2; }));
    EXPECT_EQ(fake.inService([&] { return fake.calls; }), (std::vector<std::string>{"Unmount", "Eject"}));

    fake.inService([&] {
        fake.fail = true;
        fake.calls.clear();
    });
    manager.eject(disk);
    ASSERT_TRUE(waitFor(loop, [&] { return errors.size() == 2; }));
    EXPECT_EQ(fake.inService([&] { return fake.calls; }), (std::vector<std::string>{"Unmount"}));

    manager.mount("/missing");
    EXPECT_EQ(errors.size(), 3u);

    fake.inService([&] {
        fake.fail = false;
        auto& driveProperties = fake.objects[ObjectPath{drive}][service + ".Drive"];
        driveProperties["Ejectable"] = Variant(false);
        driveProperties["CanPowerOff"] = Variant(true);
        fake.connection->emitSignal(drive, "org.freedesktop.DBus.Properties", "PropertiesChanged", service + ".Drive",
                                    driveProperties, std::vector<std::string>());
    });
    ASSERT_TRUE(waitFor(loop, [&] { return manager.volume(disk).canPowerOff; }));

    fake.inService([&] { fake.calls.clear(); });
    manager.eject(disk);
    manager.mount(disk);
    EXPECT_EQ(errors.size(), 4u);
    ASSERT_TRUE(waitFor(loop, [&] { return done.size() == 3; }));
    EXPECT_EQ(fake.inService([&] { return fake.calls; }), (std::vector<std::string>{"Unmount", "PowerOff"}));

    fake.inService([&] { fake.connection->releaseName(service); });
    ASSERT_TRUE(waitFor(loop, [&] { return removed.size() == 1; }));
    EXPECT_TRUE(manager.volumes().empty());

    fake.inService([&] { fake.connection->requestName(service); });
    ASSERT_TRUE(waitFor(loop, [&] { return added.size() == 2; }));

    fake.inService([&] {
        fake.objects.erase(ObjectPath{disk});
        fake.connection->emitSignal(root, "org.freedesktop.DBus.ObjectManager", "InterfacesRemoved", ObjectPath{disk},
                                    std::vector<std::string>{service + ".Block", service + ".Filesystem"});
    });
    ASSERT_TRUE(waitFor(loop, [&] { return removed.size() == 2; }));
    EXPECT_TRUE(manager.volumes().empty());
}

TEST(VolumeManagerTest, EncryptedVolumeUnlocksMountsAndLocks)
{
    EventLoop loop;
    Connection* bus = Connection::session(&loop);
    if (!bus)
        GTEST_SKIP() << "No session bus; run the tests in dbus-run-session";

    FakeUDisks fake;
    ASSERT_TRUE(fake.isReady());
    fake.inService([&] { fake.addEncrypted(); });

    VolumeManager manager(bus);
    std::vector<std::string> rejected;
    std::vector<std::tuple<std::string, std::string, std::string>> done, errors;
    const Subscription s1 = manager.passphraseRejected.connect([&](const std::string& id) { rejected.push_back(id); });
    const Subscription s2 = manager.operationFinished.connect(
        [&](const std::string& id, const std::string& operation, const std::string& point) {
            done.emplace_back(id, operation, point);
        });
    const Subscription s3 = manager.operationFailed.connect(
        [&](const std::string& id, const std::string& operation, const std::string& message) {
            errors.emplace_back(id, operation, message);
        });

    using files::testing::waitFor;

    ASSERT_TRUE(waitFor(loop, [&] { return !manager.volume(encryptedDisk).id.empty(); }));
    Volume locked = manager.volume(encryptedDisk);
    EXPECT_TRUE(locked.encrypted);
    EXPECT_TRUE(locked.locked());
    EXPECT_FALSE(locked.filesystem);
    EXPECT_TRUE(locked.name.ends_with(" Encrypted Volume")) << locked.name;

    manager.mount(encryptedDisk);
    ASSERT_EQ(errors.size(), 1u);
    EXPECT_EQ(std::get<1>(errors.back()), "mount");

    manager.unlock(encryptedDisk, "wrong");
    ASSERT_TRUE(waitFor(loop, [&] { return rejected.size() == 1; }));
    EXPECT_EQ(errors.size(), 1u);
    EXPECT_TRUE(manager.volume(encryptedDisk).locked());

    fake.inService([&] { fake.calls.clear(); });
    manager.unlock(encryptedDisk, "secret");
    ASSERT_TRUE(waitFor(loop, [&] { return done.size() == 1; }));
    EXPECT_EQ(done.back(), std::make_tuple(encryptedDisk, std::string("unlock"), std::string("/media/Secret")));
    EXPECT_EQ(fake.inService([&] { return fake.calls; }), (std::vector<std::string>{"Unlock", "Mount"}));
    ASSERT_TRUE(waitFor(loop, [&] { return !manager.volume(encryptedDisk).mountPoints.empty(); }));

    // The cleartext block is the encrypted volume, not one of its own.
    const Volume unlocked = manager.volume(encryptedDisk);
    EXPECT_EQ(unlocked.cleartext, cleartextDisk);
    EXPECT_TRUE(unlocked.filesystem);
    EXPECT_EQ(unlocked.name, "Secret");
    EXPECT_EQ(unlocked.mountPoints, std::vector<std::string>{"/media/Secret"});
    EXPECT_TRUE(manager.volume(cleartextDisk).id.empty());
    EXPECT_EQ(manager.volumes().size(), 2u);

    fake.inService([&] { fake.calls.clear(); });
    manager.unmount(encryptedDisk);
    ASSERT_TRUE(waitFor(loop, [&] { return done.size() == 2 && manager.volume(encryptedDisk).locked(); }));
    EXPECT_EQ(fake.inService([&] { return fake.calls; }), (std::vector<std::string>{"Unmount", "Lock"}));

    manager.unlock(encryptedDisk, "secret");
    ASSERT_TRUE(waitFor(loop, [&] { return done.size() == 3 && !manager.volume(encryptedDisk).mountPoints.empty(); }));
    fake.inService([&] { fake.calls.clear(); });
    manager.eject(encryptedDisk);
    ASSERT_TRUE(waitFor(loop, [&] { return done.size() == 4; }));
    EXPECT_EQ(fake.inService([&] { return fake.calls; }), (std::vector<std::string>{"Unmount", "Lock", "Eject"}));
    EXPECT_EQ(errors.size(), 1u);
    EXPECT_TRUE(fake.inService([&] { return fake.interactive; }));
}

TEST(VolumeManagerTest, SystemMountPointsAreLeftOut)
{
    EventLoop loop;
    Connection* bus = Connection::session(&loop);
    if (!bus)
        GTEST_SKIP() << "No session bus; run the tests in dbus-run-session";

    FakeUDisks fake;
    ASSERT_TRUE(fake.isReady());

    const auto mounted = [](const std::string& point) {
        return VariantMap{{"MountPoints", Variant(std::vector<std::vector<uint8_t>>{bytes(point + '\0')})}};
    };
    const auto internal = [&](const std::string& name, bool system, const std::string& point) {
        Interfaces& object = fake.objects[ObjectPath{root + "/block_devices/" + name}];
        object[service + ".Block"] = {{"PreferredDevice", Variant(bytes("/dev/" + name + '\0'))},
                                      {"IdLabel", Variant(name)},
                                      {"HintSystem", Variant(system)},
                                      {"Size", Variant(uint64_t(4096))}};
        object[service + ".Filesystem"] = point.empty() ? VariantMap{{"MountPoints", Variant(std::vector<std::vector<uint8_t>>())}}
                                                        : mounted(point);
    };
    fake.inService([&] {
        internal("root", true, "/");
        internal("efi", true, "/boot/efi");
        internal("home", true, "/home");
        internal("var", true, "/var");
        internal("windows", true, "");
        internal("data", true, "/mnt/data");
        internal("stick", false, "/run/media/user/Stick");
    });

    VolumeManager manager(bus);
    std::vector<std::string> removed;
    const Subscription s1 = manager.volumeRemoved.connect([&](const std::string& id) { removed.push_back(id); });

    using files::testing::waitFor;

    ASSERT_TRUE(waitFor(loop, [&] { return manager.volumes().size() == 4; }));
    std::vector<std::string> names;
    for (const Volume& volume : manager.volumes())
        names.push_back(volume.name);
    EXPECT_EQ(names, (std::vector<std::string>{"data", "USB", "stick", "windows"}));
    // A Windows partition stays: it takes an administrator to mount, nothing more.
    EXPECT_TRUE(manager.volume(root + "/block_devices/windows").system);
    EXPECT_FALSE(manager.volume(root + "/block_devices/stick").system);

    fake.inService([&] {
        const std::string stick = root + "/block_devices/stick";
        auto& filesystem = fake.objects[ObjectPath{stick}][service + ".Filesystem"];
        filesystem = mounted("/usr");
        fake.connection->emitSignal(stick, "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                    service + ".Filesystem", filesystem, std::vector<std::string>());
    });
    ASSERT_TRUE(waitFor(loop, [&] { return removed.size() == 1; }));
    EXPECT_EQ(removed.back(), root + "/block_devices/stick");
}
