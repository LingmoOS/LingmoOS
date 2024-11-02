/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "library.h"
#include <dlfcn.h>

namespace ai_engine {

Library::Library(const std::string &path): path_(path) {}

bool Library::loaded() const
{
    return handle_;
}

bool Library::load()
{
    if (path_.empty()) {
        return false;
    }

    handle_ = dlopen(path_.c_str(), RTLD_LAZY);

    if (!handle_) {
        error_ = dlerror();
        fprintf(stderr, "Load libary failed: %s. %s.\n.", error_.c_str(), path_.c_str());

        return false;
    }

    return true;
}

bool Library::unload()
{
    if (!handle_) {
        return false;
    }
    if (dlclose(handle_)) {
        error_ = dlerror();
        fprintf(stderr, "Unload libary failed: %s\n.", error_.c_str());
        return false;
    }

    handle_ = nullptr;
    return true;
}

void *Library::resolve(const char *name)
{
    auto *result = dlsym(handle_, name);
    if (!result) {
        fprintf(stderr, "Resovle libary failed: %s\n.", dlerror());
        error_ = dlerror();
    }

    return result;
}

std::string Library::error() const
{
    return error_;
}

}


