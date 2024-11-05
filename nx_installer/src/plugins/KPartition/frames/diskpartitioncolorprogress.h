#ifndef DISKPARTITIONCOLORPROGRESS_H
#define DISKPARTITIONCOLORPROGRESS_H

#include <QWidget>
#include <QMap>
#include <QHBoxLayout>
#include <QLabel>
#include "../partman/device.h"

namespace KInstaller {
using namespace Partman;


class DiskPartitionColorProgress : public QFrame
{
    Q_OBJECT
    public:
        explicit DiskPartitionColorProgress(QFrame *parent = nullptr);
        void setDevice(Device::Ptr device);
        void initUI();
        void setChildWidgetColor(QSize size , QHBoxLayout* layout , QColor color);
        QList<QSize>  getPartitionColorSize(Device::Ptr device);

    private:
        Device::Ptr m_device;
        QHBoxLayout* m_labelLayout;
        QMap<QHBoxLayout*,QList<QWidget*>> m_labels;
        QLabel* extendWidget;
        QHBoxLayout* extendLayout;

    signals:

};
}
#endif // DISKPARTITIONCOLORPROGRESS_H
