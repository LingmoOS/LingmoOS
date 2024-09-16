// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMDYNEW_H
#define COMDYNEW_H
#include <QMap>
#include <QString>
#include <QDebug>
typedef void *(*CREATE_FUNC)();
class ClassObjectFactory
{
public:
    template<class T>
    static T *creatObject(const QString &name)
    {
        auto p = creatClassObject(name);
        if (p != nullptr)
            return static_cast<T *>(p);
        return nullptr;
    }

    static void registerClass(const QString &name, CREATE_FUNC func);
    static void *creatClassObject(const QString &name);

private:
    static QMap<QString, CREATE_FUNC> *s_mapNameClass;
};

class ClassRegisterHelper
{
public:
    ClassRegisterHelper(const QString &name, CREATE_FUNC func)
    {
        qWarning() << "ClassRegisterHelper name =========== " << name;
        ClassObjectFactory::registerClass(name, func);
    }
};

#define REGISTCLASS_DECLARE(classname) \
    class classname##RegisterNameSpace{\
        static void* creatInstance(){\
            return new classname;\
        }\
        static ClassRegisterHelper s_reg1;\
    };
#define REGISTCLASS_DEFINE(classname) \
    ClassRegisterHelper classname##RegisterNameSpace::s_reg1 = ClassRegisterHelper(#classname, creatInstance);

#define REGISTCLASS(classname) \
    REGISTCLASS_DECLARE(classname) \
    REGISTCLASS_DEFINE(classname)
#endif // COMDYNEW_H
