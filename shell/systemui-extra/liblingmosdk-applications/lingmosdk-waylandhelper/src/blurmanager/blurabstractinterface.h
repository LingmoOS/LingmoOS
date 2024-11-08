#ifndef BLURABSTRACTINTERFACE_H
#define BLURABSTRACTINTERFACE_H

#include <QWindow>
#include <QRegion>

class BlurAbstractInterface:public QObject
{
public:
    BlurAbstractInterface(QObject* parent=nullptr)
        :QObject(parent){}
    virtual bool setBlurBehindWithStrength(QWindow *window, bool enable = true, const QRegion &region = QRegion(), uint32_t strength = -1)  = 0;
};

#endif // BLURABSTRACTINTERFACE_H
