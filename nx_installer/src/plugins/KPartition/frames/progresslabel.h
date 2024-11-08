#ifndef PROGRESSLABEL_H
#define PROGRESSLABEL_H

#include <QObject>
#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QGridLayout>
#include "partman/device.h"
#include "partman/partition.h"
namespace KInstaller {
using namespace Partman;

//进度条显示分区中的label显示
struct StrPartProgressView
{
    QLabel *color;
    QLabel *text;

};
class ProgressLabel : public QWidget
{
    Q_OBJECT
public:
    explicit ProgressLabel(QWidget *parent);

    QString calcValue(int nTotal);
    void initProgressLabel();
    void updateStructLabel();
    void insetStructLabel(QPixmap pix, QString str);
    void setDevice(Device::Ptr dev);

signals:

public slots:
    void slotDevPartitionColor(Device::Ptr dev);

public:
    QProgressBar *progressBar;
    QGridLayout *gridLayout;
    QWidget *widget;
    QList <struct StrPartProgressView*> parts;
    QStringList progressValue;
    QStringList colorList;

    Device::Ptr m_dev;


protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
};

}
#endif // CPROGRESSLABEL_H
