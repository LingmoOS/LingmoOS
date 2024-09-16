// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef _DMR_ACTIONS_H
#define _DMR_ACTIONS_H
#include <QtWidgets>
#include <DMenu>
#include <QPointer>
DWIDGET_USE_NAMESPACE
namespace dmr {
class PlayingMovieInfo;
/**
 * @file 菜单选项工厂类，统一管理生成菜单
 */
class ActionFactory: public QObject
{
    Q_OBJECT
signals:
    /**
     * @brief 画面操作菜单状态改变信号
     */
    void frameMenuEnable(bool);
    /**
     * @brief 播放速度菜单状态改变信号
     */
    void playSpeedMenuEnable(bool);
    /**
      * @brief 字幕菜单状态改变信号
      */
    void subtitleMenuEnable(bool);
    /**
     * @brief mircastMenuEnable 声音菜单状态改变信号
     */
    void soundMenuEnable(bool);
public:
    enum ActionKind {
        Invalid = 0,
        OpenFile = 1,
        OpenFileList,
        OpenDirectory,
        StartPlay,
        Settings,
        About,
        Help,
        Exit,
        TogglePlaylist,
        EmptyPlaylist,
        PlaylistRemoveItem,
        PlaylistOpenItemInFM,
        PlaylistItemInfo,
        MovieInfo,
        OpenUrl,
        OpenCdrom,
        ToggleFullscreen,
        QuitFullscreen,
        ToggleMiniMode,
        WindowAbove,
        LoadSubtitle,
        SelectSubtitle, // stub for subs loaded from movie
        HideSubtitle,
        MatchOnlineSubtitle,
        ChangeSubCodepage,
        Screenshot,
        BurstScreenshot,
        GoToScreenshotSolder,
        SeekForward,
        SeekForwardLarge,
        SeekBackward,
        SeekBackwardLarge,
        SeekAbsolute,
        TogglePause,
        Stop,
        AccelPlayback,
        DecelPlayback,
        ResetPlayback,
        SubDelay, //backward
        SubForward,
        //play mode
        OrderPlay,
        ShufflePlay,
        SinglePlay,
        SingleLoop,
        ListLoop,
        //play speed
        ZeroPointFiveTimes,
        OneTimes,
        OnePointTwoTimes,
        OnePointFiveTimes,
        Double,
        //frame
        DefaultFrame,
        Ratio4x3Frame,
        Ratio16x9Frame,
        Ratio16x10Frame,
        Ratio185x1Frame,
        Ratio235x1Frame,
        ClockwiseFrame,
        CounterclockwiseFrame,
        NextFrame,
        PreviousFrame,
        //sound
        Stereo,
        LeftChannel,
        RightChannel,
        LoadTrack,
        SelectTrack, // stub for tracks loaded from movie
        GotoPlaylistNext,
        GotoPlaylistPrev,
        GotoPlaylistSelected,
        VolumeUp,
        VolumeDown,
        ToggleMute,
        ChangeVolume,
        ViewShortcut,
        Hwdec,
    };
    Q_ENUM(ActionKind)
    /**
     * @brief 获取工厂类单例
     */
    static ActionFactory &get();
    /**
     * @brief 获取菜单栏
     * @return 返回菜单指针
     */
    DMenu *titlebarMenu();
    /**
     * @brief 获取主窗口菜单
     * @return 返回菜单指针
     */
    DMenu *mainContextMenu();
    /**
     * @brief 获取主窗口菜单
     * @return 返回菜单指针
     */
    template<class UnaryFunction>
    void forEachInMainMenu(UnaryFunction f);
    /**
     * @brief 获取播放列表菜单
     * @return 返回菜单指针
     */
    DMenu *playlistContextMenu();
    /**
     * @brief 查找菜单中的功能按钮
     * @param 功能按钮枚举
     * @return 功能按钮指针
     */
    QList<QAction *> findActionsByKind(ActionKind kd);
    /**
     * @brief 根据影片信息更新菜单项
     * @param 影片信息结构体
     */
    void updateMainActionsForMovie(const PlayingMovieInfo &movieInfo);
    static bool actionHasArgs(QAction *pAction)
    {
        return pAction->property("args").isValid();
    }
    static QList<QVariant> actionArgs(QAction *pAction)
    {
        return pAction->property("args").toList();
    }
    static ActionKind actionKind(QAction *pAction)
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 2)
        auto kd = (ActionKind)pAction->property("kind").value<int>();
#else
        auto kd = pAction->property("kind").value<ActionKind>();
#endif
        return kd;
    }
    static bool isActionFromShortcut(QAction *pAction)
    {
        QVariant var = pAction->property("origin");
        return var.toString() == "shortcut";
    }
private:
    ActionFactory();
    void initMember();
private:
    DMenu *m_pTitlebarMenu;        ///标题栏菜单
    DMenu *m_pContextMenu;         ///窗口主菜单
    DMenu *m_pSubtitleMenu;        ///字幕操作子菜单
    DMenu *m_pTracksMenu;          ///音轨操作子菜单
    DMenu *m_pSoundMenu;           ///声道操作子菜单
    DMenu *m_pPlaylistMenu;        ///播放列表子菜单
    DMenu *m_pSound;               ///声音操作子菜单
    QList<QPointer<QAction>> m_listContextMenuActions; ///保存所有菜单项的索引
    QActionGroup *m_pSubgroup;     ///声道子菜单分组
    QActionGroup *m_pAudiosgroup;  ///音轨子菜单分组
};
template<class UnaryFunction>
void ActionFactory::forEachInMainMenu(UnaryFunction f)
{
    auto p = m_listContextMenuActions.begin();
    while (p != m_listContextMenuActions.end()) {
        if(!*p)
        {
            m_listContextMenuActions.removeOne(*p);
            continue;
        }
        if (strcmp((*p)->metaObject()->className(), "QAction") == 0)
            f(*p);
        ++p;
    }
}
}
#endif /* ifndef _DMR_ACTIONS_H */
