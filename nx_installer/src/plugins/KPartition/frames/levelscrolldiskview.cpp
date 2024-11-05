#include "levelscrolldiskview.h"
#include <QScroller>
#include <QDebug>
#include "partman/datastruct.h"
const int disknamelen = 20;
const int disklength = 4;
LevelScrollDiskView::LevelScrollDiskView(QWidget *parent) : QWidget(parent),
    blViewPic(false),
    numDisk(0),
    m_curDevPath(""),
    movesize(0)
{
    initUI();
    addStyleSheet();
    initAllConnect();
    emit signalWidgetSelected("");
}

void LevelScrollDiskView::initUI()
{
    layout = new QHBoxLayout();
    this->setLayout(layout);

    //2021-3-6将以前的水平布局形式更换为listwidget形式展示
    listWidget = new QListWidget();

    listWidget->setObjectName("ScrollList");
    //设置为列表显示模式
    listWidget->setViewMode(QListView::ListMode);
    //设置列表从左往右排列
    listWidget->setFlow(QListView::LeftToRight);
    listWidget->setItemAlignment(Qt::AlignCenter);
    listWidget->setSpacing(10);
    listWidget->setFocusPolicy(Qt::NoFocus);
    //屏蔽水平与垂直的滑动条
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //设置为像素滚动
    listWidget->setHorizontalScrollMode(QListWidget::ScrollPerItem);
//    listWidget->setResizeMode(QListView::Adjust);

    //设置鼠标左键拖动
    QScroller::grabGesture(listWidget,QScroller::LeftMouseButtonGesture);
//    qDebug() << "CScrollList-listWidget-size" << listWidget->size();

    hlistlayout = new QHBoxLayout(this);
    hlistlayout->setSpacing(10);


    leftBtn = new QPushButton();
    leftBtn->setMinimumWidth(50);
    leftBtn->setMaximumHeight(50);
    leftBtn->setFlat(true);
    leftBtn->hide();
    leftBtn->setObjectName("ScrollListbtn");
    leftBtn->setIcon(QIcon(":/data/png/slidershow/prev.svg"));
//    m_prev->setIconSize(QSize(16, 16));
    leftBtn->setStyleSheet(QLatin1String("QPushButton{border: none;outline: none;color: white;padding:16px 16px 16px 16px;background:transparent;border:none;}"
                                        "QPushButton:hover{"
                                        "width: 48px;height: 48px;background: rgba(255, 255, 255, 0.35);border-radius: 6px;}"
                                        " QPushButton:checked{ "
                                        " height: 48px;background: rgba(255, 255, 255, 0.25); border-radius: 6px;}"
                                        "QPushButton:pressed{height: 48px;background: rgba(255, 255, 255, 0.25); border-radius: 6px;}"
                                        "QPushButton:foucs{height: 48px;background: rgba(255, 255, 255, 0.25); border-radius: 6px;}"));

    rightBtn = new QPushButton();
    rightBtn->setMinimumWidth(50);
    rightBtn->setMaximumHeight(50);
    rightBtn->setFlat(true);
    rightBtn->hide();
    rightBtn->setObjectName("ScrollListbtn");
    rightBtn->setIcon(QIcon(":/data/png/slidershow/next.svg"));
//    m_next->setIconSize(QSize(16, 16));
    rightBtn->setStyleSheet(QLatin1String("QPushButton{border: none;outline: none;color: white;padding:16px 16px 16px 16px;background:transparent;border:none;}"
                                        "QPushButton:hover{"
                                        "width: 48px;height: 48px;background: rgba(255, 255, 255, 0.35);border-radius: 6px;}"
                                        " QPushButton:checked{ "
                                        " height: 48px;background: rgba(255, 255, 255, 0.25); border-radius: 6px;}"
                                        "QPushButton:pressed{height: 48px;background: rgba(255, 255, 255, 0.25); border-radius: 6px;}"
                                        "QPushButton:foucs{height: 48px;background: rgba(255, 255, 255, 0.25); border-radius: 6px;}"));
    layout->addWidget(leftBtn, Qt::AlignLeft);
    layout->addWidget(listWidget, Qt::AlignHCenter);

//    layout->addLayout(hlistlayout, Qt::AlignCenter);
    layout->addWidget(rightBtn, Qt::AlignRight);
    layout->setSpacing(5);
}

void LevelScrollDiskView::initAllConnect()
{
    connect(leftBtn, &QPushButton::clicked, this, &LevelScrollDiskView::checkDiskLeft);
    connect(rightBtn, &QPushButton::clicked, this, &LevelScrollDiskView::checkDiskRight);

}

void LevelScrollDiskView::showBtn()
{
    if(m_disklist.length() > disklength)
    {
        leftBtn->setVisible(true);
        rightBtn->setVisible(true);
    } else {
        leftBtn->setVisible(false);
        rightBtn->setVisible(false);
    }
}

QListWidget* LevelScrollDiskView::getListWidget()
{
    if(listWidget != nullptr) {
        return listWidget;
    } else {
        return nullptr;
    }
}



void LevelScrollDiskView::addItemsToList(DeviceList devlist)
{
    for (QLayoutItem* item = hlistlayout->takeAt(0);
          item != nullptr;
          item = hlistlayout->takeAt(0)) {
       delete item->widget();
       delete item;
       item = nullptr;
     }
    QSize size;
    numDisk = devlist.length();
    for(int i = 0; i < numDisk; i++) {
        disk = new DiskInfoView();
        disk->setDevPath(devlist.at(i)->m_path);
        m_disklist.append(disk);
        m_Group.addButton(disk->dataSel,i);
        disk->mIndex=i;
        connect(disk,&DiskInfoView::signalClickView,this,&LevelScrollDiskView::selectView);
        QListWidgetItem *item = new QListWidgetItem();
        size = disk->size();
        item->setTextAlignment(Qt::AlignCenter);
        listWidget->addItem(item);
        item->setSizeHint(size);
        listWidget->setItemWidget(item, disk);

//        hlistlayout->addWidget(disk, Qt::AlignCenter);

        //显示从系统读到的硬盘信息
//        disk->setDiskType(devlist.at(i)->m_osName);
        disk->setDiskPath(devlist.at(i)->m_path);
        QString model = devlist.at(i)->m_model;
//        QString stmp = "";
//        int len = model.length();
//        if(len > disknamelen) {
//            stmp = QString("%1\n%2").arg(model.mid(0, disknamelen)).arg(model.mid(disknamelen, model.length()));
//        } else
//            stmp = model;
//        disk->setDiskName(stmp);
        disk->setDiskName(model);
        QString total = "";
        total = GetBiByateValue(QString("%1").arg(devlist.at(i)->m_diskTotal));
        qint64 usedspace;
        if(devlist.at(i)->m_partTableType == PartTableType::Empty) {
            usedspace = 0;
         } else {
            //100.0*int  ----->  double
            usedspace = (devlist.at(i)->m_diskTotal - devlist.at(i)->m_diskFreeSpace);

        }
        QString fspace = "";
        fspace = GetBiByateValue(QString("%1").arg(usedspace));
        qDebug() << "fspace/total:" << fspace << total;
        disk->setDiskSize(QString("%1/%2").arg(fspace).arg(total));
        double d =100.0 * usedspace / devlist.at(i)->m_diskTotal;
        disk->setProgressValue(d);
        qint64 t = devlist.at(i)->m_diskTotal/1024/1024/1024;
        int r = t - 20;//20G
        if(r < 0) {
            m_disklist.at(i)->setWidgetDisabled();
            m_disklist.at(i)->checkable = false;
            m_disklist.at(i)->setProgressStyle();
        }
        connect(disk, &DiskInfoView::signalClicked, this, &LevelScrollDiskView::showWidgetChecked);
        connect(disk, &DiskInfoView::destroyed, [=]{disk->deleteLater();});
    }


    showBtn();
    update();
    if(numDisk > disklength) {
        setListwidgetSize(QSize(((size.width() + 20)* disklength), size.height()));
    } else
        setListwidgetSize(QSize(((size.width() + 20) * numDisk), size.height()));
}

void LevelScrollDiskView::showWidgetChecked(QString devpath)
{

    for(int i = 0; i < m_disklist.length(); i++) {
        if(!m_disklist.at(i)->checkable) {
            m_disklist.at(i)->checkpic->setVisible(false);
            continue;
        }
        if(m_disklist.at(i)->devPath != devpath) {
            m_disklist.at(i)->checkpic->setVisible(false);

        } else if(m_disklist.at(i)->checkpic->isVisible())
        {
                m_disklist.at(i)->checkpic->setVisible(false);
              emit signalWidgetSelected("");
        }
            else{
                m_disklist.at(i)->checkpic->setVisible(true);

            emit signalWidgetSelected( devpath);
            }
        }

    if(m_disklist.length()==1)
    {
      //  m_disklist.at(0)->dataSel->setDisabled(true);
      //  m_disklist.at(0)->dataSel->setVisible(false);
    }
}

void LevelScrollDiskView::selectView(int k)
{
    if(DataDeviceIndex==k)
    {

        if( m_Group.button(k)->isChecked())
        {   m_Group.setExclusive(false);
            m_Group.button(k)->setChecked(false);
            DataDeviceIndex=-1;
             m_Group.setExclusive(true);
         }
    }
    else
        DataDeviceIndex=k;

    if(DataDeviceIndex>-1)
       emit signalDataDiskSelected(true);
    else
       emit signalDataDiskSelected(false);
}

int LevelScrollDiskView::checkDisk()
{
    int checknum = 0;

    for(int i = 0; i < m_disklist.length(); i++)
    {
        if(m_disklist.at(i)->checkpic->isVisible() == true)
        {
            checknum += 1;
        }
        else
            continue;
    }

    qDebug() << "Unchecked disk and disk number: " << checknum << m_disklist.length();
    return checknum;
}

void LevelScrollDiskView::checkDiskLeft()
{
    int check = checkDisk();
    qDebug() << "disk quantity: " << m_disklist.length();
    qDebug() << check << "disk checked!";
    if(m_disklist.length() == 0)
        return;
    if(m_disklist.length() <= 1 || check == 0 )
    {

        if(!m_disklist.at(0)->checkable) {
            m_disklist.at(0)->checkpic->setVisible(false);
        } else
            m_disklist.at(0)->checkpic->setVisible(true);
        setListItem(0);
        emit signalWidgetSelected(m_disklist.at(0)->devPath);
    }
    else
        for(int i = 0; i < m_disklist.length(); i++)
        {
            if(!m_disklist.at(i)->checkable) {
                m_disklist.at(i)->checkpic->setVisible(false);
                continue;
            }
            qDebug() << m_disklist.at(0)->devPath << m_disklist.at(m_disklist.length()-1)->devPath;
            if(m_disklist.at(i)->checkpic->isVisible() == true)
            {
                m_disklist.at(i)->checkpic->setVisible(false);
                if(i == 0)
                {
                    qDebug() << m_disklist.at(0) << m_disklist.at(m_disklist.length()-1);
                    qDebug() << "switch to " << m_disklist.at(0)->devPath;
                    m_disklist.at(m_disklist.length() - 1)->checkpic->setVisible(true);
                    setListItem(m_disklist.length() - 1);
                    emit signalWidgetSelected(m_disklist.at(1)->devPath);
                    break;
                }
                else
                {
                    qDebug() << "switch to " << m_disklist.at(i - 1)->devPath;
                    m_disklist.at(i-1)->checkpic->setVisible(true);
                    setListItem(i - 1);
                    emit signalWidgetSelected(m_disklist.at(i - 1)->devPath);
                    break;
                }

            }
        }
}

void LevelScrollDiskView::checkDiskRight()
{
    int check = checkDisk();
    qDebug() << "disk quantity: " << m_disklist.length();
    qDebug() << check << "disk checked!";
    if(m_disklist.length() == 0)
        return;
    if(m_disklist.length() <= 1 || check == 0)
    {
        if(!m_disklist.at(0)->checkable) {
            m_disklist.at(0)->checkpic->setVisible(false);
        } else
            m_disklist.at(0)->checkpic->setVisible(true);
        setListItem(0);
        emit signalWidgetSelected(m_disklist.at(0)->devPath);
    }
    else
        for(int i = 0; i < m_disklist.length(); i++)
        {
            if(!m_disklist.at(i)->checkable) {
                m_disklist.at(i)->checkpic->setVisible(false);
                continue;
            }
            qDebug() << m_disklist.at(0)->devPath << m_disklist.at(m_disklist.length()-1)->devPath;
            if(m_disklist.at(i)->checkpic->isVisible() == true)
            {
                m_disklist.at(i)->checkpic->setVisible(false);
                if(i == m_disklist.length() - 1)
                {
                    qDebug() << "switch to "<< m_disklist.at(0)->devPath;
                    m_disklist.at(0)->checkpic->setVisible(true);
                    setListItem(0);
                    emit signalWidgetSelected(m_disklist.at(0)->devPath);
                    break;
                }
                else
                {
                    qDebug() << "switch to " << m_disklist.at(i+1)->devPath;
                    m_disklist.at(i+1)->checkpic->setVisible(true);
                    setListItem(i + 1);
                    emit signalWidgetSelected(m_disklist.at(i+1)->devPath);
                    break;
                }
            }
        }
}

void LevelScrollDiskView::addStyleSheet()
{
    QFile file(":/res/qss/LevelScrollDiskView.css");
    qDebug() << Q_FUNC_INFO << file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = QString("");
    stylesheet = filetext.readAll();
    file.close();
    this->setStyleSheet(stylesheet);
}

void LevelScrollDiskView::setListwidgetSize(QSize size)
{
    listWidget->setMaximumSize(size);
    listWidget->setMinimumSize(size);
}
QSize LevelScrollDiskView::getListwidgetSize()
{
    return listWidget->size();
}

void LevelScrollDiskView::setListItem(int id)
{
    listWidget->setCurrentRow(id);
    m_disklist.at(id)->widget->setStyleSheet("#widget{background-color: rgba(255, 255, 255, 0.25);}");
    for (int i = 0; i < listWidget->count(); ++i) {
        if(i == id) {
            continue;
        } else {
            m_disklist.at(i)->widget->setStyleSheet("#widget{background-color: rgba(0, 0, 0, 0.15);}");
        }
    }
}
