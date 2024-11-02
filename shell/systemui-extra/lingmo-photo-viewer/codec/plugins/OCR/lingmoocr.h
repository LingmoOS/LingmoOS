#ifndef LINGMOOCRCLASS_H
#define LINGMOOCRCLASS_H

#include <QObject>
#include "ocr.h"
class LingmoOCR : public OCR
{
    Q_OBJECT
public:
    LingmoOCR();
    void getText(QString) override;
    //    void getRect(QString imagPath) override
    //    {
    //        Q_UNUSED(imagPath);
    //    }
};

#endif // LINGMOOCRCLASS_H
