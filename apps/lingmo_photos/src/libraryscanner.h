// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "photostore.h"
#include <base/signal.h>
#include <atomic>
#include <functional>
#include <memory>
#include <string>

namespace Photos {
class LibraryScanner {
public:
    explicit LibraryScanner(PhotoStore& store);
    ~LibraryScanner();
    void scan(std::string root, std::function<void()> done = {});
    void cancel();
    base::Signal<> changed;
private:
    PhotoStore& m_store;
    std::shared_ptr<std::atomic_bool> m_cancelled = std::make_shared<std::atomic_bool>(false);
};
} // namespace Photos
