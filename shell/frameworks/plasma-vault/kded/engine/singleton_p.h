/*
 *   SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_ENGINE_SINGLETON_P_H
#define PLASMAVAULT_KDED_ENGINE_SINGLETON_P_H

#include <memory>
#include <mutex>

namespace singleton
{
template<typename BackendType>
static std::shared_ptr<BackendType> instance()
{
    static std::mutex s_instanceMutex;
    static std::weak_ptr<BackendType> s_instance;

    std::unique_lock<std::mutex> locker(s_instanceMutex);

    auto ptr = s_instance.lock();

    if (!ptr) {
        ptr = std::make_shared<BackendType>();
        s_instance = ptr;
    }

    return ptr;
}

} // namespace singleton

#endif // include guard
