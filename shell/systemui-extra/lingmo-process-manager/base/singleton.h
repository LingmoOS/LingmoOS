/*
 * Copyright 2023 KylinSoft Co., Ltd.
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

#ifndef SINGLETON_H
#define SINGLETON_H

template <typename T>
class Singleton
{
public:
    Singleton() = delete;
    Singleton(const Singleton &) = delete;
    Singleton(Singleton &&) = delete;

    static T &instance()
    {
        static T instance;
        return instance;
    }
};

#endif // SINGLETON_H
