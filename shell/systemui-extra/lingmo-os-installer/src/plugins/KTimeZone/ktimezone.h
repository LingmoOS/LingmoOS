#ifndef KTIMEZONE_H
#define KTIMEZONE_H

#include "ktimezone_global.h"
#include "../Reflex/reflex.h"
#include "plugindll.h"

class KTIMEZONESHARED_EXPORT KTimeZone : public IPlugin
{

private:
    QWidget* m_frameTimeZone = nullptr;

public:
    KTimeZone();
    ~KTimeZone();
    QWidget* CreateObject(QWidget *parent);
    QWidget* getWidget();
    QString getWidgetName();

    void next();
    void back();
    void onActive();
    void onLeave();

    CONSTRUCTOR(KTimeZone)
};
REGISTER_REFLEX(KTimeZone)

#endif // KTIMEZONE_H
