#include "diskpartitioncolorprogress.h"
#include <QPainter>
#include <QLabel>
#include "../partman/partition.h"
#include "../PluginService/ui_unit/xrandrobject.h"
using namespace KServer;
namespace KInstaller {

const int ColorProgressBarWidth = KServer::GetScreenRect().width() * 0.6;
//const int ColorProgressBarHeigh = 15

const QStringList colorNameStr = { QString("#EA5504"),QString("#00A0DA"),QString("#B062A3"),QString("#009944"),
 QString("#74C6BE"),QString("#4D4398"),QString("#FABE00"),QString("#D12E29"),QString("#601986"),QString("#B062A3")
                                 ,QString("#F18D00"),QString("#6DBB58")};


DiskPartitionColorProgress::DiskPartitionColorProgress(QFrame *parent) : QFrame(parent)
{
    this->setObjectName("DiskPartitionColorProgress");
    this->setStyleSheet("QWidget#DiskPartitionColorProgress{border: 1px ;border-color: rgba(255,255,255,0.5);border-radius: 12px;}");
    initUI();
}

void DiskPartitionColorProgress::initUI()
{
    m_labelLayout = new QHBoxLayout;
    m_labelLayout->setMargin(0);
    m_labelLayout->setSpacing(0);
    m_labelLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(m_labelLayout);
//    ColorProgressBarWidth = this->width();
    this->setMaximumWidth(ColorProgressBarWidth);
    this->setMinimumWidth(ColorProgressBarWidth);
}

QList<QSize> DiskPartitionColorProgress::getPartitionColorSize(Device::Ptr device)
{
    QList<QSize> listSize;

    PartitionList partitions = device->partitions;
    for(Partition::Ptr partition : partitions) {
        const float ratio = static_cast<float>(partition->m_total) / static_cast<float>(device->m_diskTotal);
        int w = qRound(ratio * ColorProgressBarWidth/*this->width()*/);
        if(w <= 0) {
            listSize.append(QSize(0, 20));
        } else
            listSize.append(QSize(w, 20));
    }
    return listSize;
}

void DiskPartitionColorProgress::setDevice(Device::Ptr device)
{
    m_device = device;
    PartitionList partitions = device->partitions;

    for(auto it = m_labels.begin(); it != m_labels.end(); ++it) {
        for(QWidget* w : it.value()) {
            it.key()->removeWidget(w);
            w->deleteLater();
        }
        it.key()->deleteLater();
    }
    m_labels.clear();
    QList<QSize> sizes = getPartitionColorSize(device);
    int i = 0;
    int j = 0;
    int x = 0;
    int y = 0;
    QColor color;
    bool extendbl = false;
    for(Partition::Ptr partition : partitions) {
        if(j > colorNameStr.length() - 1) {
            j = 0;
        }
        if(partition->m_type == PartitionType::Unallocated) {
            color = QColor("#D3D3D3");
        } else {
           color = colorNameStr.at(j);
        }
        if(partition->m_type == PartitionType::Extended) {
            extendbl = true;
            QHBoxLayout* lay = new QHBoxLayout;
            lay->setMargin(0);
            lay->setSpacing(0);
            lay->insertSpacing(0,0);
            lay->setContentsMargins(0, 0, 0, 0);
            extendWidget = new QLabel;
            extendWidget->setMargin(0);
            extendWidget->setFixedSize(sizes.at(i));
            extendLayout = new QHBoxLayout(extendWidget);
            extendLayout->setMargin(0);
            extendLayout->setSpacing(0);
            extendLayout->insertSpacing(0,0);
            extendLayout->setContentsMargins(0, 0, 0, 0);
            extendLayout->setAlignment(Qt::AlignBottom);
            QPalette palette = extendWidget->palette();
            palette.setBrush(QPalette::Background, color);
            extendWidget->setAutoFillBackground(true);
            extendWidget->setPalette(palette);
            lay->addWidget(extendWidget);
            m_labels[extendLayout] << extendWidget;
            m_labelLayout->addLayout(lay);
        } else if(partition->m_type == PartitionType::Logical || extendbl) {
            QRect rect(x, y, sizes.at(i).width(), sizes.at(i).height() - 1);
            setChildWidgetColor(sizes.at(i), extendLayout, color);
            x += sizes.at(i).width();

        } else if(!extendbl){
            QHBoxLayout *layout = new QHBoxLayout;
            layout->setMargin(0);
            layout->setSpacing(0);
            layout->insertSpacing(0,0);
            layout->setContentsMargins(0, 0, 0, 0);
            QLabel* colorlabel = new QLabel;
            colorlabel->setMargin(0);
            colorlabel->setFixedSize(sizes.at(i));
            QPalette palette = colorlabel->palette();
            palette.setBrush(QPalette::Background, color);
            colorlabel->setAutoFillBackground(true);
            colorlabel->setPalette(palette);
            layout->addWidget(colorlabel);

            m_labels[layout] << colorlabel;
            m_labelLayout->addLayout(layout);
            x += sizes.at(i).width();

        }
        j++;
        i++;
    }

}

void DiskPartitionColorProgress::setChildWidgetColor(QSize size , QHBoxLayout* layout , QColor color)
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    lay->setSpacing(0);
    lay->insertSpacing(0,0);
    lay->setContentsMargins(0, 0, 0, 0);
    QLabel* colorlabel = new QLabel;
    colorlabel->setMargin(0);
    colorlabel->setFixedSize(size.width(), size.height() / 2);
    QPalette palette = colorlabel->palette();
    palette.setBrush(QPalette::Background, color);
    colorlabel->setAutoFillBackground(true);
    colorlabel->setPalette(palette);
    lay->addWidget(colorlabel);
    colorlabel->show();
    layout->addLayout(lay);

}
}
