#include "modifypartitionframe.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QEvent>
#include <QRect>
#include <QDebug>
#include <QTextStream>
#include <QListView>
#include <QStyledItemDelegate>
#include <QGridLayout>
#include <QScreen>
#include "../PluginService/ksystemsetting_unit.h"
#include "../PluginService/ui_unit/xrandrobject.h"
#include "../PluginService/sysInfo/kernelfilesystemfs.h"
#include "../PluginService/sysInfo/machine.h"
#include "QMessageBox"

namespace KInstaller {

ModifyPartitionFrame::ModifyPartitionFrame(CustomPartitiondelegate* delegate, QWidget *parent) : QWidget(parent),m_delegate(delegate),
    mountFileStr(""),
    closeTip(nullptr)
{
    this->setObjectName("ModifyPartitionFrame");
    initUI();
    addStyleSheet();
    initAllConnect();
    initBackground();

//    // 无边框
//    setWindowFlags(Qt::FramelessWindowHint);

//    //全屏
//    showFullScreen();

//    QScreen* NowScreen=QGuiApplication::primaryScreen();
//    int SCreenHeight=NowScreen->size().height();
//    int SCreenWidth=NowScreen->size().width();
//    setFixedSize(m_SCreenWidth, m_SCreenHeight);
//qDebug()<<" -----------------ModifyPartitionFrame SCreenWidth:"<<m_SCreenWidth<<" SCreenHeight："<<m_SCreenHeight;
//    QRect screenGeometry = QApplication::desktop()->screenGeometry();
//    qDebug() << "Screen size: " << screenGeometry.size();
}

ModifyPartitionFrame::~ModifyPartitionFrame()
{

}


void ModifyPartitionFrame::setModifyPartition( Partition::Ptr partition,int SCreenWidth,int SCreenHeight)
{
    m_partition = partition;
    mountFileStr = partition->m_mountPoint;
    fileFormatStr = findNameByFSType(partition->fs);
    if(fileFormatStr == "") {
        fileFormat->setCurrentText(tr("unused"));
    }
    else if (fileFormatStr.contains("efi")) {
        fileFormat->setCurrentText("efi");
        mountFile->setEnabled(true);
        mountFile->setCurrentText(mountFileStr);
    }
    else if (fileFormatStr.contains("swap")) {
        fileFormat->setCurrentText("linux-swap");
        mountFile->setEnabled(true);
        mountFile->setCurrentText(mountFileStr);
    }
    else if (fileFormatStr.contains("lingmo-data")) {
//        fileFormat->setCurrentText("lingmo-data");
        fileFormat->setCurrentText(QObject::tr("lingmo-data"));
        mountFile->setEnabled(false);
        mountFile->setCurrentText(mountFileStr);
    }
    else
    {
        fileFormat->setCurrentText(fileFormatStr);
        mountFile->setEnabled(true);
        mountFile->setCurrentText(mountFileStr);
    }

    if(partition->blFormatPartition) {
        checkFormat->setChecked(true);
        checkFormat->setDisabled(false);
    }
    else {
        checkFormat->setChecked(false);
        checkFormat->setDisabled(false);
    }

    m_SCreenWidth=SCreenWidth;
    m_SCreenHeight=SCreenHeight;
    setFixedSize(m_SCreenWidth, m_SCreenHeight);

    update();
}

void ModifyPartitionFrame::initUI()
{
    QGridLayout* gLayout = new QGridLayout;
    gLayout->setMargin(24);
    this->setLayout(gLayout);
    gLayout->setColumnStretch(0, 3);
    gLayout->setColumnStretch(1, 2);
    gLayout->setColumnStretch(2, 3);
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
    gLayout->addWidget(m_mainTitle, 2, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);
    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 3, 1, 1, 1);

    fsLabel= new QLabel;
    fsLabel->setObjectName("label");
    fileFormat = new QComboBox;
    fileFormat->setWindowFlags(Qt::FramelessWindowHint);
    fileFormat->setAttribute(Qt::WA_TranslucentBackground, true);
    QWidget* containerObj1 = static_cast<QWidget*>(fileFormat->view()->parent()); // QComboBoxPrivateContainer对象
    containerObj1->setAttribute(Qt::WA_TranslucentBackground, true);
    //    fileFormat->setView(new QListView());
    //    fileFormat->view()->parentWidget()->setWindowFlags(Qt::FramelessWindowHint);
    //    fileFormat->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground,true);
    fileFormat->setObjectName("combox");
    QStyledItemDelegate* itemDelegate = new QStyledItemDelegate();
    fileFormat->setItemDelegate(itemDelegate);
    //    filesystemList = m_delegate->getFsTypeList();
    //2021-5-24根据内核支持的文件系统对文件系统进行加载
    filesystemList = getKernelFs();

    fileFormat->setMaxVisibleItems(5);
    fileFormat->setView(new QListView());
    fileFormat->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    fileFormat->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    fileFormat->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    gLayout->addWidget(fsLabel, 4, 1, 1, 1, Qt::AlignLeft);
    gLayout->addWidget(fileFormat, 5, 1, 1, 1);


    checkFormat = new QCheckBox(this);
    checkFormat->adjustSize();
    gLayout->addWidget(checkFormat, 6, 1, 1, 1, Qt::AlignLeft);
    checkFormat->setStyleSheet("QCheckBox { font-size: 14px; font-family: Noto Sans CJK SC; font-weight: 400; line-height: 64px; color: #FFFFFF; background:transparent;opacity: 1;}"
                               "QCheckBox::indicator:enabled:unchecked:focus {image: url(:/res/pic/checkboxcheck-hover.svg);}"
                               "QCheckBox::indicator:disabled:unchecked { image: url(:/res/pic/discheckboxcheck.svg); }"
                               "QCheckBox::indicator:enabled:unchecked {image: url(:/res/pic/checkboxcheck.svg); }"
                               "QCheckBox::indicator:disabled:checked {image: url(:/res/pic/discheckboxchecked.svg);}"
                               "QCheckBox::indicator:enabled:checked {image: url(:/res/pic/checkboxchecked.svg);}"
                );
    gLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 7, 1);

    mountLabel= new QLabel;
    mountLabel->setObjectName("label");

    mountFile = new QComboBox;
    mountFile->setObjectName("combox");
    mountFile->setView(new QListView());
    mountFile->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    mountFile->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    mountFile->setItemDelegate(itemDelegate);
    mountFile->setWindowFlags(Qt::FramelessWindowHint);
    mountFile->setAttribute(Qt::WA_TranslucentBackground, true);
    QWidget* containerObj = static_cast<QWidget*>(mountFile->view()->parent()); // QComboBoxPrivateContainer对象
    containerObj->setAttribute(Qt::WA_TranslucentBackground, true);
    //    mountFile->setView(new QListView());
    //    mountFile->view()->parentWidget()->setWindowFlags(Qt::FramelessWindowHint);
    //    mountFile->view()->parentWidget()->setAttribute(Qt::WA_TranslucentBackground,true);
    mountedList = m_delegate->getMountPoints();
    mountFile->addItems(mountedList);
    mountFile->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    mountFile->setMaxVisibleItems(5);
    mountFile->setEditable(true);

    gLayout->addWidget(mountLabel, 8, 1, 1, 1, Qt::AlignLeft);
    gLayout->addWidget(mountFile, 9, 1, 1, 1);
    gLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 10, 1);

    tipLabel = new QLabel;
    gLayout->addWidget(tipLabel, 11, 1, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 12, 1, 1, 1);
    gLayout->setRowStretch(13, 1);

    m_OKBtn = new QPushButton;
    m_OKBtn->setObjectName("OKBtn");
    gLayout->addWidget(m_OKBtn, 14, 1, 1, 1, Qt::AlignCenter);
    gLayout->setRowStretch(15, 1);
    translateStr();
}
void ModifyPartitionFrame::initAllConnect()
{
    connect(m_close, &QPushButton::clicked, [=]{this->close();});
    connect(m_OKBtn, &QPushButton::clicked, this, &ModifyPartitionFrame::slotOKBtn);

    connect(fileFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFileFormat(int)));
    connect(mountFile, &QComboBox::currentTextChanged, this, &ModifyPartitionFrame::changeMountFile);
    connect(this, &ModifyPartitionFrame::destroyed, this, &ModifyPartitionFrame::deleteLater);
}
void ModifyPartitionFrame::translateStr()
{
    checkFormat->setText(tr("Format partition."));//格式化此分区
    m_OKBtn->setText(tr("OK"));//确定
    //    fsLabel->setText(tr("File system type:"));//文件系统类型
    fsLabel->setText(tr("Used to:"));//
    m_mainTitle->setText(tr("Modify Partition"));//添加分区
    //    tipLabel->setText(tr("*Please ensure that the root partition size is at least 15G.\n"
    //                         "*Please make sure the root partition is /dev/sda2 on FT1500A."));//*请保证分区大小至少为8G。   *FT1500A上请保证分区为/dev/sda2。


    mountLabel->setText(tr("Mount point"));//挂载点
    fileFormat->clear();
    for(int i = 0; i < filesystemList.length(); i++ ) {
        QString str = filesystemList.at(i);
        if(str == "unused") {
            str = tr("unused");
        } else if(str == "lingmo-data") {
            str = QObject::tr("lingmo-data");
        }
        fileFormat->addItem(str);
        fileFormat->setItemData(i, filesystemList.at(i));
    }
}
void ModifyPartitionFrame::addStyleSheet()
{
    QFile file(":/res/qss/CreatePartitionFrame.css");
    qDebug() << Q_FUNC_INFO << file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}
void ModifyPartitionFrame::initBackground()
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
    QPixmap pixmap = QPixmap(":/data/png/lmbg.png").scaled(rect.width(), rect.height(), Qt::KeepAspectRatioByExpanding);
    QPalette pal(this->palette());
    pal.setBrush(backgroundRole(), QBrush(pixmap));
    this->setAutoFillBackground(true);
    this->setPalette(pal);
}

void ModifyPartitionFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void ModifyPartitionFrame::changeFileFormat(int index)
{
    m_filesystem = fileFormat->itemData(index).toString();
    if(fileFormatStr != m_filesystem) {
        checkFormat->setChecked(true);
        checkFormat->setEnabled(false);
    } else {
        checkFormat->setChecked(true);
        checkFormat->setEnabled(true);
    }

    if(m_filesystem.contains("swap", Qt::CaseInsensitive) || m_filesystem.contains("unused", Qt::CaseInsensitive)) {
        mountFile->setDisabled(true);
        mountFile->setCurrentText("");
//        mountFileStr = "";
        checkFormat->setChecked(false);
    } else if(m_filesystem.contains("efi", Qt::CaseInsensitive)) {
        mountFile->setCurrentText("/boot/efi");
//        mountFileStr = "/boot/efi";
        mountFile->setDisabled(false);
    } else if(m_filesystem.contains("lingmo-data", Qt::CaseInsensitive)) {
         mountFile->setCurrentText("/data");
        mountFile->setEnabled(false);
    } else {
        mountFile->setDisabled(false);
        mountFile->setCurrentText("");
    }

}
void ModifyPartitionFrame::changeMountFile(const QString &text)
{
    if(fileFormat->currentData().toString().contains("efi", Qt::CaseInsensitive) && text != "/boot/efi"){
            mountFile->setCurrentText("");//efi挂载点可以取消
            mountFile->setDisabled(false);
            checkFormat->setChecked(false);
            checkFormat->setEnabled(true);
    }
    else if(text == "/") {
        checkFormat->setChecked(true);
        checkFormat->setEnabled(false);
    }
    else{
        if(fileFormatStr == m_filesystem )
        {
            checkFormat->setChecked(false);
            checkFormat->setEnabled(true);
        }
        else{
            checkFormat->setChecked(true);
            checkFormat->setEnabled(true);
        }
    }
    //  mountFileStr = text;
}

void ModifyPartitionFrame::slotOKBtn()
{
//    //龙芯只允许一个boot分区
//    if(getMachineArch()==MachineArch::LOONGSON && 0)
//    {

    m_filesystem = fileFormat->itemData(fileFormat->currentIndex()).toString();//防止初始值为空

        if(mountFile->currentText().compare("/boot")==0 &&
               mountFileStr.compare("/boot")!=0 )//所有架构都不可有重复的boot挂载点
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
                        if( q->m_mountPoint.contains("boot"))

                        {
                            if(FSType::EFI == q->fs)//过滤掉efi分区
                            {
                                continue;
                            }

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

    QString mountCurrentText = mountFile->currentText();
    //手动输入挂载点判断 2021-3-22
    if(!mountCurrentText.isEmpty()) {
        if(!mountCurrentText.startsWith("/") && !mountCurrentText.contains("unused", Qt::CaseInsensitive)) {
            mountFile->setItemText(0, "");
            message = new MessageBox;
            message->setMessageInfo(tr("Mount point starts with '/'"));
            message->setCancleHidden();
            if(message->exec() == QDialog::Accepted )
                return;
        }
    }

    Partition::Ptr partition(m_partition);
    bool checked;
    if(m_filesystem == ("unused")) {
        partition->blFormatPartition = false;
        //        checkFormat->setCheckable(false);
        mountCurrentText = "";
        partition->m_mountPoint = "";
        partition->m_isMounted = false;
        m_delegate->updateMountPoint(partition, mountCurrentText);
        //        m_delegate->formatPartition(partition, partition->fs, mountCurrentText, false);

    } else {
        if(m_filesystem.contains("lingmo-data", Qt::CaseInsensitive)) {
            partition->fs = FSType::LingmoData;
            if(checkFormat->isChecked())//需要格式化
            {
                partition->blFormatPartition = checkFormat->isChecked();
                m_delegate->formatPartition(partition, partition->fs, mountCurrentText, true);
            }
            else//不需要格式化
                m_delegate->updateMountPoint(partition, mountCurrentText);

        } else {
//            m_filesystem = fileFormat->itemText(fileFormat->currentIndex());
//            m_filesystem = fileFormat->itemData(fileFormat->currentIndex()).toString();
            FSType tmpfs = findFSTypeByName(m_filesystem);
            checked = checkFormat->isChecked();
            partition->blFormatPartition = checkFormat->isChecked();

            if(m_partition->fs != tmpfs || checked ) {
                partition->fs = tmpfs;
                m_delegate->formatPartition(partition, tmpfs, mountCurrentText, checked);
            }
        }

        if(m_partition->m_mountPoint != mountFile->currentText()) {
            qDebug()<<"currentData="+mountFile->currentText();
            partition->m_mountPoint =mountFile->currentText();
            m_delegate->updateMountPoint(partition, partition->m_mountPoint);
        }
    }
    emit finished();
}

bool ModifyPartitionFrame::eventFilter(QObject *watched, QEvent *event)
{
    if(watched->objectName() == "close") {
        if(event->type() == QEvent::Enter) {
            if(closeTip != nullptr) {
                closeTip->close();
            }
            closeTip = new ArrowWidget();
            closeTip->setObjectName("ArrowWidget");
            closeTip->setDerection(ArrowWidget::Derection::down);
            closeTip->setText(tr("close"));

            closeTip->move(m_close->x() + (m_close->width() - closeTip->width()) / 2, m_close->y() + m_close->height() + 4);
            //            w->move(pos.x() - 20, pos.y() - w->height());
            closeTip->show();
            connect(m_close, &QPushButton::clicked, [=]{closeTip->close();});
            return true;

        } else if(event->type() == QEvent::Leave){
            if(closeTip != nullptr) {
                closeTip->close();
                return false;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}


}
