#ifndef KYLICENSE_H
#define KYLICENSE_H

#include "kylicense_global.h"
#include "../Reflex/reflex.h"
#include "plugindll.h"
#include "licenseframe.h"
class KYLICENSESHARED_EXPORT KyLicense : public IPlugin
{
private:
   QWidget* m_frameLicense = nullptr;

public:
    KyLicense();
    ~KyLicense();

    QWidget* CreateObject(QWidget *parent);
    QWidget* getWidget();
    QString getWidgetName();
    void next();
    void back();
    void onActive();
    void onLeave();

    CONSTRUCTOR(KyLicense)
};
REGISTER_REFLEX(KyLicense)

#endif // KYLICENSE_H
