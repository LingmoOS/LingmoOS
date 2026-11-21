// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/base/accelerators/accelerator_map.h
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: the positional remap goes through the evdev code and
// positionalUsKeysym() instead of DomCode and KeycodeConverter.
#pragma once


#include <unordered_map>
#include <utility>

#include "accelerators/accelerator.h"

namespace Lingmo::Accelerators {

// A map keyed by Accelerator with ChromeOS's positional lookup: punctuation
// shortcuts are defined against the US layout, and on other layouts the key at
// the same physical position triggers them. Letters, digits, function keys and
// the rest always follow the current layout.
template<typename V>
class AcceleratorMap
{
public:
    using Map = std::unordered_map<Accelerator, V>;
    using iterator = typename Map::iterator;
    using const_iterator = typename Map::const_iterator;

    const V *find(const Accelerator &accelerator) const
    {
        auto it = findImpl(accelerator);
        return it == m_map.end() ? nullptr : &it->second;
    }

    V *find(const Accelerator &accelerator)
    {
        return const_cast<V *>(static_cast<const AcceleratorMap *>(this)->find(accelerator));
    }

    // The DomCode never goes into the map; only the lookup consults it.
    V &getOrInsertDefault(const Accelerator &accelerator) { return m_map[stripped(accelerator)]; }

    // Erases the entry find() returns.
    bool erase(const Accelerator &accelerator)
    {
        return m_map.erase(remapForLookup(accelerator)) > 0;
    }

    void clear() { m_map.clear(); }

    // Returns false when the accelerator was already present.
    bool insertNew(const Accelerator &accelerator, const V &value)
    {
        return m_map.emplace(stripped(accelerator), value).second;
    }

    iterator begin() { return m_map.begin(); }
    iterator end() { return m_map.end(); }
    const_iterator begin() const { return m_map.begin(); }
    const_iterator end() const { return m_map.end(); }

    size_t size() const { return m_map.size(); }

    void setUsePositionalLookup(bool use) { m_usePositionalLookup = use; }

    bool usePositionalLookup() const { return m_usePositionalLookup; }

    // The key find() looks up for the accelerator.
    Accelerator remapForLookup(const Accelerator &accelerator) const
    {
        uint32_t keysym = accelerator.keysym();
        if (m_usePositionalLookup && accelerator.evdevCode() != 0) {
            const uint32_t remapped = positionalUsKeysym(accelerator.evdevCode(), keysym);
            if (remapped != 0)
                keysym = remapped;
        }
        return Accelerator(keysym, accelerator.modifiers(), accelerator.keyState());
    }

private:
    static Accelerator stripped(const Accelerator &accelerator)
    {
        Accelerator copy = accelerator;
        copy.resetEvdevCode();
        return copy;
    }

    const_iterator findImpl(const Accelerator &accelerator) const
    {
        return m_map.find(remapForLookup(accelerator));
    }

    Map m_map;
    bool m_usePositionalLookup = false;
};

} // namespace Lingmo::Accelerators

