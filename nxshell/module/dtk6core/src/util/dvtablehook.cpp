// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dvtablehook.h"

#include <QFileInfo>
#include <algorithm>
#ifdef Q_OS_LINUX
#include <sys/mman.h>
#include <unistd.h>
#include <dlfcn.h>

QT_BEGIN_NAMESPACE
QFunctionPointer qt_linux_find_symbol_sys(const char *symbol);
QT_END_NAMESPACE

#endif

DCORE_BEGIN_NAMESPACE

QMap<quintptr**, quintptr*> DVtableHook::objToOriginalVfptr;
QMap<const void*, quintptr*> DVtableHook::objToGhostVfptr;
QMap<const void*, quintptr> DVtableHook::objDestructFun;

bool DVtableHook::copyVtable(quintptr **obj)
{
    int vtable_size = getVtableSize(obj);

    if (vtable_size == 0)
        return false;

    // 多开辟一个元素, 新的虚表结构如下:
    // 假设原虚表内存布局如下(考虑多继承):
    //                                             C Vtable (7 entities)
    //                                             +--------------------+
    // struct C                                     | offset_to_top (0)  |
    // object                                       +--------------------+
    // 0 - struct A (primary base)                  |     RTTI for C     |
    // 0 -   vptr_A ----------------------------->  +--------------------+
    // 8 -   int ax                                 |       C::f0()      |
    // 16 - struct B                                +--------------------+
    // 16 -   vptr_B ----------------------+        |       C::f1()      |
    // 24 -   int bx                       |        +--------------------+
    // 28 - int cx                         |        | offset_to_top (-16)|
    // sizeof(C): 32    align: 8           |        +--------------------+
    //                                     |        |     RTTI for C     |
    //                                     +------> +--------------------+
    //                                              |    Thunk C::f1()   |
    //                                              +--------------------+
    // 则新的表结构为:
    //                                              C Vtable (7 entities)            C hooked Vtable (7 entities)
    //                                              +--------------------+           +--------------------+
    // struct C                                     | offset_to_top (0)  |           | offset_to_top (0)  |
    // object                                       +--------------------+           +--------------------+
    // 0 - struct A (primary base)                  |     RTTI for C     |           |     RTTI for C     |
    // 0 -   vptr_A -----------------------------// +--------------------+ //------> +--------------------+
    // 8 -   int ax                                 |       C::f0()      |\          |       C::f0()      | (or override custom function pointer)
    // 16 - struct B                                +--------------------+ \         +--------------------+
    // 16 -   vptr_B ----------------------+        |       C::f1()      |  \        |       C::f1()      | (or override custom function pointer)
    // 24 -   int bx                       |        +--------------------+   \       +--------------------+
    // 28 - int cx                         |        | offset_to_top (-16)|    \      | offset_to_top (-16)|
    // sizeof(C): 32    align: 8           |        +--------------------+     \     +--------------------+
    //                                     |        |     RTTI for C     |      +    |     RTTI for C     |
    //                                     +------> +--------------------+      |    +--------------------+
    //                                              |    Thunk C::f1()   |      |    |    Thunk C::f1()   |
    //                                              +--------------------+      |    +--------------------+
    //                                                                          |    |          0         |
    //                                                                          |    +--------------------+
    //                                                                          +----|   original entry   |
    //                                                                               +--------------------+
    quintptr *new_vtable = new quintptr[vtable_size + 2];  // store 0 and oringinal entry

    memcpy(new_vtable, adjustToTop(*obj), vtable_size * sizeof(quintptr));
    new_vtable[vtable_size] = 0;

    //! save original vfptr
    objToOriginalVfptr[obj] = *obj;
    // 存储对象原虚表入口地址
    new_vtable[vtable_size + 1] = quintptr(*obj);

    *obj = adjustToEntry(new_vtable);
    //! save ghost vfptr
    objToGhostVfptr[obj] = new_vtable;

    return true;
}

bool DVtableHook::clearGhostVtable(const void *obj)
{
    if (!objToOriginalVfptr.remove((quintptr **)obj)) // Uninitialized memory may have values, for resetVtable
        return false;
    objDestructFun.remove(obj);

    quintptr *vtable = objToGhostVfptr.take(obj);

    if (vtable) {
        delete[] vtable;

        return true;
    }

    return false;
}

/**
  \brief 通过遍历尝试找到析构函数在虚表中的位置
  \a obj
  \a destoryObjFun
  \return
 */
int DVtableHook::getDestructFunIndex(quintptr **obj, std::function<void(void)> destoryObjFun)
{
    class _DestoryProbe
    {
    public:
        static quintptr probe(quintptr obj) {
            static quintptr _obj = 0;

            if (obj == 0) {
                obj = _obj;
                _obj = 0;
            } else {
                _obj = obj;
            }

            return obj;
        }

        static void nothing() {

        }
    };

    quintptr *vtable = *obj;
    int vtable_size = getVtableSize(obj);

    if (vtable_size == 0)
        return -1;

    quintptr *new_vtable = new quintptr[vtable_size];
    std::fill(adjustToEntry(new_vtable), new_vtable + vtable_size, quintptr(&_DestoryProbe::nothing));

    // 给对象设置新的虚表
    *obj = adjustToEntry(new_vtable);

    int index = -1;

    for (int i = adjustToEntry(0); i < vtable_size; ++i) {
        new_vtable[i] = quintptr(&_DestoryProbe::probe);

        // 尝试销毁此对象, 且观察_DestoryProbe::probe是否被调用
        // 如果被调用, 则证明覆盖此虚函数能达到监控对象被销毁的目的
        destoryObjFun();

        if (_DestoryProbe::probe(0) == quintptr(obj)) {
            index = adjustToTop(i);
            break;
        }
    }

    // 恢复旧的虚表
    *obj = vtable;
    // 销毁临时虚表
    delete[] new_vtable;

    return index;
}

void DVtableHook::autoCleanVtable(const void *obj)
{
    quintptr fun = objDestructFun.value(obj);

    if (!fun)
        return;

    if (hasVtable(obj)) {// 需要判断一下，有可能在执行析构函数时虚表已经被删除
        // clean
        clearGhostVtable(obj);
    }

    typedef void(*Destruct)(const void*);
    Destruct destruct = reinterpret_cast<Destruct>(fun);
    // call origin destruct function
    destruct(obj);
}

bool DVtableHook::ensureVtable(const void *obj, std::function<void ()> destoryObjFun)
{
    quintptr **_obj = (quintptr**)(obj);

    if (objToOriginalVfptr.contains(_obj)) {
        // 不知道什么原因, 此时obj对象的虚表已经被还原
        if (objToGhostVfptr.value((void *)obj) != adjustToTop(*_obj)) {
            clearGhostVtable((void*)obj);
        } else {
            return true;
        }
    }

    if (!copyVtable(_obj))
        return false;

    // 查找对象的析构函数
    int index = getDestructFunIndex(_obj, destoryObjFun);

    // 虚析构函数查找失败
    if (index < 0) {
        qCWarning(vtableHook) << "Failed do override destruct function: " << obj;
        abort();
    }

    quintptr *new_vtable = *_obj;
    // 保存对象真实的析构函数
    objDestructFun[(void*)obj] = new_vtable[index];

    // 覆盖析构函数, 用于在对象析构时自动清理虚表
    new_vtable[index] = reinterpret_cast<quintptr>(&autoCleanVtable);

    return true;
}

/*!
  \brief DVtableHook::hasVtable 对象的虚表已经被覆盖时返回true，否则返回false
  \a obj
  \return
 */
bool DVtableHook::hasVtable(const void *obj)
{
    quintptr **_obj = (quintptr**)(obj);

    return objToGhostVfptr.contains(_obj);
}

void DVtableHook::resetVtable(const void *obj)
{
    quintptr **_obj = (quintptr**)obj;
    int vtable_size = getVtableSize(_obj);
    // 获取obj对象原本虚表的入口
    auto vtableHead = adjustToTop(*_obj);
    quintptr *vfptr_t2 = (quintptr*)vtableHead[vtable_size + 1]; // _obj - 2 + vtable_size + 1

    if (!vfptr_t2)
        return;

    if (!clearGhostVtable(obj))
        return;

    // 还原虚表
    *_obj = vfptr_t2;
}

/*!
  \brief 将偏移量为\a functionOffset 的虚函数还原到原本的实现
  \a obj
  \a functionOffset
  \return 如果成功, 返回还原之前obj对象虚表中存储的函数指针, 否则返回0
 */
quintptr DVtableHook::resetVfptrFun(const void *obj, quintptr functionOffset)
{
    quintptr *vfptr_t1 = *(quintptr **)obj;
    quintptr current_fun = *(vfptr_t1 + functionOffset / sizeof(quintptr));
    quintptr origin_fun = originalFun(obj, functionOffset);

    if (!origin_fun) {
        return 0;
    }

    // reset to original fun
    *(vfptr_t1 + functionOffset / sizeof(quintptr)) = origin_fun;

    return current_fun;
}

/*!
  \brief 获取 \a obj 对象偏移量为 \a functionOffset 的虚函数原本的函数指针
  \return 如果obj对象虚表没有被覆盖, 或者函数偏移量正确, 将返回0
 */
quintptr DVtableHook::originalFun(const void *obj, quintptr functionOffset)
{
    quintptr **_obj = (quintptr **)obj;
    if (!hasVtable(obj)) {
        qCWarning(vtableHook) << "Not override the object virtual table: " << obj;
        return 0;
    }

    int vtable_size = getVtableSize(_obj);
    // 获取obj对象原本虚表的入口
    quintptr *vfptr_t2 = (quintptr*)(*_obj)[vtable_size - 1];

    if (functionOffset > UINT_LEAST16_MAX) {
        qCWarning(vtableHook, "Is not a virtual function, function address: 0X%llx", functionOffset);
        return 0;
    }

    return *(vfptr_t2 + functionOffset / sizeof(quintptr));
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
bool DVtableHook::isFinalClass(quintptr *obj)
{
    Q_UNUSED(obj);
    return true;
}

quintptr **DVtableHook::adjustThis(quintptr *obj)
{
    Q_UNUSED(obj);
    return nullptr;
}
#endif

#if defined(Q_OS_LINUX)
static int readProtFromPsm(quintptr adr, size_t length)
{
    int prot = PROT_NONE;
    QString fname = "/proc/self/maps";
    QFile f(fname);
    if (!f.open(QIODevice::ReadOnly)) {
        qFatal("%s", f.errorString().toStdString().data());
        //return prot; // never be executed
    }

    QByteArray data = f.readAll();
    bool ok = false;
    quintptr startAddr = 0, endAddr = 0;
    QTextStream ts(data);
    while (Q_UNLIKELY(!ts.atEnd())) {
        const QString line = ts.readLine();
        const QStringList &maps = line.split(' ');
        if (Q_UNLIKELY(maps.size() < 3)) {
            data = f.readLine();
            continue;
        }

        //"00400000-00431000" "r--p"
        const QStringList addrs = maps.value(0).split('-');
        startAddr = addrs.value(0).toULongLong(&ok, 16);
        Q_ASSERT(ok);
        endAddr = addrs.value(1).toULongLong(&ok, 16);
        Q_ASSERT(ok);
        if (Q_LIKELY(adr >= endAddr)) {
            continue;
        }
        if (adr >= startAddr && adr + length <= endAddr) {
            QString ps = maps.value(1);
            //qDebug() << maps.value(0) << maps.value(1);
            for (QChar c : ps) {
                switch (c.toLatin1()) {
                case 'r':
                    prot |= PROT_READ;
                    break;
                case 'w':
                    prot |= PROT_WRITE;
                    break;
                case 'x':
                    prot |= PROT_EXEC;
                    break;
                default:
                    break; // '-' 'p' don't care
                }
            }
            break;
        } else if (adr < startAddr) {
            qFatal("%p not found in proc maps", reinterpret_cast<void *>(adr));
            //break; // 超出了地址不需要再去检查了
        }
    }

    return prot;
}
#endif

bool DVtableHook::forceWriteMemory(void *adr, const void *data, size_t length)
{
#ifdef Q_OS_LINUX
    int page_size = sysconf(_SC_PAGESIZE);
    quintptr x = reinterpret_cast<quintptr>(adr);
    // 不减去一个pagesize防止跨越两个数据区域(对应/proc/self/maps两行数据)
    void *new_adr = reinterpret_cast<void *>((x /*- page_size - 1*/) & ~(page_size - 1));
    size_t override_data_length = length + x - reinterpret_cast<quintptr>(new_adr);

    int oldProt = readProtFromPsm(quintptr(new_adr), override_data_length);
    bool writeable = oldProt & PROT_WRITE;
    // 增加判断是否已经可写，不能写才调用。
    // 失败时直接放弃
    if (!writeable && mprotect(new_adr, override_data_length, PROT_READ | PROT_WRITE)) {
        qCWarning(vtableHook, "mprotect(change) failed: %s", strerror(errno));
        return false;
    }
#endif
    // 复制数据
    memcpy(adr, data, length);
#ifdef Q_OS_LINUX
    // 恢复内存标志位
    if (!writeable && mprotect(new_adr, override_data_length, oldProt)) {
        qCWarning(vtableHook, "mprotect(restore) failed: %s", strerror(errno));
        return false;
    }
#endif

    return true;
}

QFunctionPointer DVtableHook::resolve(const char *symbol)
{
#ifdef Q_OS_LINUX
  /**
  ！！不要使用qt_linux_find_symbol_sys函数去获取符号
  
  在龙芯平台上，qt_linux_find_symbol_sys 无法获取部分已加载动态库的符号，
  可能的原因是这个函数对 dlsym 的调用是在 libQt5Core 动态库中，这个库加载的比较早，
  有可能是因此导致无法获取比这个库加载更晚的库中的符号(仅为猜测)
  */
    return QFunctionPointer(dlsym(RTLD_DEFAULT, symbol));
#else
    // TODO
    return nullptr;
#endif
}

DCORE_END_NAMESPACE
