// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfinstallnormalwindow.h"
#include "fontmanagercore.h"
#include "dfontpreviewlistdatathread.h"
#include "dfinstallerrordialog.h"
#include "dcomworker.h"

#include <DApplication>
#include <DFontSizeManager>
#include <DTipLabel>

#include <QFileInfo>
#include <QVBoxLayout>
#include <QFontDatabase>

DWIDGET_USE_NAMESPACE

class Worker: public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);

public slots:
    /**
     * @brief slot_dowork 字体安装及之前的准备工作
     * @param w
     */
    void slot_dowork(DFInstallNormalWindow *w);
};

Worker::Worker(QObject *parent)
    : QObject(parent)
{

}

void Worker::slot_dowork(DFInstallNormalWindow *w)
{
    // Install the font list ,which may be changed in exception window
    if (nullptr != w) {
        w->getAllSysfiles();
        // Check installed & damaged font file here
        w->verifyFontFiles();
        w->batchInstall();
    }
}

/*************************************************************************
 <Function>      DFInstallNormalWindow
 <Description>   构造函数
 <Author>
 <Input>
    <param1>     files           Description:待安装字体文件
    <param2>     parent          Description:父控件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFInstallNormalWindow::DFInstallNormalWindow(const QStringList &files, QWidget *parent)
    : DFontBaseDialog(parent)
    , m_installFiles(files)
    , m_fontInfoManager(DFontInfoManager::instance())
    , m_fontManager(FontManagerCore::instance())
    , m_pworker(new Worker())
    , m_pthread(new QThread())
{
    initUI();
    initConnections();
    m_pworker->moveToThread(m_pthread);
    dowork();
}

/*************************************************************************
 <Function>      ~DFInstallNormalWindow
 <Description>   析构函数处理函数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFInstallNormalWindow::~DFInstallNormalWindow()
{
    qDebug() << __func__ << "start" << endl;

    // 结束线程
    m_pthread->quit();
    m_pthread->wait();
    m_pthread->deleteLater();

    m_installFiles.clear();
    m_installedFiles.clear();
    m_newInstallFiles.clear();
    m_damagedFiles.clear();
    m_systemFiles.clear();
    m_outfileList.clear();
    m_errorList.clear();
    m_AllSysFilesfamilyName.clear();
    m_installedFontsFamilyname.clear();
    m_halfInstalledFiles.clear();
    this->hide();
    //如果有tab焦点且为取消或X号，则恢复tab焦点状态
    if (m_AddBtnHasTabs && !m_skipStateRecovery) {
        emit m_signalManager->requestSetTabFocusToAddBtn();
        //恢复标志位
        setAddBtnHasTabs(false);
    } else {
        //恢复标志位
        m_skipStateRecovery = false;
    }
    qDebug() << __func__ << "end" << this << endl;
}

/*************************************************************************
 <Function>      initUI
 <Description>   初始化主页面
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::initUI()
{
    setFixedSize(QSize(380, 136));
    setTitle(DApplication::translate("NormalInstallWindow", "Install Font"));

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 0, 0);

    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(10, 0, 0, 0);

    m_progressStepLabel = new DLabel(this);
    DFontSizeManager::instance()->bind(m_progressStepLabel, DFontSizeManager::T6);
    m_progressStepLabel->setFixedHeight(m_progressStepLabel->fontMetrics().height());
    m_progressStepLabel->setText(DApplication::translate("NormalInstallWindow", "Verifying..."));

    m_currentFontLabel = new DTipLabel("", this);
    DFontSizeManager::instance()->bind(m_currentFontLabel, DFontSizeManager::T8);
    m_currentFontLabel->setAlignment(Qt::AlignLeft);
    m_currentFontLabel->setFixedHeight(m_currentFontLabel->fontMetrics().height());
    m_currentFontLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_progressBar = new DProgressBar(this);
    m_progressBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_progressBar->setFixedSize(380 - 42, 6);

    contentLayout->addSpacing(10);
    contentLayout->addWidget(m_progressStepLabel);
    contentLayout->addSpacing(7);
    contentLayout->addWidget(m_currentFontLabel);
    contentLayout->addSpacing(10);
    contentLayout->addWidget(m_progressBar);
    contentLayout->addSpacing(20);

    mainLayout->addLayout(contentLayout);

    m_mainFrame = new QWidget(this);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mainFrame->setLayout(mainLayout);

    addContent(m_mainFrame);
#ifdef FTM_DEBUG_LAYOUT_COLOR
    m_titleFrame->setStyleSheet("background: red");
    m_mainFrame->setStyleSheet("background: green");
    m_progressStepLabel->setStyleSheet("background: silver");
    m_currentFontLabel->setStyleSheet("background: silver");
    m_progressBar->setStyleSheet("background: silver");
#endif
}

/*************************************************************************
 <Function>      initConnections
 <Description>   初始化信号和槽connect连接函数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::initConnections()
{
    connect(m_signalManager, &SignalManager::installDuringPopErrorDialog, this, &DFInstallNormalWindow::batchHalfwayInstall);

    connect(m_fontManager, &FontManagerCore::batchInstall, this, &DFInstallNormalWindow::onProgressChanged);

    connect(m_fontManager, &FontManagerCore::installFinished, this, &DFInstallNormalWindow::onInstallFinished);

    connect(m_fontManager, &FontManagerCore::reInstallFinished, this, &DFInstallNormalWindow::onReInstallFinished);

    connect(DFontPreviewListDataThread::instance(), &DFontPreviewListDataThread::requestBatchReInstallContinue,
            this, &DFInstallNormalWindow::batchReInstallContinue);

    connect(qApp, &QApplication::fontChanged, this, [ = ]() {
        m_progressStepLabel->setFixedHeight(m_progressStepLabel->fontMetrics().height());
        m_currentFontLabel->setFixedHeight(m_currentFontLabel->fontMetrics().height());
    });

    connect(m_pthread, &QThread::finished, m_pworker, &QObject::deleteLater); //线程结束资源释放
    connect(this, &DFInstallNormalWindow::sigdowork, m_pworker, &Worker::slot_dowork);
    connect(this, &DFInstallNormalWindow::sigShowInstallErrDlg, this, &DFInstallNormalWindow::showInstallErrDlg);
}

/*************************************************************************
 <Function>      getAllSysfiles
 <Description>   从数据库中读取系统字体，用于之后的判断
 <Author>        UT000442
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::getAllSysfiles()
{
    for (auto &font : DFontPreviewListDataThread::instance()->getFontModelList()) {
        if (Q_UNLIKELY(font.fontInfo.filePath.contains("/usr/share/"))) {
            QString systemFileName;
            systemFileName.append(font.fontInfo.familyName).append(font.fontInfo.styleName);
            m_fontName.append(font.fontData.strFontName);
            qDebug() << font.fontInfo.fullname << endl;
            m_AllSysFilesfamilyName.append(systemFileName);

        }
    }
}


/*************************************************************************
 <Function>      verifyFontFiles
 <Description>   字体文件过滤器，过滤后得到需要新安装的字体，重复安装字体，损毁字体，系统字体,以及字体验证框弹出时安装的字体
                 过滤后进行安装
 <Author>
 <Input>
    <param1>     isHalfwayInstall Description:需要过滤的字体文件
 <Return>        null             Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::verifyFontFiles()
{
    DFontInfo fontInfo;
    QStringList fontInfos;
    QStringList instFontInfos;

    m_damagedFiles.clear();
    m_installedFiles.clear();
    m_newInstallFiles.clear();
    m_systemFiles.clear();
    m_errorList.clear();
    m_newHalfInstalledFiles.clear();
    m_oldHalfInstalledFiles.clear();

    for (auto &it : m_installFiles) {
        fontInfo = m_fontInfoManager->getFontInfo(it);
        if(m_fontInfoManager->isFontInInstalledDirs(fontInfo.filePath)){
            needRefresh = true;
        }
        if (Q_UNLIKELY(fontInfo.isError)) {
            m_damagedFiles.append(it);

#ifdef QT_QML_DEBUG
//            qDebug() << __FUNCTION__ << " (" << it << " :Damaged file)";
#endif
        } else if (fontInfo.isInstalled && !isSystemFont(fontInfo)) {
            QString familyStyleName = getFamilyStyleName(fontInfo);
            if (!instFontInfos.contains(familyStyleName)) {
                instFontInfos.append(familyStyleName);
                m_installedFiles.append(it);
            }

#ifdef QT_QML_DEBUG
//            qDebug() << __FUNCTION__ << " (" << it << " :Installed file)";
#endif
        } else if (isSystemFont(fontInfo)) {
            m_systemFiles.append(it);

#ifdef QT_QML_DEBUG
//            qDebug() << __FUNCTION__ << " (" << it << " :System file)";
#endif
        } else {
            QString familyStyleName = getFamilyStyleName(fontInfo);
            if (!fontInfos.contains(familyStyleName)) {
                fontInfos.append(familyStyleName);
                /*如果是字体验证框弹出时再进行安装的话,因为这一步骤安装的字体没有插入数据库,所以需要再判断下是否为安装过的.*/
                if (m_installedFontsFamilyname.contains(familyStyleName)) {
                    /*这里获取需要新添加到验证框中的字体m_newHalfInstalledFiles和之前出现过的字体m_oldHalfInstalledFiles
                    ,用于之后listview滚动和设置选中状态使用*/
                    if (!m_halfInstalledFiles.contains(it)) {
                        m_newHalfInstalledFiles.append(it);
                    } else {
                        m_oldHalfInstalledFiles.append(it);
                    }
                } else {
                    m_newInstallFiles.append(it);
                }
            }

#ifdef QT_QML_DEBUG
//            qDebug() << __FUNCTION__ << " (" << it << " :New file)";
#endif
        }
    }
    m_errorList = m_damagedFiles + m_installedFiles + m_systemFiles;
}

/*************************************************************************
 <Function>      ifNeedShowExceptionWindow
 <Description>   检测是否要弹出字体验证框，存在重复安装字体，系统字体时，损坏字体时弹出字体验证框
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
bool DFInstallNormalWindow::ifNeedShowExceptionWindow() const
{
    // Skip Exception dialog
    if (m_isNeedSkipException) {
        return false;
    }
    // If have new install file,install first,then check the exception list
    if (InstallState::Install == m_installState && m_newInstallFiles.size() > 0) {
        return false;
    }

    // For all selected files is installed & damage
    if (InstallState::Install == m_installState
            && (m_installedFiles.size() > 0 || m_damagedFiles.size() > 0)) {
        return true;
    }

    if (InstallState::Install == m_installState && m_systemFiles.size() > 0) {
        return true;
    }

    if (InstallState::reinstall == m_installState && m_installedFiles.size() > 0) {
        return true;
    }

    if (InstallState::reinstall == m_installState && m_damagedFiles.size() > 0) {
        return true;
    }

    return false;
}

/*************************************************************************
 <Function>      isSystemFont
 <Description>   判断当前字体是否为系统字体
 <Author>
 <Input>
    <param1>     f               Description:传入当前字体
 <Return>        bool            Description:true 表示当前字体为系统字体；false 表示当前字体不是系统字体
 <Note>          null
*************************************************************************/
bool DFInstallNormalWindow::isSystemFont(DFontInfo &f)
{
    QString fontFullname = f.familyName + f.styleName;
    bool ret = m_AllSysFilesfamilyName.contains(fontFullname);

    // ttc文件包含多种ttf字体，需特殊处理
    if (!ret && f.filePath.endsWith(QLatin1String(".ttc"), Qt::CaseInsensitive)) {
        QStringList fontFamilyList = QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(f.filePath));
        if (fontFamilyList.size() > 1) {
            for (QString &fontFamily : fontFamilyList) {
                fontFullname = fontFamily + f.styleName;
                ret = (ret || m_AllSysFilesfamilyName.contains(fontFullname));
                if (ret) {
                    break;
                }
            }
        }
    }


    return ret;
}

/*************************************************************************
 <Function>      checkShowMessage
 <Description>   根据字体安装或重复安装状态更新标志位getInstallMessage
                 新安装的字体在安装完成时 getInstallMessage置为true。重复安装的字体安装完成时 getReInstallMessage置为true
                 列表刷新完成后 m_installAdded置为true。三者都为true时表示一次安装过程结束。
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::checkShowMessage()
{
    qDebug() << "Install over" << endl;

    if (getInstallMessage == true && getReInstallMessage == true) {
        qDebug() << "install refresh over";
        finishInstall();
    } else if (getInstallMessage == true && m_popedInstallErrorDialg == false) {
        if (ifNeedShowExceptionWindow()) {
            qDebug() << "need reinstall " << endl;
            Q_EMIT sigShowInstallErrDlg();
        } else {
            qDebug() << "no need reinstall" << endl;
            //不需恢复添加按钮tab状态
            m_skipStateRecovery = true;
            finishInstall();
        }
    }
}

/**
* @brief DFInstallNormalWindow::finishInstall 所有字体安装完成
* @param void
* @return void
*/
void DFInstallNormalWindow::finishInstall()
{
    getInstallMessage = false;
    getReInstallMessage = false;

    Q_EMIT m_signalManager->finishFontInstall(m_outfileList);
    m_cancelInstall = false;
    close();
}

/*************************************************************************
 <Function>      getNoSameFilesCount
 <Description>   获取新增字体文件
 <Author>
 <Input>
    <param1>     filesList       Description:传入字体文件列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::getNoSameFilesCount(const QStringList &filesList)
{
    if (filesList.isEmpty())
        return;

    for (auto &it : filesList) {
        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(it);
        QString familyStyleName = getFamilyStyleName(fontInfo);

        if (!m_installedFontsFamilyname.contains(familyStyleName)) {
            m_installedFontsFamilyname.append(familyStyleName);
            m_outfileList.append(fontInfo.filePath);
        }
    }
}

/**
* @brief DFInstallNormalWindow::installFinished 新字体安装完成
* @param fileList 传入的安装字体列表
* @return void
*/
void DFInstallNormalWindow::installFinished()
{
    getInstallMessage = true;
    checkShowMessage();
}

/**
* @brief DFInstallNormalWindow::reInstallFinished 重复字体安装完成
* @param fileList 传入的安装字体列表
* @return void
*/
void DFInstallNormalWindow::reInstallFinished(const QStringList &fileList)
{
    getReInstallMessage = true;
    getNoSameFilesCount(fileList);
    if (getInstallMessage == true && getReInstallMessage == true) {
        qDebug() << "install refresh over";
        finishInstall();
    }
}

/*************************************************************************
 <Function>      closeEvent
 <Description>   重新实现关闭事件处理函数-停止字体管理线程
 <Author>
 <Input>
    <param1>     event           Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    qDebug() << __FUNCTION__ << m_cancelInstall;

    if (m_cancelInstall) {
        m_fontManager->cancelInstall();
        reject();
    }
}

/*************************************************************************
 <Function>      batchInstall
 <Description>   批量安装处理函数
 <Author>
 <Input>
    <param1>     reinstallFiles  Description:待重装字体文件列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::batchInstall()
{
    // Check&Sort uninstalled ,installed & damaged font file here

    QStringList installList;

    if (m_newInstallFiles.size() > 0) {
        installList << m_newInstallFiles;
    } else {
        m_fontManager->setCacheStatus(FontManagerCore::NoNewFonts);
        installFinished();
        return;
    }

    m_newInstallFiles.clear();

    //ToDo:
    //    A temp resolution for installtion.
    //dfont-install don't need query database anymore
    QStringList installListWithFamliyName;
    for (auto &it : installList) {
        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(it);
        QString familyName = getFamilyName(fontInfo);
        installListWithFamliyName.append(it + "|" + familyName);

//        qDebug() << " Prepare install file: " << it + "|" + familyName;
    }

//    qDebug() << installListWithFamliyName << endl;

    if (ifNeedShowExceptionWindow()) {
        m_fontManager->setCacheStatus(FontManagerCore::CacheLater);
    } else {
        m_fontManager->setCacheStatus(FontManagerCore::CacheNow);
    }

    m_fontManager->setType(FontManagerCore::Install);

    m_fontManager->setInstallFileList(installListWithFamliyName);
    m_fontManager->start();
}

/*************************************************************************
 <Function>      batchReInstall
 <Description>   批量重新安装处理函数
 <Author>
 <Input>
    <param1>     reinstallFiles  Description:待重装字体文件列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::batchReInstall(const QStringList &reinstallFiles)
{
    qDebug() << "start" << __FUNCTION__ << endl;

    QStringList installListWithFamliyName;
    for (auto &it : reinstallFiles) {
        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(it);
        QString familyName = getFamilyName(fontInfo);
        installListWithFamliyName.append(it + "|" + familyName);

//        qDebug() << " Prepare install file: " << it + m_loadingSpinner"|" + familyName;
    }

    m_fontManager->setType(FontManagerCore::ReInstall);
    m_fontManager->setInstallFileList(installListWithFamliyName);
    m_fontManager->start();
}

/*************************************************************************
 <Function>      batchHalfwayInstall
 <Description>   字体验证框弹出时在文件管理器进行安装
 <Author>
 <Input>
    <param1>     filelist        Description:并行安装新增文件列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::batchHalfwayInstall(const QStringList &filelist)
{
    m_installFiles = filelist;
    verifyFontFiles();

    qDebug() << m_newHalfInstalledFiles.count() << "*" << m_oldHalfInstalledFiles.count() << endl;
    m_halfInstalledFiles.append(m_newHalfInstalledFiles);

    //当安装的字体是需要新添加到字体验证框时或者已经添加到字体验证框时,刷新listview.
    if (m_errorList.count() + m_newHalfInstalledFiles.count() + m_oldHalfInstalledFiles.count() > 0) {
        emit m_signalManager->updateInstallErrorListview(m_errorList, m_halfInstalledFiles, m_newHalfInstalledFiles, m_oldHalfInstalledFiles);
    }

    QStringList installListWithFamliyName;
    for (auto &it : m_newInstallFiles) {
        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(it);
        QString familyName = getFamilyName(fontInfo);
        installListWithFamliyName.append(it + "|" + familyName);
//        qDebug() << " Prepare install file: " << it + "|" + familyName;
    }

    m_fontManager->setType(FontManagerCore::HalfwayInstall);
    m_fontManager->setCacheStatus(FontManagerCore::CacheLater);
    m_fontManager->setInstallFileList(installListWithFamliyName);
    m_fontManager->start();
}

/*************************************************************************
 <Function>      batchReInstallContinue
 <Description>   重装验证页面，继续按钮处理函数-继续批量安装
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::batchReInstallContinue()
{
    if (m_installState == InstallState::reinstall) {
        m_installedFiles.clear();
    }

    QStringList installListWithFamliyName;
    for (auto &it : m_installFiles) {
        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(it);
        QString familyName = getFamilyName(fontInfo);
        installListWithFamliyName.append(it + "|" + familyName);

//        qDebug() << " Prepare install file: " << it + "|" + familyName;
    }

    m_fontManager->setType(FontManagerCore::ReInstall);
    m_fontManager->setInstallFileList(installListWithFamliyName);
    m_fontManager->start();
}

/*************************************************************************
 <Function>      onCancelInstall
 <Description>   重装验证页面，取消按钮处理函数-QStringList()
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::onCancelInstall()
{
#ifdef QT_QML_DEBUG
    qDebug() << __FUNCTION__ << " called";
#endif
    m_errCancelInstall = true;

    if(needRefresh){
        emit m_signalManager->refreshUserFont();
    }
}

/*************************************************************************
 <Function>      onContinueInstall
 <Description>   重装验证页面，继续按钮处理函数-继续安装
 <Author>
 <Input>
    <param1>     continueInstallFontFileList    Description:重装字体文件列表
 <Return>        null                           Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::onContinueInstall(const QStringList &continueInstallFontFileList)
{
    qDebug() << __FUNCTION__ << " called:" << continueInstallFontFileList;

//ut000442 安装重复字体过程开始时,将之前的进度条清空
    m_progressBar->setValue(0);
    m_installState = InstallState::reinstall;
    //继续安装不需恢复添加按钮tab聚焦状态
    m_skipStateRecovery = true;
    batchReInstall(continueInstallFontFileList);
    if(needRefresh){
        emit m_signalManager->refreshUserFont();
    }
}

/*************************************************************************
 <Function>      onProgressChanged
 <Description>   刷新安装进度显示内容
 <Author>
 <Input>
    <param1>     familyName      Description:当前字体组名
    <param2>     percent         Description:用于计算百分百参数
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::onProgressChanged(const QString &familyName, const double &percent)
{
    if (familyName.isEmpty()) {
        return;
    }

//    qDebug() << __FUNCTION__ << familyName << percent;
    m_currentFontLabel->setText(familyName);

    m_progressBar->setValue(static_cast<int>(percent));
    m_progressBar->setTextVisible(false);
}

/*************************************************************************
 <Function>      onInstallFinished
 <Description>   字体安装后的处理函数
 <Author>
 <Input>
    <param1>     state           Description:完成状态
    <param2>     fileList        Description:安装字体文件列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::onInstallFinished(int state, const QStringList &fileList)
{
    // ToDo:
    //   May send signal to mainwindow refresh new installed font
    // QMIT notfiyRefresh;
    if (0 == state || 1 == state) {
        m_installFiles.clear();
        m_installState = InstallState::Install;

        // Update the installtion file list showed in exception dialog
        m_installFiles << m_installedFiles;

        m_installFiles << m_damagedFiles;

        //TODO:
        //   Notify UI refresh after installtion.
        // (need to refresh everytime???)

        for (const QString &file : fileList) {
            DFontInfo fontInfo = m_fontInfoManager->getFontInfo(file);
            QString familyStyleName = getFamilyStyleName(fontInfo);
            if (!m_installedFontsFamilyname.contains(familyStyleName)) {
                m_installedFontsFamilyname.append(familyStyleName);
                m_outfileList.append(fontInfo.filePath);
            }
        }
    }
    installFinished();
}

/*************************************************************************
 <Function>      onReInstallFinished
 <Description>   字体重新安装后的处理函数
 <Author>
 <Input>
    <param1>     state           Description:完成状态
    <param2>     fileList        Description:重装字体文件列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::onReInstallFinished(int state, const QStringList &fileList)
{
    // ToDo:
    //   May send signal to mainwindow refresh new installed font
    // QMIT notfiyRefresh;

    if (0 == state) {
        m_installFiles.clear();
        m_installState = InstallState::reinstall;

        // Update the installtion file list showed in exception dialog
        m_installFiles << m_installedFiles;

        m_installFiles << m_damagedFiles;
    }
    reInstallFinished(fileList);
}

/*************************************************************************
 <Function>      showInstallErrDlg
 <Description>   弹出字体验证框
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::showInstallErrDlg()
{
    m_popedInstallErrorDialg = true;

    m_pexceptionDlg = new DFInstallErrorDialog(this, m_errorList);
    m_pexceptionDlg->setParent(this);
    //取消安装
    connect(m_pexceptionDlg, &DFInstallErrorDialog::onCancelInstall, this,
            &DFInstallNormalWindow::onCancelInstall);
    //继续安装
    connect(m_pexceptionDlg, &DFInstallErrorDialog::onContinueInstall, this,
            &DFInstallNormalWindow::onContinueInstall);

    connect(m_pexceptionDlg, &DFInstallErrorDialog::closed, this, [ = ]() {
        if (!m_errCancelInstall)
            return;
        qDebug() << " DFInstallErrorDialog cancel called";
        Q_EMIT FontManagerCore::instance()->cacheFinish();
        finishInstall();
    });

    qDebug() << geometry().center() << "+" << m_pexceptionDlg->rect().center() << endl;

    m_pexceptionDlg->move(geometry().center() - m_pexceptionDlg->rect().center());
    m_pexceptionDlg->exec();
}

/*************************************************************************
 <Function>      setAddBtnHasTabs
 <Description>   记录addbutton是否有tab焦点
 <Author>
 <Input>
    <param1>     AddBtnHasTabs   Description:是否有焦点
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::setAddBtnHasTabs(bool AddBtnHasTabs)
{
    m_AddBtnHasTabs = AddBtnHasTabs;
}
void DFInstallNormalWindow::dowork()
{
    m_pthread->start();
    emit sigdowork(this);
}

/*************************************************************************
 <Function>      setSkipException
 <Description>   设置m_isNeedSkipException标志位状态
 <Author>
 <Input>
    <param1>     skip            Description:状态类型
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::setSkipException(bool skip)
{
    m_isNeedSkipException = skip;
}

/*************************************************************************
 <Function>      breakInstalltion
 <Description>   打断安装操作-关闭验证框
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::breakInstalltion()
{
    //Todo:
    //   Just close the installtion window
    if (m_pexceptionDlg->isVisible()) {
        m_pexceptionDlg->close();
        m_pexceptionDlg = nullptr;
    }

    this->hide();
}

/*************************************************************************
 <Function>      keyPressEvent
 <Description>   键盘press事件处理函数-esc键退出安装和关闭窗口
 <Author>
 <Input>
    <param1>     event           Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        reject();
        close();
    }
}

#include "dfinstallnormalwindow.moc"
