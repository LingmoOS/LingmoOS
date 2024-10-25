/*
 * Copyright (C) 2024 LingmoOS Team and zhuzichu520.
 *
 * Author:     Elysia <c.elysia@foxmail.com>
 *
 * Original Author: zhuzichu520 <https://github.com/zhuzichu520>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef SINGLETON_H
#define SINGLETON_H

#include <QMutex>

template <typename T>
class Singleton {
 public:
  static T* getInstance();

 private:
  Q_DISABLE_COPY_MOVE(Singleton)
};

template <typename T>
T* Singleton<T>::getInstance() {
  static QMutex mutex;
  QMutexLocker locker(&mutex);
  static T* instance = nullptr;
  if (instance == nullptr) {
    instance = new T();
  }
  return instance;
}

#define SINGLETON(Class)         \
 private:                        \
  friend class Singleton<Class>; \
                                 \
 public:                         \
  static Class* getInstance() { return Singleton<Class>::getInstance(); }

#define HIDE_CONSTRUCTOR(Class)       \
 private:                             \
  Class() = default;                  \
  Class(const Class& other) = delete; \
  Q_DISABLE_COPY_MOVE(Class);

#endif  // SINGLETON_H
