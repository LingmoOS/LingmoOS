#ifndef BLURXCBINTERFACE_H
#define BLURXCBINTERFACE_H

#include <QObject>
#include "blurabstractinterface.h"

class BlurXcbInterface : public BlurAbstractInterface
{
    Q_OBJECT
public:
    explicit BlurXcbInterface(QObject *parent = nullptr);

    bool setBlurBehindWithStrength(QWindow *window, bool enable = true, const QRegion &region = QRegion(), uint32_t strength = -1) override;
};

#endif // BLURXCBINTERFACE_H
