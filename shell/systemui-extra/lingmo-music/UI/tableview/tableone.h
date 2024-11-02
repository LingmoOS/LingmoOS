#ifndef TableOne_H
#define TableOne_H

#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QVBoxLayout>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFileSystemWatcher>
#include <QStandardPaths>
#include <QScrollArea>
#include <QScrollBar>
#include <QProcess>

#include "UIControl/base/musicDataBase.h"
#include "UIControl/tableview/musiclistmodel.h"
#include "UIControl/player/player.h"
#include "tableviewdelegate.h"
#include "tablebaseview.h"
#include "UIControl/base/musicfileinformation.h"
#include "UI/base/mylabel.h"
#include "UI/base/widgetstyle.h"
#include "UI/base/allpupwindow.h"

class TableOne : public QWidget
{
    Q_OBJECT
public:
    explicit TableOne(QString listName,QWidget *parent = nullptr);
    ~TableOne();
public:
    /// 基于MVC设计模式设计的, C(controllor)控制在Qt中被弱化，与View合并到一起。
    /// 表格视图控件 V(TableBaseView)，需要和 M(MusicListModel), 配套使用
    MusicListModel *m_model = nullptr; // 数据模型，不能单独显示出来
    TableBaseView *tableView = nullptr; // 视图，要来显示数据模型MusicListModel， 歌单列表，包括歌单标题和内容

    QLabel *listTitleLabel = nullptr; // 歌单名称
    QString nowListName;
    MusicInfoDialog *infoDialog = nullptr;
    void changeNumber();
    void initTableViewStyle();
    void setHightLightAndSelect();

    void getMusicList();

    void playMusicforIndex(QString listName,int index);
    //通过列表名和索引值播放相应歌曲
    void showTitleText(QString listName);

    void initStyle();
    void addMusicToDatebase(QStringList fileNames);
public Q_SLOTS:
    void slotSearchTexts(QString text);
    void slotSearchReturnPressed(QString listName);
    void slotLableSetFontSize(int size);
    void slotFilePath(QString path);
    void slotSongListBySinger(QString singer);
    void slotSongListByAlbum(QString album);
private:
    void initUI();  //初始化ui
    void initConnect();  //信号绑定
    void initRightMenu();  //初始化右键菜单
    void tableViewDoubleClicked();  //双击播放

//    static void deleteImage(const QString &savepath);

    //成功添加多少首歌曲
    void importFinished(int successCount, int failCount, int allCount);
    //导入失败
    void importFailed(int successCount, int failCount, int allCount);

    void showRightMenu(const QPoint &pos);

    void isDeleteSongs(); //是否从歌单中删除歌曲
    void isDeleteLocalSongs(); //是否从本地删除歌曲
    void deleteSongs();  //从歌单中删除歌曲
    void deleteLocalSongs(); //从本地以及歌单中删除歌曲
    void deleteLocalSongsWayland(); //从本地以及歌单中删除歌曲

    void playSongs();
    void showInfo();

    void addToOtherList(QAction *listNameAction);

    void addMusicSlot(); // 添加歌曲文件槽函数
    void addDirMusicSlot();  //添加文件夹槽函数
//    void addMusicToDatebase(QStringList fileNames);

//    void importSongs(QString path);

    void clearSearchList();

    QMenu *m_menu = nullptr;  //新建一个Menu属性
    QSqlQueryModel *tableModel = nullptr;

    QAction * playRow = nullptr;
    QAction * removeRow = nullptr;
    QAction * removeLocalRow = nullptr;
    QAction * showInfoRow = nullptr;
    QMenu * addToOtherListMenu = nullptr;
    QMap<int,QString> getSelectedTaskIdList();

    QMenu *add_menu = nullptr;
    QAction *addMusicFileAction = nullptr;
    QAction *addDirMusicAction = nullptr;

    TableViewDelegate *delegate = nullptr;

    QHBoxLayout *listTitleHBoxLayout = nullptr;
    QToolButton *addMusicButton = nullptr;  //添加歌曲按钮
    QPushButton *playAllButton = nullptr;

    QLabel *listTotalNumLabel = nullptr; // 统计歌单中歌曲总数
    int heightLightIndex = -1;
    QString nowPlayListName;  //后端播放器中正在播放的列表名
Q_SIGNALS:
    void sendPathToPlayer(QString fp);
    void countChanges();
    void hoverIndexChanged(QModelIndex index);
    void refreshHistoryListSignal();
//    void heightIndexChanged(int index);
    void addILoveFilepathSignal(QString filePath);  //传递我喜欢歌单中添加歌曲的信号
    void removeILoveFilepathSignal(QString filePath);  //传递我喜欢歌单中删除歌曲的信号
    void signalListSearch();    //取消侧边栏所有按钮的选中状态
    void signalSongListHigh();  //高亮左侧歌曲列表按钮
public Q_SLOTS:
    void selectListChanged(QString listname);  //切换歌单
    void playListRenamed(QString oldName,QString newName);  //歌单重命名
    void getHightLightIndex(int index, QString listName); //获得正在播放的歌曲索引和歌单名

    void playAll(QString listName);  //播放全部歌曲
    void playAllButtonClicked();
    void slotReturnText(QString text);
protected:
    void dragEnterEvent(QDragEnterEvent *event)Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *event)Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event)Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event)Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
private:
    QWidget *m_musicWidget = nullptr;
    QVBoxLayout *m_historyLayout = nullptr;
    QWidget *nullPageWidget = nullptr;
    QVBoxLayout *nullPageVLayout = nullptr;
    QHBoxLayout *nullPageHLayout = nullptr;
    QPushButton *n_addMusicButton = nullptr;
    QPushButton *n_addDirMusicButton = nullptr;
    QLabel *nullPageIconLabel = nullptr;
    QLabel *nullPageTextLabel = nullptr;

    QHeaderView *horizonHeader = nullptr; // 表头内容
    QWidget *titleWid = nullptr;

    int showScrollbarNumber = 10;
    QString m_text;
    QMap<int,QString> m_map;
};

#endif // TableOne_H
