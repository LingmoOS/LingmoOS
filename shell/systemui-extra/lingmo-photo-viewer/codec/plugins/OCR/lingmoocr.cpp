#include <libkyocr.hpp>

#include "lingmoocr.h"

LingmoOCR::LingmoOCR() {}

void LingmoOCR::getText(QString imgPath)
{
    QVector<QString> result;
    kdk::kdkOCR OCR;
    std::vector<std::string> ret = OCR.getCls(imgPath.toStdString(), 4);
    std::transform(ret.begin(), ret.end(), std::back_inserter(result), [](const std::string &v) {
        return QString::fromStdString(v);
    });
    Q_EMIT recResult(imgPath, result);
    return;
}
