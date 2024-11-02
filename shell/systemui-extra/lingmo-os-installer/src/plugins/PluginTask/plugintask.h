#ifndef PLUGINTASK_H
#define PLUGINTASK_H

#include "plugintask_global.h"
#include "../Reflex/reflex.h"
#include <QObject>
#include <QThread>




class PLUGINTASKSHARED_EXPORT TPlugin : public ReflexBase
{

public:
    TPlugin();
    virtual ~TPlugin();
    virtual QObject* CreateObject(QObject* parent = nullptr) = 0;
    virtual QObject* GetObject() = 0;
    virtual void ExecObjectProcess() = 0;

public:
    QThread* m_thread;

};

#endif // PLUGINTASK_H
