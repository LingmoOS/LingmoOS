/*
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2019-2021 Harald Sitter <sitter@kde.org>
*/

#pragma once

#include <memory>
#include <systemd/sd-journal.h>

namespace std
{
template<>
struct default_delete<sd_journal> {
    void operator()(sd_journal *ptr) const
    {
        sd_journal_close(ptr);
    }
};
} // namespace std

template<typename T, typename Deleter>
struct Expected {
    const int ret; // return value of call
    const int error; // errno immediately after the call
    std::unique_ptr<T, Deleter> value; // the newly owned object (may be null)
};

// Wrapper around C double pointer API of which we must take ownership.
// errno may or may not be
template<typename T, typename Func, typename... Args>
Expected<T, std::default_delete<T>> owning_ptr_call(Func func, Args &&...args)
{
    T *raw = nullptr;
    const int ret = func(&raw, std::forward<Args>(args)...);
    return {ret, errno, std::unique_ptr<T>(raw)};
}

// Same as owning_ptr_call but for (sd_journal *, foo **, ...) API
template<typename T, typename Deleter, typename Func, typename... Args>
Expected<T, Deleter> contextual_owning_ptr_call(Func func, sd_journal *context, Deleter deleter = std::default_delete<T>{}, Args &&...args)
{
    T *raw = nullptr;
    const int ret = func(context, &raw, std::forward<Args>(args)...);
    return {ret, errno, std::unique_ptr<T, Deleter>(raw, deleter)};
}
