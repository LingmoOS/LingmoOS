#ifndef KPARTITION_H
#define KPARTITION_H
#include "QObject"
#include "kpartition_global.h"
#include "../Reflex/reflex.h"
#include "plugindll.h"
#include "mainpartframe.h"
class KPARTITIONSHARED_EXPORT KPartition : public IPlugin
{

private:
    QWidget* m_mainPartFrame = nullptr;

public:
    KPartition();
    ~KPartition();
    QWidget* CreateObject(QWidget *parent);
    MainPartFrame* getWidget();
    QString getWidgetName();
    void next();
    void back();
    void onActive();
    void onLeave();

    CONSTRUCTOR(KPartition)
};
REGISTER_REFLEX(KPartition)

#endif // KPARTITION_H
