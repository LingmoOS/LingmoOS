// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "model/places.h"
#include "core/config.h"
#include "core/mounts.h"
#include "core/preferences.h"
#include "core/standardfolders.h"
#include "ops/operations.h"

#include "base/i18n.h"
#include "base/paths.h"
#include "base/settings.h"
#include "base/strings.h"
#include "base/url.h"
#include "ui/drag.h"

#include "lingmo/files/file_item.h"
#include "lingmo/files/volume_manager.h"

#include <algorithm>
#include <set>
#include <sys/stat.h>
#include <unistd.h>

namespace fm {

using base::dbus::Variant;
using base::dbus::VariantMap;
using Lingmo::base::paths::UserDirectory;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trNoop;

static constexpr const char* kFavorites = trNoop("Favorites");
static constexpr const char* kLocations = trNoop("Locations");
static constexpr const char* kInternalDrives = trNoop("Internal Drives");
static constexpr const char* kNetwork = trNoop("Network");

static std::string settingsPath()
{
    return settingsFile();
}

static bool isDirectory(const std::string& path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

static PlacesModel::Place folderPlace(const std::string& path)
{
    PlacesModel::Place place;
    const std::string standard = standardFolderName(path);
    place.name = standard.empty() ? fileName(path) : standard;
    place.url = base::url::fromLocalFile(path);
    const std::string icon = files::standardFolderIconName(path);
    place.iconName = icon.empty() ? "folder" : icon;
    return place;
}

// Phones and servers have URLs for ids; disks have UDisks object paths.
static bool isMountId(const std::string& id)
{
    return id.find("://") != std::string::npos;
}

std::string PlacesModel::Place::path() const
{
    if (url.empty())
        return {};
    if (const auto local = base::url::toLocalFile(url))
        return *local;
    return url;
}

std::string PlacesModel::Place::displayName() const
{
    return path() == "/" ? std::string(tr("Computer")) : name;
}

PlacesModel::PlacesModel(bool volumes, std::string settingsFile, base::dbus::Connection* volumeBus)
    : m_settingsFile(settingsFile.empty() ? settingsPath() : std::move(settingsFile))
{
    using base::paths::userDirectory;
    const std::vector<std::pair<std::string, std::string>> defaults = {
        {homePath(), "folder-home"},
        {userDirectory(UserDirectory::Desktop), "folder-desktop"},
        {userDirectory(UserDirectory::Documents), "folder-documents"},
        {userDirectory(UserDirectory::Downloads), "folder-download"},
        {userDirectory(UserDirectory::Music), "folder-music"},
        {userDirectory(UserDirectory::Pictures), "folder-picture"},
        {userDirectory(UserDirectory::Videos), "folder-video"},
    };
    std::vector<Place> standard;
    for (const auto& [path, icon] : defaults) {
        if (path.empty() || !isDirectory(path))
            continue;
        Place place;
        place.name = standardFolderName(path);
        place.url = base::url::fromLocalFile(path);
        place.iconName = icon;
        standard.push_back(place);
    }

    base::Settings settings(m_settingsFile);
    if (settings.contains("Sidebar/favorites")) {
        for (const std::string& entry : settings.stringListValue("Sidebar/favorites")) {
            const auto local = base::url::toLocalFile(entry);
            if (!local)
                continue;
            const auto known = std::find_if(standard.begin(), standard.end(),
                                            [&entry](const Place& place) { return place.url == entry; });
            m_items.push_back(known != standard.end() ? *known : folderPlace(*local));
        }
    } else {
        m_items = standard;
    }
    for (Place& place : m_items)
        place.category = kFavorites;

    if (volumes) {
        m_volumes = volumeBus ? std::make_unique<files::VolumeManager>(volumeBus)
                              : std::make_unique<files::VolumeManager>();
        m_subscriptions.push_back(m_volumes->volumeAdded.connect([this](const std::string& id) { deviceAdded(id); }));
        m_subscriptions.push_back(
                m_volumes->volumeRemoved.connect([this](const std::string& id) { deviceRemoved(id); }));
        m_subscriptions.push_back(m_volumes->volumeChanged.connect([this](const std::string& id) { deviceAdded(id); }));
        m_subscriptions.push_back(m_volumes->operationFinished.connect(
                [this](const std::string& id, const std::string& operation, const std::string& path) {
                    if ((operation != "mount" && operation != "unlock") || path.empty())
                        return;
                    for (Place& place : m_items) {
                        if (place.udi != id)
                            continue;
                        files::Volume volume = m_volumes->volume(id);
                        volume.mountPoints = {path};
                        setVolume(place, volume);
                        changed.emit();
                        deviceSetupDone.emit(path);
                        break;
                    }
                }));
        m_subscriptions.push_back(m_volumes->operationFailed.connect(
                [this](const std::string&, const std::string&, const std::string& message) {
                    deviceOperationFailed.emit(message);
                }));
        m_subscriptions.push_back(m_volumes->operationFinished.connect(
                [this](const std::string& id, const std::string& operation, const std::string&) {
                    if (operation != "eject")
                        return;
                    const std::string name = m_volumes->volume(id).name;
                    deviceEjected.emit(name.empty() ? id : name);
                }));
        m_subscriptions.push_back(m_volumes->passphraseRejected.connect([this](const std::string& id) {
            passphraseRequested.emit(id, m_volumes->volume(id).name, true);
        }));
    }

    Place trash;
    trash.name = tr("Trash");
    trash.url = "trash:///";
    trash.iconName = "user-trash";
    m_items.push_back(trash);

    m_subscriptions.push_back(MountsClient::instance().mountsChanged.connect([this] { syncMounts(); }));
    syncMounts();
    if (m_volumes) {
        for (const files::Volume& volume : m_volumes->volumes())
            deviceAdded(volume.id);
    }
}

PlacesModel::~PlacesModel()
{
    *m_alive = false;
}

void PlacesModel::retranslate()
{
    for (Place& place : m_items) {
        if (place.isDevice())
            continue;
        if (place.url == "trash:///") {
            place.name = tr("Trash");
            continue;
        }
        const std::string standard = standardFolderName(place.path());
        if (!standard.empty())
            place.name = standard;
    }
    changed.emit();
}

void PlacesModel::setVolume(Place& place, const files::Volume& volume)
{
    place.udi = volume.id;
    place.name = volume.name;
    place.iconName = volume.icon;
    place.opticalDisc = volume.optical;
    place.canEject = volume.ejectable || volume.canPowerOff;
    place.hasFilesystem = volume.filesystem;
    place.locked = volume.locked();
    place.accessible = !volume.mountPoints.empty();
    place.url = place.accessible ? base::url::fromLocalFile(volume.mountPoints.front()) : std::string();
}

int PlacesModel::favoriteCount() const
{
    int count = 0;
    while (count < int(m_items.size()) && m_items[size_t(count)].category == kFavorites)
        ++count;
    return count;
}

void PlacesModel::syncMounts()
{
    std::set<std::string> present;
    for (const MountsClient::Mount& mount : MountsClient::instance().mounts()) {
        present.insert(mount.id);
        files::Volume volume;
        volume.id = mount.id;
        volume.name = mount.name;
        volume.icon = mount.icon;
        volume.filesystem = true;
        volume.ejectable = mount.ejectable;
        if (!mount.path.empty())
            volume.mountPoints = {mount.path};

        const auto existing = std::find_if(m_items.begin(), m_items.end(),
                                           [&mount](const Place& place) { return place.udi == mount.id; });
        if (existing != m_items.end()) {
            setVolume(*existing, volume);
            continue;
        }
        Place place;
        setVolume(place, volume);
        insertLocation(place, mount.network ? kNetwork : kLocations);
    }

    std::erase_if(m_items, [&present](const Place& place) {
        return isMountId(place.udi) && !present.contains(place.udi);
    });
    changed.emit();
}

// Sections need their rows together: Locations, then Internal Drives, then Network.
static int sectionOrder(const std::string& category)
{
    if (category == kNetwork)
        return 3;
    if (category == kInternalDrives)
        return 2;
    return category == kLocations ? 1 : 0;
}

void PlacesModel::insertLocation(Place place, const std::string& category)
{
    place.category = category;
    const auto at = std::find_if(m_items.begin(), m_items.end(), [&category](const Place& existing) {
        return sectionOrder(existing.category) > sectionOrder(category);
    });
    m_items.insert(at, std::move(place));
}

void PlacesModel::deviceAdded(const std::string& id)
{
    const files::Volume volume = m_volumes->volume(id);
    if (!volume.filesystem && !volume.optical && !volume.locked()) {
        deviceRemoved(id);
        return;
    }
    for (Place& place : m_items) {
        if (place.udi == id) {
            setVolume(place, volume);
            changed.emit();
            return;
        }
    }
    Place place;
    setVolume(place, volume);
    insertLocation(place, volume.system ? kInternalDrives : kLocations);
    changed.emit();
}

void PlacesModel::deviceRemoved(const std::string& id)
{
    const size_t before = m_items.size();
    std::erase_if(m_items, [&id](const Place& place) { return place.udi == id; });
    if (m_items.size() != before)
        changed.emit();
}

std::vector<std::string> PlacesModel::recentServers() const
{
    return Preferences::instance().recentServers();
}

void PlacesModel::addRecentServer(const std::string& address)
{
    Preferences::instance().addRecentServer(address);
    recentServersChanged.emit();
}

void PlacesModel::removeRecentServer(const std::string& address)
{
    std::vector<std::string> servers = recentServers();
    if (std::erase(servers, address) == 0)
        return;
    base::Settings settings(m_settingsFile);
    settings.setValue("Network/recentServers", servers);
    settings.sync();
    recentServersChanged.emit();
}

void PlacesModel::connectToServer(const std::string& address)
{
    const std::string location = MountsClient::normalizedAddress(address);
    if (location.empty()) {
        deviceOperationFailed.emit(format(tr("“{address}” is not a server address. Enter one such as "
                                             "smb://server/share or sftp://user@server."),
                                          {{"address", base::strings::trimmed(address)}}));
        return;
    }
    m_connecting[location] = {};
    mountLocation(location);
}

void PlacesModel::mountLocation(const std::string& address)
{
    MountsClient::instance().requestMount(
            address, m_connecting[address],
            [this, alive = m_alive, address](const std::string& status, const VariantMap& details) {
                if (!*alive || !m_connecting.contains(address))
                    return;
                const auto text = [&details](const char* key) {
                    const auto found = details.find(key);
                    return found != details.end() ? found->second.value<std::string>() : std::string();
                };
                const auto flag = [&details](const char* key) {
                    const auto found = details.find(key);
                    return found != details.end() && found->second.value<bool>();
                };
                if (status == "password") {
                    passwordRequested.emit(address, text("user"), text("domain"), flag("needsDomain"),
                                           flag("anonymousAllowed"), flag("retry"));
                    return;
                }
                if (status == "hostkey") {
                    hostKeyRequested.emit(address, text("fingerprint"), flag("changed"));
                    return;
                }
                m_connecting.erase(address);
                if (status == "mounted") {
                    addRecentServer(address);
                    deviceSetupDone.emit(text("path"));
                } else {
                    deviceOperationFailed.emit(text("message"));
                }
            });
}

void PlacesModel::answerPassword(const std::string& address, const std::string& user, const std::string& domain,
                                 const std::string& password, bool anonymous, bool remember)
{
    if (!m_connecting.contains(address))
        return;
    VariantMap& options = m_connecting[address];
    options["user"] = Variant(user);
    options["domain"] = Variant(domain);
    options["password"] = Variant(password);
    options["anonymous"] = Variant(anonymous);
    options["remember"] = Variant(remember);
    mountLocation(address);
}

void PlacesModel::acceptHostKey(const std::string& address, const std::string& fingerprint)
{
    if (!m_connecting.contains(address))
        return;
    m_connecting[address]["hostKey"] = Variant(fingerprint);
    mountLocation(address);
}

void PlacesModel::cancelConnection(const std::string& address)
{
    m_connecting.erase(address);
}

void PlacesModel::answerPassphrase(const std::string& udi, const std::string& passphrase)
{
    if (m_volumes)
        m_volumes->unlock(udi, passphrase);
}

bool PlacesModel::canAddFavorites(const std::vector<std::string>& urls) const
{
    if (urls.empty())
        return false;
    for (const std::string& url : urls) {
        const auto local = base::url::toLocalFile(url);
        if (!local || !isDirectory(*local))
            return false;
    }
    return true;
}

bool PlacesModel::addFavorites(const std::vector<std::string>& urls, int before)
{
    if (before < 0 || before > favoriteCount() || !canAddFavorites(urls))
        return false;
    for (const std::string& input : urls) {
        const std::string path = cleanPath(*base::url::toLocalFile(input));
        const std::string url = base::url::fromLocalFile(path);
        bool duplicate = false;
        for (int row = 0; row < favoriteCount(); ++row)
            duplicate |= m_items[size_t(row)].url == url;
        if (duplicate)
            continue;
        Place place = folderPlace(path);
        place.category = kFavorites;
        m_items.insert(m_items.begin() + before++, place);
    }
    saveFavorites();
    changed.emit();
    return true;
}

bool PlacesModel::moveFavorite(int from, int before)
{
    const int count = favoriteCount();
    if (from < 0 || from >= count || before < 0 || before > count)
        return false;
    if (before == from || before == from + 1)
        return true;
    Place place = m_items[size_t(from)];
    m_items.erase(m_items.begin() + from);
    m_items.insert(m_items.begin() + (before > from ? before - 1 : before), place);
    saveFavorites();
    changed.emit();
    return true;
}

bool PlacesModel::canDropInto(const std::vector<std::string>& urls, int row, uint32_t action) const
{
    if (urls.empty() || row < 0 || row >= count())
        return false;
    const Place& place = at(row);
    const bool toTrash = place.url.starts_with("trash:");
    const std::string target = cleanPath(place.path());
    if (!toTrash) {
        if (place.path().empty() || !isDirectory(target) || ::access(target.c_str(), W_OK | X_OK) != 0)
            return false;
    }
    const bool moving = toTrash || action == ui::MoveAction;
    for (const std::string& url : urls) {
        const auto local = base::url::toLocalFile(url);
        if (!local)
            return false;
        if (moving && !files::canMoveStandardFolderTo(files::Url(url), files::Url(place.url)))
            return false;
        if (toTrash)
            continue;
        const std::string source = cleanPath(*local);
        // Into the parent is a no-op; into itself or a descendant impossible.
        if (parentPath(source) == target || target == source || target.starts_with(source + '/'))
            return false;
    }
    return true;
}

bool PlacesModel::dropInto(const std::vector<std::string>& urls, int row, uint32_t action)
{
    if (!canDropInto(urls, row, action))
        return false;
    const Place& place = at(row);
    if (place.url.starts_with("trash:"))
        OperationsClient::instance().trash(urls);
    else if (action == ui::CopyAction)
        OperationsClient::instance().copy(urls, base::url::fromLocalFile(place.path()));
    else
        OperationsClient::instance().move(urls, base::url::fromLocalFile(place.path()));
    return true;
}

void PlacesModel::removeFavorite(int row)
{
    if (row < 0 || row >= favoriteCount())
        return;
    m_items.erase(m_items.begin() + row);
    saveFavorites();
    changed.emit();
}

void PlacesModel::saveFavorites()
{
    std::vector<std::string> urls;
    for (int row = 0; row < favoriteCount(); ++row)
        urls.push_back(m_items[size_t(row)].url);
    base::Settings settings(m_settingsFile);
    settings.setValue("Sidebar/favorites", urls);
    settings.sync();
    favoritesChanged.emit();
}

void PlacesModel::requestSetup(int row)
{
    if (row < 0 || row >= count() || !at(row).isDevice())
        return;
    const std::string id = at(row).udi;
    if (at(row).locked) {
        passphraseRequested.emit(id, at(row).name, false);
        return;
    }
    if (!isMountId(id)) {
        if (m_volumes)
            m_volumes->mount(id);
        return;
    }
    MountsClient::instance().requestMount(
            id, {}, [this, alive = m_alive](const std::string& status, const VariantMap& details) {
                if (!*alive)
                    return;
                const auto found = details.find(status == "mounted" ? "path" : "message");
                const std::string text = found != details.end() ? found->second.value<std::string>() : std::string();
                if (status == "mounted")
                    deviceSetupDone.emit(text);
                else
                    deviceOperationFailed.emit(text);
            });
}

void PlacesModel::unmountLocation(const std::string& id)
{
    MountsClient::instance().requestUnmount(id, [this, alive = m_alive](const std::string& error) {
        if (!*alive || error.empty())
            return;
        // fusermount refuses while anything has files open there.
        deviceOperationFailed.emit(error.find("busy") != std::string::npos
                                           ? std::string(tr("It is in use. Close the files open there and try again."))
                                           : error);
    });
}

void PlacesModel::requestEject(int row)
{
    if (row < 0 || row >= count() || !at(row).isDevice())
        return;
    const std::string id = at(row).udi;
    if (isMountId(id)) {
        MountsClient::instance().requestUnmount(id, [this, name = at(row).name](const std::string& error) {
            if (error.empty())
                deviceEjected.emit(name);
            else
                deviceOperationFailed.emit(error);
        });
    } else if (m_volumes) {
        m_volumes->eject(id);
    }
}

void PlacesModel::requestEjectById(const std::string& id)
{
    for (int row = 0; row < count(); ++row) {
        if (at(row).udi == id) {
            requestEject(row);
            return;
        }
    }
}

void PlacesModel::requestTeardown(int row)
{
    if (row < 0 || row >= count() || !at(row).isDevice())
        return;
    const std::string id = at(row).udi;
    if (isMountId(id))
        unmountLocation(id);
    else if (m_volumes)
        m_volumes->unmount(id);
}

} // namespace fm
