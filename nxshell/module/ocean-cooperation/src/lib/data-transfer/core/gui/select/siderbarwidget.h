#ifndef SIDERBARWIDGET_H
#define SIDERBARWIDGET_H
#include <QFrame>
#include <QListView>
#include <QMap>
#include <QStandardItemModel>
#include "item.h"
class QLabel;
class QHBoxLayout;
class QStackedWidget;
class SelectListView;
class ProgressBarLabel;
class QStorageInfo;

struct DiskInfo
{
    QString name{ "" };
    QString rootPath{ "" };
    quint64 size{ 0 };
    int curSelectFileNum{ 0 };
    int allFileNum{ 0 };
};
class SidebarWidget : public QListView
{
    Q_OBJECT
public:
    SidebarWidget(QWidget *parent = nullptr);
    ~SidebarWidget();

    QMap<QStandardItem *, DiskInfo> *getSidebarDiskList();

    void updateUserSelectFileSizeUi();
    void initSiderDataAndUi();
    void updateSiderDataAndUi(QStandardItem *siderbarItem, quint64 size);

    void updateAllSizeUi(const quint64 &size, const bool &isAdd);

    void addDiskFileNum(QStandardItem *siderbarItem, int num);

    void changeUI();
public slots:
    void updateSelectSizeUi(const QString &sizeStr);
    void updateSiderbarFileSize(quint64 fileSize, const QString &path);
    void getUpdateDeviceSingla();
    void updateCurSelectFileNum(const QString &path,bool isAdd);
    void onClick(const QModelIndex &index);
    void updateRemoteSpaceSize(int size);
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initData();
    void initUi();
    void updateDevice(const QStorageInfo &device, const bool &isAdd);

    void initSidebarSize();
    void initSidebarUi();

    void updateSidebarSize(QStandardItem *siderbarItem, quint64 size);
    void updateSiderbarUi(QStandardItem *siderbarItem);

    void updatePorcessLabel();
    void updateAllSize();

    void updateSelectFileNumState(QStandardItem *siderbarItem);
signals:
    void updateFileview(QStandardItem *siderbarItem, const bool &isAdd);
    void selectOrDelAllFileViewItem(QStandardItem *siderbarItem);
    void updateSelectBtnState(QStandardItem *siderbarItem,ListSelectionState state);
private:
    QMap<QStandardItem *, DiskInfo> sidebarDiskList;

    QLabel *userSelectFileSize{ nullptr };
    ProgressBarLabel *processLabel{ nullptr };

    QLabel *localTransferLabel{ nullptr };
    QList<QStorageInfo> deviceList;

    QString selectSizeStr{ "0B" };
    QString allSizeStr{ "0B" };
    quint64 allSize{ 0 };
    QString remoteSpaceSize{"0B"};
    QMap<QString, QString> UserPath;
};
#endif // SIDERBARWIDGET_H
