#include "page2.h"
#include<QMessageBox>
#include<QDBusMessage>
#include <ksoundeffects.h>
#include <QThread>
using namespace kdk;
Page2::Page2(QWidget *parent) : QWidget(parent)
{
    round1 = new KProgressCircle(this);
    round1->setValue(0);
    round1->show();
    //    round = new QRoundProgressBar(this);
    //  round->setBarStyle(QRoundProgressBar::StyleLine);
    //  round->setOutlinePenWidth(15);
    //  round->setDataPenWidth(5);
    //  round->setMinimum(0);
    //  round->setMaximum(100);
    //   round->setValue(0);
    //    round->setPersent(0);
    //    round->show();
    menu=new menuModule(this);

    lableMovie=new QLabel(this);
    QSize movieSize(95,95);
    round1->setFixedSize(movieSize);
    lableMovie->setFixedSize(movieSize);
    lableMovie->hide();

    lableText=new QLabel;
    lableText->setWordWrap(true);
    lableText->setAlignment(Qt::AlignHCenter);


    QWidget *btnwidget=new QWidget(this);
    returnPushButton=new QPushButton(btnwidget);
    cancelbtn = new QPushButton(btnwidget);
    cancelbtn->setText(tr("Cancel"));

    connect(cancelbtn,&QPushButton::clicked,this,&Page2::returnMain);
    connect(returnPushButton,&QPushButton::clicked,[=]{movieStatus = loading;});
    connect(menu,&menuModule::iconchange,this,&Page2::setIconchange);
    QLabel *filling1 = new QLabel;
    QLabel *filling2 = new QLabel;
    QLabel *filling3 = new QLabel;
    QLabel *filling4 = new QLabel;
    QLabel *filling5 = new QLabel;
    QLabel *filling6 = new QLabel;
    QHBoxLayout *hlt1=new QHBoxLayout;
    hlt1->setMargin(0);
    hlt1->setSpacing(0);
    hlt1->addWidget(filling1,9);
    hlt1->addWidget(lableMovie);
    hlt1->addWidget(round1);
    hlt1->addWidget(filling2,9);
    QHBoxLayout *hlt2=new QHBoxLayout;
    hlt2->setMargin(0);
    hlt2->setSpacing(0);
    hlt2->addWidget(filling3,9);
    hlt2->addWidget(lableText);
    hlt2->addWidget(filling4,9);
    QHBoxLayout *hlt3=new QHBoxLayout;
    btnwidget->setLayout(hlt3);
    hlt3->setMargin(0);
    hlt3->setSpacing(0);
    hlt3->addWidget(filling5,9);
    hlt3->addWidget(cancelbtn);
    hlt3->addStretch(1);
    hlt3->addWidget(returnPushButton);
    hlt3->addWidget(filling6,9);
    filling6->show();
    QVBoxLayout *vlt1=new QVBoxLayout;
    vlt1->setMargin(0);
    vlt1->setSpacing(0);
    vlt1->addSpacing(115);
    vlt1->addLayout(hlt1);
    vlt1->addSpacing(26);
    vlt1->addLayout(hlt2);
    vlt1->addSpacing(220);
    vlt1->addWidget(btnwidget);
    vlt1->addStretch();
    this->setLayout(vlt1);
    connect(returnPushButton,&QPushButton::clicked,this,[=](){
        allclose();
    });
}
bool Page2::selectpath(QString path){
    QFile file(path);
    if(file.exists()){
        return true;
    }else{
        return false;
    }
}
void Page2::makestar(QString path,QString sourcepath){
    isopath=path;
    usbpath=sourcepath;
    selectpath(path);
    qDebug()<<__LINE__<<__func__<<usbpath;
}
void Page2::allclose(){
    if(returnPushButton->text()==tr("Retry")){
        QDBusMessage m = QDBusMessage::createMethodCall("com.lingmousbcreator.systemdbus","/",
                                                        "com.lingmousbcreator.interface","MakeStart");
        m<<isopath;
        m<<usbpath;
        m<<m_userName;
        QDBusConnection::systemBus().call(m);
        startMaking();
    }
    if(returnPushButton->text()==tr("Close")){
        emit mainclose();
    }
}
void Page2::playLoadingGif()
{
    returnPushButton->setEnabled(false);
    returnPushButton->setText(tr("In production"));
    cancelbtn->setText(tr("Cancel"));
    movieStatus = loading;
    round1->show();
    lableMovie->hide();
    lableText->setText(tr("Please do not remove the USB driver or power off now"));
}

void Page2::playFinishGif()
{
    round1->hide();
    lableMovie->show();
    lableMovie->setScaledContents(true);
    lableMovie->setPixmap(QIcon::fromTheme("lingmo-dialog-success").pixmap(96,96));
    returnPushButton->setEnabled(true);
    cancelbtn->setText(tr("Return"));
    returnPushButton->setText(tr("Close"));
    lableText->setText(tr("Finish"));
    lableMovie->clear();
    movieStatus = finish;
    kdk::KSoundEffects::playSound(kdk::SoundType::COMPLETE);
    if(themeStatus == DARKTHEME)
    {
        // round->setPersent(0);
        round1->setValue(0);
        lableMovie->setPixmap(QIcon::fromTheme("lingmo-dialog-success").pixmap(96,96));
    }else
    {
        lableMovie->setPixmap(QIcon::fromTheme("lingmo-dialog-success").pixmap(96,96));
    }
}

void Page2::playErrorGif()
{
    round1->hide();
    returnPushButton->setEnabled(true);
    cancelbtn->setText(tr("Return"));
    returnPushButton->setText(tr("Retry"));
    movieStatus = failed;
    lableText->setText(tr("Creation Failed"));
    lableMovie->show();
    lableMovie->clear();
    lableMovie->setScaledContents(true);
    lableMovie->setPixmap(QIcon::fromTheme("dialog-error").pixmap(95,95));
    kdk::KSoundEffects::playSound(kdk::SoundType::DIALOG_ERROR);
}

void Page2::startMaking()
{
    playLoadingGif();
    QDBusConnection::systemBus().connect(QString(),QString("/"),"com.lingmousbcreator.interface","workingProgress",this,SLOT(dealWorkingProgress(int)));
    QDBusConnection::systemBus().connect(QString(),QString("/"),"com.lingmousbcreator.interface","makeFinish",this,SLOT(dealMakeFinish(QString)));
}

void Page2::dealWorkingProgress(int progress){
    QString ii=QString::number(progress);
    if(ii.isEmpty()){
        return;
    }else{
        // round->setPersent(progress);
        round1->setValue(progress);
        qInfo()<<"progress"<<progress;
    }
}

void Page2::dealMakeFinish(QString status)
{
     qDebug()<<"receive signal make finish"<<status;
     m_isFinishstatus=status;
     if("success" == status){
         QThread::msleep(200);
         playFinishGif();

     }else if(status=="unmount_error"){
            playLoadingGif();
     }else if(status=="false"){
         playErrorGif();
     }else if(status=="fail"){
         qDebug()<<"异常退出";
         playErrorGif();
     }else if(status=="make_error"){
         qDebug()<<"dd异常退出";
         playErrorGif();
     }
     if(status=="sync"){
         qDebug()<<"sync";
          cancelbtn->setEnabled(false);
     }if(status=="unsync"){
         cancelbtn->setEnabled(true);
     }
     emit makeFinish();
}
//bool Page2::mountDevice(QString target){
//    QProcess mount;
//    qDebug()<<"start mounting disk:"<<target;
//    //mount.start("udisksctl",QStringList{"mount","-b",target});
//   // mount.start("gio",QStringList{"mount","-d",target});
//    QString str,str1;
//    mount.startDetached("gio",QStringList{"mount","-d",target});
////    str1=mount.readAllStandardError();
////    qDebug()<<__LINE__<<str;
////    str=mount.readAllStandardOutput();
////    qDebug()<<__LINE__<<str1;
//    if(QProcess::NormalExit == mount.exitCode()){
//        qDebug()<<"Mount success!"<<mount.exitCode();
//        return true;
//    }
//    qWarning()<<"An unknown error occurred! Mount process exit code:"<<mount.exitCode();
//    return true;
//}

qint64 Page2::getFileSize(QString filePath)
{
    QFileInfo info(filePath);
    return info.size()/1048576;
}

void Page2::finishEvent()
{
    diskRefreshDelay = new QTimer;
    connect(diskRefreshDelay,&QTimer::timeout,[=]{
        diskRefreshDelay->stop();
        if(isMakingSuccess())
        {
            playLoadingGif();
        }else
        {
            playErrorGif();
        }
        emit makeFinish();

    });
    diskRefreshDelay->start(1000);
}
bool Page2::isMakingSuccess()
{
    QList<QStorageInfo> diskList = QStorageInfo::mountedVolumes(); //mounted volumes
    for(QStorageInfo& disk : diskList)
    {
        QString diskPath = disk.device();
        diskPath = diskPath.mid(0,8);
        if(uDiskPath == diskPath)
        {
            return true;
        }
    }
    return false;
}

void Page2::movieRefresh()
{
    lableMovie->clear();
    switch(movieStatus)
    {
    case 0:
        playLoadingGif();
        break;
    case 1:
        playFinishGif();
        break;
    case 2:
        playErrorGif();
        break;
    }
}
void Page2::getmountpoint()
{
    QProcess lsblk;
    lsblk.start("lsblk -J");
    lsblk.waitForFinished();

    QProcess lsblk2;
    lsblk2.start("lsblk -JS");
    lsblk2.waitForFinished();
    QJsonArray arr1 = QStringToJsonArray(QString::fromLocal8Bit(lsblk.readAllStandardOutput()));  //获取json类型的shell执行结果
    QJsonArray arr2 = QStringToJsonArray(QString::fromLocal8Bit(lsblk2.readAllStandardOutput()));
    QJsonArray arr3 ;
    QJsonArray mountpointarr4 ;
    QJsonArray mountpointarr5 ;
    foreach (const QJsonValue& value, arr1) {
        QJsonObject jsonObj1 = value.toObject();
        foreach (const QJsonValue& value, arr2) {
            QJsonObject jsonObj2 = value.toObject();
            if(jsonObj1["name"] == jsonObj2["name"] && jsonObj2["tran"] == "usb" && jsonObj2["type"] == "disk"){
                if(jsonObj1["mountpoints"].toArray().isEmpty()==true){
                    arr3=jsonObj1["children"].toArray();
                    qDebug()<<__LINE__<<__func__<<"Page1"<<arr3;
                    foreach (const QJsonValue& Value, arr3) {
                        QJsonObject jsonObj3 = Value.toObject();
                        mountpointarr5=jsonObj3["mountpoints"].toArray();
                        foreach (const QJsonValue& Value, mountpointarr5) {
                            if(mountpoint.isNull()==false){
                            if(mountpoint.contains("/media")){
                                mountpoint = Value.toString();
                            }else{
                                mountpoint=value.toString();
                            }
                            }
                        }
                        if(mountpoint!=NULL&&"/dev/"+jsonObj3["name"].toString()==uDiskPath){
                            qDebug()<<__LINE__<<__func__<<"Page2"<<mountpoint;
                        }
                    }
                }else if(jsonObj1["mountpoints"].toString()!=NULL&&uDiskPath=="/dev/"+jsonObj1["name"].toString()){
                    mountpointarr4=jsonObj1["mountpoints"].toArray();
                    foreach (const QJsonValue& Value, mountpointarr4) {
                        if(mountpoint.contains("/media")){
                            mountpoint = Value.toString();
                        }else{
                            mountpoint=value.toString();
                        }
                    }
                    mountpoint=true;
                    qDebug()<<__LINE__<<__func__<<"page2"<<mountpoint;
                }
            }
        }
    }
}

QJsonArray  Page2::QStringToJsonArray(const QString jsonString){
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

void Page2::checkmd5(){
    m_isMd5Finish=false;
    ismakesuccess=false;
    md5=new QProcess();
    connect(md5,SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(readStandardOutput(int,QProcess::ExitStatus)));

    QDir::setCurrent(mountpoint);
    qDebug()<<QDir::currentPath();
    checkMd5FinishTimer->start(2000);
    md5->start("md5sum",QStringList{"-c","md5sum.txt"});
    connect(md5, &QProcess::readyReadStandardOutput, this, [=]() {
        MD5str=this->md5->readAllStandardOutput();
        qDebug()<<__LINE__<<__func__<<MD5str;
    });
    qDebug()<<"70ag0"<<md5->arguments();
}

void Page2::readStandardOutput(int exitCode,QProcess::ExitStatus exitStatus){
    qDebug()<<"md5退出"<<exitCode;
    if(exitStatus==QProcess::NormalExit && exitCode==0){
        if(MD5str.isEmpty()){
            QDir::setCurrent(QDir::homePath());
            qDebug()<<__LINE__<<__func__<<"输出为空"<<MD5str;
            ismakesuccess=false;
        }else{
            if (MD5str.contains(tr("Fail")) || MD5str.contains(tr("FAILED")))
            {
                qDebug()<<__LINE__<<__func__<<"有错误字符"<<MD5str;
                QDir::setCurrent(QDir::homePath());
                qDebug()<<"md5 fail!";
                ismakesuccess=false;
            }
            else{
                qDebug()<<__LINE__<<__func__<<"完全正确"<<MD5str;
                QDir::setCurrent(QDir::homePath());
                ismakesuccess=true;
            }
        }

        qDebug()<<"md5 success!";
    }else{
        QDir::setCurrent(QDir::homePath());
    }
    qDebug()<<"md5退出"<<exitCode;
    m_isMd5Finish=true;

}
void Page2::checkmd5finish(){
    if(m_isMd5Finish==true){
        checkMd5FinishTimer->stop();
        if(ismakesuccess==true){
            playFinishGif();
            qDebug()<<"status"<<m_isFinishstatus;
        }else if(ismakesuccess==false){
            playErrorGif();
            qDebug()<<"ismakesuccess"<<ismakesuccess;
            qDebug()<<"status"<<m_isFinishstatus;
        }else{
            playLoadingGif();
            qDebug()<<"ismakesuccess"<<ismakesuccess;
            qDebug()<<"status"<<m_isFinishstatus;
        }
    }
}

void Page2::setThemeStyleLight()
{
    themeStatus = LIGHTTHEME;
    theme::baseColorFront=QColor(70,159,255);
    theme::backcolcr = QColor(250,250,250);
    theme::baseColorAfter=QColor(227,236,248);
    movieRefresh();
}

void Page2::setThemeStyleDark()
{
    themeStatus = DARKTHEME;
    theme::backcolcr=QColor(18,18,18);
    theme::baseColorFront=QColor(111,145,255);
    theme::baseColorAfter=QColor(43,44,46);
    movieRefresh();
}
void Page2::setIconchange(){
    if(lableText->text()==tr("Creation Failed")){
        lableMovie->setPixmap(QIcon::fromTheme("dialog-error").pixmap(96,96));
    }
    if(lableText->text()==tr("Finish")){
        lableMovie->setPixmap(QIcon::fromTheme("lingmo-dialog-success").pixmap(96,96));
    }
}
