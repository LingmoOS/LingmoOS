// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/files/volume_manager.h"

#include <algorithm>
#include <chrono>

#include "base/event_loop.h"
#include "base/locale.h"
#include "framework_i18n.h"
#include "path_utils_p.h"

namespace Lingmo::Files {

using namespace Lingmo::Base;
using namespace Lingmo::Base::dbus;
using namespace std::chrono_literals;
using Lingmo::framework::tr;
using Lingmo::i18n::format;

static const std::string service = "org.freedesktop.UDisks2";
static const std::string root = "/org/freedesktop/UDisks2";
static const std::string blockInterface = service + ".Block";
static const std::string driveInterface = service + ".Drive";
static const std::string filesystemInterface = service + ".Filesystem";
static const std::string encryptedInterface = service + ".Encrypted";

static std::string decodePath(const std::vector<uint8_t>& bytes)
{
    std::string path(bytes.begin(), bytes.end());
    if (!path.empty() && path.back() == '\0')
        path.pop_back();

    return path;
}

static bool isSet(const ObjectPath& path)
{
    return !path.path.empty() && path.path != "/";
}

static bool isSystemMountPoint(const std::string& path)
{
    for (const std::string prefix : {"/media/", "/run/media/", "/mnt/"}) {
        if (path.starts_with(prefix))
            return false;
    }
    return path != "/mnt";
}

template<typename T>
static T value(const VariantMap& map, const std::string& key, T fallback = T())
{
    const auto found = map.find(key);
    if (found == map.cend())
        return fallback;

    // Values arrive inside a variant.
    const Variant& variant = found->second;
    if (const auto direct = variant.get<T>())
        return *direct;
    if (const auto inner = variant.get<Variant>())
        return inner->get<T>().value_or(fallback);
    return fallback;
}

VolumeManager::VolumeManager(Connection* bus)
    : m_bus(bus)
{
    if (!m_bus)
        return;

    const auto refreshOnSignal = [this](const Message&) {
        refresh();
    };
    m_matches.push_back(m_bus->subscribe(
        {service, root, {}, "org.freedesktop.DBus.ObjectManager", "InterfacesAdded", {}}, refreshOnSignal));
    m_matches.push_back(m_bus->subscribe(
        {service, root, {}, "org.freedesktop.DBus.ObjectManager", "InterfacesRemoved", {}}, refreshOnSignal));
    m_matches.push_back(m_bus->subscribe(
        {service, {}, {}, "org.freedesktop.DBus.Properties", "PropertiesChanged", {}}, refreshOnSignal));

    m_ownerWatch = m_bus->watchName(service, [this](const std::string& owner) {
        // The first answer is the owner as of now, which the first refresh reads.
        if (!m_ownerKnown) {
            m_ownerKnown = true;
            return;
        }

        ++m_generation;
        m_objects.clear();
        rebuild();

        if (!owner.empty())
            refresh();
    });

    refresh();
}

VolumeManager::~VolumeManager() = default;

std::vector<Volume> VolumeManager::volumes() const
{
    std::vector<Volume> result;
    result.reserve(m_volumes.size());
    for (const auto& [id, volume] : m_volumes)
        result.push_back(volume);
    return result;
}

Volume VolumeManager::volume(const std::string& id) const
{
    const auto found = m_volumes.find(id);
    return found == m_volumes.cend() ? Volume() : found->second;
}

void VolumeManager::release(uint64_t call)
{
    EventLoop::current()->post([self = m_weakFactory.getWeakPtr(), call] {
        if (VolumeManager* manager = self.get())
            manager->m_calls.erase(call);
    });
}

void VolumeManager::refresh()
{
    if (!m_bus)
        return;

    if (m_refreshing) {
        m_refreshAgain = true;
        return;
    }

    m_refreshing = true;
    const uint64_t generation = m_generation;
    const uint64_t id = m_nextCall++;
    const Proxy manager(m_bus, service, root, "org.freedesktop.DBus.ObjectManager");

    m_calls[id] = manager.callAsync("GetManagedObjects", [this, generation, id](const Reply& reply) {
        release(id);
        m_refreshing = false;

        if (generation == m_generation) {
            if (reply.isError()) {
                errorOccurred.emit(reply.error().message);
            } else {
                m_objects = reply.value<Objects>().value_or(Objects());
                rebuild();
            }
        }

        if (m_refreshAgain) {
            m_refreshAgain = false;
            refresh();
        }
    });
}

void VolumeManager::rebuild()
{
    std::map<std::string, Volume> next;

    for (const auto& [path, interfaces] : m_objects) {
        const auto blockFound = interfaces.find(blockInterface);
        if (blockFound == interfaces.cend())
            continue;

        const VariantMap& block = blockFound->second;
        if (value<bool>(block, "HintIgnore") || isSet(value<ObjectPath>(block, "CryptoBackingDevice")))
            continue;

        Volume v;
        v.id = path.path;
        v.drive = value<ObjectPath>(block, "Drive").path;

        VariantMap drive;
        const auto driveObject = m_objects.find(ObjectPath{v.drive.empty() ? std::string("/") : v.drive});
        if (driveObject != m_objects.cend()) {
            if (const auto found = driveObject->second.find(driveInterface); found != driveObject->second.cend())
                drive = found->second;
        }

        const Interfaces* contents = &interfaces;
        const VariantMap* cleartextBlock = nullptr;
        if (const auto encrypted = interfaces.find(encryptedInterface); encrypted != interfaces.cend()) {
            v.encrypted = true;
            const ObjectPath cleartext = value<ObjectPath>(encrypted->second, "CleartextDevice");
            const auto cleartextObject = isSet(cleartext) ? m_objects.find(cleartext) : m_objects.cend();
            if (cleartextObject != m_objects.cend()) {
                v.cleartext = cleartext.path;
                contents = &cleartextObject->second;
                if (const auto found = contents->find(blockInterface); found != contents->cend())
                    cleartextBlock = &found->second;
            }
        }

        v.device = decodePath(value<std::vector<uint8_t>>(block, "PreferredDevice"));
        v.size = value<uint64_t>(block, "Size");
        v.filesystem = contents->contains(filesystemInterface);
        v.system = value<bool>(block, "HintSystem");
        v.optical = value<bool>(drive, "Optical");
        v.removable = value<bool>(drive, "Removable") || value<std::string>(drive, "ConnectionBus") == "usb";
        v.ejectable = value<bool>(drive, "Ejectable");
        v.canPowerOff = value<bool>(drive, "CanPowerOff");

        v.name = value<std::string>(block, "HintName");
        if (v.name.empty() && cleartextBlock)
            v.name = value<std::string>(*cleartextBlock, "IdLabel");
        if (v.name.empty())
            v.name = value<std::string>(block, "IdLabel");
        if (v.name.empty())
            // TRANSLATORS: a disk without a label, named by its size, such as "16 GB Volume".
            v.name = v.size ? format(v.encrypted ? tr("{size} Encrypted Volume") : tr("{size} Volume"),
                                     {{"size", Locale::system().formattedDataSize(int64_t(v.size))}})
                            : internal::fileNameOf(v.device);

        v.icon = value<std::string>(block, "HintIconName");
        if (v.icon.empty())
            v.icon = v.optical ? "drive-optical" : v.removable ? "drive-removable-media" : "drive-harddisk";

        if (const auto filesystem = contents->find(filesystemInterface); filesystem != contents->cend()) {
            for (const auto& point : value<std::vector<std::vector<uint8_t>>>(filesystem->second, "MountPoints")) {
                const std::string mountPoint = decodePath(point);
                if (!mountPoint.empty())
                    v.mountPoints.push_back(mountPoint);
            }
        }

        if (std::ranges::any_of(v.mountPoints, isSystemMountPoint))
            continue;

        next.emplace(v.id, v);
    }

    const auto previous = std::exchange(m_volumes, next);

    for (const auto& [id, volume] : previous) {
        if (!next.contains(id))
            volumeRemoved.emit(id);
    }

    for (const auto& [id, volume] : next) {
        const auto old = previous.find(id);
        if (old == previous.cend())
            volumeAdded.emit(id);
        else if (!(old->second == volume))
            volumeChanged.emit(id);
    }
}

// The block whose filesystem a volume mounts.
static std::string filesystemPath(const Volume& volume)
{
    return volume.encrypted ? volume.cleartext : volume.id;
}

void VolumeManager::mount(const std::string& id)
{
    const auto v = volume(id);
    if (v.locked()) {
        operationFailed.emit(id, "mount", tr("The device is locked."));
        return;
    }
    start(id, "mount", {{filesystemPath(v), filesystemInterface, "Mount", {}}});
}

void VolumeManager::unmount(const std::string& id)
{
    const auto v = volume(id);
    std::vector<Step> steps;
    if (!v.encrypted || !v.mountPoints.empty())
        steps.push_back({filesystemPath(v), filesystemInterface, "Unmount", {}});
    if (v.encrypted && !v.locked())
        steps.push_back({id, encryptedInterface, "Lock", {}});
    start(id, "unmount", std::move(steps));
}

void VolumeManager::unlock(const std::string& id, const std::string& passphrase)
{
    const auto v = volume(id);
    if (!v.locked()) {
        mount(id);
        return;
    }
    start(id, "unlock", {{id, encryptedInterface, "Unlock", passphrase}, {{}, filesystemInterface, "Mount", {}}});
}

void VolumeManager::eject(const std::string& id)
{
    const auto v = volume(id);
    std::vector<Step> steps;
    for (const auto& [otherId, other] : m_volumes) {
        if (other.drive != v.drive)
            continue;
        if (!other.mountPoints.empty())
            steps.push_back({filesystemPath(other), filesystemInterface, "Unmount", {}});
        if (other.encrypted && !other.locked())
            steps.push_back({other.id, encryptedInterface, "Lock", {}});
    }
    if (v.ejectable || v.canPowerOff)
        steps.push_back({v.drive, driveInterface, v.ejectable ? "Eject" : "PowerOff", {}});
    start(id, "eject", std::move(steps));
}

void VolumeManager::start(const std::string& id, const std::string& operation, std::vector<Step> steps)
{
    const auto v = volume(id);
    bool supported = !v.id.empty() && !steps.empty();
    if (operation == "eject")
        supported = supported && (v.ejectable || v.canPowerOff);
    else if (operation == "mount")
        supported = supported && v.filesystem;
    else if (operation == "unmount")
        supported = supported && (v.filesystem || v.encrypted);
    else
        supported = supported && v.encrypted;

    if (!supported) {
        operationFailed.emit(id, operation, tr("The device does not support this operation."));
        return;
    }

    bool busy = m_pending.contains(id);
    for (const auto& pending : m_pending)
        busy = busy || (v.drive != "/" && !v.drive.empty() && volume(pending).drive == v.drive);

    if (busy) {
        operationFailed.emit(id, operation, tr("A device operation is already in progress."));
        return;
    }

    m_pending.insert(id);
    perform(id, operation, std::move(steps), m_generation);
}

void VolumeManager::perform(const std::string& id, const std::string& operation, std::vector<Step> steps,
                            uint64_t generation)
{
    const Step step = steps.front();
    steps.erase(steps.begin());

    Proxy proxy(m_bus, service, step.path, step.interface);
    proxy.setTimeout(120s);
    // An internal device takes an administrator's password to mount.
    proxy.setInteractiveAuthorizationAllowed(true);
    const uint64_t callId = m_nextCall++;

    auto done = [this, callId, id, operation, step, steps, generation](const Reply& reply) mutable {
        release(callId);

        if (generation != m_generation || (operation != "eject" && !m_volumes.contains(id))) {
            m_pending.erase(id);
            operationFailed.emit(id, operation, tr("The device is no longer available."));
            return;
        }

        if (reply.isError()) {
            m_pending.erase(id);
            // libblockdev reports cryptsetup's EPERM for a passphrase that opens no key slot.
            const bool rejected = step.method == "Unlock"
                    && reply.error().message.find("Operation not permitted") != std::string::npos;
            if (rejected)
                passphraseRejected.emit(id);
            else
                operationFailed.emit(id, operation, reply.error().message);
        } else if (!steps.empty()) {
            if (step.method == "Unlock" && steps.front().path.empty())
                steps.front().path = reply.value<ObjectPath>().value_or(ObjectPath()).path;
            perform(id, operation, std::move(steps), generation);
            return;
        } else {
            m_pending.erase(id);

            std::string mountPoint;
            if (step.method == "Mount")
                mountPoint = reply.value<std::string>().value_or(std::string());

            operationFinished.emit(id, operation, mountPoint);
        }

        refresh();
    };

    if (step.passphrase)
        m_calls[callId] = proxy.callAsync(step.method, std::move(done), *step.passphrase, VariantMap());
    else
        m_calls[callId] = proxy.callAsync(step.method, std::move(done), VariantMap());
}

} // namespace Lingmo::Files
