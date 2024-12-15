// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSINGLETON_H
#define DSINGLETON_H

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

/*!
  a simple singleton template for std c++ 11 or later.
  
  example:

   \code
   class ExampleSingleton : public QObject, public Dtk::DSingleton<ExampleSingleton>
   {
       Q_OBJECT
       friend class DSingleton<ExampleSingleton>;
   };
   \endcode

  \note: for Qt, "public DSingleton<ExampleSingleton>" must be after QObject.
 */

/*!
  通过c++11的特性实现的单例模板
  
  使用示例:

```
   class ExampleSingleton : public QObject, public Dtk::DSingleton<ExampleSingleton>
   {
       Q_OBJECT
       friend class DSingleton<ExampleSingleton>;
   };
```

  \note 对于Qt程序 public DSingleton<ExampleSingleton>" 必须在卸载QObject后面出现。
 */

template <class T>
class LIBDTKCORESHARED_EXPORT DSingleton
{
public:
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    QT_DEPRECATED_X("Use ref")
    static inline T *instance()
    {
        static T  *_instance = new T;
        return _instance;
    }
#endif
    static T& ref()
    {
        static T instance;
        return instance;
    }

    DSingleton(T&&) = delete;
    DSingleton(const T&) = delete;
    void operator= (const T&) = delete;

protected:
    DSingleton() = default;
    virtual ~DSingleton() = default;
};

DCORE_END_NAMESPACE

#endif // DSINGLETON_H
