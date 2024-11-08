/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef CSINGLETON_H
#define CSINGLETON_H


template <typename T>
class SingleTon
{
public:
    // 创建单例实例
    template<typename... Args>
    static T* instance(Args&&... args)
    {
        if (m_pInstance == nullptr)
        {
            m_pInstance = new T(std::forward<Args>(args)...);
        }

        return m_pInstance;
    }

    // 获取单例
    static T* getInstance()
    {
        return m_pInstance;
    }

    // 删除单例
    static void destroyInstance()
    {
        delete m_pInstance;
        m_pInstance = nullptr;
    }

private:
    SingleTon();
    virtual ~SingleTon();

private:
    static T* m_pInstance;
};

template <class T>
T* SingleTon<T>::m_pInstance = nullptr;


#endif
