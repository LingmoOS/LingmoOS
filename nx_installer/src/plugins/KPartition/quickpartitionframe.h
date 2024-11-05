#ifndef QUIKPARTITIONFRAME_H
#define QUIKPARTITIONFRAME_H
#include <parted/parted.h>
#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QStackedLayout>
#include <QWidgetList>
#include "frames/diskinfoview.h"
#include "frames/levelscrolldiskview.h"
#include "partman/partition_server.h"
#include "createpartitionframe.h"
#include "custompartitionframe.h"
#include "delegate/full_partition_delegate.h"


namespace KInstaller{
enum class PARTTYPEBTN {
    quikInstall,
    coexistInstall,
    customInstall
};
class FullPartitionFrame : public QWidget
{
    Q_OBJECT
public:
    explicit FullPartitionFrame(FullPartitionDelegate* quickDelegate, QWidget *parent = nullptr);

public:
    void initUI();
    void initAllConnect();
    void translateStr();
    void addStyleSheet();

    QWidgetList getChildWidgets();
    void showListDisk();
    void showTableDisk();
    void addItemsToList();


signals:
    void signalSeclectedListItem(bool flag);
    void signalFinishedLoadDisk(int DiskNum);//
    void enterpressed();
    void backspacepressed();
    void leftCliked();
    void rightClicked();
private:
    void changeEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event);
public slots:

    void activatedItem(QListWidgetItem *);
    void ChangedItem(QListWidgetItem*,QListWidgetItem*);
    void currentDiskID(QString devpath);
    void repaintDevices(DeviceList devs);
    void setEncryptyToDisk();
    void DataDiskCheck(bool value);
    void setInstallImmutableSystem();

public:

    LevelScrollDiskView *slistDisk;

    int numDisk;//是否选择了需要安装的磁盘
    PARTTYPEBTN partType;
    QLabel* promptLabel;

    QStackedLayout* m_midStackLayout;
    FullPartitionDelegate* m_quickDelegate;
    int m_devCount;
    QString m_devPath;
    QLabel* m_tip;

    DeviceList devlist;
    QTimer* timer;
    QLabel* m_loadpic;
    int var;
    //加密
    QCheckBox* m_encryptBox;
    //出厂备份
    QCheckBox* m_firstback;
    //不可变系统安装
    QCheckBox* m_InstallImmutableSystem;

    bool nt_bt_last_state=false;

};
}
#endif // PARTITIONFRAME_H
