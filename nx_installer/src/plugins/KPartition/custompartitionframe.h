#ifndef CUSTOMPARTITIONFRAME_H
#define CUSTOMPARTITIONFRAME_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include "frames/progresslabel.h"

#include "partman/device.h"
#include "partman/partition.h"
#include "frames/tablewidgetview.h"
#include "createpartitionframe.h"
#include "./delegate/custom_partition_delegate.h"
#include "frames/diskpartitioncolorprogress.h"
#include "modifypartitionframe.h"
namespace KInstaller{
class CustomPartitionFrame : public QWidget
{
    Q_OBJECT
public:
    explicit CustomPartitionFrame(CustomPartitiondelegate* customDelegate, QWidget *parent = nullptr);

    void initUI();
    void initAllConnect();
    void translateStr();
    void addStyleSheet();
    void addPartProgress();

    void addTableWidget();
    void newTableViews(DeviceList devs);
    void initLayout();
    void revertPartition();
    void updateScrollAreaWidget();


signals:
    void signalNextBtn(bool flag);
    void signalCreatePartTable(Device::Ptr& dev);
    void signalFormatDisk(Device::Ptr dev);
    void signalFinishedLoadDisk();
    void enterpressed();
    void backspacepressed();
private:
    void changeEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

public slots:
    void slotCreatePartition(Device::Ptr dev, Partition::Ptr part);//增加一个分区
    void slotDeletePartition(Device::Ptr dev, Partition::Ptr part);//删除一个分区
    void slotModifyPartition(Device::Ptr dev, Partition::Ptr part);//修改一个分区
    void slotRestorePartition();
    void slotSelectBootCombox(QString bootstr);
    void repaintDevice();
    void updateTableView(TableWidgetView* tablewidget);
//    void setSizebyFrame(QWidget *pFrame, int SCreenWidth, int SCreenHeight);

public:
    QList<TableWidgetView*> m_tableViews;
    QGridLayout* gridLayout;
    DiskPartitionColorProgress* m_colorProgressFrame;
    //新建分区表New partition table,还原revert
    QPushButton* m_revertBtn;
    QLabel* m_mainTitle;
    QScrollArea* pScroll;

    DeviceList devlist;
    CustomPartitiondelegate* m_delegate;
    CreatePartitionFrame* m_createPartFrame;
    ModifyPartitionFrame* m_modifyPartFrame;
    QLabel* m_bootlabel;
    QComboBox* m_bootCombox;
    QLabel *m_BCToolTip;
    QString m_bootstr;
    Device::Ptr m_dev;

    QWidget* tableViewWidget ;
    QWidget* psWidget;
    QVBoxLayout* vtlayout;
    int layout1Width;

    QWidget *m_pFrame;
    int m_SCreenWidth;
    int m_SCreenHeight;
};
}
#endif // CUSTOMPARTITIONFRAME_H
