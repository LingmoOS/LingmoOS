/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MINIWIDGET_H
#define MINIWIDGET_H

#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>

#include "UIControl/player/player.h"
#include "UIControl/base/musicDataBase.h"
#include "UI/base/mylabel.h"
#include "UIControl/base/musicfileinformation.h"

const QString miniMainColor = "QFrame{background:#FFFFFF;}";

#pragma pack(push)
#pragma pack(1)

class miniWidget : public QFrame
{
    Q_OBJECT
public:
    enum PlaybackMode { CurrentItemOnce=0, CurrentItemInLoop, Sequential, Loop,  Random }; //播放循环模式枚举
    explicit miniWidget(QWidget *parent = nullptr);

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);

    void songText(QString songName); //mini 正在播放
    QString getWindowId();
protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
Q_SIGNALS:
    //发送信号mini窗口我喜欢按钮改变
    void signalFavBtnChange(QString filePath);
    //刷新我喜欢列表
    void signalRefreshFav(QString listName);
    //空格键控制播放暂停
    void signalSpaceKey();
    // Space 控制播放/暂停
    void playPauseKeySignal();
    // Ctrl+Left 控制上一首
    void previousPlayKeySignal();
    // Ctrl+Right 控制下一首
    void nextPlayKeySignal();
    // Ctrl+Up 控制音量增加
    void addVolumeKeySignal();
    // Ctrl+down 控制音量降低
    void downVolumeKeySignal();
    // Ctrl+M 切换 Mini/完整模式
    void miniCompleteSwitchKeySignal();
    // Ctrl+L 将歌曲添加到喜欢歌单
    void loveSongKeySignal();
    //Ctrl+T 切换播放模式
    void changePlaybackMode(int mode);

private:
    QString m_window_id;
    PlaybackMode m_playbackMode = Loop; //当前列表的循环模式
    void initAction();
    void initConnect();
    //初始化样式
    void initStyle();
    //通过路径查询歌曲的title和时长
    void songInfo(QString path);
    //显示时长
    void slotPositionChanged(qint64 position);
public Q_SLOTS:
    void playerStateChange(playController::PlayState newState);
    void slotFavExixts();
    void slotFavExixtsDark();
    void slotFavIsExixts(QString filePaths);
    void slotSongInfo(QString path);
    void slotFav();
    //接收播放区我喜欢按钮改变
    void slotFavBtnChange(QString filePath);
    //获取正在播放的title
    void slotPlayingLab(QString playing);
    //获取正在播放的时长
    void slotTimeLab(QString time);
    //设置播放模式
    void slotPlayModeClicked();
    //设置图标
    void setPlayMode(int playModel);
    //没有歌曲在播放
    void slotNotPlaying();
    //判断当前点击的按钮是否为我喜欢（歌单名）
    void slotText(QString btnText);
    //字体
    void slotLableSetFontSize(int size);
public:
    QFrame *m_mainFrame = nullptr;

    QLabel *m_coverLabel = nullptr;
    QPushButton *m_loveBtn = nullptr;
    QPushButton *m_volBtn = nullptr;
    QPushButton *m_orderBtn = nullptr;

    QPushButton *m_closeBtn = nullptr;
    QPushButton *m_recoveryWinBtn = nullptr;

    QPushButton *m_preBtn = nullptr;
    QPushButton *m_playStateBtn = nullptr;
    QPushButton *m_nextBtn = nullptr;

    MyLabel *m_songNameLab = nullptr;
    QLabel *m_timeLab = nullptr;

    void minicolor();

    QMenu *playModeMenu = nullptr;
    QAction *playMode_Sequential_Action = nullptr;
    QAction *playMode_Loop_Action = nullptr;
    QAction *playMode_CurrentItemInLoop_Action = nullptr;
    QAction *playMode_Random_Action = nullptr;

private:
    void init_miniWidget();
    void setCloseAndRecoveryButtonStyle();
    bool m_mouseState;

    QPoint  m_WindowPos;
    QPoint m_MousePos;

    QHBoxLayout *m_hMainLayout = nullptr;
    QHBoxLayout *m_hPlayLayout = nullptr;
    QHBoxLayout *m_hLayout = nullptr;

    QFrame *m_palyFrame = nullptr;
    QWidget *coverWid = nullptr;

    QHBoxLayout *m_HMainLayout = nullptr;

    QVBoxLayout *m_vInfoLayout = nullptr;
    QVBoxLayout *m_vSysLayout = nullptr;
    QString filePath;
    //歌单名
    QString listName;
    //正在播放歌曲的歌曲名
    QString m_title;
    //正在播放歌曲的时长
    QString m_time;
};

#pragma pack(pop)

#endif // MINIWIDGET_H
