// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNOTEMAINWINDOW_H
#define VNOTEMAINWINDOW_H

#include "common/datatypedef.h"
#include "globaldef.h"
#include "widgets/vnoteiconbutton.h"
#include "widgets/vnotepushbutton.h"
#include "common/vnoteitem.h"

#include <DMainWindow>
#include <DSearchEdit>
#include <DSplitter>
#include <DTextEdit>
#include <DStackedWidget>
#include <DLabel>
#include <DFloatingButton>
#include <DAnchors>
#include <DFloatingMessage>
#include <DMenu>

#include <QShortcut>
#include <QStandardItem>
#include <QList>
#include <QDBusPendingReply>
#include <DToolButton>
DWIDGET_USE_NAMESPACE

class VNoteRecordBar;
class VNoteIconButton;
class VNoteA2TManager;
class LeftView;
class MiddleView;
class WebRichTextEditor;
class HomePage;
class SplashView;
class VoiceNoteItem;
class DBusLogin1Manager;
class VNMainWndDelayInitTask;
class UpgradeView;
//多选操作页面
class VnoteMultipleChoiceOptionWidget;
class VNoteMainWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit VNoteMainWindow(QWidget *parent = nullptr);
    virtual ~VNoteMainWindow() override;
    //提示框选择是否关闭应用
    bool checkIfNeedExit();
    //设置标题栏tab焦点
    void setTitleBarTabFocus(QKeyEvent *event = nullptr);
    //是否需要显示最大化窗口
    bool needShowMax();

    enum WindowType {
        WndSplashAnim,
        WndHomePage,
        WndNoteShow,
#ifdef IMPORT_OLD_VERSION_DATA
        //*******Upgrade old Db code here only********
        WndUpgrade,
#endif
        //Add other type at here.
    };
    enum SpecialStatus {
        InvalidStatus = 0,
        SearchStart,
        SearchEnd,
        PlayVoiceStart,
        PlayVoiceEnd,
        RecordStart,
        RecordEnd,
        VoiceToTextStart,
        VoiceToTextEnd
    };
    //多选详情页id
    enum ButtonValue {
        Move = 1,
        SaveAsTxT = 2,
        SaveAsVoice = 3,
        Delete = 4
    };

protected:
    //初始化UI布局
    void initUI();
    //加载数据
    void initData();
    //初始化配置参数
    void initAppSetting();
    //连接槽函数
    void initConnections();
    //初始化快捷键
    void initShortcuts();
    //初始化标题栏
    void initTitleBar();
    //初始化主窗口
    void initMainView();
    //初始化记事本列表
    void initLeftView();
    //初始化记事项列表
    void initMiddleView();
    //初始化详情页
    void initRightView();
    //初始化语音转文字模块
    void initA2TManager();
    //初始化系统注销关机提示模块
    void initLogin1Manager();
    //阻塞关机/注销
    void holdHaltLock();
    //取消阻塞关机/注销
    void releaseHaltLock();
    //初始化延时任务
    void initDelayWork();
    //延时任务
    void delayInitTasks();

#ifdef IMPORT_OLD_VERSION_DATA
    //*******Upgrade old Db code here only********
    void initUpgradeView();
#endif
    //初始化有数据的窗口
    void initSpliterView();
    //初始化加载动画
    void initSplashView();
    //初始化无数据窗口
    void initEmptyFoldersView();
    //设置一些操作状态
    void setSpecialStatus(SpecialStatus status);
    //窗口切换
    void switchWidget(WindowType type);
    //重写窗口事件
    //大小改变
    void resizeEvent(QResizeEvent *event) override;
    //窗口关闭
    void closeEvent(QCloseEvent *event) override;
    //事件过滤
    bool eventFilter(QObject *o, QEvent *e) override;

public slots:
    //记事本数据加载完成
    void onVNoteFoldersLoaded();
    //当前记事本改变
    void onVNoteFolderChange(const QModelIndex &current, const QModelIndex &previous);
    //当前记事项改变
    void onVNoteChange(const QModelIndex &previous);
    //处理右键菜单
    void onMenuAction(QAction *action);
    //右键菜单弹出显示处理
    void onMenuAbout2Show();
    //搜索
    void onVNoteSearch();
    //搜索关键字改变
    void onVNoteSearchTextChange(const QString &text);
    //开始录音
    void onStartRecord(const QString &path);
    //结束录音
    void onFinshRecord(const QString &voicePath, qint64 voiceSize);
    //播放窗口播放事件处理
    void onPlayPlugVoicePlay(VNVoiceBlock *voiceData);
    //播放窗口暂停播放事件处理
    void onPlayPlugVoicePause(VNVoiceBlock *voiceData);
    //播放窗口停止播放
    void onPlayPlugVoiceStop(VNVoiceBlock *voiceData);
    //新建记事本
    void onNewNotebook();
    //新建记事项
    void onNewNote();
    //开始语音转文字
    void onA2TStart(const VNVoiceBlock *voiceBlock);
    //转写错误
    void onA2TError(int error);
    //转写成功
    void onA2TSuccess(const QString &text);
    //快捷键帮助
    void onPreviewShortcut();
    //初始化转写错误提示窗口
    void initAsrErrMessage();
    //初始化设备异常提示窗口
    void initDeviceExceptionErrMessage();
    //显示转写失败提示框
    void showAsrErrMessage(const QString &strMessage);
    //显示设备异常提示框
    void showDeviceExceptionErrMessage();
    //关闭设备异常提示框
    void closeDeviceExceptionErrMessage();
    //System shutdon
    void onSystemDown(bool active);
    //打开隐私政策
    void onShowPrivacy();
    //参数设置
    void onShowSettingDialog();
    //笔记拖拽结束
    void onDropNote(bool dropCancel);
    //响应多选详情页操作
    void handleMultipleOption(int id);
    //快捷键
    void onDeleteShortcut();
    void onEscShortcut();
    void onPoppuMenuShortcut();
    void onAddNotepadShortcut();
    void onReNameNotepadShortcut();
    void onAddNoteShortcut();
    void onRenameNoteShortcut();
    void onPlayPauseShortcut();
    void onRecordShorcut();
    void onSaveNoteShortcut();
    void onSaveVoicesShortcut();

    //主题切换
    void onThemeChanged();
    //记事本列表显示或隐藏事件响应
    void onViewChangeClicked();
    //富文本编辑器插入图片
    void onInsertImageToWebEditor();
    //响应web前端语音播放控制
    void onWebVoicePlay(const QVariant &json, bool bIsSame);
    //当前编辑区内容搜索为空
    void onWebSearchEmpty();

    void onVNoteFolderRename(QString name);

private:
    //左侧列表视图操作相关
    //添加记事本
    void addNotepad();
    //记事本重命名
    void editNotepad();
    //记事本删除
    void delNotepad();
    //初始化数据
    int loadNotepads();

    //中间列表视图操作
    //添加记事项
    void addNote();
    //记事项重命名
    void editNote();
    //删除记事项
    void delNote();
    //初始化数据
    int loadNotes(VNoteFolder *folder);
    //根据搜索关键字加载数据
    int loadSearchNotes(const QString &key);

    //Check if wen can do shortcuts
    bool canDoShortcutAction() const;
    //资源释放
    void release();
    //标题栏菜单项功能扩展
    void initMenuExtension();
    //标题栏图标控件初始化
    void initTitleIconButton();
    //刷新详情页显示
    void changeRightView(bool isMultiple);
    //设置tab焦点
    bool setTabFocus(QObject *obj, QKeyEvent *event);
    //设置标题栏关闭按钮下个控件tab焦点
    bool setTitleCloseButtonNext(QKeyEvent *event);
    //设置笔记列表下个控件tab焦点
    bool setMiddleviewNext(QKeyEvent *event);
    //设置添加笔记按钮下个控件tab焦点
    bool setAddnoteButtonNext(QKeyEvent *event);
    //设置添加记事本按钮下个控件tab焦点
    bool setAddnotepadButtonNext(QKeyEvent *event);
    //显示记事本列表
    void showNotepadList();
    //更新2栏显示笔记本名称
    void updateFolderName(QString name = "");

private:
    DSearchEdit *m_noteSearchEdit {nullptr};

#ifdef IMPORT_OLD_VERSION_DATA
    //*******Upgrade old Db code here only********
    bool m_fNeedUpgradeOldDb = false;
#endif
    //多选操作页面
    VnoteMultipleChoiceOptionWidget *m_multipleSelectWidget {nullptr};
    QStackedWidget *m_stackedRightMainWidget {nullptr};

    QWidget *m_rightViewHolder {nullptr};
    QWidget *m_recordBarHolder {nullptr};
    QWidget *m_leftViewHolder {nullptr};
    QWidget *m_middleViewHolder {nullptr};
    QWidget *m_centerWidget {nullptr};
    DLabel  *m_noteBookName {nullptr};
    QString m_noteBookNameStr;

    DSplitter *m_mainWndSpliter {nullptr};
    LeftView *m_leftView {nullptr};
    MiddleView *m_middleView {nullptr};
    WebRichTextEditor *m_richTextEdit {nullptr};

    DPushButton *m_addNotepadBtn {nullptr};
    DFloatingButton *m_addNoteBtn {nullptr};
    DAnchorsBase *m_buttonAnchor {nullptr};

    VNoteRecordBar *m_recordBar {nullptr};
    VNoteA2TManager *m_a2tManager {nullptr};

    DToolButton *m_viewChange {nullptr}; //记事本列表收起控件
    DToolButton *m_imgInsert {nullptr}; //图片插入控件

    UpgradeView *m_upgradeView {nullptr};
    SplashView *m_splashView {nullptr};
    HomePage *m_wndHomePage {nullptr};
    DStackedWidget *m_stackedWidget {nullptr};
    bool m_rightViewHasFouse {true};
    bool m_showSearchEditMenu {false};
    bool m_needShowMax {false};
    const VNVoiceBlock *m_voiceBlock {nullptr}; //语音数据

    QScopedPointer<VNVoiceBlock> m_currentPlayVoice {nullptr};

    //Shortcuts key
    //*****************Shortcut key begin*********************
    QScopedPointer<QShortcut> m_stNewNotebook; //Ctrl+N
    QScopedPointer<QShortcut> m_stRemNotebook; //F2

    QScopedPointer<QShortcut> m_stNewNote; //Ctrl+B
    QScopedPointer<QShortcut> m_stRemNote; //F3

    QScopedPointer<QShortcut> m_stPlayorPause; //Space
    QScopedPointer<QShortcut> m_stRecording; //Ctrl+R
    QScopedPointer<QShortcut> m_stVoice2Text; //Ctrl+W
    QScopedPointer<QShortcut> m_stSaveAsText; //Ctrl+S
    QScopedPointer<QShortcut> m_stSaveVoices; //Ctrl+Y
    QScopedPointer<QShortcut> m_stDelete; //Delete
    QScopedPointer<QShortcut> m_stEsc; //Esc
    QScopedPointer<QShortcut> m_stPopupMenu; //Alt+M
    QScopedPointer<QShortcut> m_stPreviewShortcuts;
    //     Help                               //F1 DTK Implementaion
    //*****************Shortcut keys end**********************

    QString m_searchKey;
    DFloatingMessage *m_asrErrMeassage {nullptr};
    DFloatingMessage *m_pDeviceExceptionMsg {nullptr};
    DMenu *m_menuExtension {nullptr};
    //Login session manager
    DBusLogin1Manager *m_pLogin1Manager {nullptr};
    QDBusPendingReply<QDBusUnixFileDescriptor> m_lockFd;

    friend class VNMainWndDelayInitTask;
};

#endif // VNOTEMAINWINDOW_H
