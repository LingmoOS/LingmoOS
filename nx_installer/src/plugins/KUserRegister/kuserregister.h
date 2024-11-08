#ifndef KUSERREGISTER_H
#define KUSERREGISTER_H
#include "../Reflex/reflex.h"
#include "plugindll.h"
#include "kuserregister_global.h"
#include "userframe.h"

class KUSERREGISTERSHARED_EXPORT KUserRegister : public IPlugin
{
private:
   QWidget* m_frameUser = nullptr;
public:
    KUserRegister();
    ~KUserRegister();

    QWidget* CreateObject(QWidget *parent);
    QWidget* getWidget();
    QString getWidgetName();
    void next();
    void back();
    void onActive();
    void onLeave();

    CONSTRUCTOR(KUserRegister)
};
REGISTER_REFLEX(KUserRegister)
#endif // KUSERREGISTER_H
