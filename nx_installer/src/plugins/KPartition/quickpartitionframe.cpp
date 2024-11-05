#include "quickpartitionframe.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include "delegate/customize_device_model.h"
#include "delegate/full_partition_delegate.h"
#include "partman/partition_server.h"
#include "../PluginService/ui_unit/messagebox.h"
#include "./frames/encryptsetframe.h"
namespace KInstaller {
using namespace Partman;
FullPartitionFrame::FullPartitionFrame(FullPartitionDelegate* quickDelegate, QWidget *parent) : QWidget(parent),
    m_quickDelegate(quickDelegate),
    m_devPath(""),
     var(0)
{

    initUI();
    addStyleSheet();
    initAllConnect();
    this->setFocus();
}
void FullPartitionFrame::initUI()
{
    QGridLayout* gLayout = new QGridLayout();
    gLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(gLayout);
    gLayout->setColumnStretch(0, 3);
    gLayout->setColumnStretch(1, 2);
    gLayout->setColumnStretch(2, 3);


    slistDisk = new LevelScrollDiskView();
    slistDisk->setObjectName("slistDisk");
    m_midStackLayout = new QStackedLayout();
    m_midStackLayout->setContentsMargins(0, 0, 0, 0);
    gLayout->addLayout(m_midStackLayout, 2, 1, 1, 1);
    m_midStackLayout->addWidget(slistDisk);

    //2021-4-16 当只有一个磁盘，而且不能选时（50G）提示信息
    m_tip = new QLabel;
    m_tip->hide();
    gLayout->addWidget(m_tip, 3, 1, 1, 1, Qt::AlignCenter);

    gLayout->addItem(new QSpacerItem(10, 190, QSizePolicy::Expanding, QSizePolicy::Expanding), 4, 1, 1, 1);
    gLayout->addItem(new QSpacerItem(10, 190, QSizePolicy::Expanding, QSizePolicy::Expanding), 5, 1, 1, 1);


     QHBoxLayout *midhlayout=new QHBoxLayout;
     midhlayout->setSpacing(60);
    //添加出厂备份
    m_firstback = new QCheckBox;
    m_firstback->setHidden(true);
    m_firstback->setObjectName("m_firstback");


    //全盘安装加密
    m_encryptBox = new QCheckBox;
    m_encryptBox->setHidden(true);
    m_encryptBox->setObjectName("encryptBox");

    //不可变系统安装
    m_InstallImmutableSystem = new QCheckBox;
    m_InstallImmutableSystem->setHidden(false);
    m_InstallImmutableSystem->setObjectName("InstallImmutableSystem");

    midhlayout->addItem(new QSpacerItem(1000, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    midhlayout->addWidget(m_firstback,Qt::AlignCenter);
//    midhlayout->addItem(new QSpacerItem(160, 10, QSizePolicy::Minimum, QSizePolicy::Minimum));
    midhlayout->addWidget(m_encryptBox,Qt::AlignCenter);
    midhlayout->addWidget(m_InstallImmutableSystem,Qt::AlignCenter);
    midhlayout->addItem(new QSpacerItem(1000, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));

     gLayout->addLayout(midhlayout, 6, 1, 1, 1);
    gLayout->setRowStretch(7, 1);

    if(slistDisk->m_disklist.length()==1)
    {
        currentDiskID(slistDisk->m_disklist.at(0)->devPath);
    }


    m_encryptBox->hide();
    m_encryptBox->setDisabled(true);
    m_encryptBox->setVisible(false);
    m_firstback->hide();
    m_firstback->setDisabled(true);
    m_firstback->setVisible(false);
    m_InstallImmutableSystem->setDisabled(false);
    m_InstallImmutableSystem->setVisible(true);
    translateStr();
}

void FullPartitionFrame::showListDisk()
{
    if(slistDisk) {
        m_midStackLayout->setCurrentWidget(slistDisk);
        this->setFocus();
    }
}

void FullPartitionFrame::showTableDisk()
{
//    if(customPartitionFrame) {
//        m_midStackLayout->setCurrentWidget(customPartitionFrame);
//    }

}

void FullPartitionFrame::repaintDevices(DeviceList devs)
{

    emit signalFinishedLoadDisk(slistDisk->m_disklist.count());
    slistDisk->m_disklist.clear();
    slistDisk->numDisk = 0;
    slistDisk->addItemsToList(devs);
    devlist = devs;
    if(devs.length() == 1 && (devs.at(0)->m_diskTotal/1024/1024/1024 - 20 < 0)) {
        m_tip->show();
    }
    //2021-3-4 14:37
//    if(devs.length() > 0)
    //        m_devPath = devs.at(0)->m_path;
}


void FullPartitionFrame::initAllConnect()
{
    connect(m_quickDelegate, &FullPartitionDelegate::deviceRefreshed, this, &FullPartitionFrame::repaintDevices);
    connect(slistDisk, &LevelScrollDiskView::signalWidgetSelected, this, &FullPartitionFrame::currentDiskID);
    connect(slistDisk,&LevelScrollDiskView::signalDataDiskSelected,this,&FullPartitionFrame::DataDiskCheck);
    connect(this, &FullPartitionFrame::leftCliked, slistDisk, &LevelScrollDiskView::checkDiskLeft);
    connect(this, &FullPartitionFrame::rightClicked, slistDisk, &LevelScrollDiskView::checkDiskRight);
    connect(m_encryptBox, &QCheckBox::clicked, this, &FullPartitionFrame::setEncryptyToDisk);
    connect(m_InstallImmutableSystem, &QCheckBox::clicked, this, &FullPartitionFrame::setInstallImmutableSystem);

//    connect(slistDisk, &LevelScrollDiskView::signalWidgetSelected, m_quickDelegate, &QuickPartitionDelegate::);
}

void FullPartitionFrame::addStyleSheet()
{
    QFile file(":/res/qss/KPartition.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = "";
    stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}


void FullPartitionFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}


void FullPartitionFrame::translateStr()
{
    m_tip->setText(tr("Please choose custom way to install, disk size less than 20G!"));//请选择自定义方式进行安装！
    m_encryptBox->setText(tr("Full disk encryption"));
    m_firstback->setText(tr("Factory backup"));
    m_InstallImmutableSystem->setText(tr("Installation of immutable system"));//不可变系统安装
//     promptLabel->setText(tr("Quick installation will format the entire hard drive!"
//                             "Please back up important data, and then select the system installation location."));//快速安装将格式化整个硬盘！
////     请备份好重要数据，然后选择系统安装位置。
}

void FullPartitionFrame::currentDiskID(QString devpath)
{
    if(devpath.isEmpty()) {
        emit signalSeclectedListItem(false);
        qDebug() << "devPath is error";
        return;
    }
    for(Device::Ptr device : devlist) {
        if(device->m_path == devpath) {
            m_devPath = devpath;
            m_quickDelegate->addSelectDevice(device);

        }
    }

    emit signalSeclectedListItem(true);

}

void FullPartitionFrame::keyPressEvent(QKeyEvent *event)
{
    qDebug() << Q_FUNC_INFO << event->key() << "key pressed! quick part";
    if (event->key() == Qt::Key_Left) {
        emit leftCliked();
    }
    else if (event->key() == Qt::Key_Right){
        emit rightClicked();
    }
    else if(event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
    {
        emit leftCliked();
    }
    else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        emit enterpressed();
    }
    else if(event->key() == Qt::Key_Backspace)
    {
        emit backspacepressed();
    }
    else if(event->key() == Qt::Key_Escape || event->key() == Qt::Key_Control)
    {
        this->setFocus();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}

void FullPartitionFrame::setEncryptyToDisk()
{
    if(m_encryptBox->isChecked()) {

        EncryptSetFrame *frame = new EncryptSetFrame();
        if(frame->exec() == frame->Accepted) {
            WriteSettingToIni("Encrypty", "encrypty", "true");
            WriteSettingToIni("Encrypty", "encryptyPWD", frame->m_pwd->getLineEditText());
        } else {
            m_encryptBox->setChecked(false);
            WriteSettingToIni("Encrypty", "encrypty", "false");
        }
    } else {
        WriteSettingToIni("Encrypty", "encrypty", "false");
    }

}

void FullPartitionFrame::DataDiskCheck(bool value)
{
        m_encryptBox->setDisabled(value);
}

void FullPartitionFrame::setInstallImmutableSystem()
{
    if(m_InstallImmutableSystem->isChecked()) {
        MessageBox *message = new MessageBox(this);
        QString strFirst(tr("Your current selected system is an immutable system, where the core files of this type of system cannot be modified after installation. There are certain differences in software installation, system updates, and user operations compared to traditional systems."));
        QString strSecond(tr("The current version is an experimental version. Please do not store important data, and use it with caution!"));
        message->setMessageInfo(strFirst, strSecond);
        if(message->exec() == QDialog::Accepted ) {
            m_InstallImmutableSystem->setChecked(true);
//            this->setFocus();
            WriteSettingToIni("config", "ostree_value", "true");
            if(!m_encryptBox->isHidden())
                m_encryptBox->setDisabled(true);
            if(!m_firstback->isHidden())
                m_firstback->setDisabled(true);
        }
        else{
            m_InstallImmutableSystem->setChecked(false);
            WriteSettingToIni("config", "ostree_value", "false");
            if(!m_encryptBox->isHidden())
                m_encryptBox->setDisabled(false);
            if(!m_firstback->isHidden())
                m_firstback->setDisabled(false);
        }
        delete message;
    } else {
        WriteSettingToIni("config", "ostree_value", "false");
        if(!m_encryptBox->isHidden())
            m_encryptBox->setDisabled(false);
        if(!m_firstback->isHidden())
            m_firstback->setDisabled(false);
    }
}

}
