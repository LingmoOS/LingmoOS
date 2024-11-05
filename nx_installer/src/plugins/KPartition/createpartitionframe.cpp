#include "createpartitionframe.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QRect>
#include <QEvent>
#include <QDebug>
#include <QListView>
#include <QTextStream>
#include <QGridLayout>
#include <QScreen>
#include "../PluginService/ksystemsetting_unit.h"
#include "../PluginService/ui_unit/xrandrobject.h"
#include "../PluginService/sysInfo/kernelfilesystemfs.h"
#include "qscrollbar.h"
#include "../PluginService/sysInfo/machine.h"
#include "qmessagebox.h"
using namespace KServer;
namespace KInstaller {
using namespace Partman;
CreatePartitionFrame::CreatePartitionFrame(CustomPartitiondelegate* delegate,
                                            DeviceList *pdevlst,
                                           QWidget *parent) : QWidget(parent),
    m_delegate(delegate),
    devlst(pdevlst),
    mountFileStr(""),
    m_tTip(nullptr)
{
    this->setObjectName("CreatePartitionFrame");
    initUI();
    addStyleSheet();
    initAllConnect();
    initBackground();
//    this->setFocus();
}

void CreatePartitionFrame::initBackground()
{
    // 无边框
//    setWindowFlags(Qt::FramelessWindowHint |
//                   Qt::CustomizeWindowHint|
////                   Qt::WindowStaysOnTopHint |
//                   Qt::Tool |
//                   Qt::Dialog);

    //全屏
    showFullScreen();

     //设置背景blue色
    QRect rect = KServer::GetScreenRect();
    QPixmap pixmap = QPixmap(":/data/png/sp2-bk@1x.png").scaled(rect.width(), rect.height(), Qt::KeepAspectRatioByExpanding);
    QPalette pal(this->palette());
    pal.setBrush(backgroundRole(), QBrush(pixmap));
    this->setAutoFillBackground(true);
    this->setPalette(pal);


}

void CreatePartitionFrame::setNewPartition(Device::Ptr dev,
                                           Partition::Ptr partition,bool pritypebl,int SCreenWidth,int SCreenHeight)
{
    m_partition = partition;
    m_dev = dev;

    sizeStr = GetMebiByateValue(m_partition->getByteLength());
    sizeStr.chop(5);
    sizeEdit->setValue(sizeStr);

    fileFormatStr = m_partition->m_fsname;
    mountFileStr = m_partition->m_mountPoint;
    priTypebl = pritypebl;
    startPosbl = true;
    primaryPart->setChecked(priTypebl);
    lvmPart->setChecked(!priTypebl);
    startPosition->setChecked(startPosbl);
    endPosition->setChecked(!startPosbl);
    fileFormat->setCurrentIndex(0);
    mountFile->setCurrentIndex(0);

    if(extendedPartitionIndex(dev->partitions) > -1) {
        primaryPart->setEnabled(false);
        priTypebl = false;
        lvmPart->setChecked(true);
    } else {
        primaryPart->setEnabled(true);
    }

    if(m_dev->m_partTableType == PartTableType::MsDos) {
        lvmPart->setVisible(true);
    } else {
        lvmPart->setVisible(false);
    }

    m_SCreenWidth=SCreenWidth;
    m_SCreenHeight=SCreenHeight;
    setFixedSize(m_SCreenWidth, m_SCreenHeight);

    update();
}

void CreatePartitionFrame::initUI()
{
    QGridLayout* gLayout = new QGridLayout;
    gLayout->setMargin(24);
    this->setLayout(gLayout);
    gLayout->setColumnStretch(0, 3);
    gLayout->setColumnStretch(1, 1);
    gLayout->setColumnStretch(2, 1);
    gLayout->setColumnStretch(3, 3);
    gLayout->setVerticalSpacing(5);

    QHBoxLayout* hlay = new QHBoxLayout;
    hlay->setMargin(0);
    m_close = new QPushButton;
    m_close->setObjectName("close");
    m_close->installEventFilter(this);
    hlay->addStretch();
    hlay->addWidget(m_close);
    gLayout->addLayout(hlay, 0, 3, 1, 1, Qt::AlignCenter | Qt::AlignTop);


    m_mainTitle = new QLabel;
    m_mainTitle->setObjectName("mainTitle");
    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 1, 1, 1, 1);
    gLayout->addWidget(m_mainTitle, 2, 1, 1, 2, Qt::AlignCenter | Qt::AlignTop);
    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 3, 1, 1, 1);

    typeLabel= new QLabel;
    typeLabel->setObjectName("label");
    primaryPart = new QRadioButton;
    lvmPart = new QRadioButton;
//    partGroup = new QButtonGroup;
//    partGroup->addButton(primaryPart, 0);
//    partGroup->addButton(lvmPart, 1);

    gLayout->addWidget(typeLabel, 4, 1, 1, 1, Qt::AlignLeft);
    gLayout->addWidget(primaryPart, 5, 1, 1, 1, Qt::AlignLeft);
    gLayout->addWidget(lvmPart, 5, 2, 1, 1, Qt::AlignLeft);
    gLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 6, 1);

    positionLabel= new QLabel;
    positionLabel->setObjectName("label");
    posGroup = new QButtonGroup;
    startPosition = new QRadioButton;
    endPosition = new QRadioButton;
    posGroup->addButton(startPosition, 0);
    posGroup->addButton(endPosition, 1);


    gLayout->addWidget(positionLabel, 7, 1, 1, 1, Qt::AlignLeft);
    gLayout->addWidget(startPosition, 8, 1, 1, 1, Qt::AlignLeft);
    gLayout->addWidget(endPosition, 8, 2, 1, 1, Qt::AlignLeft);
    gLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 9, 1);

    fsLabel= new QLabel;
    fsLabel->setObjectName("label");
    fileFormat = new QComboBox;
    fileFormat->setView(new QListView);
    fileFormat->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    fileFormat->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    fileFormat->setObjectName("combox");
//    filesystemList = m_delegate->getFsTypeList();
    //2021-5-24根据内核支持的文件系统对文件系统进行加载
    filesystemList = getKernelFs();

//    fileFormat->addItems(filesystemList);
    fileFormat->setMaxVisibleItems(5);
    fileFormat->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    fileFormat->view()->verticalScrollBar()->setStyleSheet("QScrollBar{width:15px;}");
    gLayout->addWidget(fsLabel, 10, 1, 1, 2, Qt::AlignLeft);
    gLayout->addWidget(fileFormat, 11, 1, 1, 2);
    gLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 12, 1);

    mountLabel= new QLabel;
    mountLabel->setObjectName("label");

    mountFile = new QComboBox;
    mountFile->setView(new QListView);
    mountFile->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    mountFile->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    mountFile->setObjectName("combox");
    mountedList = m_delegate->getMountPoints();
    mountFile->addItems(mountedList);
    mountFile->setEditable(true);
    mountFile->setMaxVisibleItems(5);
    mountFile->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    gLayout->addWidget(mountLabel, 13, 1, 1, 2, Qt::AlignLeft);
    gLayout->addWidget(mountFile, 14, 1, 1, 2);
    gLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 15, 1);

    sizeLabel = new QLabel;
    sizeLabel->setObjectName("label");

    sizeEdit = new LineEditAddReduce();
//    sizeEdit->setFocusPolicy(Qt::StrongFocus);

    gLayout->addWidget(sizeLabel, 16, 1, 1, 1, Qt::AlignLeft);
    gLayout->addWidget(sizeEdit, 17, 1, 1, 2);
    gLayout->addItem(new QSpacerItem(30, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 18, 1);


    tipLabel = new QLabel;
    gLayout->addWidget(tipLabel, 19, 1, 1, 3, Qt::AlignLeft | Qt::AlignTop);
    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 20, 1, 1, 1);
    gLayout->setRowStretch(19, 1);

    m_OKBtn = new QPushButton;
    m_OKBtn->setObjectName("OKBtn");
    gLayout->addWidget(m_OKBtn, 21, 1, 1, 2, Qt::AlignCenter);
    gLayout->setRowStretch(22, 1);
    translateStr();

}


void CreatePartitionFrame::initAllConnect()
{
    connect(primaryPart,SIGNAL(clicked()),this,SLOT(clickPrimaryPart()));
    connect(lvmPart,SIGNAL(clicked()),this,SLOT(clickLvmPart()));
    connect(startPosition,SIGNAL(clicked()),this,SLOT(clickStartPos()));
    connect(endPosition,SIGNAL(clicked()),this,SLOT(clickEndPos()));
    connect(m_close, &QPushButton::clicked, [=]{this->close();});
    connect(m_OKBtn, &QPushButton::clicked, this, &CreatePartitionFrame::slotOKBtn);

    connect(fileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFileFormat(int)));
    connect(mountFile, &QComboBox::currentTextChanged, this, &CreatePartitionFrame::changeMountFile);
    connect(this, &CreatePartitionFrame::destroyed, this, &CreatePartitionFrame::deleteLater);
}

void CreatePartitionFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}


void CreatePartitionFrame::setPartitionValue(Partition::Ptr partition)
{
    sizevalue = sizeEdit->getValue();
    partition->m_devPath = m_dev->m_path;
    if(priTypebl) {
        //primary
        partition->m_type = PartitionType::Normal;
    } else {
        //logical
        partition->m_type = PartitionType::Logical;
    }

    if(startPosbl) {
        //start
        partition->m_sectorStart = m_partition->m_sectorStart;
        partition->m_sectorSize = m_partition->m_sectorSize;
        partition->m_sectorEnd = m_partition->m_sectorStart + (sizevalue * kMebiByte / m_partition->m_sectorSize);
    } else {
        //end
        partition->m_sectorEnd = m_partition->m_sectorEnd;
        partition->m_sectorStart = m_partition->m_sectorEnd - (sizevalue * kMebiByte / m_partition->m_sectorSize);
        partition->m_sectorSize = m_partition->m_sectorSize;
    }
    if(mountFileStr.isEmpty()) {
        partition->m_isMounted = false;
    } else {
        partition->m_isMounted = true;
        partition->m_mountPoint = mountFile->currentText();
//        m_delegate->updateMountPoint(partition, mountFile->currentText());
    }
    if(fileFormat->itemText(fileFormat->currentIndex()).contains("efi", Qt::CaseInsensitive)) {
        partition->fs = FSType::EFI;
        partition->m_isMounted= true;
        partition->m_mountPoint = mountFile->currentText();
    } else
        partition->fs = findFSTypeByName(fileFormat->itemText(fileFormat->currentIndex()));
//    m_delegate->formatPartition(partition, partition->fs, mountpoint);
}

void CreatePartitionFrame::slotOKBtn()
{
//    //龙芯只允许一个boot分区
//    if(getMachineArch()==MachineArch::LOONGSON && 0)
//    {
    fileFormatStr = fileFormat->itemData(fileFormat->currentIndex()).toString();//防止初始值为空

    if(mountFile->currentText().compare("/")==0 )
//            && mountFileStr.compare("/")!=0 )//所有架构都不可有重复的根挂载点
    {
        Device::Ptr p;
        Partition::Ptr q;
        //遍历所有挂在磁盘，是否已存在boot挂载点的分区
        for(int k=0;k<devlst->count();k++)
        {
            p=(Device::Ptr)(devlst->at(k));
            for(int i=0;i<p->partitions.count();i++)
            {
                q=(Partition::Ptr)(p->partitions.at(i));
                if( q->m_mountPoint.compare("/")==0)
                {
                    message=new MessageBox;
                    message->setMessageInfo(tr("The disk can only create one root partition!"));
                    message->setCancleHidden();
                    message->show();
                    mountFile->setCurrentIndex(0);
                    return;
                }
            }
        }
    }

    if(mountFile->currentText().compare("/boot")==0 &&
           mountFileStr.compare("boot")!=0 )//所有架构都不可有重复的boot挂载点
    {
        Device::Ptr p;
        Partition::Ptr q;
        //遍历所有挂在磁盘，是否已存在boot挂载点的分区
        for(int k=0;k<devlst->count();k++)
        {
            p=(Device::Ptr)(devlst->at(k));
            for(int i=0;i<p->partitions.count();i++)
            {
                q=(Partition::Ptr)(p->partitions.at(i));
                if( q->m_mountPoint.compare("/boot")==0)
                {
                    message=new MessageBox;
                    message->setMessageInfo(tr("The disk can only create one boot partition!"));
                    message->setCancleHidden();
                    message->show();
                    mountFile->setCurrentIndex(0);
                    return;
                }
            }
        }
    }

//    }

    //手动输入挂载点判断 2021-3-22
    if(!mountFileStr.isEmpty()) {
        if(!mountFileStr.startsWith("/") && !mountFileStr.contains("unused", Qt::CaseInsensitive)) {
            mountFile->setItemText(0, "");
            message = new MessageBox;
            message->setMessageInfo(tr("Mount point starts with '/'"));
            message->setCancleHidden();
            message->show();
//            if(message->exec() == QDialog::Accepted )//去掉此句，以防止点击右上角关闭按钮时跳过此段逻辑
            return;
        }
    }
    Partition::Ptr partition(new Partition);
    setPartitionValue(partition);
    if(fileFormat->itemText(fileFormat->currentIndex()).contains("unused", Qt::CaseInsensitive)) {
        partition->blFormatPartition = false;
        m_delegate->createPartition(m_partition,
                                    partition->m_type,
                                    startPosbl,
                                    FSType::Empty,
                                    partition->m_mountPoint,
                                    sizevalue * kMebiByte / m_partition->m_sectorSize,
                                    0);
    }
    if(fileFormatStr.contains("lingmo-data", Qt::CaseInsensitive)) {
        partition->fs = FSType::LingmoData;
    }
    m_delegate->createPartition(m_partition,
                                partition->m_type,
                                startPosbl,
                                partition->fs,
                                partition->m_mountPoint,
                                sizevalue * kMebiByte / m_partition->m_sectorSize);
    emit finished();
    //this->releaseKeyboard();
}

void CreatePartitionFrame::keyPressEvent(QKeyEvent *event)
{
    qDebug() << Q_FUNC_INFO << "create partition";
    if(event->key() == Qt::Key_Escape || event->key() == Qt::Key_Control)
    {
//        this->setFocus();
    }
    else
        QWidget::keyPressEvent(event);
}

void CreatePartitionFrame::addStyleSheet()
{
    QFile file(":/res/qss/CreatePartitionFrame.css");
    qDebug() << Q_FUNC_INFO << file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void CreatePartitionFrame::translateStr()
{
    m_OKBtn->setText(tr("OK"));//确定
//    fsLabel->setText(tr("File system type:"));//文件系统类型
    fsLabel->setText(tr("Used to:"));//
    m_mainTitle->setText(tr("Create Partition"));//添加分区


    mountLabel->setText(tr("Mount point"));//挂载点

    positionLabel->setText(tr("Location for the new partition"));//新分区的位置
    endPosition->setText(tr("End of this space"));//剩余空间尾部
    startPosition->setText(tr("Beginning of this space"));//剩余空间头部

    typeLabel->setText(tr("Type for the new partition:"));//新分区的类型
    lvmPart->setText(tr("Logical"));//逻辑分区
    primaryPart->setText(tr("Primary"));//主分区
//    tipLabel->setText(tr("*Please ensure that the root partition size is at least 15G.\n"
//                         "*Please make sure the root partition is /dev/sda2 on FT1500A."));//*请保证分区大小至少为8G。   *FT1500A上请保证分区为/dev/sda2。

    sizeLabel->setText(tr("Size(MiB)"));//新建分区大小
    fileFormat->clear();
    for(int i = 0; i < filesystemList.length(); i++ ) {
        QString str = filesystemList.at(i);
        if(str == "unused") {
            str = QObject::tr("unused");
        } else if(str == "lingmo-data") {
            str = QObject::tr("lingmo-data");
        }
        fileFormat->addItem(str);
        fileFormat->setItemData(i, filesystemList.at(i));
    }
}

void CreatePartitionFrame::clickPrimaryPart()
{
    priTypebl = true;
    primaryPart->setChecked(priTypebl);
    lvmPart->setChecked(!priTypebl);
}
void CreatePartitionFrame::clickLvmPart()
{
    priTypebl = false;
    primaryPart->setChecked(priTypebl);
    lvmPart->setChecked(!priTypebl);
}
void CreatePartitionFrame::clickStartPos()
{
    startPosbl = true;
    startPosition->setChecked(startPosbl);
    endPosition->setChecked(!startPosbl);
}
void CreatePartitionFrame::clickEndPos()
{
    startPosbl = false;
    startPosition->setChecked(startPosbl);
    endPosition->setChecked(!startPosbl);
}
void CreatePartitionFrame::changeFileFormat(int index)
{
    fileFormatStr = fileFormat->itemData(index).toString();
    fileFormat->setCurrentText(fileFormatStr);
    if(fileFormatStr.contains("swap", Qt::CaseInsensitive) ||
       fileFormatStr.contains("unused", Qt::CaseInsensitive) ) {
        mountFile->setEnabled(false);
        mountFile->setCurrentText("");
    }
    else if(fileFormatStr.contains("lingmo-data", Qt::CaseInsensitive)) {
        mountFile->setCurrentText("/data");
        mountFile->setEnabled(false);
    }
    else if(fileFormatStr.contains("efi", Qt::CaseInsensitive)) {
        mountFile->setCurrentText("/boot/efi");
        mountFile->setEnabled(false);
    }
    else {
        mountFile->setEnabled(true);
    }

    if(fileFormatStr.contains("efi", Qt::CaseInsensitive)) {
        tipLabel->setText(tr("Recommended efi partition size is between 256MiB and 2GiB."));//EFI分区大小256MiB到2GiB之间

    } else {
        tipLabel->setText("");
    }

}
void CreatePartitionFrame::changeMountFile(const QString &text)
{
    mountFileStr = text;
    mountFile->setCurrentText(mountFileStr);
    //for(int k=0;k<m_delegate->getMountPoints().count();k++)
      // {
    //  if(((QString)m_delegate->getMountPoints().at(k)).contains("boot"))
     //  {

    //QMessageBox::information("只允许建一个boot分区");


       //}
   // }


    if(text.contains("/boot", Qt::CaseInsensitive)) {
        tipLabel->setText(tr("Recommended boot partition size is between 500MiB and 2GiB."));//boot分区大小推荐500MiB到2GiB之间
    } else if(text == "/") {
        tipLabel->setText(tr("Recommended Root partition size is greater than 15GiB"));//根分区大小大于15GiB，华为机器要求大于25GiB
    } else {
        tipLabel->setText("");
    }
}


bool CreatePartitionFrame::eventFilter(QObject *watched, QEvent *event)
{
    if(watched->objectName() == "close") {
        if(event->type() == QEvent::Enter) {
            if(m_tTip != nullptr) {
                m_tTip->close();
            }
            m_tTip = new ArrowWidget();
            m_tTip->setObjectName("ArrowWidget");
            m_tTip->setDerection(ArrowWidget::Derection::down);
            m_tTip->setText(tr("close"));
            m_tTip->move(m_close->x() + (m_close->width() - m_tTip->width()) / 2, m_close->y() + m_close->height() + 4);
//            w->move(pos.x() - 20, pos.y() - w->height());
            m_tTip->show();
            connect(m_close, &QPushButton::clicked, [=]{m_tTip->close();});

        } else if(event->type() == QEvent::Leave){
            if(m_tTip != nullptr) {
                m_tTip->close();
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}


}
