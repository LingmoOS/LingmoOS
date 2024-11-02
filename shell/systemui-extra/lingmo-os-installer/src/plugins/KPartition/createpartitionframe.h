#ifndef CREATEPARTITIONFRAME_H
#define CREATEPARTITIONFRAME_H

#include <QObject>
#include <QWidget>
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QRadioButton>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QButtonGroup>
#include <QKeyEvent>

#include "partman/device.h"
#include "partman/partition.h"
#include "frames/lineeditaddreduce.h"
#include "./delegate/custom_partition_delegate.h"
#include "../PluginService/ui_unit/messagebox.h"
#include "../PluginService/ui_unit/arrowtooltip.h"

namespace KInstaller {
using namespace Partman;

struct NewPartitionStruct {
    PartitionType partType;
    bool blpos;//start true,end false
    QString size;
    FSType fs;
    QString mountPoint;
};

class CreatePartitionFrame : public QWidget
{
    Q_OBJECT
public:
    explicit CreatePartitionFrame(CustomPartitiondelegate* delegate,
                                  DeviceList *pdevlst,
                                  QWidget *parent = nullptr);

    void initUI();
    void initAllConnect();
    void translateStr();
    void addStyleSheet();
    void initBackground();
    void setNewPartition(Device::Ptr dev,  Partition::Ptr partition,bool pritypebl,int SCreenWidth,int SCreenHeight);
    void setPartitionValue(Partition::Ptr partition);

signals:
    void signalCreatePartiton(Partition::Ptr partition);
    void signalChangePartition(Device::Ptr dev, Partition::Ptr partition);
    void finished();
    void signalCloseBtn();
private:
    void changeEvent(QEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
public slots:
    void clickPrimaryPart();
    void clickLvmPart();
    void clickStartPos();
    void clickEndPos();
    void changeFileFormat(int index);
    void changeMountFile(const QString &text);

    void slotOKBtn();
protected:
    void keyPressEvent(QKeyEvent *event);

public:
    QLabel *m_mainTitle, *tipLabel, *sizeLabel, *typeLabel, *positionLabel, *fsLabel, *mountLabel;
    QButtonGroup  *partGroup, *posGroup;
    QRadioButton *primaryPart, *lvmPart, *startPosition, *endPosition;
    QComboBox *fileFormat, *mountFile;
    LineEditAddReduce *sizeEdit;

    QStringList filesystemList, mountedList;
    QString fileFormatStr, mountFileStr;
    QString sizeStr;
    qint64 sizevalue;
    bool priTypebl, startPosbl;

    QPushButton* m_close;
    QPushButton* m_OKBtn;

    CustomPartitiondelegate* m_delegate;
    Device::Ptr m_dev;
    Partition::Ptr m_partition;
    bool m_blCreatorPartition;
    MessageBox *message;
    ArrowWidget *m_tTip;
    DeviceList *devlst;

    int m_SCreenWidth;
    int m_SCreenHeight;
};
}
#endif // CREATEPARTITIONFRAME_H
