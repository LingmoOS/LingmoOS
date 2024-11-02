#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H

#include <QWidget>
#include <QKeyEvent>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QListView>
#include <QTreeView>
#include <QFrame>
#include <QX11Info>
#include <QApplication>
#include <QScreen>
#include <QTimer>

#include "UI/search/musicsearchlistview.h"
#include "UI/player/searchedit.h"

class MusicSearchListview;
class SearchEdit;
class SearchResult : public QWidget
{
    Q_OBJECT
public:
    explicit SearchResult(QWidget *parent = nullptr);
    ~SearchResult();

public:
    void autoResize();
    //设置各列表搜索关键字
    void setListviewSearchString(const QString &str);
    //点击列表时设置lineEdit显示文字
    void setLineEditSearchString(const QString &str);
    void selectUp();
    void selectDown();
    int  getCurrentIndex();
    void setSearchEdit(SearchEdit *edit);

    void changeSrearchResultPos(int posX, int posY, int width, int height);

public Q_SLOTS:
    void onReturnPressed();
    void slotLableSetFontSize(int size);
    void slotMusicItemClicked(QModelIndex index);
    void slotSingerItemClicked(QModelIndex index);
    void slotAlbumItemClicked(QModelIndex index);
protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event);
Q_SIGNALS:
    void signalFilePath(QString path);
    void signalSongListBySinger(QString singer);
    void signalSongListByAlbum(QString album);
public:
    MusicSearchListview *m_MusicView  = nullptr;
    MusicSearchListview *m_SingerView = nullptr;
    MusicSearchListview *m_AlbumView  = nullptr;

private:
    void setCursorWithXEvent();
    QVBoxLayout *vlayout = nullptr;

    QLabel *m_MusicLabel = nullptr;
    QLabel *m_SingerLabel = nullptr;
    QLabel *m_AlbumLabel = nullptr;

    int               m_CurrentIndex  = -1;
    int               m_Count         = 0;
    QFrame *m_SingerFrame = nullptr;
    QFrame *m_AlbumFrame  = nullptr;

    SearchEdit          *m_searchEdit = nullptr;

    int m_resultPosX;
    int m_resultPosY;
    int m_resultPosWidth;
    int m_resultPosHeight;
    quint32 m_key;
};

#endif // SEARCHRESULT_H
