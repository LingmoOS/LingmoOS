#ifndef LEVELSCROLLDISKVIEW_H
#define LEVELSCROLLDISKVIEW_H

#include <QObject>
#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include "diskinfoview.h"
#include "partman/device.h"
#include "QButtonGroup"

using namespace KInstaller;
using namespace Partman;

class LevelScrollDiskView : public QWidget
{
    Q_OBJECT
public:
    explicit LevelScrollDiskView(QWidget *parent = nullptr);
    void initUI();
    void initAllConnect();
    void addItemsToList(DeviceList devlist);
    void addDot();
    void showBtn();

    void setListwidgetSize(QSize size);
    QSize getListwidgetSize();
    void addStyleSheet();
    void setListItem(int id);

    QListWidget* getListWidget();
private:
//    bool eventFilter(QObject *watched, QEvent *event);
//    void paintEvent();
    int checkDisk();
signals:
    void signalWidgetSelected(QString dev);
    void signalDataDiskSelected(bool value);


public slots:
    void clickLeftBtn();
    void clickRightBtn();
    void showWidgetCheckdIteed(QString dev);
    void showWidgetChecked(QString devpath);
    void selectView(int k);

    void checkDiskLeft();
    void checkDiskRight();
public:
    QListWidget *listWidget;
    QList<DiskInfoView*> m_disklist;
    QButtonGroup m_Group;
    QWidget *widget;
    DiskInfoView* disk;
    bool blViewPic;
    int numDisk;
    QString m_curDevPath;
    QPushButton *leftBtn, *rightBtn;
    QGridLayout *dotlayout;
    QList<QLabel*> dotlist;
    int movesize;

    int DataDeviceIndex=-1;


    QHBoxLayout *layout;
    QHBoxLayout* hlistlayout;
};

#endif // LEVELSCROLLDISKVIEW_H
