// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testutil.h"

#include "core/standardfolders.h"
#include "model/pathbarmodel.h"
#include "model/places.h"

#include "base/dbus.h"
#include "base/url.h"
#include "lingmo/files/volume_manager.h"
#include "ui/application.h"

#include <gtest/gtest.h>
#include <systemd/sd-bus.h>

#include <filesystem>

using namespace Lingmo;
using base::dbus::Connection;
using base::dbus::MethodCall;
using base::dbus::ObjectPath;
using base::dbus::Variant;
using base::dbus::VariantMap;
using fm::PlacesModel;
using fmtest::TemporaryDir;

extern ui::Application* s_app;

TEST(PlacesModelTest, FavoritesAddMoveRemoveAndPersist)
{
    TemporaryDir config;
    TemporaryDir folders;
    ASSERT_TRUE(config.isValid() && folders.isValid());
    const std::string settings = config.filePath("filemanager.conf");
    ASSERT_TRUE(fmtest::writeFile(settings, "[Sidebar]\nfavorites=\n"));
    std::filesystem::create_directories(folders.filePath("first"));
    std::filesystem::create_directories(folders.filePath("second"));
    const std::string first = Lingmo::base::url::fromLocalFile(folders.filePath("first"));
    const std::string second = Lingmo::base::url::fromLocalFile(folders.filePath("second"));

    const auto checkTrash = [](const PlacesModel& places) {
        const PlacesModel::Place& trash = places.at(places.favoriteCount());
        EXPECT_EQ(trash.url, "trash:///");
        EXPECT_TRUE(trash.category.empty());
        EXPECT_FALSE(trash.isDevice());
    };

    PlacesModel model(false, settings);
    EXPECT_EQ(model.favoriteCount(), 0);
    checkTrash(model);
    EXPECT_FALSE(model.canAddFavorites({}));
    EXPECT_FALSE(model.addFavorites({"https://example.com"}, 0));
    EXPECT_FALSE(model.addFavorites({first, Lingmo::base::url::fromLocalFile(folders.filePath("missing"))}, 0));
    EXPECT_EQ(model.favoriteCount(), 0);
    EXPECT_TRUE(model.addFavorites({first, second}, 0));
    EXPECT_EQ(model.favoriteCount(), 2);
    checkTrash(model);
    // Already there: nothing is added twice.
    EXPECT_TRUE(model.addFavorites({first}, 0));
    EXPECT_EQ(model.favoriteCount(), 2);

    EXPECT_FALSE(model.moveFavorite(2, 0));
    EXPECT_FALSE(model.moveFavorite(0, 3));
    EXPECT_TRUE(model.moveFavorite(0, 2));
    EXPECT_EQ(model.at(0).url, second);
    EXPECT_TRUE(model.moveFavorite(1, 0));
    EXPECT_EQ(model.at(0).url, first);
    EXPECT_TRUE(model.moveFavorite(0, 1));
    model.removeFavorite(0);
    EXPECT_EQ(model.favoriteCount(), 1);
    // Removing a favourite leaves the folder alone.
    EXPECT_TRUE(std::filesystem::is_directory(folders.filePath("first")));

    {
        PlacesModel restored(false, settings);
        EXPECT_EQ(restored.favoriteCount(), 1);
        checkTrash(restored);
        EXPECT_EQ(restored.at(0).url, second);
    }
    model.removeFavorite(0);
    PlacesModel empty(false, settings);
    EXPECT_EQ(empty.favoriteCount(), 0);
    checkTrash(empty);
}

TEST(PlacesModelTest, PathCrumbsNameHomeAndStandardFolders)
{
    const std::string home = fm::homePath();
    const std::vector<fm::Crumb> crumbs = fm::pathCrumbs(Lingmo::base::url::fromLocalFile(home + "/a/b"));
    ASSERT_GE(crumbs.size(), 3u);
    EXPECT_EQ(crumbs.front().name, "Home");
    EXPECT_EQ(crumbs[crumbs.size() - 2].name, "a");
    EXPECT_EQ(crumbs.back().name, "b");
    EXPECT_EQ(fm::standardFolderName(home), "Home");
    EXPECT_TRUE(fm::standardFolderName(home + "/surely-not-a-standard-folder").empty());
}

static const std::string kService = "org.freedesktop.UDisks2";
static const std::string kRoot = "/org/freedesktop/UDisks2";
static const std::string kStick = kRoot + "/block_devices/sdb1";
static const std::string kCleartext = kRoot + "/block_devices/dm_2d0";

static std::vector<uint8_t> pathBytes(const std::string& path)
{
    std::vector<uint8_t> bytes(path.begin(), path.end());
    bytes.push_back(0);
    return bytes;
}

static VariantMap mountPoints(const std::string& path)
{
    std::vector<std::vector<uint8_t>> points;
    if (!path.empty())
        points.push_back(pathBytes(path));
    return {{"MountPoints", Variant(points)}};
}

static int rowOf(const PlacesModel& places, const std::string& udi)
{
    for (int row = 0; row < places.count(); ++row) {
        if (places.at(row).udi == udi)
            return row;
    }
    return -1;
}

static int managedObjects(sd_bus_message* raw, void* data, sd_bus_error*)
{
    const base::dbus::Message message(raw);
    if (message.interface() != "org.freedesktop.DBus.ObjectManager" || message.member() != "GetManagedObjects")
        return 0;
    MethodCall call(message);
    call.reply(*static_cast<files::VolumeManager::Objects*>(data));
    return 1;
}

TEST(PlacesModelTest, EncryptedAndInternalDisks)
{
    base::EventLoop& loop = s_app->loop();
    Connection* client = Connection::session(&loop);
    std::unique_ptr<Connection> service = Connection::open(Connection::Bus::Session, &loop);
    ASSERT_TRUE(client && service && service->requestName(kService));

    files::VolumeManager::Objects objects;
    objects[ObjectPath{kRoot + "/drives/usb"}][kService + ".Drive"] = {{"Removable", Variant(true)}};
    objects[ObjectPath{kStick}][kService + ".Block"] = {{"Drive", Variant(ObjectPath{kRoot + "/drives/usb"})},
                                                        {"IdLabel", Variant(std::string("Stick"))}};
    objects[ObjectPath{kStick}][kService + ".Encrypted"] = {{"CleartextDevice", Variant(ObjectPath{"/"})}};
    const auto internal = [&objects](const std::string& name, const std::string& mountPoint) {
        const ObjectPath path{kRoot + "/block_devices/" + name};
        objects[path][kService + ".Block"] = {{"IdLabel", Variant(name)}, {"HintSystem", Variant(true)}};
        objects[path][kService + ".Filesystem"] = mountPoints(mountPoint);
    };
    internal("Windows", "");
    internal("Root", "/");
    internal("Boot", "/boot/efi");

    sd_bus_slot* managerSlot = nullptr;
    ASSERT_GE(sd_bus_add_object(service->bus(), &managerSlot, kRoot.c_str(), managedObjects, &objects), 0);
    base::dbus::Slot manager(managerSlot);

    base::dbus::Object encrypted(service.get(), kStick, kService + ".Encrypted");
    encrypted.addMethod("Unlock", "sa{sv}", "o", [&](MethodCall& call) {
        std::string passphrase;
        if (!call.read(passphrase) || passphrase != "secret") {
            call.replyError("org.freedesktop.UDisks2.Error.Failed",
                            "Error unlocking /dev/sdb1: Failed to activate device: Operation not permitted");
            return;
        }
        objects[ObjectPath{kCleartext}][kService + ".Block"] = {{"CryptoBackingDevice", Variant(ObjectPath{kStick})}};
        objects[ObjectPath{kCleartext}][kService + ".Filesystem"] = mountPoints("");
        objects[ObjectPath{kStick}][kService + ".Encrypted"]["CleartextDevice"] = Variant(ObjectPath{kCleartext});
        call.reply(ObjectPath{kCleartext});
    });
    base::dbus::Object cleartext(service.get(), kCleartext, kService + ".Filesystem");
    cleartext.addMethod("Mount", "a{sv}", "s", [&](MethodCall& call) {
        objects[ObjectPath{kCleartext}][kService + ".Filesystem"] = mountPoints("/media/Stick");
        call.reply(std::string("/media/Stick"));
    });
    ASSERT_TRUE(encrypted.publish() && cleartext.publish());

    TemporaryDir config;
    PlacesModel places(true, config.filePath("filemanager.conf"), client);
    ASSERT_TRUE(fmtest::runUntil(loop, [&] { return rowOf(places, kStick) >= 0; }));

    const int stickRow = rowOf(places, kStick);
    const int windowsRow = rowOf(places, kRoot + "/block_devices/Windows");
    ASSERT_GE(windowsRow, 0);
    EXPECT_LT(stickRow, windowsRow);
    EXPECT_EQ(places.at(stickRow).category, "Locations");
    EXPECT_TRUE(places.at(stickRow).locked);
    EXPECT_TRUE(places.at(stickRow).setupNeeded());
    EXPECT_EQ(places.at(windowsRow).category, "Internal Drives");
    EXPECT_TRUE(places.at(windowsRow).setupNeeded());
    // Root and boot partitions are no places.
    EXPECT_EQ(rowOf(places, kRoot + "/block_devices/Root"), -1);
    EXPECT_EQ(rowOf(places, kRoot + "/block_devices/Boot"), -1);

    std::vector<std::tuple<std::string, std::string, bool>> asked;
    std::vector<std::string> opened, failed;
    const base::Subscription s1 = places.passphraseRequested.connect(
            [&](std::string udi, std::string name, bool retry) { asked.emplace_back(udi, name, retry); });
    const base::Subscription s2 = places.deviceSetupDone.connect([&](std::string path) { opened.push_back(path); });
    const base::Subscription s3 =
            places.deviceOperationFailed.connect([&](std::string message) { failed.push_back(message); });

    places.requestSetup(stickRow);
    ASSERT_EQ(asked.size(), 1u);
    EXPECT_EQ(asked.back(), std::make_tuple(kStick, std::string("Stick"), false));

    places.answerPassphrase(kStick, "wrong");
    ASSERT_TRUE(fmtest::runUntil(loop, [&] { return asked.size() == 2; }));
    EXPECT_EQ(asked.back(), std::make_tuple(kStick, std::string("Stick"), true));

    places.answerPassphrase(kStick, "secret");
    ASSERT_TRUE(fmtest::runUntil(loop, [&] { return opened.size() == 1; }));
    EXPECT_EQ(opened.back(), "/media/Stick");
    ASSERT_TRUE(fmtest::runUntil(loop, [&] { return !places.at(rowOf(places, kStick)).locked; }));
    const PlacesModel::Place& stick = places.at(rowOf(places, kStick));
    EXPECT_FALSE(stick.setupNeeded());
    EXPECT_EQ(stick.path(), "/media/Stick");
    EXPECT_TRUE(failed.empty());
}
