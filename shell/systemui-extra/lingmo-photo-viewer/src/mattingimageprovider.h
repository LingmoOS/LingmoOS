#ifndef MATTINGIMAGEPROVIDER_H
#define MATTINGIMAGEPROVIDER_H

#include <QQuickAsyncImageProvider>
#include "./view/sizedate.h"

class MattingImageProvider : public QQuickImageProvider
{
public:
    MattingImageProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
    void setImage(QImage &img);
private:
    QImage m_image;
};

class ShowMattingImage : public QObject
{
    Q_OBJECT
public:
    explicit ShowMattingImage(QObject *parent = 0);
    MattingImageProvider *getImgProvider();

public Q_SLOTS:
    void setImage(QImage &img);
Q_SIGNALS:
    void callQmlRefeshImg();

private:
    MattingImageProvider *m_pImgProvider = nullptr;
};

#endif // MATTINGIMAGEPROVIDER_H
