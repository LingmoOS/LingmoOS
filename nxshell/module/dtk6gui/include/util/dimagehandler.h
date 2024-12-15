// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIMAGEHANDLER_H
#define DIMAGEHANDLER_H

#include <dtkgui_global.h>
#include <DObject>

#include <QObject>
#include <QImage>

DGUI_BEGIN_NAMESPACE

class DImageHandlerPrivate;
class DImageHandler : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT

public:
    explicit DImageHandler(QObject *parent = nullptr);
    ~DImageHandler();

    void setFileName(const QString &fileName);
    QString fileName() const;

    QImage readImage();
    QImage thumbnail(const QSize &size, Qt::AspectRatioMode mode);
    QString imageFormat() const;
    QSize imageSize();
    QHash<QString, QString> findAllMetaData();
    void clearCache();

    bool saveImage(const QString &fileName, const QString &format = QString());
    bool saveImage(const QImage &image, const QString &fileName, const QString &format = QString());
    bool rotateImage(QImage &image, int angle);
    bool rotateImageFile(const QString &fileName, int angle);

    bool isReadable() const;
    bool isWriteable() const;
    bool isRotatable() const;

    QString lastError() const;

    static QStringList supportFormats();
    static QString detectImageFormat(const QString &fileName);

    static QImage oldColorFilter(const QImage &img);
    static QImage warmColorFilter(const QImage &img, int intensity = 30);
    static QImage coolColorFilter(const QImage &img, int intensity = 30);
    static QImage grayScaleColorFilter(const QImage &img);
    static QImage antiColorFilter(const QImage &img);
    static QImage metalColorFilter(const QImage &img);

    static QImage bilateralFilter(const QImage &img, double spatialDecay = 0.02, double photometricStandardDeviation = 10);
    static QImage contourExtraction(const QImage &img);
    static QImage binaryzation(const QImage &img);
    static QImage grayScale(const QImage &img);

    static QImage laplaceSharpen(const QImage &img);
    static QImage sobelEdgeDetector(const QImage &img);

    static QImage changeLightAndContrast(const QImage &img, int light = 100, int contrast = 150);
    static QImage changeBrightness(const QImage &img, int brightness);
    static QImage changeTransparency(const QImage &img, int transparency);
    static QImage changeStauration(const QImage &img, int saturation);
    static QImage replacePointColor(const QImage &img, QColor oldColor, QColor newColor);

    static QImage flipHorizontal(const QImage &img);
    static QImage flipVertical(const QImage &img);

private:
    D_DECLARE_PRIVATE(DImageHandler)
    Q_DISABLE_COPY(DImageHandler)
};

DGUI_END_NAMESPACE

#endif  // DIMAGEHANDLER_H
