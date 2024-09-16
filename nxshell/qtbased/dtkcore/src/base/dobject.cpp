// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dobject.h"
#include "base/private/dobject_p.h"

DCORE_BEGIN_NAMESPACE

DObjectPrivate::DObjectPrivate(DObject *qq)
    : q_ptr(qq)
{

}

DObjectPrivate::~DObjectPrivate()
{

}

/*!
  \headerfile <dobject.h>
  \inmodule dtkcore

  \brief 一些宏的定义.
*/

/*!
  \class Dtk::Core::DObject
  \inmodule dtkcore
  \brief deepin-tool-kit 中所有公开类的祖先类.
  
  通过和 D_DECLARE_PRIVATE 、D_DECLARE_PUBLIC
  等宏的配合方便派生类中实现 D-Point 结构。虽然 QObject 中已经有了这样的实现结构，但是没有
  办法在不使用 Qt 私有模块的情况下，在 DTK 库中达到同样的目的。D-Point 结构由“公共接口类”
  和“私有数据类”两部分组成，在 DTK 中，DObjectPrivate 是所有数据类的祖先类。在这种结构下，
  只有 DObject 这个基类中定了一个指向于私有数据类的对象指针，派生类中不会也不应该再定义任何
  成员变量，派生类中需要添加数据成员时，可以继承 DObjectPrivate，将新的成员变量放到私有类中
  
  例子：
  
  a.h
  \code
  class APrivate;
  class A : public DObject
  {
      D_DECLARE_PRIVATE(A)
  public:
      A();
      int test() const;
  
  protected:
      A(APrivate &dd, DObject *parent = nullptr);
  };
  \endcode
  a.cpp
  \code
  class APrivate : public DObjectPrivate
  {
  public:
      APrivate(A *qq)
          : DObjectPrivate(qq)
      {
  
      }
  
      D_DECLARE_PUBLIC(A)
      // 此处添加数据成员
      int data;
  };
  
  A::A()
      : DObject(*new APrivate(this))
  {
  
  }
  
  int test() const
  {
      D_D(A);
  
      return d->data;
  }
  
  A::A(APrivate &dd, DObject *parent)
      : DObject(dd, parent)
  {
  
  }
  \endcode
  一般来讲，DObject 只会用在 DTK 库中定义的类，对于使用 DTK 库的应用程序来说不用关心它的存在
  \sa {https://wiki.qt.io/D-Pointer/zh}{类的 D-Point 结构}
 */

/*!
  \brief 只有在不需要数据成员的派生类中才会使用
  \a parent 父类指针
 */
DObject::DObject(DObject * /*parent = nullptr*/)
{

}

/*!
  \brief 在派生类中比较常用的构造函数
  \a dd 私有类对象
 */
DObject::DObject(DObjectPrivate &dd, DObject * /*parent = nullptr*/):
    d_d_ptr(&dd)
{

}

DObject::~DObject()
{

}

/*!
   \macro D_DECLARE_PRIVATE(Class)
   \relates Dtk::Core::DObject

   \brief 这个宏一定要放到类的私有区域，它定义了 d_func() 这个函数用于返回私有类的对象，
   这个对象只应该在类的内部使用，另外将私有类声明为公开类的友元类。
   \a Class 公开类的类名
   \sa D_DECLARE_PUBLIC D_D D_DC
*/

/*!
   \macro D_DECLARE_PUBLIC(Class)
   \relates Dtk::Core::DObject

   \brief 这个宏用于私有类中，它定义了 q_func() 这个函数用于返回公开类的对象，另外将公开类
   声明为私有类的友元类。
   \a Class 公开类的类名
   \sa D_DECLARE_PRIVATE D_Q D_QC
*/

/*!
   \macro D_D(Class)
   \relates Dtk::Core::DObject

   \brief 这个宏用于公开类中，它定义了一个名字为 d 的变量存储 d_func() 的返回值。用于在公开
   类中需要访问私有类的数据成员的函数中。
   \a Class 公开类的类名
   \sa D_DECLARE_PRIVATE D_DC
*/

/*!
   \macro D_DC(Class)
   \relates Dtk::Core::DObject

   \brief 同 D_D，用在公开类加了 const 修饰符的成员函数中。
   \a Class 公开类的类名
   \sa D_DECLARE_PRIVATE D_D
*/

/*!
   \macro D_Q(Class)
   \relates Dtk::Core::DObject

   \brief 这个宏用于私有类中，它定义了一个名字为 q 的变量存储 q_func() 的返回值。用于在私有
   类中需要调用公开类的成员函数时。
   \a Class 公开类的类名
   \sa D_DECLARE_PUBLIC D_QC
*/

/*!
   \macro D_QC(Class)
   \relates Dtk::Core::DObject

   \brief 同 D_Q，用在私有类加了 const 修饰符的成员函数中。
   \a Class 公开类的类名
   \sa D_DECLARE_PUBLIC D_Q
*/

/*!
 \macro D_PRIVATE_SLOT(Func)
 \relates Dtk::Core::DObject

 \brief 同 Q_PRIVATE_SLOT，用在继承了 QObject 的公开类中，在公开类中定一个槽函数，且函数
 必须在私有类中有实现。用这个方式定义的槽函数无法被直接调用，只能用于 QObject::connect
 使用 SIGNAL 和 SLOT 的方式连接信号，或者使用 QMetaObject::invokeMethod 调用。
 一般来讲，这个槽函数应该只在类内部使用，外界不应该通过任何方式来调用它。

 例子：

 a.h
 \code
 class APrivate;
 class A : public DObject
 {
     D_DECLARE_PRIVATE(A)
 public:
     A();

 protected:
     A(APrivate &dd, DObject *parent = nullptr);

 private:
     D_PRIVATE_SLOT(void _q_testSlot() const)
 };
 \endcode
a.cpp
 \code
 class APrivate : public DObjectPrivate
 {
 public:
     D_DECLARE_PUBLIC(A)

     APrivate(A *qq)
         : DObjectPrivate(qq)
     {
         QTimer *timer = new QTimer();
         QObject::connect(timer, SIGNAL(timeout()), qq, SLOT(_q_testSlot()));
         timer->start(1000);
     }

     void _q_testSlot() const
     {
         qDebug() << "slot";
     }
 };

 A::A()
     : DObject(*new APrivate(this))
 {

 }

 A::A(APrivate &dd, DObject *parent)
     : DObject(dd, parent)
 {

 }

 \#include "moc_a.cpp"
 \endcode
 \a Func 槽函数的完整签名
 \note 添加或更新私有槽之后需要重新手动调用 qmake
 \sa D_DECLARE_PUBLIC D_Q
*/

DCORE_END_NAMESPACE
