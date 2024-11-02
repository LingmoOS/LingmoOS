#ifndef MULTITIFFOPERATE_H
#define MULTITIFFOPERATE_H

#include <QObject>
#include <QThread>
#include "model/processing/processing.h"
class MultiTiffOperate : public QThread
{
    Q_OBJECT
public:
    MultiTiffOperate(const QString &path, const QList<Processing::FlipWay> operateList);
Q_SIGNALS:
    void tiffOPerateFinish(QString path);

public:
    void run();

private:
    QString m_path;
    Processing::FlipWay m_operateWay;
    QList<FIBITMAP *> m_fibilist;
    QList<Processing::FlipWay> m_operateList;
    void finishOperate();
    //    QPixmap changImageSize(const QPixmap &pix);
};

#endif // MULTITIFFOPERATE_H
