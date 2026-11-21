// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "base/weak_ptr.h"

namespace Lingmo::Files {

struct Volume
{
    // The block device's UDisks2 object path.
    std::string id;
    std::string drive;
    std::string device;
    std::string name;
    std::string icon;
    std::vector<std::string> mountPoints;
    uint64_t size = 0;
    // The filesystem, of the unlocked cleartext device for an encrypted volume.
    bool filesystem = false;
    // UDisks2's HintSystem: an internal device, which takes an administrator
    // to mount.
    bool system = false;
    bool encrypted = false;
    // The cleartext block's object path; empty while the volume is locked.
    std::string cleartext;
    bool optical = false;
    bool removable = false;
    bool ejectable = false;
    bool canPowerOff = false;

    bool locked() const { return encrypted && cleartext.empty(); }

    bool operator==(const Volume&) const = default;
};

// UDisks2's block devices, kept up to date from its signals, and mounting,
// unmounting and ejecting them. Calls never block. Blocks mounted outside
// /media, /run/media and /mnt (the root, home and boot filesystems) are left
// out, and an unlocked encrypted volume stands for its cleartext block.
class VolumeManager
{
public:
    explicit VolumeManager(base::dbus::Connection* bus = base::dbus::Connection::system());
    ~VolumeManager();

    VolumeManager(const VolumeManager&) = delete;
    VolumeManager& operator=(const VolumeManager&) = delete;

    std::vector<Volume> volumes() const;
    // An empty volume for an unknown id.
    Volume volume(const std::string& id) const;

    void mount(const std::string& id);
    // Locks an encrypted volume once its filesystem is unmounted.
    void unmount(const std::string& id);
    // Unlocks an encrypted volume and mounts its filesystem; finishes as
    // "unlock" with the mount point. A wrong passphrase fails with
    // passphraseRejected instead of operationFailed.
    void unlock(const std::string& id, const std::string& passphrase);
    // Unmounts every volume of the drive and locks its encrypted ones, then
    // ejects it, or powers it off when it cannot be ejected.
    void eject(const std::string& id);

    base::Signal<const std::string&> volumeAdded;
    base::Signal<const std::string&> volumeRemoved;
    base::Signal<const std::string&> volumeChanged;
    // id, "mount", "unmount", "unlock" or "eject", and the mount point a mount
    // or unlock chose.
    base::Signal<const std::string&, const std::string&, const std::string&> operationFinished;
    base::Signal<const std::string&, const std::string&, const std::string&> operationFailed;
    base::Signal<const std::string&> passphraseRejected;
    base::Signal<const std::string&> errorOccurred;

    using Interfaces = std::map<std::string, base::dbus::VariantMap>;
    using Objects = std::map<base::dbus::ObjectPath, Interfaces>;

private:
    struct Step
    {
        // Empty for the cleartext block the Unlock before it returned.
        std::string path;
        std::string interface;
        std::string method;
        std::optional<std::string> passphrase;
    };

    void refresh();
    void rebuild();
    void start(const std::string& id, const std::string& operation, std::vector<Step> steps);
    void perform(const std::string& id, const std::string& operation, std::vector<Step> steps, uint64_t generation);
    // Drops a finished call's slot once its callback has returned.
    void release(uint64_t call);

    base::dbus::Connection* m_bus;
    Objects m_objects;
    std::map<std::string, Volume> m_volumes;
    std::set<std::string> m_pending;
    bool m_refreshing = false;
    bool m_refreshAgain = false;
    bool m_ownerKnown = false;
    uint64_t m_generation = 0;
    uint64_t m_nextCall = 1;
    std::map<uint64_t, base::dbus::PendingCall> m_calls;
    std::vector<base::dbus::Match> m_matches;
    base::dbus::NameWatch m_ownerWatch;
    base::WeakPtrFactory<VolumeManager> m_weakFactory{this};
};

} // namespace Lingmo::Files
