// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "include/gpu/ganesh/GrContextOptions.h"

namespace Lingmo::ui::gpu {

// Linked GL programs kept on disk between runs, so a first frame does not
// compile the shaders earlier runs drew with. A binary only loads on the
// driver that made it, so each driver has a directory of its own and the
// others are removed.
class ProgramCache final : public GrContextOptions::PersistentCache
{
public:
    // The driver as GL names it: vendor, renderer and version.
    explicit ProgramCache(const std::string& driver);

    sk_sp<SkData> load(const SkData& key) override;
    void store(const SkData& key, const SkData& data, const SkString& description) override;

private:
    std::string pathFor(const SkData& key) const;

    std::string m_root;
    std::string m_directory;
    bool m_pruned = false;
};

} // namespace Lingmo::ui::gpu
