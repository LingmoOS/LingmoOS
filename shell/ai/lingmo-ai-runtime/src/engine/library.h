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

#ifndef LIBRARY_H
#define LIBRARY_H

#include <string>

namespace ai_engine {

class Library {
public:
    Library(const std::string &path);

    bool loaded() const;
    bool load();
    bool unload();
    void *resolve(const char *name);

    std::string error() const;

    template <typename Func>
    static auto toFunction(void *ptr) {
        return reinterpret_cast<Func *>(ptr);
    }

private:
    std::string path_;
    std::string error_;
    void *handle_ = nullptr;
};

}



#endif // LIBRARY_H
