// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "uistruct.h"
#include "queries.h"
#include "commonstruct.h"
#include "archivejob.h"

#include <DMainWindow>
#include <DFileWatcher>

class QStackedWidget;
class QSettings;
class QShortcut;

class LoadingPage;
class HomePage;
class CompressPage;
class CompressSettingPage;
class UnCompressPage;
class ProgressPage;
class SuccessPage;
class FailurePage;
class SettingDialog;
class ProgressDialog;
class CommentProgressDialog;
class DDesktopServicesThread;
class ArchiveManager;
class OpenFileWatcher;
class QFileSystemWatcher;
class CalculateSizeThread;
class TitleWidget;

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

// 主界面
class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    enum ArgumentType {
        AT_Open,            // 打开
        AT_RightMenu,       // 右键操作
        AT_DragDropAdd      // 拖拽追加
    };

public :
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    /**
     * @brief checkHerePath 检查目标路径是否可写/可执行
     * @param strPath       全路径
     * @return
     */
    bool checkHerePath(const QString &strPath);

    /**
     * @brief checkSettings 检测目标文件合法性
     * @param file  文件名
     * @return
     */
    bool checkSettings(const QString &file);

    /**
    * @brief LogCollectorMain::handleApplicationTabEventNotify
    * 处理application中notify的tab keyevent ,直接在dapplication中调用
    * 只调整我们需要调整的顺序,其他的默认
    * @param obj 接收事件的对象
    * @param evt 对象接收的键盘事件
    * @return true处理并屏蔽事件 false交给application的notify处理
     */
    bool handleApplicationTabEventNotify(QObject *obj, QKeyEvent *evt);

    /**
     * @brief handleQuit    处理退出
     */
    void handleQuit();

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initTitleBar  初始化标题栏
     */
    void initTitleBar();

    /**
     * @brief initData  初始化相关数据
     */
    void initData();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

    /**
     * @brief refreshPage   刷新界面页
     */
    void refreshPage();

    /**
     * @brief calSelectedTotalFileSize  计算本地选择的文件大小
     * @param files 选择文件
     * return 文件总大小
     */
    qint64 calSelectedTotalFileSize(const QStringList &files);

    /**
     * @brief calFileSizeByThread  用多线程的方式计算文件或文件夹所有文件大小
     * @param path 文件或文件夹路径
     */
    void calFileSizeByThread(const QString &path);

    /**
     * @brief setTitleButtonStyle   设置标题栏按钮样式以及显隐状态
     * @param bVisible  显示/隐藏添加按钮
     * @param bVisible2  显示/隐藏文件信息按钮
     * @param pixmap    图片样式
     */
    void setTitleButtonStyle(bool bVisible, bool bVisible2, DStyle::StandardPixmap pixmap = DStyle::StandardPixmap::SP_IncreaseElement);

    /**
     * @brief loadArchive       加载压缩包
     * @param strArchiveFullPath    压缩包全路径
     */
    void loadArchive(const QString &strArchiveFullPath);
    /**
     * @brief Extract2PathFinish 提取成功提示
     * @param msg
     */
    void Extract2PathFinish(const QString &msg);

    /**
     * @brief createTempPath    创建唯一标识临时路径
     * @return
     */
    QString createTempPath();

    /**
     * @brief getExtractPath   根据设置选项是否自动创建文件夹获取解压路径
     * @param strArchiveFullPath
     * @return
     */
    QString getExtractPath(const QString &strArchiveFullPath);

    /**
     * @brief handleJobNormalFinished   处理job正常结束
     * @param eType     job类型
     */
    void handleJobNormalFinished(ArchiveJob::JobType eType, ErrorType eErrorType = ET_NoError);

    /**
     * @brief handleJobCancelFinished   处理job取消结束
     * @param eType     job类型
     */
    void handleJobCancelFinished(ArchiveJob::JobType eType);

    /**
     * @brief handleJobErrorFinished   处理job错误结束
     * @param eJobType      job类型
     * @param eErrorType    错误类型
     */
    void handleJobErrorFinished(ArchiveJob::JobType eJobType, ErrorType eErrorType);

    /**
     * @brief addFiles2Archive  向压缩包中添加文件
     * @param listFiles         需要添加的文件
     * @param strPassword       密码
     */
    void addFiles2Archive(const QStringList &listFiles, const QString &strPassword = QString());

    /**
     * @brief resetMainwindow   重置界面（刷新数据）
     */
    void resetMainwindow();

    /**
     * @brief deleteWhenJobFinish   压缩/解压完成之后删除源文件
     * @param eType     job类型（压缩/解压）
     */
    void deleteWhenJobFinish(ArchiveJob::JobType eType);

    /**
     * @brief ConstructAddOptions  构造追加压缩选项
     * @param files 选择文件
     */
    void ConstructAddOptions(const QStringList &files);

    /**
     * @brief ConstructAddOptionsByThread  用多线程的方式构造追加压缩选项
     * @param path 文件或文件夹路径
     */
    void ConstructAddOptionsByThread(const QString &path);

    /**
     * @brief showSuccessInfo   显示成功信息
     * @param eSuccessInfo      成功信息
     */
    void showSuccessInfo(SuccessInfo eSuccessInfo, ErrorType eErrorType = ET_NoError);

    /**
     * @brief showErrorMessage  显示错误信息
     * @param eErrorInfo        错误信息
     * @param bShowRetry        是否显示重试按钮
     */
    void showErrorMessage(FailureInfo fFailureInfo, ErrorInfo eErrorInfo, bool bShowRetry = false);

    /**
     * @brief getConfigWinSize  通过配置文件获取之前保存的窗口大小
     * @return  之前保存的窗口大小
     */
    QSize getConfigWinSize();

    /**
     * @brief saveConfigWinSize     保存窗口尺寸
     * @param w     宽度
     * @param h     高度
     */
    void saveConfigWinSize(int w, int h);

    /**
     * @brief convertArchive 格式转换
     * @param convertType 转换后的文件类型
     */
    void convertArchive(const QString &convertType);

    /**
     * @brief updateArchiveComment 更新压缩包的注释
     */
    void updateArchiveComment();
    /**
     * @brief addArchiveComment 压缩后添zip压缩包的注释
     */
    void addArchiveComment();
    /**
     * @brief watcherArchiveFile   监听本地压缩包
     * @param strFullPath 压缩包名称（全路径）
     */
    void watcherArchiveFile(const QString &strFullPath);

    /**
     * @brief creatShorcutJson  创建快捷键配置
     * @return
     */
    QJsonObject creatShorcutJson();

    /**
     * @brief handleArguments_Open  处理文管参数-打开文件
     * @param listParam
     */
    bool handleArguments_Open(const QStringList &listParam);

    /**
     * @brief handleArguments_RightMenu     处理文管参数-右键操作
     * @param listParam
     */
    bool handleArguments_RightMenu(const QStringList &listParam);

    /**
     * @brief handleArguments_Append        处理文管操作-拖拽追加
     * @param listParam
     */
    bool handleArguments_Append(const QStringList &listParam);

    /**
     * @brief rightExtract2Path     右键解压到指定目录（当前文件夹、指定路径）
     * @param eType
     * @param listFiles
     */
    void rightExtract2Path(StartupType eType, const QStringList &listFiles/*, const QString &strTargetPath*/);

    /**
     * @brief showWarningDialog 通用的警告对话框
     * @param msg
     * @param strToolTip
     * @return
     */
    int showWarningDialog(const QString &msg, const QString &strToolTip = "");

    /**
     * @brief moveDialogToCenter    移动对话框至mainwindow中间
     * @param dialog
     */
    void moveDialogToCenter(DDialog *dialog);

    /**
     * @brief delayQuitApp  延时退出应用
     */
    void delayQuitApp();

    /**
     * @brief getCurrentStatus  获取当前解压、压缩任务是否暂停
     * @return 任务状态
     */
    bool getCurrentStatus();

    // QWidget interface
protected:
    /**
     * @brief timerEvent    定时
     */
    void timerEvent(QTimerEvent *) override;

    /**
     * @brief closeEvent    关闭
     */
    void closeEvent(QCloseEvent *) override;


Q_SIGNALS:
    /**
     * @brief sigquitApp    应用程序退出信号
     */
    void sigquitApp();
    /**重命名信号
      */
    void sigRenameFile();

private Q_SLOTS:
    /**
     * @brief slotHandleArguments   处理文管操作
     * @param listParam     参数
     * @param eType     参数
     */
    void slotHandleArguments(const QStringList &listParam, MainWindow::ArgumentType eType);

    /**
     * @brief slotTitleBtnClicked   标题栏按钮点击
     */
    void slotTitleBtnClicked();

    /**
     * @brief slotTitleBtnClicked   选择文件触发
     */
    void slotChoosefiles();

    /**
     * @brief slotDragSelectedFiles     拖拽添加文件
     * @param listFiles     拖拽的文件
     */
    void slotDragSelectedFiles(const QStringList &listFiles);

    /**
     * @brief compressLevelChanged  处理压缩层级变化
     * @param bRootIndex    是否是根目录
     */
    void slotCompressLevelChanged(bool bRootIndex);

    /**
     * @brief slotCompressNext  压缩界面点击下一步按钮处理操作
     */
    void slotCompressNext();

    /**
     * @brief slotCompress  压缩点击
     */
    void slotCompress(const QVariant &val);

    /**
     * @brief slotJobFinshed    操作结束处理
     * @param eJobType            job类型
     * @param eFinishType           结束类型
     * @param eErrorType            错误类型
     */
    void slotJobFinished(ArchiveJob::JobType eJobType, PluginFinishType eFinishType, ErrorType eErrorType);

    /**
     * @brief slotUncompressClicked     解压按钮点击，执行解压操作
     * @param strUncompressPath         解压路径
     */
    void slotUncompressClicked(const QString &strUncompressPath);

    /**
     * @brief slotReceiveProgress   进度信号处理
     * @param dPercentage   进度值
     */
    void slotReceiveProgress(double dPercentage);

    /**
     * @brief slotReceiveCurFileName    当前操作的文件名显示处理
     * @param strName       当前文件名
     */
    void slotReceiveCurFileName(const QString &strName);

    /**
     * @brief slotReceiveFileWriteErrorName    创建失败的文件处理
     * @param strName       当前文件名
     */
    void slotReceiveFileWriteErrorName(const QString &strName);

    /**
     * @brief slotQuery   发送询问信号
     * @param query 询问类型
     */
    void slotQuery(Query *query);

    /**
     * @brief slotExtract2Path  提取压缩包中文件
     * @param listSelEntry      选中的提取文件
     * @param stOptions         提取参数
     */
    void slotExtract2Path(const QList<FileEntry> &listSelEntry, const ExtractionOptions &stOptions);

    /**
     * @brief slotDelFiels    删除压缩包中文件
     * @param listSelEntry      当前选中的文件
     * @param qTotalSize        删除文件总大小
     */
    void slotDelFiles(const QList<FileEntry> &listSelEntry, qint64 qTotalSize);

    /**
     * @brief slotDelFiels    重命名压缩包中文件
     * @param listSelEntry      当前选中的文件
     * @param qTotalSize        重命名文件总大小
     */
    void slotRenameFile(const FileEntry &SelEntry, qint64 qTotalSize);

    /**
     * @brief slotReceiveCurArchiveName  当前正在操作的压缩包名称显示
     * @param strArchiveName        压缩包名称
     */
    void slotReceiveCurArchiveName(const QString &strArchiveName);

    /**
     * @brief slotOpenFile    打开压缩包中文件
     * @param entry             待打开的文件
     * @param strProgram        应用程序名（为空时，用默认应用程序打开）
     */
    void slotOpenFile(const FileEntry &entry, const QString &strProgram = "");

    /**
     * @brief slotOpenFileChanged   打开文件变化通知
     * @param strPath               文件全路径
     */
    void slotOpenFileChanged(const QString &strPath);

    /**
     * @brief slotPause       暂停
     */
    void slotPause();

    /**
     * @brief slotContinue    继续
     */
    void slotContinue();

    /**
     * @brief slotCancel       取消
     */
    void slotCancel();

    /**
     * @brief slotAddFiles      追加压缩
     * @param listFiles         需要追加压缩的文件
     * @param strPassword       加密选项
     */
    void slotAddFiles(const QStringList &listFiles, const QString &strPassword);

    /**
     * @brief slotSuccessView   成功之后查看操作
     */
    void slotSuccessView();

    /**
     * @brief slotSuccessReturn     解压成功返回
     */
    void slotSuccessReturn();

    /**
     * @brief slotFailureRetry  错误重试
     */
    void slotFailureRetry();

    /**
     * @brief slotFailureReturn 错误返回
     */
    void slotFailureReturn();

    /**
     * @brief slotTitleCommentButtonPressed 注释按钮点击
     */
    void slotTitleCommentButtonPressed();

    /**
     * @brief slotThemeChanged 系统主题变化
     */
    void slotThemeChanged();

    /**
     * @brief slotShowShortcutTip 显示快捷键提示
     */
    void slotShowShortcutTip();

    /**
     * @brief slotFinishCalculateSize 计算文件大小后开始追加流程
     */
    void slotFinishCalculateSize(qint64 size, QString strArchiveFullPath, QList<FileEntry> listAddEntry, CompressOptions stOptions, QList<FileEntry> listAllEntry);

    /**
     * @brief slotCheckFinished     文件检测结束处理
     * @param strError              错误信息
     * @param strToolTip            文件信息
     */
    void slotCheckFinished(const QString &strError, const QString &strToolTip);

private:
    QString m_strProcessID;              // 应用唯一标识（用于退出应用时清除缓存文件）
    bool m_initFlag = false;        // 界面是否初始化标志

    QStackedWidget *m_pMainWidget;  // 中心面板

    // 界面页
    HomePage *m_pHomePage;            // 首页
    CompressPage *m_pCompressPage;    // 压缩列表界面
    CompressSettingPage *m_pCompressSettingPage;  // 压缩设置界面
    UnCompressPage *m_pUnCompressPage;    // 解压列表界面
    ProgressPage *m_pProgressPage;    // 进度界面
    SuccessPage *m_pSuccessPage;  // 成功界面
    FailurePage *m_pFailurePage;  // 失败界面
    LoadingPage *m_pLoadingPage;  // 加载界面
    TitleWidget *m_pTitleWidget;
//    DIconButton *m_pTitleButton; // 标题栏按钮（添加文件）
//    DIconButton *m_pTitleCommentButton = nullptr; // 标题栏按钮（注释信息）
    QAction *m_pOpenAction;                                 // 菜单 - 打开

    // 弹窗
    ProgressDialog *m_pProgressdialog; // 进度对话框
    SettingDialog *m_pSettingDlg;       // 设置界面

    QSettings *m_pSettings;     // 默认配置信息
    QShortcut *m_openkey; // Ctrl+Shift+? 显示快捷键提示的快捷键

    Page_ID m_ePageID = PI_Home;      // 界面标识
    Archive_OperationType m_operationtype = Operation_NULL; // 操作类型

    int m_iInitUITimer = 0;                       // 初始化界面定时器
    int m_iCompressedWatchTimerID = 0;            // 压缩文件监视定时器ID

    DDesktopServicesThread *m_pDDesktopServicesThread = nullptr;    // 打开文管界面线程服务
    OpenFileWatcher *m_pOpenFileWatcher;    // 打开压缩包文件监控

    CompressParameter m_stCompressParameter;        // 压缩参数（压缩、追加压缩等）
    UnCompressParameter m_stUnCompressParameter;    // 解压参数（加载、解压等）

    UpdateOptions m_stUpdateOptions;        // 更新压缩包时选项

    CommentProgressDialog *m_commentProgressDialog;
    QString m_comment;
    bool m_isFirstViewComment = true;

    DFileWatcher *m_pFileWatcher = nullptr;                 // 文件监控
    QString m_strFinalConvertFile;     // 格式转换最终的文件全路径
    QString m_fileWriteErrorName;     // 创建失败的文件名

    StartupType m_eStartupType = StartupType::ST_Normal;      // 操作方式
    CalculateSizeThread *m_mywork = nullptr; // 计算文件(夹)大小线程
    // 适配arm平台
#ifdef __aarch64__
    qint64 maxFileSize_ = 0;
#endif

    QString m_strCurrentName;
};

class TitleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TitleWidget(QWidget *parent = nullptr);

    /**
     * @brief setTitleButtonStyle   设置标题栏按钮样式以及显隐状态
     * @param bVisible  显示/隐藏添加按钮
     * @param bVisible2  显示/隐藏文件信息按钮
     * @param pixmap    图片样式
     */
    void setTitleButtonStyle(bool bVisible, bool bVisible2, DStyle::StandardPixmap pixmap = DStyle::StandardPixmap::SP_IncreaseElement);

    /**
     * @brief setTitleButtonEnable 设置标题栏按钮是否可用
     * @param enable bool
     */
    void setTitleButtonEnable(bool enable);

    /**
     * @brief setTitleButtonVisible 设置标题栏按钮是否可见
     * @param visible bool
     */
    void setTitleButtonVisible(bool visible);

    /**
     * @brief slotThemeChanged 系统主题变化
     */
    void slotThemeChanged();

signals:
    /**
     * @brief sigTitleClicked : m_pTitleButton  点击后的信号
     */
    void sigTitleClicked();

    /**
     * @brief sigCommentClicked : m_pTitleCommentButton 点击后的信号
     */
    void sigCommentClicked();

private:
    void initUI();

    void initConnection();

private:
    DIconButton *m_pTitleButton = nullptr; // 标题栏按钮（添加文件）
    DIconButton *m_pTitleCommentButton = nullptr; // 标题栏按钮（注释信息）
};

#endif // MAINWINDOW_H
