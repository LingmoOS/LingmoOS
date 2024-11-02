#ifndef GLOBALSIGNAL_H
#define GLOBALSIGNAL_H

#include <QObject>


/** ******************************
 * 用户全局信号，主要用来发送主动信号
 *********************************/
class GlobalUserSignal : public QObject
{
    Q_OBJECT

public:
    static GlobalUserSignal* getInstance();

    bool historyButtonIsShow = false;

    void exitApp(); // 退出
    void deleteAllHistoryPlaylistMessageBox(); // 显示历史列表删除对话框
    void hideSearchResult(); // 隐藏搜索列表
    void repaintLeftSidebarWidget(); // 刷新左侧歌单

//signals:
Q_SIGNALS:
    void sigExit();
    void sigDeleteAllHistoryPlaylistMessageBox();
    void sigHideSearchResult();
    void sigRepaintLeftSidebarWidget();

private:
    explicit GlobalUserSignal(QObject *parent = nullptr);
    static GlobalUserSignal* instance;
};


#define g_user_signal GlobalUserSignal::getInstance()

#endif // GLOBALSIGNAL_H
