#ifndef MODIFYPARTITIONFRAME_H
#define MODIFYPARTITIONFRAME_H

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
#include <QCheckBox>
#include <QKeyEvent>

#include "partman/device.h"
#include "partman/partition.h"
#include "frames/lineeditaddreduce.h"
#include "./delegate/custom_partition_delegate.h"
#include "../PluginService/ui_unit/messagebox.h"
#include "../PluginService/ui_unit/arrowtooltip.h"


namespace KInstaller {
using namespace Partman;

class ModifyPartitionFrame : public QWidget
{
    Q_OBJECT
public:
    explicit ModifyPartitionFrame(CustomPartitiondelegate* delegate, QWidget *parent = nullptr);
    ~ModifyPartitionFrame();
    void setModifyPartition( Partition::Ptr partition,int SCreenWidth,int SCreenHeight);
    void initUI();
    void initAllConnect();
    void translateStr();
    void addStyleSheet();
    void initBackground();

private:
    void changeEvent(QEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
 signals:
        void finished();
        void signalCloseBtn();
public slots:
    void changeFileFormat(int index);
    void changeMountFile(const QString &text);

    void slotOKBtn();

public:
    QLabel *tipLabel, *m_mainTitle, *fsLabel, *mountLabel;
    QComboBox *fileFormat, *mountFile;
    QCheckBox* checkFormat;
    QStringList filesystemList, mountedList;
    QString fileFormatStr, mountFileStr;
    QPushButton* m_close;
    QPushButton* m_OKBtn;
    CustomPartitiondelegate* m_delegate = nullptr;
    Device::Ptr m_dev;
    Partition::Ptr m_partition;
    QString m_filesystem;
    bool m_blCreatorPartition;
    MessageBox *message;
    ArrowWidget *closeTip;
    DeviceList *devlst;
    int m_SCreenWidth;
    int m_SCreenHeight;
};
}
#endif // MODIFYPARTITIONFRAME_H
