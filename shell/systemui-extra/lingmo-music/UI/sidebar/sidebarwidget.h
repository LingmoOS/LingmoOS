#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <UI/base/allpupwindow.h>
#include <QPalette>
#include <QPainter>
#include <QPainterPath>
#include <QColor>
#include <QStyleOption>
#include <QGSettings>

#include "myscrollarea.h"
#include "mytoolbutton.h"
#include "leftsiderbarwidget.h"

class SideBarWidget : public LeftsiderbarWidget
{
    Q_OBJECT
public:
    explicit SideBarWidget(QWidget *parent = nullptr);
public:
    void sidecolor();
    void initAddButton();

public:
    //新建歌单弹窗界面
    AllPupWindow *newSonglistPup = nullptr;
    //重命名歌单弹窗界面
    AllPupWindow *renameSongListPup = nullptr;
    MyToolButton *playListBtn = nullptr;
public Q_SLOTS:
    //添加歌单弹窗输入框
    void addPlayList();
    //添加歌单
    void addItemToSongList();
    //歌单播放
    void playAll(QString btnText);
    void rename(QString btnText);
    //歌单重命名
    void renamePlayList();
    //删除歌单
    void removePlayList(QString text);
    //取消侧边栏所有按钮的选中状态
    void slotListSearch();
    //高亮歌曲列表按钮
    void slotSongListHigh();
protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void playListBtnCliced(QString listname);
    void playListRemoved(QString listname);
    void playListRenamed(QString oldName,QString newName);
    void playListAdded(QString listName);

    void signalPlayAll(QString listName);

private:
    void initWidget();
    void initConnect();
    //获取歌单名称
    void getPlayListName();
    //输入为空添加歌单
    QString newPlayListName();


private:
    //歌曲列表 QToolButton
    //MyToolButton *playListBtn;
    //我喜欢 QToolButton
//    MyToolButton *loveBun;

    QGSettings *m_transparencyGSettings = nullptr; // 控制面板透明度
    double m_transparency = 1.0;  // 透明度

    //添加歌单按钮
    QPushButton *myPlayListBtn = nullptr;
    //新建歌单按钮界面(包含我喜欢在内)
    myScrollArea *scrollArea;

    QWidget *newPlayListWidget = nullptr;
    QWidget *logoWid = nullptr;
    QWidget *mainWid = nullptr;
//    QWidget *sideWid = nullptr;
    QVBoxLayout *newPlayListLayout = nullptr;
    //歌单名
    QStringList playListName;
    //音乐库label
    QLabel *libraryLabel = nullptr;
    //我的歌单label
    QLabel *myPlayListLabel = nullptr;
    //获取歌单名
    QString btnText;
    //保存程序关闭之前歌单按钮
    MyToolButton *m_lastBtn = nullptr;
};

#endif // SIDEBARWIDGET_H
