// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/display/display_prefs.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: confirmed monitor properties and topology profiles
// are atomically stored in one bounded per-user JSON file.

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "display/display_types.h"

namespace Lingmo::Display {

class DisplayPreferences
{
public:
    explicit DisplayPreferences(std::string path = {});

    bool save(const HardwareSnapshot &hardware, const DisplayConfiguration &configuration,
              DisplayError *error = nullptr) const;
    std::optional<DisplayConfiguration> restore(const HardwareSnapshot &hardware,
                                                uint64_t appliedRevision,
                                                DisplayError *error = nullptr) const;
    std::vector<std::string> persistentKeys() const;
    static std::string topologyKey(const HardwareSnapshot &hardware);

private:
    std::string m_path;
};

} // namespace Lingmo::Display
