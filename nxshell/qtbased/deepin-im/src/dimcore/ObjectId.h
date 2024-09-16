// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OBJECTID_H
#define OBJECTID_H

#include <stdint.h>

template<typename T>
class ObjectId
{
public:
    ObjectId()
        : id_(created_)
    {
        created_++;
    }

    inline uint32_t id() { return id_; }

private:
    static uint32_t created_;
    const uint32_t id_;
};

template<typename T>
uint32_t ObjectId<T>::created_{ 1 };

#endif // !OBJECTID_H
