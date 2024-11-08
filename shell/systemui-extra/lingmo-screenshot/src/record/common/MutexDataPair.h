/*
 * Copyright (c) 2012-2020 Maarten Baert <maarten-baert@hotmail.com>

 * This file is part of Lingmo-Screenshot.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef MUTEXDATAPAIR_H
#define MUTEXDATAPAIR_H

#include <mutex>

template<typename T>
class MutexDataPair
{
public:
    class Lock {
    private:
        std::unique_lock<std::mutex> m_lock;
        T* m_data;
    public:
        inline Lock(MutexDataPair* parent): m_lock(parent->m_mutex), m_data(&parent->m_data) {}
        inline T* operator->() {return m_data; }
        inline T* get() {return m_data; }
        inline std::unique_lock<std::mutex>& lock() {return m_lock; }
    };

private:
    std::mutex m_mutex;
    T m_data;
public:
    inline T* data() {return &m_data; }
};


#endif // MUTEXDATAPAIR_H
