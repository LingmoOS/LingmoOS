// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "lingmo/files/url.h"

namespace Lingmo::Files::internal {

// A trash directory, holding files/ and info/. Its items are trash:/<id>/<name>.
struct TrashDirectory
{
    std::string id;
    std::string path;
    // The mount point of a volume's trash, which relative info paths start from;
    // empty for the home trash.
    std::string topdir;
};

TrashDirectory homeTrash();
// $topdir/.Trash/$uid when shared, else $topdir/.Trash-$uid.
TrashDirectory volumeTrash(const std::string& topdir, bool shared);

// The trash a URL below trash:/ is in, and in inside the path below its files/;
// throws EINVAL for trash:/ itself or an id that names no trash.
TrashDirectory trashDirectory(const Url& url, std::string* inside = nullptr);

// False when the mount table lists topdir with a type that holds no trash.
bool volumeTakesTrash(const std::string& topdir);

// Creates a volume's trash the way the trash specification asks, or checks the one
// there; returns whether items can be moved into it.
bool prepareVolumeTrash(const TrashDirectory& trash);

// The home trash, whether it exists or not, then each mounted volume's trash that
// does. mountPoints receives the mount points looked in.
std::vector<TrashDirectory> trashDirectories(std::vector<std::string>* mountPoints = nullptr);

// Folder paths to the names of their entries that count.
using WatchEntries = std::map<std::string, std::vector<std::string>>;

// What the trash listing watches: the files/ folders that exist, and the entries
// whose appearance would add a trash or a files/ folder.
void trashWatches(std::vector<std::string>* paths, WatchEntries* entries);

struct TrashInfo
{
    // Absolute; empty when the info file names no usable path, or in a volume's
    // trash one outside the volume.
    std::string path;
    // Milliseconds since the epoch of the local time the file gives.
    std::optional<int64_t> deletionTime;
};

// A file in a trash, up to a size no trash file needs. A volume's trash is the
// medium's to fill: a pipe there, or a link to a device, would block or never end.
std::optional<std::string> readTrashFile(const std::string& path);

// Reads the info file of the item name at the top of trash; false when it cannot.
bool readTrashInfo(const TrashDirectory& trash, const std::string& name, TrashInfo* info);

// Tests list mounts from a file of their own, so they never touch the trash on the
// volumes of the machine they run on.
void setMountTable(const std::string& path);
// And keep the home trash in a folder of their own; empty restores the default.
void setHomeTrashRoot(const std::string& path);
std::string homeTrashRoot();

} // namespace Lingmo::Files::internal
