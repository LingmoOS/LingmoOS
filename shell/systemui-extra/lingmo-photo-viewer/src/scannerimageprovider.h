#ifndef SCANNERIMAGEPROVIDER_H
#define SCANNERIMAGEPROVIDER_H

#include <QObject>
#include <QQuickImageProvider>
#include <QDebug>
#include "./global/variable.h"

class ScannerImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    ScannerImageProvider(): QQuickImageProvider(QQuickImageProvider::Image){}

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override
    //图片路径"image://name/id"
    {
        if(id == "orig"){
            return m_origImage;
        }else if(id == "scan"){
            return m_scanImage;
        }else if(id == "watermark"){
            return m_warkmarkImage;
        }else{
            return QImage();
        }
    }
    void setOrigImage(QImage image){
        m_origImage = image;
    }
    void setScanImage(QImage image){
        m_scanImage = image;
    }
    void setWarkmarkImage(QImage image){
        m_warkmarkImage = image;
    }

private:

    QImage m_origImage;
    QImage m_scanImage;
    QImage m_warkmarkImage;

};

#endif // SCANNERIMAGEPROVIDER_H
