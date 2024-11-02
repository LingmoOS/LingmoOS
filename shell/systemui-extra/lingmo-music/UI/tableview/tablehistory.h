#ifndef TABLEHISTORY_H
#define TABLEHISTORY_H

#include <QWidget>
#include <QListView>
#include <QLabel>
#include <QToolButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QSqlRelationalTableModel>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QVBoxLayout>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>

#include "tablebaseview.h"
#include "UIControl/tableview/musiclistmodel.h"
#include "UIControl/base/musicDataBase.h"
#include "UIControl/player/player.h"
#include "UI/base/widgetstyle.h"

class TableHistory : public QDialog
{
    Q_OBJECT
public:
    explicit TableHistory(QWidget *parent = nullptr);
    TableBaseView *m_tableHistory = nullptr;
    MusicListModel *m_model = nullptr;
    QMap <QString ,QString> removeList = {{"",""}}; // 移除列表
    void changeNumber();
    void setHighlight(int index);
    void initStyle();
    void initTableStyle();
    bool isHightLight = false;
    void refreshHistoryTable();
    void noRefreshHistory();
    void deleteAllClicked();
    void changePlayHistoryPos(int posX, int posY, int width, int height);
public Q_SLOTS:
    void showHistroryPlayList();
//    void addMusicToHistoryListSlot();
    void slotPlayIndexChanged(int index, QString listname);
    void slotPlayPathChanged(QString songPath);
    //字体
    void slotLableSetFontSize(int size);
protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;
Q_SIGNALS:
    void signalHistoryPlaying();
    void signalHistoryBtnChecked(bool checked);
private Q_SLOTS:
    void playSongs();
    void deleteSongs();
    void playNextRowClicked();
    void showRightMenu(const QPoint &pos);
    void slotAddToRList(QString path,QString owner);
    void slotRemoveFromRList(QString path);


private:
    void initRightMenu();
    QVBoxLayout *mainVLayout = nullptr;
    QHBoxLayout *titleHBoxLayout = nullptr;
    void initSetModel();
    void initConnect();
    QLabel *listCountLabel = nullptr;
    QLabel *historyTitileLabel = nullptr;
    QToolButton *deleteAllBtn = nullptr;
    QWidget *historyTitileWidget = nullptr;

    QAction *playRow = nullptr;
    QAction *removeRow = nullptr;
    QAction *playNextRow = nullptr;
    QMenu *m_menu = nullptr;
    QString nowListName = HISTORY;
    QString nowPlayListName = "";
    int nowPlayIndex = -1;

    QWidget *nullPageWidget = nullptr;
    QVBoxLayout *nullPageLayout = nullptr;
    QLabel *nullIconLabel = nullptr;
    QLabel *nullTextLabel = nullptr;
    QHeaderView *horizonHeader = nullptr;
private:
    int playHistoryPosX;
    int playHistoryPosY;
    int playHistoryPosWidth;
    int playHistoryPosHeight;
};

#endif // TABLEHISTORY_H
