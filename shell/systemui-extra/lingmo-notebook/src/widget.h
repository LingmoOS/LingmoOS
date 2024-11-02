/*
* Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlError>
#include <QListWidgetItem>
#include <QDesktopWidget>
#include <iostream>
#include <QTranslator>
#include <QStandardPaths>
#include <QPushButton>
#include <QSettings>
#include <QThread>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QLabel>
#include <QProgressDialog>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QCheckBox>
#include <QBitmap>
#include <vector>
#include <QTableView>
#include <QPainterPath>
#include <QSettings>
#include <QGSettings>
#include <QDBusInterface>
#include <unistd.h>
#include <QShortcut>
#include <QClipboard>

#include "noteView.h"
#include "noteData.h"
#include "noteModel.h"
#include "dbManager.h"
#include "editPage.h"
#include "noteExitWindow.h"
#include "ui_editPage.h"
#include "adaptscreeninfo.h"
#include "about.h"
#include "emptyNotes.h"
#include "listViewModeDelegate.h"
#include "iconViewModeDelegate.h"

#include "ksearchlineedit.h"
#include "kpushbutton.h"

#include "custom_widget.h"
#include "libkydate.h"

#define     FIRST_LINE_MAX 80

/**
 * lingmo style
 */
#define THEME_QT_SCHEMA "org.lingmo.style"
#define MODE_QT_KEY "style-name"
#define FONT_SIZE "systemFontSize"
#define FONT_STYLE "systemFont"

/**
 * lingmo-control-center
 */
#define PERSONALISE_SCHEMA "org.lingmo.control-center.personalise"

/**
 * lingmo-user-guide
 */
#define USER_GUIDE_SCHEMA "com.lingmoUserGuide.hotel_1000"

extern QFont g_currentFont;

namespace Ui {
class Widget;
}

class Widget : public CustomWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    Ui::Widget *ui;                                                 //主ui
    std::vector<EditPage*> m_editors;                              //c++数组容器

    int m_isThemeChanged;                                           //主题
    int getListFlag() const;                                        //获取列表切换标志位
    void setListFlag(const int &listflag);                          //设置列表切换标志位
    void createNewNote();                                           //新建便签
    void openMemoWithId(int noteId);
    QString getWindowId(void);
public slots:
    void sltMessageReceived(/*const QString &msg*/);                //接收socket信息
    void textForNewEditpageSigReceived();                           //将选中内容复制到新便签页功能的内容传递函数
    void iniNoteModeRead();                                         //读取配置文件iniNoteMode字段的内容
    void createNewNoteIfEmpty();                                    //初始时创建一个便签

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) override;             /* 键盘响应事件 */

private:
    QString m_windowId = "";
    EditPage *m_notebook;                                          //新建便签指针
    int m_listflag;                                                 //平铺/展开列表切换
    int sortflag;                                                   //升降序切换
    noteExitWindow* m_noteExitWindow;                               //退出弹窗
    emptyNotes* m_emptyNotes;                                       //清除便签页弹窗
    QTimer* m_autoSaveTimer;                                        //自动保存定时器
    QSettings* m_settingsDatabase;                                  //配置文件
    kdk::KSearchLineEdit *m_searchLine;                             //搜索栏
    kdk::KPushButton* m_newKynote;                                       //新建按钮
    QPushButton* m_viewChangeButton;                                //列表/平铺切换按钮
    NoteView* m_noteView;                                           //listview
    listViewModeDelegate* m_plistDelegate;
    QTableView* m_noteTable;                                        //tableview
    NoteModel* m_noteModel;                                         //便签模板
    NoteModel* m_deletedNotesModel;                                 //删除模板
    QSortFilterProxyModel* m_proxyModel;                            //对项目进行排序，过滤
    QModelIndex m_currentSelectedNoteProxy;                         //当前列表
    QModelIndex m_tmpIndex;                                         //编辑时临时列表
    QModelIndex m_tmpColorIndex;                                    //调色板临时列表
    QQueue<QString> m_searchQueue;                                  //搜索队列
    DBManager* m_dbManager;                                         //数据库
    QThread* m_dbThread;                                            //数据库线程
    QMenu* m_menu;                                                  //功能菜单
    QAction* m_menuActionEmpty;                                     //菜单动作-清空便签
    QAction* m_menuExit;                                            //菜单动作-退出便签本
    //QAction* m_menuActionSet;                                       //菜单动作-设置界面
    adaptScreenInfo *m_pSreenInfo;                                  //屏幕信息
    QPoint dragPosition;                                            //拖动坐标
    bool mousePressed;                                              //鼠标是否按下
    QString m_currentTheme;                                           //当前主题名
    double m_transparency;                                          //透明度
    QDBusInterface *userGuideInterface;                                   //用户手册
    QDBusInterface *dateInterface;                                   //日期

    int m_noteCounter;                                              //便签总数
    int m_trashCounter;                                             //废纸篓总数
    bool m_isContentModified;                                       //便签内容是否修改
    bool m_isColorModified;                                         //便签颜色是否修改
    bool m_isOperationRunning;
    bool m_isTextCpNew;


    void kyNoteInit(QWidget *parent);                                              //加载界面组件
    void kyNoteConn();                                              //绑定槽函数
    void initIconMode();                                            //初始化图标
    void initListMode();                                            //初始化列表
    void listenToGsettings();                                       //监听gsettings
    void btnInit();                                                 //初始化按钮
    void searchInit();                                              //初始化搜索栏
    void deleteNote(const QModelIndex& noteIndex, bool isFromUser=true);//删除便签
    void deleteSelectedNote();                                      //获取选中便签/列表
    void setupDatabases();                                          //配置数据库
    void initializeSettingsDatabase();                              //初始化配置文件

    void setupListModeModel();                                      //代理列表模板
    void setupIconModeModel();                                      //代理图标模板
    void saveNoteToDB(const QModelIndex& noteIndex);                //保存到数据库
    NoteData* generateNote(const int noteID);                       //新建便签时初始化
    QString getFirstLine(const QString& str);                       //获取文本内容第一行
    QString getNoteDateEditor (QString dateEdited);                 //获取时间
    QDateTime getQDateTime(QString date);                           //获取日期
    void showNoteInEditor(const QModelIndex& noteIndex);            //加载便签页内容
    void selectFirstNote();                                         //选中列表头便签
    void moveNoteToTop();                                           //移动便签到列表头
    void clearSearch();                                             //清空搜索栏
    void findNotesContain(const QString &keyword);                  //过滤字符串
    void selectNote(const QModelIndex& noteIndex);                  //双击前选中目标列表
    void checkMigration();                                          //迁移sync
    void migrateNote(QString notePath);                             //便签数据迁移
    void closeAllEditors();


private slots:
    void initData();                                                //初始化加载第一个便签
    void loadNotes(QList<NoteData *> noteList, int noteCounter);    //加载便签列表
    void exitSlot();                                                //关闭按钮槽函数
    void trashSlot();                                               //清空便签页按钮槽函数
    void miniSlot();                                                //最小化按钮槽函数
    void newSlot();                                                 //新建按钮槽函数
    void listClickSlot(const QModelIndex &index);                   //item单击事件槽函数
    void listDoubleClickSlot(const QModelIndex &);                  //item双击事件槽函数
    void onTextEditTextChanged(int noteId, int i);                  //文本改变槽函数
    void onColorChanged(const QColor &color, int noteId);           //便签颜色改变槽函数
    void onTrashButtonClicked();                                    //删除槽函数
    void onSearchEditTextChanged(const QString& keyword);           //搜索栏文本改变槽函数
    void sortSlot(int index);                                       //排序槽函数
    void changePageSlot();                                          //列表平铺切换槽函数
    void setNoteNullSlot();                                         //便签页关闭置空槽函数
    void clearNoteSlot();                                           //清空便签槽函数
    void onF1ButtonClicked();                                       //快捷键F1槽函数
    void ShowContextMenu(const QPoint &pos);
    void triggerMenu(QAction *action);
    void ShortDateChangeSlot(QString date);

signals:
    void requestNotesList();                                        //加载列表请求信号
    void requestOpenDBManager(QString path, bool doCreate);         //打开数据库信号
    void requestCreateUpdateNote(NoteData* note);                   //数据库更新信号
    void requestDeleteNote(NoteData* note);                         //数据库同步删除信号
    void requestClearNote();                                        //清空数据库

    void requestMigrateNotes(QList<NoteData *> noteList);           //迁移信号
    void requestMigrateTrash(QList<NoteData *> noteList);           //迁移废纸篓信号
    void requestForceLastRowIndexValue(int index);                  //请求返回受结果的SQL语句影响的行数信号
    void switchSortTypeRequest(int index);                          //切换排序类型信号

protected:
    void handlePCMode() override;
    void handlePADMode() override;
};

#endif // WIDGET_H
