// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "base/dbus.h"
#include "base/signal.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::Files {
class VolumeManager;
struct Volume;
}

namespace fm {

// PlacesModel: the sidebar's rows. Favourites (kept in filemanager.conf
// [Sidebar] favorites), the trash, removable disks under Locations, internal
// ones under Internal Drives, and the phones and servers lingmo-mountd
// serves, under Locations and Network.
class PlacesModel
{
public:
    struct Place
    {
        std::string name;
        std::string iconName;
        // file:// or trash:///; empty for a disk that is not mounted.
        std::string url;
        std::string category;
        // A disk's UDisks object path, or a mount's URL; empty for a folder.
        std::string udi;
        bool opticalDisc = false;
        bool canEject = false;
        bool hasFilesystem = false;
        // An encrypted disk that takes a passphrase before it can be mounted.
        bool locked = false;
        bool accessible = false;

        bool isDevice() const { return !udi.empty(); }
        bool setupNeeded() const { return (hasFilesystem || locked) && !accessible; }
        // A local path, or the URL for the trash.
        std::string path() const;
        // The name shown: "Computer" for the root folder.
        std::string displayName() const;
    };

    // Disks from UDisks2 on the system bus unless another bus is given, or
    // none (tests). filemanager.conf unless another settings file is given
    // (tests).
    explicit PlacesModel(bool volumes = true, std::string settingsFile = {},
                         base::dbus::Connection* volumeBus = nullptr);
    ~PlacesModel();

    int count() const { return int(m_items.size()); }
    const Place& at(int row) const { return m_items[size_t(row)]; }
    // The standard folders' and the trash's names in the language of now.
    void retranslate();
    int favoriteCount() const;

    bool canAddFavorites(const std::vector<std::string>& urls) const;
    bool addFavorites(const std::vector<std::string>& urls, int before);
    bool moveFavorite(int from, int before);
    bool canDropInto(const std::vector<std::string>& urls, int row, uint32_t action) const;
    bool dropInto(const std::vector<std::string>& urls, int row, uint32_t action);
    void removeFavorite(int row);

    // A locked disk asks for its passphrase through passphraseRequested.
    void requestSetup(int row);
    void requestEject(int row);
    void requestEjectById(const std::string& id);
    void requestTeardown(int row);

    std::vector<std::string> recentServers() const;
    void removeRecentServer(const std::string& address);
    // Connects to a server, asking through passwordRequested and hostKeyRequested.
    void connectToServer(const std::string& address);
    void answerPassword(const std::string& address, const std::string& user, const std::string& domain,
                        const std::string& password, bool anonymous, bool remember);
    void acceptHostKey(const std::string& address, const std::string& fingerprint);
    void cancelConnection(const std::string& address);
    // Unlocks the encrypted disk and mounts it; a wrong passphrase asks again.
    void answerPassphrase(const std::string& udi, const std::string& passphrase);

    base::Signal<> changed;
    base::Signal<std::string> deviceEjected;
    base::Signal<> favoritesChanged;
    base::Signal<std::string> deviceSetupDone;
    base::Signal<std::string> deviceOperationFailed;
    base::Signal<> recentServersChanged;
    // address, user, domain, needsDomain, anonymousAllowed, retry
    base::Signal<std::string, std::string, std::string, bool, bool, bool> passwordRequested;
    // address, fingerprint, changed
    base::Signal<std::string, std::string, bool> hostKeyRequested;
    // udi, name, retry
    base::Signal<std::string, std::string, bool> passphraseRequested;

private:
    std::string m_settingsFile;

    void saveFavorites();
    void syncMounts();
    void insertLocation(Place place, const std::string& category);
    void mountLocation(const std::string& address);
    void addRecentServer(const std::string& address);
    void deviceAdded(const std::string& id);
    void deviceRemoved(const std::string& id);
    void setVolume(Place& place, const files::Volume& volume);
    void unmountLocation(const std::string& id);

    std::vector<Place> m_items;
    std::unique_ptr<files::VolumeManager> m_volumes;
    std::map<std::string, base::dbus::VariantMap> m_connecting;
    std::vector<base::Subscription> m_subscriptions;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace fm
