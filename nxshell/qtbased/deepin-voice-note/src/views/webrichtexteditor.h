// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WEBRICHTEXTEDITOR_H
#define WEBRICHTEXTEDITOR_H

#include "common/vnoteitem.h"

#include <QObject>
#include <QtWebChannel/QWebChannel>
#include <QtWebEngineWidgets/QWebEngineView>

#include <QtDBus>
#include <QDBusInterface>

//获取字号接口
#ifdef OS_BUILD_V23
#define DEEPIN_DAEMON_APPEARANCE_SERVICE          "org.deepin.dde.Appearance1"
#define DEEPIN_DAEMON_APPEARANCE_PATH             "/org/deepin/dde/Appearance1"
#define DEEPIN_DAEMON_APPEARANCE_INTERFACE        "org.deepin.dde.Appearance1"
#else
#define DEEPIN_DAEMON_APPEARANCE_SERVICE           "com.deepin.daemon.Appearance"
#define DEEPIN_DAEMON_APPEARANCE_PATH              "/com/deepin/daemon/Appearance"
#define DEEPIN_DAEMON_APPEARANCE_INTERFACE         "com.deepin.daemon.Appearance"
#endif

struct VNoteItem;
class VNoteRightMenu;
class ImageViewerDialog;
class WebRichTextEditor : public QWebEngineView
{
    Q_OBJECT
public:
    explicit WebRichTextEditor(QWidget *parent = nullptr);
    ~WebRichTextEditor();

    enum Menu {
        PictureMenu = 0,
        VoiceMenu,
        TxtMenu,
        MaxMenu,
    };

    /**
     * @brief 设置笔记内容
     * @param data: 笔记内容
     * @param reg: 搜索关键字
     * @param focus: 焦点
     */
    void initData(VNoteItem *data, const QString &reg, bool focus = false);
    /**
     * @brief 插入语音
     * @param voicePath：语音路径
     * @param voiceSize: 语音时长，单位毫秒
     */
    void insertVoiceItem(const QString &voicePath, qint64 voiceSize);
    /**
     * @brief 更新编辑区内容
     */
    void updateNote();
    /**
     * @brief 搜索当前笔记
     * @param searchKey : 搜索关键字
     */
    void searchText(const QString &searchKey);

    /**
     * @brief 解除绑定的笔记数据
     */
    void unboundCurrentNoteData();

    /**
     * @brief 快捷键触发右键菜单
     */
    void shortcutPopupMenu();

    /**
     * @brief 清空Js详情页内容
     */
    void clearJSContent();
signals:
    /**
     * @brief 发送开始语音转文字的信号
     * @param voiceBlock 语音信息
     */
    void asrStart(const VNVoiceBlock *voiceBlock);

    /**
     * @brief 当前编辑区搜索内容为空
     */
    void currentSearchEmpty();

    /**
     * @brief 编辑区内容更新
     */
    void contentChanged();

public slots:
    /**
    * @brief 编辑区内容变化
    */
    void onTextChange();
    /**
    * @brief 粘贴
    */
    void onPaste(bool isVoicePaste = false);
    /**
    * @brief saveMenuParam 接收web弹出菜单类型及数据
    * @param type  菜单类型
    * @param x 弹出位置横坐标
    * @param y 弹出位置纵坐标
    * @param json  内容
    */
    void saveMenuParam(int m_menuType, const QVariant &json);
    /**
    * @brief 编辑区内容设置完成
    */
    void onSetDataFinsh();
    /**
    * @brief 右键菜单项点击响应
    * @param action
    */
    void onMenuActionClicked(QAction *action);
    /**
     * @brief 删除选中内容
     */
    void deleteSelectText();
    /**
     * @brief 查看图片
     */
    void viewPicture(const QString &filePath);

    /**
     * @brief 系统主题发生改变处理
     */
    void onThemeChanged();

    /**
     * @brief 显示编辑工具栏
     * @param pos 位置
     */
    void onShowEditToolbar(const QPoint &pos);

    /**
     * @brief 隐藏编辑工具栏
     */
    void onHideEditToolbar();
    /**
     * @brief 与web端通信建立完成
     */
    void onLoadFinsh();

    /**
     * @brief 传递字体信息
     */
    void onSetFontListInfo();

protected:
    void contextMenuEvent(QContextMenuEvent *e) override;
    //拖拽事件
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;

private:

    /**
     * @brief 初始化字体列表信息
     */
    void initFontsInformation();

    /**
     * @brief 初始化数据更新定时器
     */
    void initUpdateTimer();
    /**
     * @brief 初始化编辑区
     */
    void initWebView();
    /**
     * @brief 初始化右键菜单
     */
    void initRightMenu();
    /**
     * @brief 文字菜单显示
     */
    void showTxtMenu(const QPoint &pos);
    /**
     * @brief 图片菜单显示
     */
    void showPictureMenu(const QPoint &pos);
    /**
     * @brief 语音菜单显示
     */
    void showVoiceMenu(const QPoint &pos);
    /**
     * @brief 另存图片
     */
    void savePictureAs();
    /**
     * @brief 另存语音
     */
    void saveMP3As();
    /**
     * @brief 另存文件
     */
    QString saveAsFile(const QString &originalPath, QString dirPath = "", const QString &defalutName = "");

    /**
     * @brief 设置笔记内容
     * @param data 笔记数据
     * @param reg 搜索关键字
     */
    void setData(VNoteItem *data, const QString &reg);

    /**
     * @brief 判断是否为语音粘贴
     * @return
     */
    bool isVoicePaste();

private:
    VNoteItem *m_noteData {nullptr};
    QTimer *m_updateTimer {nullptr};
    bool m_textChange {false};
    QString m_searchKey {""};
    Menu m_menuType = MaxMenu;
    QVariant m_menuJson = {};
    ImageViewerDialog *imgView {nullptr}; //
    QRect m_editToolbarRect; //编辑工具栏坐标
    QPoint m_mouseClickPos {-1, -1}; //鼠标点击位置
    bool m_setFocus {false}; //是否设置焦点
    //右键菜单
    VNoteRightMenu *m_pictureRightMenu {nullptr}; //图片右键菜单
    VNoteRightMenu *m_voiceRightMenu {nullptr}; //语音右键菜单
    VNoteRightMenu *m_txtRightMenu {nullptr}; //文字右键菜单
    bool m_loadFinshSign = false; //后台与web通信连通标志 true: 连通， false: 未联通

    QScopedPointer<VNVoiceBlock> m_voiceBlock {nullptr}; //待另存的语音数据
    /**
     * @brief 音频服务dbus接口
     * 对应为 (org.deepin.dde.Audio)
     */
    QDBusInterface *m_appearanceDBusInterface = nullptr;

    QString                     m_FontDefault = "";        //默认字体
    QStringList                 m_fontList;                //字体列表

};

#endif // WEBRICHTEXTEDITOR_H
