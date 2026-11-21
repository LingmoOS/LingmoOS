namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gpu/program_cache.h"

#include <filesystem>
#include <optional>
#include <string_view>
#include <system_error>

#include "include/core/SkData.h"

#include "base/file.h"
#include "base/hash.h"
#include "base/paths.h"
#include "base/thread_pool.h"

namespace Lingmo::ui::gpu {

static std::string hexDigest(std::string_view data)
{
    return base::CryptographicHash::hashHex(data, base::CryptographicHash::Algorithm::Sha256);
}

ProgramCache::ProgramCache(const std::string& driver)
    : m_root(base::paths::cacheHome() + "/lingmo-toolkit/programs")
    , m_directory(m_root + "/" + hexDigest(driver).substr(0, 16))
{
}

std::string ProgramCache::pathFor(const SkData& key) const
{
    return m_directory + "/" + hexDigest(std::string_view(static_cast<const char*>(key.data()), key.size()));
}

sk_sp<SkData> ProgramCache::load(const SkData& key)
{
    const std::optional<std::string> contents = base::file::read(pathFor(key));
    if (!contents || contents->empty())
        return nullptr;
    return SkData::MakeWithCopy(contents->data(), contents->size());
}

void ProgramCache::store(const SkData& key, const SkData& data, const SkString&)
{
    const bool prune = !m_pruned;
    m_pruned = true;
    base::ThreadPool::global().post([root = m_root, directory = m_directory, path = pathFor(key),
                                     contents = std::string(static_cast<const char*>(data.data()), data.size()),
                                     prune] {
        std::error_code error;
        if (prune) {
            for (const auto& entry : std::filesystem::directory_iterator(root, error)) {
                if (entry.path() != directory)
                    std::filesystem::remove_all(entry.path(), error);
            }
        }
        base::file::writeAtomically(path, contents);
    });
}

} // namespace Lingmo::ui::gpu
