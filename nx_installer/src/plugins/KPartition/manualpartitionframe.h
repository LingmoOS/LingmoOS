#ifndef MANUALPARTITIONFRAME_H
#define MANUALPARTITIONFRAME_H

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include "frames/progresslabel.h"
#include "./frames/tableWidget/cbasetablewidget.h"
#include "partman/device.h"
#include "partman/partition.h"
using namespace KInstaller;
class ManualPartitionFrame : public QWidget
{
    Q_OBJECT
public:
    explicit ManualPartitionFrame(QWidget *parent = nullptr);

    void addUI();
    void initAllConnect();
    void initTranslate();
    void addStyleSheet();
    void addCustomPartTable();

    void addTableWidget();
    void initLayout();
    void addPartProgress();
    void partValueChanged();

signals:

public slots:
    void slotCreatePartitionTable();
    void slotCreatePartition();
    void slotDeletePartition();
    void slotRestorePartitinTable();

public:
    CBaseTableWidget *tableWidget;
    QList<CBaseTableItem *> items;
    QGridLayout* gridLayout;
    //显示分区状态
    ProgressLabel *progressLbale;

    QFrame *installLine1, *installLine2, *installLine3;
    //创建分区Create partition,删除分区Delete partition,修改分区Modify partition
    //新建分区表New partition table,还原revert
    QPushButton *createPartBtn, *deletePartBtn, *modifyPartBtn, *newPartTableBtn, *reventBtn;
    QLabel* m_mainTitle;
    QPushButton* quikInstallBtn;
    QPushButton* customInstallBtn;
    QPushButton* coexistInstallBtn;

    QLabel* promptLabel;


};

#endif // MANUALPARTITIONFRAME_H
