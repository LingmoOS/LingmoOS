#ifndef KCHOICELANGUAGE_H
#define KCHOICELANGUAGE_H

#include "kchoicelanguage_global.h"
#include "../Reflex/reflex.h"
#include "plugindll.h"


class KCHOICELANGUAGESHARED_EXPORT KChoiceLanguage : public IPlugin
{

private:
    QWidget* m_frameLanguage = nullptr;

public:
    KChoiceLanguage();
    ~KChoiceLanguage();

    QWidget* CreateObject(QWidget *parent);
    QWidget* getWidget();
    QString getWidgetName();

    void next();
    void back();
    void onActive();
    void onLeave();

    CONSTRUCTOR(KChoiceLanguage)
};
REGISTER_REFLEX(KChoiceLanguage)
#endif // KCHOICELANGUAGE_H
