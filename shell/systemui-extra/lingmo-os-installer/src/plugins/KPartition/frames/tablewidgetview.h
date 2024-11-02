#ifndef TABLEWIDGETVIEW_H
#define TABLEWIDGETVIEW_H

#include <QWidget>
#include "../partman/device.h"
#include "../partman/partition.h"
#include "./frames/tableWidget/cbasetablewidget.h"
#include "../PluginService/ui_unit/arrowtooltip.h"
#include <QVBoxLayout>
#include <QPushButton>
namespace KInstaller {
using namespace Partman;

class TableWidgetView :public QWidget
{
    Q_OBJECT
public:
    explicit TableWidgetView(QWidget *parent = nullptr);
    ~TableWidgetView();
    void initUI();
    void addTableWidget();
    void getItems(Device::Ptr dev);
    void addStyleSheet();
    void addPartitionRow(Partition::Ptr partition);
    void addFreeSpaceRow(Partition::Ptr partition);
    void clearTableWidget();
    void updateDiskTableWidgetInfo(Device::Ptr dev);
    void translateStr();
//    void updateTableShow();//update item show icon

private:
    void changeEvent(QEvent *event) override;


public slots:
    void buttonChangeClicked();
    void buttonDeleteClicked();
    void buttonAddClicked();
    void checkBoxChecked();
    void slotShowTableItem(int curRow, int curCol, int preRow, int preCol);
    void setCreatePartTableBtnState(bool flag);
    void showTableWidgetTip(int row, int col);
    void showResult();

signals:
    void signalChange(Device::Ptr dev, Partition::Ptr part);
    void signalDelete(Device::Ptr dev, Partition::Ptr part);
    void signalAdd(Device::Ptr dev, Partition::Ptr part);
    void signalCreateTable(Device::Ptr& dev);
    void signalSelectTable(TableWidgetView *tableWidget);
    void signalGetCreateResult();
//    void signalFinishedLoadDisk();



    void signalSelectRow();
public:
    QPushButton* createPartTableBtn;
    CBaseTableWidget *tableWidget;
    QList<CBaseTableItem *> items;
    QVBoxLayout* layout;
    Device::Ptr m_dev;
    Partition::Ptr m_curPartition;
    int m_curRow;
    bool m_formatORcreatorTable;
    QWidget* widget ;
    QHBoxLayout* hlayout;
    int colorID;
    QStringList header;
    ArrowWidget* wtip;

};
}
#endif // TABLEWIDGETVIEW_H
