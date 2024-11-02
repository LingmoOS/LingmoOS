#include "mainpartframe.h"
#include <QMessageBox>
#include <QFile>
#include <QDebug>
#include <QTextStream>

#include "../PluginService/klinguist.h"
#include "../PluginService/ksystemenv.h"
#include "../PluginService/ksystemsetting_unit.h"
#include "../PluginService/kcommand.h"
#include "partman/partition_unit.h"
#include "../PluginService/kthread.h"
#include "delegate/customize_device_model.h"
#include "delegate/custom_partition_delegate.h"
#include "delegate/full_partition_delegate.h"
#include "../PluginService/ui_unit/arrowtooltip.h"

using namespace KServer;
namespace KInstaller
{
using namespace Partman;
MainPartFrame::MainPartFrame(QWidget *parent) : MiddleFrameManager(parent),
    m_prepareFrame(new PrepareInstallFrame),
    m_partDelegate(new CustomPartitiondelegate(this)),
    m_quickDelegate(new FullPartitionDelegate(this)),
    m_partModel(new PartitionModel(this)),
    enumCheckBtn(InstallBtn::QuikPart),
    message(nullptr),
    m_partModelThread(new QThread)
{
    this->setObjectName("KPartition");

    //创建线程读取磁盘信息
    m_partModel->moveToThread(m_partModelThread);
    m_partModelThread->start();

    initUI();
    initAllConnect();
    addStyleSheet();
    translateStr();
    scanDevices();

    this->setFocus();
}

MainPartFrame::~MainPartFrame()
{
    quitThreadRun(m_partModelThread);
}

void MainPartFrame::initUI()
{
    QGridLayout* gLayout = new QGridLayout;
    gLayout->setContentsMargins(0, 0, 0, 0);
    m_Widget->setLayout(gLayout);
    gLayout->setColumnStretch(0, 1);
    gLayout->setColumnStretch(1, 6);
    gLayout->setColumnStretch(2, 1);

    m_mainTitle = new QLabel(this);
    m_mainTitle->setObjectName("mainTitle");
    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 1, 1, 1);
    gLayout->addWidget(m_mainTitle, 1, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);
    gLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Preferred), 2, 1, 1, 1);

    //添加快速安装,并存安装和自定义安装
    m_fullInstallBtn = new QPushButton();
    m_fullInstallBtn->setDefault(1);
    m_fullInstallBtn->setCheckable(true);
    m_fullInstallBtn->setChecked(true);
    m_customInstallBtn = new QPushButton();

    //添加按钮之间的分割线
    QHBoxLayout *hBtnLayout = new QHBoxLayout(this);
    QFrame* installLine1 = new QFrame(this);
    installLine1->setFrameShape(QFrame::VLine);
    installLine1->setFrameShadow(QFrame::Sunken);
    installLine1->setMaximumWidth(1);
    installLine1->setMaximumHeight(15);
    hBtnLayout->setSpacing(5);
    hBtnLayout->addWidget(m_fullInstallBtn);
    hBtnLayout->addWidget(installLine1);
    hBtnLayout->addWidget(m_customInstallBtn);

    gLayout->addLayout(hBtnLayout, 3, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);
    gLayout->addItem(new QSpacerItem(10, 30, QSizePolicy::Expanding, QSizePolicy::Fixed), 4, 1, 1, 1);

    m_fullInstallBtn->setObjectName("quikInstallBtn");
    m_customInstallBtn->setObjectName("customInstallBtn");
    m_mainStackWidget = new QStackedWidget();

    m_fullPartFrame = new FullPartitionFrame(m_quickDelegate, this);
    m_customPartFrame = new CustomPartitionFrame(m_partDelegate, this);

    m_mainStackWidget->addWidget(m_fullPartFrame);
    m_mainStackWidget->addWidget(m_customPartFrame);

    m_mainStackWidget->setCurrentWidget(m_fullPartFrame);

    //准备安装提示窗口
    m_loadpic = new QLabel;
    m_loadpic->setObjectName("loadpic");
    m_loadpic->setPixmap(QPixmap(":/res/pic/loading/lingmo-loading-0.svg").scaled(QSize(48, 48), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_loadpic->setScaledContents(true);
    gLayout->addWidget(m_loadpic, 5, 1, 2, 1, Qt::AlignCenter);
    timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, &QTimer::timeout, this, &MainPartFrame::finishLoadDisk);
    timer->start();

    this->setFocus();
    gLayout->addWidget(m_mainStackWidget, 5, 1, 1, 1);
    gLayout->setRowStretch(6, 1);
    m_nextBtn->setStyleSheet("color: rgba(255, 255, 255, 0.4);background:#2D6BAA;");
}

void MainPartFrame::showCustomPartFrame()
{
    m_fullPartFrame->nt_bt_last_state=m_nextBtn->isEnabled();

    if(m_fullPartFrame->m_InstallImmutableSystem->isChecked())
        return;

    if(m_customPartFrame) {
        m_mainStackWidget->setCurrentWidget(m_customPartFrame);
        enumCheckBtn = InstallBtn::CustomPart;
        m_customInstallBtn->setCheckable(true);
        m_customInstallBtn->setChecked(true);
        m_fullInstallBtn->setChecked(false);
        getChoiceDisk(true);
        //        m_coexistInstallBtn->setChecked(false);
    } else {
        qDebug() << "m_manualPartFrame:" << "not defined" ;
    }
}

void MainPartFrame::scanDevices()
{
    m_partModel->scanDevices();
}

void MainPartFrame::showQuikPartFrame()
{
    getChoiceDisk(m_fullPartFrame->nt_bt_last_state);
    if(m_fullPartFrame) {
        m_mainStackWidget->setCurrentWidget(m_fullPartFrame);
        enumCheckBtn = InstallBtn::QuikPart;
        m_customInstallBtn->setChecked(false);
        m_fullInstallBtn->setCheckable(true);
        m_fullInstallBtn->setChecked(true);
        this->setFocus();
    }

    else {
        qDebug() << "m_quikPartFrame:not defined" ;
    }
}
void MainPartFrame::showCoexistFrame()
{
    if(m_coexitPartFrame) {
        m_mainStackWidget->setCurrentWidget(m_coexitPartFrame);
        enumCheckBtn = InstallBtn::CoexistPart;
        m_customInstallBtn->setChecked(false);
        m_fullInstallBtn->setChecked(false);
    } else {
        qDebug() << "m_coexitPartFrame:not defined" ;
    }
}

void MainPartFrame::finishLoadDisk()
{
    var++;
    if(var > 7) {
        var = 0;
    }
    QString s = QString(":/res/pic/loading/lingmo-loading-%1.svg").arg(var);
    m_loadpic->setPixmap(QPixmap(s).scaled(QSize(48, 48), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_loadpic->setScaledContents(true);
    m_loadpic->update();
}

void MainPartFrame::initAllConnect()
{
    connect(m_fullPartFrame, &FullPartitionFrame::signalSeclectedListItem, this, &MainPartFrame::getChoiceDisk);
    connect(m_customPartFrame, &CustomPartitionFrame::signalNextBtn, this, &MainPartFrame::getChoiceDisk);
    connect(m_fullPartFrame, &FullPartitionFrame::enterpressed, this, &MainPartFrame::enterpressed);
    connect(m_customPartFrame, &CustomPartitionFrame::enterpressed, this, &MainPartFrame::enterpressed);
    connect(m_fullPartFrame, &FullPartitionFrame::backspacepressed, this, &MainPartFrame::backspacepressed);
    connect(m_customPartFrame, &CustomPartitionFrame::backspacepressed, this, &MainPartFrame::backspacepressed);
    connect(m_prepareFrame, &PrepareInstallFrame::backspacepressed, this, &MainPartFrame::backspacepressed);
    connect(this, &MainPartFrame::leftKeyPressed, m_fullPartFrame, &FullPartitionFrame::leftCliked);
    connect(this, &MainPartFrame::rightKeyPressed, m_fullPartFrame, &FullPartitionFrame::rightClicked);
    connect(m_fullPartFrame, &FullPartitionFrame::signalFinishedLoadDisk,this,&MainPartFrame::DiskNumCheck);

    connect(m_partModel, &PartitionModel::deviceRefreshed, m_quickDelegate, &FullPartitionDelegate::onDeviceRefreshed);
    connect(m_partModel, &PartitionModel::deviceRefreshed, m_partDelegate, &CustomPartitiondelegate::onDeviceRefreshed);
    //    connect(m_prepareFrame, &PrepareInstallFrame::signalNextStartInstalling, [=]{emit signalStartInstall();});
    connect(m_fullInstallBtn, &QPushButton::clicked, this, &MainPartFrame::showQuikPartFrame);
    connect(m_customInstallBtn, &QPushButton::clicked, this, &MainPartFrame::showCustomPartFrame);

    connect(m_customPartFrame, &CustomPartitionFrame::signalCreatePartTable, m_partDelegate, &CustomPartitiondelegate::createDeviceTable);
    //    qDebug() << "signalquickpart:" << connect(m_prepareFrame, &PrepareInstallFrame::signalQuickPart, m_partModel, &PartitionModel::quickPart);
    connect(m_prepareFrame, &PrepareInstallFrame::signalQuickPart, [=]{
        emit signalStartInstall();
        m_partModel->fullPart();
    });
    connect(m_prepareFrame, &PrepareInstallFrame::signalCustomPart, [=] {
        emit signalStartInstall();
        //        //设置鼠标等待状态
        //        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        m_partModel->customPart(m_partDelegate->operations());
        //        emit m_partModel->signalInstallCustomPart(m_partDelegate->operations());
    });
    qDebug() << "PartitionModel::customPartDone" << connect(m_partModel, &PartitionModel::customPartDone, [=](int execcode) {
        if(execcode == 0) {
            emit signalInstalScriptlFinished();
        } else
            emit signalInstallError(QString::number(execcode));
    });
    qDebug() << "PartitionModel::autoPartDone" << connect(m_partModel, &PartitionModel::fullPartDone, [=](int execcode) {
        if(execcode == 0) {
            emit signalInstalScriptlFinished();
        } else
            emit signalInstallError(QString::number(execcode));});
}

void MainPartFrame::clickNextButton()
{
    qDebug()<<"clicked the m_nextBtn is disable!";
    if(m_nextBtn->isEnabled()==false)
    {
        qDebug()<<"the m_nextBtn is disable!";
        return;
    }
    if(message != nullptr) {
        message->close();
    }
    message = new MessageBox;
    m_prepareFrame->setInstallBtn(enumCheckBtn);

    switch (enumCheckBtn) {
    case InstallBtn::QuikPart:
        if(!m_quickDelegate->DiskSizeValidate(m_fullPartFrame->m_devPath)) {
            return;
        }

        m_prepareFrame->setMainTitleStr(tr("Confirm Full Installation"));//快速安装
        m_prepareFrame->m_tipCheck->setText(tr("Formatted the whole disk"));
        if(!m_quickDelegate->isMBRPreferredDe()) {
            if(m_quickDelegate->DiskSizeLimited(m_fullPartFrame->m_devPath))
            {
                partStr = QString(tr("EFI partition #1 of %1 as vfat;\n"
                                     "boot partition #2 of  %1 as ext4;\n"
                                     "/ partition #3 of  %1 as ext4;\n")).arg(m_fullPartFrame->m_devPath);
            }
            else
            {
                partStr = QString(tr("EFI partition #1 of %1 as vfat;\n"
                                     "boot partition #2 of  %1 as ext4;\n"
                                     "/ partition #3 of  %1 as ext4;\n"
                                     "backup partition #4 of  %1 as ext4;\n"
                                     "data partition #5 of  %1 as ext4;\n"
                                     "swap partition #6 of  %1 as swap;\n")).arg(m_fullPartFrame->m_devPath);
            }
        } else {
            if(m_quickDelegate->DiskSizeLimited(m_fullPartFrame->m_devPath))
            {
                partStr = QString(tr("boot partition #1 of %1 as vfat;\n"
                                      "/ partition #2 of  %1 as ext4;\n")).arg(m_fullPartFrame->m_devPath);
            }
            else
            {
                partStr = QString(tr("boot partition #1 of %1 as vfat;\n"
                                     "extend partition #2 of  %1 ;\n"
                                     "/ partition #5 of  %1 as ext4;\n"
                                     "backup partition #6 of  %1 as ext4;\n"
                                     "data partition #7 of  %1 as ext4;\n"
                                     "swap partition #8 of  %1 as swap;\n")).arg(m_fullPartFrame->m_devPath);
            }
        }

        //添加数据盘
        WriteSettingToIni("config", "data-device","");
        foreach (DiskInfoView* mb,m_fullPartFrame->slistDisk->m_disklist) {
            if(mb->dataSel->isChecked())
            {
                WriteSettingToIni("config", "data-device", mb->devPath);
                partStr.append("\n"+mb->devPath+" set to data device;\n");
            }
        }

        m_prepareFrame->setPartInfo(partStr);
        writeSettingIni(InstallBtn::QuikPart);
        break;
    case InstallBtn::CoexistPart:
        m_prepareFrame->setMainTitleStr(tr("Coexist Install"));//并存安装
        break;
    case InstallBtn::CustomPart:
        if(!validatePartition()) {
            message->setMessageInfo(validateMessage.left(validateMessage.size() - 1));
            message->setCancleHidden();
            if (message->exec() == QDialog::Accepted ){
                this->setFocus();
                return;
            } else if(message->execid == 1){
                this->setFocus();
                return;
            }
        } else if(validatePartition() == -1) {
            message->setMessageInfo(validateMessage.left(validateMessage.size() - 1));
            if(message->exec() == QDialog::Rejected  && message->execid == 1){
                this->setFocus();
                return;
            }
        }
        //提示为解决磁盘当前的分区表类型和引导不一致导致不能 正常安装问题
        if(!checkLoaderAndPartitionTable(m_partDelegate->findDevice(m_customPartFrame->m_bootCombox->currentText().split(" ").at(0)))) {
            message->setMessageInfo(checkLoaderAndPartitionTableStr());
            if(message->exec() == QDialog::Rejected && message->execid == 1) {
                this->setFocus();
                return;
            }
        }
        m_prepareFrame->setMainTitleStr(tr("Confirm Custom Installation"));//确认自定义安装

        /*
         * 如果您继续，以下所列出的修改内容将会写入到磁盘中。或者，您也可以手动来进行其它修改。
         * 警告：任何已删除和将要格式化的分区上的数据都将被摧毁。
         */
        partStr = m_partDelegate->comboxOperatorPartitions();
        m_prepareFrame->setPartInfo(partStr);
        m_prepareFrame->m_tipCheck->setText(tr("Confirm the above operations"));
        m_partDelegate->writeMountOperate();
        writeSettingIni(InstallBtn::CustomPart);

        break;

    }
    emit signalStackFrameManager((QWidget*)m_prepareFrame);
}

//bug 51780 add message
QString MainPartFrame::checkLoaderAndPartitionTableStr()
{
    bool table = checkLoaderAndPartitionTable(m_partDelegate->findDevice(m_customPartFrame->m_bootCombox->currentText().split(" ").at(0)));
    bool start = checkSpecialModel();

    QString info;
    if(start != table) {
        if(start == 1) {
            info = QString(tr("BootLoader method %1 inconsistent with the disk partition table \n"
                              "type %2.").arg("UEFI").arg("MSDos"));
        } else {
            info = QString(tr("BootLoader method %1 inconsistent with the disk partition table \n"
                              "type %2, cannot have efi partition.").arg("legacy").arg("GPT"));
        }
    } else {
        if(start == 0) {
            info = QString(tr("BootLoader method %1 inconsistent with the disk partition table \n"
                              "type %2, cannot have efi partition.").arg("legacy").arg("MSDos"));
        }
    }
    return info;
}

//返回值为1：检测通过，0：检测不通过，-1：检测不存在可继续操作
int MainPartFrame::validatePartition()
{
    validateMessage.clear();
    m_validateStates = m_partDelegate->validate();
    if(m_validateStates.isEmpty()) {
        return 1;
    } else {
        for(ValidateState state : m_validateStates) {
            qDebug() <<"MainPartFrame::validatePartition:" << state->state();
            switch (state->state()) {
            case ValidateState::InvalidId:
                validateMessage.append(tr("InvalidId\n"));
                break;
            case ValidateState::BootFsInvalid:
                validateMessage.append(tr("Boot filesystem invalid\n"));
                return 0;
            case ValidateState::BootPartNumberInvalid:
                validateMessage.append(tr("Boot partition not in the first partition\n"));
                return 0;
            case ValidateState::BootTooSmall:
                validateMessage.append(tr("Boot partition too small\n"));
                return 0;
            case ValidateState::BootMissing:
                validateMessage.append(tr("No boot partition\n"));
                return 0;
            case ValidateState::EfiMissing:
                validateMessage.append(tr("No Efi partition\n"));
                return 0;
            case ValidateState::EfiSizeError:
                validateMessage.append(tr("the EFI partition size should be set between 256MB and 2GB\n"));
                return 0;
            case ValidateState::EfiTooMany:
                validateMessage.append(tr("Only one EFI partition is allowed\n"));
                return 0;
            case ValidateState::EfiPartNumberInvalid:
                validateMessage.append(tr("Efi partition number invalid\n"));
                break;
            case ValidateState::RootMissing:
                validateMessage.append(tr("No root partition\n"));
                return 0;
            case ValidateState::RootTooSmall:
                validateMessage.append(tr("Recommended Root partition size greater than 15GiB"));//根分区大小大于15GiB，华为机器要求大于25GiB
                return 0;
            case ValidateState::PartitionTooSmall:
                validateMessage.append(tr("Partition too small\n"));
                break;
//            case ValidateState::BackUpPartitionTooSmall:
//                validateMessage.append(tr("BackUp partition too small\n"));
//                break;
            case ValidateState::RepeatedMountPoint:
                validateMessage.append(tr("Repeated mountpoint\n"));
                return 0;
//            case ValidateState::DataORBackupPartition:
//                validateMessage.append(tr("No backup partition\n"));
//                return -1;//
                //           case ValidateState::HUAWEIKirin990:
                //               validateMessage.append(tr("HUAWEI Kirin 990 format EFI partition to fat16"));
                //               return 0;
                //           case ValidateState::MipsBootExt3:
                //               validateMessage.append(tr("Mips format Boot partition to Ext3\n"));
                //               return 0;
            case ValidateState::MipsNotSupportEFI:
                validateMessage.append(tr("This machine not support EFI partition\n"));
                return 0;
          //  case ValidateState::InvalidFS:
            //    validateMessage.append(tr("The filesystem type FAT16 or FAT32 is not fully-function filesystem,\n"
             //                             "except EFI partition, other partition not proposed"));
                return 0;
            default:
                break;
            }
        }
        return true;
    }
}

void MainPartFrame::addStyleSheet()
{
    QFile file(":/res/qss/KPartition.css");
    qDebug() << file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void MainPartFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void MainPartFrame::translateStr()
{
    QString strLan = ReadSettingIni("config", "language");
    if("kk_KZ" == strLan || "ug_CN" == strLan || "ky_KG" == strLan){
        m_fullInstallBtn->setFixedWidth(250);
        m_customInstallBtn->setFixedWidth(250);
    }else if("bo_CN" == strLan ){
        m_fullInstallBtn->setFixedWidth(150);
        m_customInstallBtn->setFixedWidth(150);
    }else if("mn_MN" == strLan ){
        m_fullInstallBtn->setFixedWidth(260);
        m_customInstallBtn->setFixedWidth(260);
    }else if( "de_DE" == strLan){
        m_fullInstallBtn->setFixedWidth(230);
        m_customInstallBtn->setFixedWidth(230);
    }else if( "es_ES" == strLan|| "fr_FR" == strLan){
        m_fullInstallBtn->setFixedWidth(200);
        m_customInstallBtn->setFixedWidth(200);
    }else{
        m_fullInstallBtn->setFixedWidth(120);
        m_customInstallBtn->setFixedWidth(120);
    }

    m_mainTitle->setText(tr("Choose Installation Method"));
    m_fullInstallBtn->setText(tr("Full install"));
    m_customInstallBtn->setText(tr("Custom install"));
    //    m_coexistInstallBtn->setText(tr("Coexist install"));
    m_nextBtn->setText(tr("Next"));
}

void MainPartFrame::writeSettingIni(InstallBtn btn)
{
    if(btn == InstallBtn::QuikPart) {
        WriteSettingToIni("config", "devpath", m_fullPartFrame->m_devPath);
        WriteSettingToIni("config", "automatic-installation", QString::number(1));

        WriteSettingToIni("config", "factory-backup", QString::number(
                              m_fullPartFrame->m_firstback->isChecked()));

    } else if(btn == InstallBtn::CustomPart) {
        WriteSettingToIni("config", "automatic-installation", QString::number(0));
        WriteSettingToIni("custompartition", "partitions", m_partDelegate->iniPartitions);
        WriteSettingToIni("custompartition", "bootloader", m_partDelegate->getBootLoadPath());

    }
}

void MainPartFrame::getChoiceDisk(bool flag)
{
    if(flag) {
        m_nextBtn->setEnabled(true);
        m_nextBtn->setStyleSheet("background: rgb(29, 142, 255);");
    } else {
        m_nextBtn->setEnabled(false);
        m_nextBtn->setStyleSheet("background:#2D6BAA;");
    }
}

void MainPartFrame::keyPressEvent(QKeyEvent *event)
{
    this->setFocus();
    qDebug() << Q_FUNC_INFO << "keyPressEvent";
    if (event->key() == Qt::Key_Left) {
        qDebug() << "left key pressed!";
        emit leftKeyPressed();
    }
    else if (event->key() == Qt::Key_Right){
        qDebug() << "right key pressed!";
        emit rightKeyPressed();
    }
    else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        emit enterpressed();
    }
    else if(event->key() == Qt::Key_Backspace)
    {
        emit backspacepressed();
    }
    else if (event->key() == Qt::Key_Control || event->key() == Qt::Key_Escape) {
        this->setFocus();
    }
    else
        QWidget::keyPressEvent(event);
}

void MainPartFrame::DiskNumCheck(int num)
{
    m_loadpic->hide();
    m_fullInstallBtn->setEnabled(true);
    m_customInstallBtn->setEnabled(true);
    getChoiceDisk(1);//使能往下一步的按钮
    if(m_fullInstallBtn->isChecked()==true)
    {
        if(num==1)
        {
            m_fullPartFrame->m_devPath=m_fullPartFrame->slistDisk->m_disklist.at(0)->devPath;
        }
        else
        getChoiceDisk(0);
    }
}

void MainPartFrame::setSizebyFrame(QWidget *pFrame, int SCreenWidth, int SCreenHeight)
{
    m_pFrame = pFrame;
    m_SCreenWidth = SCreenWidth;
    m_SCreenHeight= SCreenHeight;
    if(m_customPartFrame){
        m_customPartFrame->m_pFrame = m_pFrame;
        m_customPartFrame->m_SCreenWidth = m_SCreenWidth;
        m_customPartFrame->m_SCreenHeight = m_SCreenHeight;
    }
}

}

