#ifndef KChoiceImg_H
#define KChoiceImg_H

#include "kchoiceimg_global.h"
#include "../Reflex/reflex.h"
#include "plugindll.h"
#include "imgframe.h"
class KCHOICEIMGSHARED_EXPORT KChoiceImg : public IPlugin
{
private:
   QWidget* m_frameLicense = nullptr;

public:
    KChoiceImg();
    ~KChoiceImg();

    QWidget* CreateObject(QWidget *parent);
    QWidget* getWidget();
    QString getWidgetName();
    void next();
    void back();
    void onActive();
    void onLeave();

    CONSTRUCTOR(KChoiceImg)
};
REGISTER_REFLEX(KChoiceImg)

#endif // KChoiceImg_H
