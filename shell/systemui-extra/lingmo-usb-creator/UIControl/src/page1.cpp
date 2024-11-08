/*
 * Copyright (C) 2002  KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "page1.h"
#include "include/xatom-helper.h"
#include <QStandardPaths>
#include <QThread>
Page1::Page1()
{
    initControlQss();//初始化样式
    getStorageInfo();//获取磁盘信息
}
void Page1::initControlQss()
{
    edit = new line;
    edit->installEventFilter(this);
    topwidget=new QWidget(this);
    fram=new QFrame(this);
    fram->setFrameShape(QFrame::NoFrame);
    fram->setGeometry(40,95,450,320);
    fram->hide();
    topwidget->setFixedSize(450,320);
//    tabUdisk1 = new QLabel(fram);
//    tabUdisk1->setFixedSize(127,127);
    tabUdisk2 =new QPushButton(fram);
    tabUdisk2->setFixedSize(128, 128);
    tabUdisk2->setProperty("isRoundButton", true);
    tabUdisk2->setIcon(QIcon::fromTheme("list-add-symbolic"));
    tabUdisk2->setIconSize(QSize(50,50));
    urlIso=new QLabel(topwidget);
    urlIso1=new QLabel(fram);
    QPixmap pix(":/data/button lightmode inactive.png");
    urlIso1->setText(tr("Drag and drop the image file here"));
//    tabUdisk1->setPixmap(pix);
    urlIso->setWordWrap(true);
    urlIso->setEnabled(false);
    urlIso->setText(tr("Click or drag to add a mirror file"));
    urlIso->setAlignment(Qt::AlignHCenter);
    connect(this, &Page1::setIsoPath, this, &Page1::dealIsoPath);
    urlIso1->setAlignment(Qt::AlignHCenter);
    urlIso1->setWordWrap(true);
    comboUdisk = new QComboBox(this);
    comboUdisk->setFixedSize(400,40);
    comboUdisk->setEnabled(false);
    connect(comboUdisk, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        if (this->m_canChange) {
            if (index >= 0 && m_udiskList.count() > index)
                this->m_curUdiskName = this->m_udiskList.at(index);
            else
                this->m_curUdiskName = "";
        }
    });
    QWidget *btnwidget = new QWidget(this);
    findIso=new QPushButton(btnwidget);
//    findIso->setStyleSheet("QPushButton{border-image:url(:/data/button lightmode normal.png);}"
//                           "QPushButton:hover{border-image:url(:/data/button lightmode hover.png);}"
//                           "QPushButton:clicked{border-image:url(:/data/button lightmode click.png);}");
    findIso->setFixedSize(128,128);
    findIso->setProperty("isRoundButton", true);
    findIso->setIcon(QIcon::fromTheme("list-add-symbolic"));
    findIso->setIconSize(QSize(50,50));

    creatStart=new QPushButton(btnwidget);
    creatStart->setText(tr("Start"));
    creatStart->setEnabled(false);
    connect(findIso,&QPushButton::clicked,this,[=]{
        isoPath = QFileDialog::getOpenFileName(this,tr("choose iso file"),QDir::homePath(),"ISO(*.iso)");
        dealSelectedFile(isoPath);
    });
    connect(edit,&QLineEdit::textChanged,this,&Page1::ifStartBtnChange);
    connect(creatStart,&QPushButton::clicked,[=]{
        QString ss=diskInfos[comboUdisk->currentIndex()]->diskCapicity;
        QFileInfo info(isoPath);
        isoSize = info.size()/1024/1024;
        QString is=QString::number(isoSize,'f',1);
        isoSize=is.toFloat();
        if(ss.right(1)=="M"){
            ss=ss.left(ss.size()-1);
            USize=ss.toFloat();
            qDebug()<<__LINE__<<__func__<<USize;
            if(isoSize>USize){
                QMessageBox::StandardButton result =  QMessageBox::warning(this,tr("Warning"),tr("The image file is larger than the current U disk capacity, please format the U disk"),
                                                                           QMessageBox::Yes);
                switch (result)
                {
                case QMessageBox::Yes:
                    break;
                }

            }else{
                emit starmake(isoPath,diskInfos[comboUdisk->currentIndex()]->devicePath);
                QDBusMessage m = QDBusMessage::createMethodCall("com.lingmousbcreator.systemdbus","/",
                                                                "com.lingmousbcreator.interface","MakeStart");
                m<<isoPath;
                QString usb=diskInfos[comboUdisk->currentIndex()]->devicePath;
                //m<<diskInfos[comboUdisk->currentIndex()]->devicePath;
                m<<usb;
                QDBusConnection::systemBus().call(m);
            }
        }
        if(ss.right(1)=="G"){
            qDebug()<<__LINE__<<__func__<<"ss==="<<ss;
            ss=ss.left(ss.size()-1);
            qDebug()<<__LINE__<<__func__<<"ss-1==="<<ss;
            if(ss.contains(",")){
                int commaPos=ss.indexOf(",");
                ss[commaPos]='.';
            }
            USize=ss.toFloat()*1000;
            qDebug()<<__LINE__<<__func__<<USize;
            if(isoSize>USize){
                QMessageBox::StandardButton result =  QMessageBox::warning(this,tr("Warning"),tr("The image file is larger than the current U disk capacity, please format the U disk"),
                                                                           QMessageBox::Yes);
                switch (result)
                {
                case QMessageBox::Yes:
                    break;
                }

            }
            else{
                emit starmake(isoPath,diskInfos[comboUdisk->currentIndex()]->devicePath);
                QDBusMessage m = QDBusMessage::createMethodCall("com.lingmousbcreator.systemdbus","/",
                                                                "com.lingmousbcreator.interface","MakeStart");
                m<<isoPath;
                QString usb=diskInfos[comboUdisk->currentIndex()]->devicePath;
                //m<<diskInfos[comboUdisk->currentIndex()]->devicePath;
                m<<usb;
                QDBusConnection::systemBus().call(m);
            }
        }
    });
    QHBoxLayout *hlay=new QHBoxLayout(this);
    btnwidget->setLayout(hlay);
    hlay->addStretch(100);
    hlay->addWidget(creatStart);
    hlay->addStretch(100);
    QVBoxLayout *vlay1 = new QVBoxLayout(this);
    QVBoxLayout *vlay2 = new QVBoxLayout(this);
    topwidget->setLayout(vlay1);
    fram->setLayout(vlay2);
    vlay1->addWidget(findIso,0,Qt::AlignCenter);
    vlay1->addWidget(urlIso,0,Qt::AlignCenter);
    vlay1->addSpacing(70);
//    vlay2->addWidget(tabUdisk1,0,Qt::AlignCenter);
    vlay2->addWidget(tabUdisk2,0,Qt::AlignCenter);
    vlay2->addSpacing(70);
    vlay2->addWidget(urlIso1,0,Qt::AlignCenter);
    vlay2->addSpacing(70);
    QVBoxLayout *vlay =new QVBoxLayout(this);
    vlay->addSpacing(100);
    vlay->addWidget(fram,0,Qt::AlignCenter);
    vlay->addWidget(topwidget,0,Qt::AlignCenter);
    vlay->addSpacing(50);
    vlay->addWidget(comboUdisk,0,Qt::AlignCenter);
    vlay->addSpacing(30);
    vlay->addWidget(btnwidget);
    vlay->addSpacing(30);
    udiskPlugWatcherInit(); //监控U盘插拔
}

void Page1::udiskPlugWatcherInit()
{
    comboUdisk->setEnabled(true);
    udiskplugwatcher = new QFileSystemWatcher(this);
 //   QString usrname=qgetenv("USER");
  //  qDebug()<<__LINE__<<__func__<<"username===="<<usrname;
    udiskplugwatcher->addPath(QString("/media/")/*+usrname*/);
    connect(udiskplugwatcher,&QFileSystemWatcher::directoryChanged,this,&Page1::getStorageInfo);
}

bool Page1::isCapicityAvailable(QString str)
{
    if(str[str.length()-1] == 'G')
    {
        str.chop(1);//去掉最后一位
        if(str.toFloat() < 65 && str.toFloat() > 4)
        {
            return true;
        }
    }
    return false;
}

bool Page1::checkISO(const QString fileName){
    // Check if there's an MBR signature
    // MBR signature will be in last two bytes of the boot record
    QByteArray mbr;
    QFile mbrTest(fileName);
    mbrTest.open(QIODevice::ReadOnly);
    mbrTest.seek(510);
    mbr = mbrTest.read(2);
    mbrTest.close();
    if (mbr.toHex() != "55aa"){ //MBR signature "55aa"
        qDebug()<<"wrong iso,filename = "<<fileName<<"MBR signature = "<<mbr.toHex();
        return false;
    }
    return true;
}
void Page1::wait(uint sec)
{
    QEventLoop loop;
    qDebug()<<__LINE__<<__func__<<sec;
    QTimer::singleShot(1000 *sec,&loop,SLOT(quit()));
    loop.exec();

}
void Page1::getUdiskPathAndCap()
{
    diskInfos.clear();
    QProcess lsblk;
    lsblk.start("lsblk -J");
    lsblk.waitForFinished();

    QProcess lsblk2;
    lsblk2.start("lsblk -JS");
    lsblk2.waitForFinished();
    QJsonArray arr1 = QStringToJsonArray(QString::fromLocal8Bit(lsblk.readAllStandardOutput()));  //获取json类型的shell执行结果
    QJsonArray arr2 = QStringToJsonArray(QString::fromLocal8Bit(lsblk2.readAllStandardOutput()));
    QJsonArray arr3;
    QJsonArray mountpointarr4 ;
    QJsonArray mountpointarr5 ;
    foreach (const QJsonValue& value, arr1) {
        QJsonObject jsonObj1 = value.toObject();
        qDebug()<<__LINE__<<__func__<<jsonObj1["mountpoints"].toArray();
        foreach (const QJsonValue& value, arr2) {
            QJsonObject jsonObj2 = value.toObject();
            if(jsonObj1["name"] == jsonObj2["name"] && jsonObj2["tran"] == "usb" && jsonObj2["type"] == "disk"){
                qDebug()<<__LINE__<<__func__<<jsonObj1["name"].toString();
                mountpointarr4=jsonObj1["mountpoints"].toArray();
                foreach (const QJsonValue& Value, mountpointarr4) {
                    if(Value.isNull()==true){
                            arr3=jsonObj1["children"].toArray();
                            qDebug()<<__LINE__<<__func__<<"父挂载点为空";
                            foreach (const QJsonValue& Value, arr3) {
                                QJsonObject jsonObj3 = Value.toObject();
                               mountpointarr5 =jsonObj3["mountpoints"].toArray();
                                foreach (const QJsonValue& Value, mountpointarr5) {
                                if(Value.isNull()==false){
                                 qDebug()<<__LINE__<<__func__<<"子挂载点不为空"<<jsonObj3["name"].toString();
                                AvailableDiskInfo *tmp = new AvailableDiskInfo("/dev/" + jsonObj3["name"].toString(),jsonObj2["model"].toString(),jsonObj3["size"].toString());
                                diskInfos.append(tmp);
                                }
                                }
                            }
                    }else{
                            qDebug()<<__LINE__<<__func__<<"父挂载点不为空"<<jsonObj1["mountpoints"].toArray();
                            AvailableDiskInfo *tmp = new AvailableDiskInfo("/dev/" + jsonObj1["name"].toString(),jsonObj2["model"].toString(),jsonObj1["size"].toString());
                            diskInfos.append(tmp);
                    }
                }
            }
        }
    }
}

QJsonArray  Page1::QStringToJsonArray(const QString jsonString){
    QJsonParseError err;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonString.toLocal8Bit().data(),&err);
    if(jsonDocument.isNull())
    {
        qWarning()<< "JsonDocument is NULL.Origin data is:"<< jsonString.toLocal8Bit().data();
    }
    if(err.error != QJsonParseError::NoError){
        qWarning()<<"Parase json"<<jsonString<<" error:"<<err.error;
    }
    QJsonObject obj = jsonDocument.object();
    return obj["blockdevices"].toArray();
}
void Page1::getUdiskName()
{
    qDebug()<<__LINE__<<__func__;
    QList<QStorageInfo> storageInfo = QStorageInfo::mountedVolumes();
    for(int i=0;i<storageInfo.size();i++){
       qDebug()<<__LINE__<<__func__<<storageInfo.at(i).device();

    }
    foreach(AvailableDiskInfo* tmp,diskInfos)  //lsblk命令拿到的可用U盘信息
    {
        foreach(QStorageInfo disk,storageInfo)
        {
            if((disk.device().length() == 8 && tmp->devicePath == disk.device()))
            {
                tmp->displayName = disk.displayName();
                float cap = disk.bytesTotal();
                tmp->diskCapicity = QString::number(cap/1000000000,'f',1) + 'G';
                continue;
            }
            if(tmp->devicePath + '1' == disk.device())  //使用第一个分区的名字做展示名
            {
                tmp->displayName = disk.displayName();
                tmp->diskCapicity = disk.bytesTotal();
                float cap = disk.bytesTotal();
                tmp->diskCapicity = QString::number(cap / 1000000000,'f',1) + 'G';
                continue;
            }
        }
    }
}


void Page1::getStorageInfo()
{
    m_canChange = false;
    comboUdisk->clear();
    m_udiskList.clear();
    //wait(1);
    QThread::msleep(500);
    qDebug()<<__LINE__<<__func__;
    getUdiskPathAndCap();
    qDebug()<<__LINE__<<__func__<<m_canChange;

    foreach(AvailableDiskInfo *diskInfo,diskInfos)
    {
        //过长的名称做去尾加省略号
        if(diskInfo->displayName.length() > UDISK_NAME_MAX_LENGTH)
        {
            diskInfo->displayName = diskInfo->displayName.mid(0,UDISK_NAME_MAX_LENGTH - 1) + "…";
        }
        QString info = diskInfo->displayName+" ("+diskInfo->devicePath + ") " + diskInfo->diskCapicity;
        m_udiskList << info;
        comboUdisk->addItem(info,diskInfo->devicePath);
        qDebug()<<__LINE__<<__func__<<diskInfo->devicePath;
        if(!isoPath.isEmpty()){
            qInfo()<<__LINE__<<__func__<<isoPath;
            creatStart->setEnabled(true);
            creatStart->setProperty("isImportant", true);
        }
        comboUdisk->setEnabled(true);
    }
    m_canChange = true;
    if (0 == comboUdisk->count())
    {
        comboUdisk->addItem(tr("No USB drive available"));
        comboUdisk->setEnabled(false);
        m_curUdiskName = "";
        creatStart->setEnabled(false);
    } else if (comboUdisk->count() > 1){
        if (m_curUdiskName.isEmpty() || !m_udiskList.contains(m_curUdiskName)) {
            //            comboUdisk->setCurrentIndex(-1);
            //            comboUdisk->setLineEdit(edit);
            //            edit->setText(tr("select USB flash drive"));
            //            edit->setReadOnly(true);
        } else {
            comboUdisk->setCurrentIndex(m_udiskList.indexOf(m_curUdiskName));
        }
    } else {
        comboUdisk->setCurrentIndex(0);
    }

    //ifStartBtnChange();
}

void Page1::creatStartSlots()
{
    creatStart->setEnabled(false);
}

bool Page1::ifStartBtnChange()
{
    if(comboUdisk->currentText() != tr("No USB drive available")&&edit->text()!=tr("select USB flash drive") && urlIso->text()==isoPath)
    {
        creatStart->setEnabled(true);
        creatStart->setProperty("isImportant", true);
        return true;
    }else{
        creatStart->setEnabled(false);
        return false;
    }
}

void Page1::dealDialogCancel()
{
    ifStartBtnChange();
}

void Page1::dealComboBoxChangeButton()
{
    ifStartBtnChange();
}

void Page1::dealAuthDialogClose()
{
    creatStart->setEnabled(true);
    creatStart->setProperty("isImportant", true);
}

void Page1::setThemeStyleLight()
{
    themeStatus = LIGHTTHEME;
    QPixmap pix(":/data/button lightmode inactive.png");
//    tabUdisk1->setPixmap(pix);
//    findIso->setStyleSheet("QPushButton{border-image:url(:/data/button lightmode normal.png);}"
//                           "QPushButton:hover{border-image:url(:/data/button lightmode hover.png);}"
//                           "QPushButton:clicked{border-image:url(:/data/button lightmode click.png);}");
  //  QPixmap pix(":/data/Placeholder lighter.png");
  //  tabUdisk->setPixmap(pix);

    ifStartBtnChange();
}

void Page1::setThemeStyleDark()
{
    themeStatus = DARKTHEME;
    QPixmap pix(":/data/button darkmode inactive.png");
//    tabUdisk1->setPixmap(pix);
//    findIso->setStyleSheet("QPushButton{border-image:url(:/data/button darkmode normal.png);}"
//                           "QPushButton:hover{border-image:url(:/data/button darkmode hover.png);}"
//                           "QPushButton:clicked{border-image:url(:/data/button darkmode click.png);}");
 //   QPixmap pix(":/data/Placeholder dark.png");
  //  tabUdisk->setPixmap(pix);
    ifStartBtnChange();
}

QString Page1::getDevPath(){
    if(diskInfos.size()!=0){
        return diskInfos[comboUdisk->currentIndex()]->devicePath;
    }/*else{
        return comboUdisk->addItem(tr("No USB drive available"));

    }*/
}

void Page1::dealSelectedFile(QString isoPath){
    if(isoPath.isEmpty()){
        return ;
    }
    if(comboUdisk->currentText() != tr("No USB drive available") && edit->text() != tr("select USB flash drive")){
        creatStart->setEnabled(true);
        creatStart->setProperty("isImportant", true);
    }
    if(!checkISO(isoPath)){
       // QMessageBox::StandardButton result =  QMessageBox::warning(this,tr("Warning"),tr("MBR signature not detected,continue anyway?"),
       //                                                            QMessageBox::Ok | QMessageBox::Cancel);
        QMessageBox box(QMessageBox::Warning, tr("Warning"),tr("MBR signature not detected,continue anyway?"));
              box.addButton(tr("Cancel"), QMessageBox::RejectRole);
              box.addButton(tr("Ok"), QMessageBox::AcceptRole);
              int result = box.exec();
        switch (result){
        case QMessageBox::RejectRole:
            if(comboUdisk->currentText() != tr("No USB drive available")){
                creatStart->setEnabled(true);
                creatStart->setProperty("isImportant", true);
            }
            //以右键点击镜像打开方式u盘启动器点击制作会提示没有文件路径会发现是因为isopath是成员变量开启，需要进行传值。才可以制作。
            this->isoPath = isoPath;
            urlIso->setToolTip("");
            urlIso->setToolTip(isoPath);
            urlIso->setText(isoPath);
            urlIso1->setText(isoPath);
            urlIso1->setToolTip(isoPath);
            break;
        case QMessageBox::Cancel:
            if(creatStart->isEnabled()==true){
                creatStart->setEnabled(false);
            }
            urlIso->setText(tr("Click or drag to add a mirror file"));
            urlIso1->setText(tr("Click or drag to add a mirror file"));
            isoPath.clear();
            break;

        }
    }
    else{
        //以右键点击镜像打开方式u盘启动器点击制作会提示没有文件路径会发现是因为isopath是成员变量开启，需要进行传值。才可以制作。
        this->isoPath = isoPath;
        urlIso->setToolTip("");
        urlIso->setToolTip(isoPath);
        urlIso->setText(isoPath);
        urlIso1->setText(isoPath);
        urlIso1->setToolTip(isoPath);
    }
    return ;
}
void Page1::dealIsoPath(const QString &iso_path){
    qInfo()<<"文件路径"<<iso_path;
    if(iso_path.isEmpty()){
        return ;
    }
//    if(comboUdisk->currentText() != tr("No USB drive available")){
//        creatStart->setEnabled(true);
//        creatStart->setProperty("isImportant", true);
//    }
    if(!checkISO(iso_path)){
        QMessageBox::StandardButton result =  QMessageBox::warning(this,tr("Warning"),tr("MBR signature not detected,continue anyway?"),
                                                                   QMessageBox::Ok | QMessageBox::Cancel);
        switch (result)
        {
        case QMessageBox::Ok:
            if(comboUdisk->currentText() != tr("No USB drive available")){
                creatStart->setEnabled(true);
                creatStart->setProperty("isImportant", true);
            }
            this->isoPath = iso_path;
            urlIso->setText(iso_path);
            urlIso->setToolTip(iso_path);
            break;
        case QMessageBox::Cancel:
            creatStart->setEnabled(false);
            urlIso->setText(tr("Click or drag to add a mirror file"));
            urlIso1->setText(tr("Click or drag to add a mirror file"));
            break;
        }
    }else{
        this->isoPath = iso_path;
        urlIso->setText(iso_path);
        urlIso->setToolTip(iso_path);
    }
}
void Page1::dragEnterEvent(QDragEnterEvent *event){
    auto urls = event->mimeData()->urls();
    if(urls.length() != 1){
        return ;
    }
    QString filePath = urls.at(0).toLocalFile();
    if("iso" != QFileInfo(filePath).suffix().toLower()){
        return ;
    }
    if(event->mimeData()->hasFormat("text/uri-list")){
        qInfo()<<"437"<<filePath;
        fram->setFrameShape(QFrame::StyledPanel);
       // fram->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        urlIso1->setFixedHeight(270);
        fram->setFixedSize(450,270);
        fram->show();
        topwidget->hide();
        event->acceptProposedAction();
    }
    return QWidget::dragEnterEvent(event);
}
void Page1::dragLeaveEvent(QDragLeaveEvent *event){
    findIso->show();
    topwidget->show();
    fram->hide();
    return QWidget::dragLeaveEvent(event);

}
void Page1::dropEvent(QDropEvent *event){
    auto urls = event->mimeData()->urls();
    if(urls.isEmpty())
        return ;
    for(auto url:urls){
        dealSelectedFile(url.toLocalFile());
        findIso->show();
        topwidget->show();
        fram->hide();
        //ifStartBtnChange();
    }
    return ;
}
void Page1::settingStyle(){
    subject=new GsettingSubject;
    connect(subject,&GsettingSubject::blackStyle,this,[=]{
        this->setThemeStyleDark();
    });
    connect(subject,&GsettingSubject::whiteStyle,this,[=]{
        this->setThemeStyleLight();
    });
    subject->iniWidgetStyle();
}
bool Page1::eventFilter(QObject *watched, QEvent *event){
    if(watched==edit){
        if(event->type()==QEvent::MouseButtonPress){
            if(comboUdisk->currentText()!=tr("No USB drive available")){
                comboUdisk->showPopup();
            }
        }
    }
    return QWidget::eventFilter(watched,event);
}

