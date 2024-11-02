#ifndef BLURMANAGER_H
#define BLURMANAGER_H

#include <QObject>
#include <QWindow>
#include <QRegion>

#include "blurabstractinterface.h"

namespace kdk
{
class BlurManagerPrivate;

class Q_DECL_EXPORT BlurManager : public QObject
{
    Q_OBJECT
public:
    static BlurManager*self();
    static bool setBlurBehindWithStrength(QWindow *window, bool enable = true, const QRegion &region = QRegion(), uint32_t strength = -1);


private:
    BlurManager(QObject *parent = nullptr);
    BlurAbstractInterface *interface();
    BlurAbstractInterface *m_blurInterface = nullptr;
};

}

#endif // BLURMANAGER_H
