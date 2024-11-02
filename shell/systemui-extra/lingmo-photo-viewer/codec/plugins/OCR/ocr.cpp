#include <QtConcurrent>
#include <QMutex>
#include <QDebug>
#include <QFile>
#include "ocr.h"
#include "lingmoocr.h"

#define LOG_FLAG "OCR_MODULE#"

OCR::OCR() {}

OCR::~OCR()
{
    if (m_ocr) {
        delete m_ocr;
        m_ocr = nullptr;
    }
}

void OCR::setEngineType(EngineType type)
{
    if (m_ocr) {
        delete m_ocr;
        m_ocr = nullptr;
    }
    // TODO:目前因为子类中无私有属性，目前不会造成内存泄漏，所以先作为待办项
    switch (type) {
    case EngineType::KySDK:
        m_ocr = dynamic_cast<OCR *>(new LingmoOCR());
        break;
    default:
        qWarning() << LOG_FLAG << "Invaild OCR engine type";
    }
}

void OCR::getText(QString imgPath)
{
    if (!QFile::exists(imgPath) || !m_ocr) {
        qWarning() << LOG_FLAG << "getText Failed!Check image path or set engine type first?";
        return;
    }

    connect(m_ocr, &OCR::recResult, this, &OCR::recResult);

    m_ocr->getText(imgPath);
}

// void OCR::getRect(QString imgPath)
//{
//    // 接口暂未启用
//    Q_UNUSED(imgPath);
//    return;
//}
