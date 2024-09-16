// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"
#include "actions.h"
#include "player_engine.h"
#include "compositing_manager.h"

namespace dmr {
static ActionFactory *pActionFactory = nullptr;
ActionFactory &ActionFactory::get()
{
    if (pActionFactory == nullptr) {
        pActionFactory = new ActionFactory();
    }
    return *pActionFactory;
}
#define DEF_ACTION(NAME, KD) do { \
        QPointer<QAction> pAct = pMenu_p->addAction((NAME)); \
        pAct->setProperty("kind", KD); \
        m_listContextMenuActions.append(pAct); \
    } while (0)
#define DEF_ACTION_CHECKED(NAME, KD) do { \
        QPointer<QAction> pAct = pMenu_p->addAction((NAME)); \
        pAct->setCheckable(true); \
        pAct->setProperty("kind", KD); \
        m_listContextMenuActions.append(pAct); \
    } while (0)
#define DEF_ACTION_CHECKED_NEW(NAME, KD) do { \
        QPointer<QAction> pAct = pMenu->addAction((NAME)); \
        pAct->setCheckable(true); \
        pAct->setProperty("kind", KD); \
        m_listContextMenuActions.append(pAct); \
    } while (0)
#define DEF_ACTION_GROUP(NAME, KD, GROUP) do { \
        QPointer<QAction> pAct = pMenu->addAction((NAME)); \
        pAct->setProperty("kind", KD); \
        m_listContextMenuActions.append(pAct); \
    } while (0)
#define DEF_ACTION_CHECKED_GROUP(NAME, KD, GROUP) do { \
        QPointer<QAction> pAct = pMenu->addAction((NAME)); \
        pAct->setCheckable(true); \
        pAct->setProperty("kind", KD); \
        pAct->setActionGroup(GROUP); \
        m_listContextMenuActions.append(pAct); \
    } while (0)
DMenu *ActionFactory::titlebarMenu()
{
    if (!m_pTitlebarMenu) {
        DMenu *pMenu_p = new DMenu();
        DEF_ACTION(tr("Open file"), ActionKind::OpenFileList);
        if (!CompositingManager::isPadSystem()) {
            DEF_ACTION(tr("Open folder"), ActionKind::OpenDirectory);
            DEF_ACTION(tr("Settings"), ActionKind::Settings);
            pMenu_p->addSeparator();
            // these seems added by titlebar itself
            //DEF_ACTION("About", ActionKind::About);
            //DEF_ACTION("Help", ActionKind::Help);
            //DEF_ACTION("Exit", ActionKind::Exit);
        } else {
            m_pTitlebarMenu = pMenu_p;
            {
                DMenu *pParent = pMenu_p;
                DMenu *pMenu = new DMenu(tr("Play"));
                QActionGroup *pActionGroup = new QActionGroup(pMenu);
                DEF_ACTION_GROUP(tr("Previous"), ActionKind::GotoPlaylistPrev, pActionGroup);
                DEF_ACTION_GROUP(tr("Next"), ActionKind::GotoPlaylistNext, pActionGroup);
                pMenu->addSeparator();
                DEF_ACTION_CHECKED_GROUP(tr("Order play"), ActionKind::OrderPlay, pActionGroup);
                DEF_ACTION_CHECKED_GROUP(tr("Shuffle play"), ActionKind::ShufflePlay, pActionGroup);
                DEF_ACTION_CHECKED_GROUP(tr("Single play"), ActionKind::SinglePlay, pActionGroup);
                DEF_ACTION_CHECKED_GROUP(tr("Single loop"), ActionKind::SingleLoop, pActionGroup);
                DEF_ACTION_CHECKED_GROUP(tr("List loop"), ActionKind::ListLoop, pActionGroup);
                pParent->addMenu(pMenu);
            }
            {
                DMenu *pParent = pMenu_p;
                DMenu *pMenu = new DMenu(tr("Frame"));
                QActionGroup *pActionGroup = new QActionGroup(pMenu);
                DEF_ACTION_CHECKED_GROUP(tr("Default"), ActionKind::DefaultFrame, pActionGroup);
                DEF_ACTION_CHECKED_GROUP(("4:3"), ActionKind::Ratio4x3Frame, pActionGroup);
                DEF_ACTION_CHECKED_GROUP(("16:9"), ActionKind::Ratio16x9Frame, pActionGroup);
                DEF_ACTION_CHECKED_GROUP(("16:10"), ActionKind::Ratio16x10Frame, pActionGroup);
                DEF_ACTION_CHECKED_GROUP(("1.85:1"), ActionKind::Ratio185x1Frame, pActionGroup);
                DEF_ACTION_CHECKED_GROUP(("2.35:1"), ActionKind::Ratio235x1Frame, pActionGroup);
                pMenu->addSeparator();
                DEF_ACTION_GROUP(tr("Clockwise"), ActionKind::ClockwiseFrame, pActionGroup);
                DEF_ACTION_GROUP(tr("Counterclockwise"), ActionKind::CounterclockwiseFrame, pActionGroup);
                pMenu->addSeparator();
                DEF_ACTION_GROUP(tr("Next frame"), ActionKind::NextFrame, pActionGroup);
                DEF_ACTION_GROUP(tr("Previous frame"), ActionKind::PreviousFrame, pActionGroup);
                pParent->addMenu(pMenu);
                pMenu->setEnabled(false);
                connect(this, &ActionFactory::frameMenuEnable, this, [ = ](bool statu) {
                    pMenu->setEnabled(statu);
                });
            }
            {
                DMenu *pParent = pMenu_p;
                DMenu *pMenu = new DMenu(tr("Playback speed"));
                QActionGroup *pActionGroup = new QActionGroup(pMenu);
                DEF_ACTION_CHECKED_GROUP(tr("0.5x"), ActionKind::ZeroPointFiveTimes, pActionGroup);
                DEF_ACTION_CHECKED_GROUP(tr("1.0x"), ActionKind::OneTimes, pActionGroup);
                DEF_ACTION_CHECKED_GROUP(tr("1.2x"), ActionKind::OnePointTwoTimes, pActionGroup);
                DEF_ACTION_CHECKED_GROUP(tr("1.5x"), ActionKind::OnePointFiveTimes, pActionGroup);
                DEF_ACTION_CHECKED_GROUP(tr("2.0x"), ActionKind::Double, pActionGroup);
                pParent->addMenu(pMenu);
                pMenu->setEnabled(false);
                connect(this, &ActionFactory::playSpeedMenuEnable, this, [ = ](bool statu) {
                    pMenu->setEnabled(statu);
                });
            }
        }

        m_pTitlebarMenu = pMenu_p;
    }
    return m_pTitlebarMenu;
}

DMenu *ActionFactory::mainContextMenu()
{
    if(m_pContextMenu) {
       return m_pContextMenu;
    }

    if (CompositingManager::isMpvExists()) {
        DMenu *pMenu_p = new DMenu();
        DEF_ACTION(tr("Open file"), ActionKind::OpenFileList);
        DEF_ACTION(tr("Open folder"), ActionKind::OpenDirectory);
        DEF_ACTION(tr("Open URL"), ActionKind::OpenUrl);
        DEF_ACTION(tr("Open CD/DVD"), ActionKind::OpenCdrom);
        pMenu_p->addSeparator();
        DEF_ACTION_CHECKED(tr("Fullscreen"), ActionKind::ToggleFullscreen);
        DEF_ACTION_CHECKED(tr("Mini mode"), ActionKind::ToggleMiniMode);
        DEF_ACTION_CHECKED(tr("Always on Top"), ActionKind::WindowAbove);
        pMenu_p->addSeparator();
        {
            DMenu *pParent = pMenu_p;         //这里使用代码块和局部变量为了使结构清晰
            DMenu *pMenu = new DMenu(tr("Play"));
            QActionGroup *pActionGroup = new QActionGroup(pMenu);
            DEF_ACTION_GROUP(tr("Previous"), ActionKind::GotoPlaylistPrev, pActionGroup);
            DEF_ACTION_GROUP(tr("Next"), ActionKind::GotoPlaylistNext, pActionGroup);
            pMenu->addSeparator();
            DEF_ACTION_CHECKED_GROUP(tr("Order play"), ActionKind::OrderPlay, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("Shuffle play"), ActionKind::ShufflePlay, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("Single play"), ActionKind::SinglePlay, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("Single loop"), ActionKind::SingleLoop, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("List loop"), ActionKind::ListLoop, pActionGroup);
            pParent->addMenu(pMenu);
        }
        {
            DMenu *pParent = pMenu_p;
            DMenu *pMenu = new DMenu(tr("Playback speed"));
            QActionGroup *pActionGroup = new QActionGroup(pMenu);
            DEF_ACTION_CHECKED_GROUP(tr("0.5x"), ActionKind::ZeroPointFiveTimes, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("1.0x"), ActionKind::OneTimes, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("1.2x"), ActionKind::OnePointTwoTimes, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("1.5x"), ActionKind::OnePointFiveTimes, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("2.0x"), ActionKind::Double, pActionGroup);
            pParent->addMenu(pMenu);
            pMenu->setEnabled(false);
            connect(this, &ActionFactory::playSpeedMenuEnable, this, [ = ](bool statu) {
                pMenu->setEnabled(statu);
            });
        }
        {
            DMenu *pParent = pMenu_p;
            DMenu *pMenu = new DMenu(tr("Frame"));
            QActionGroup *pActionGroup = new QActionGroup(pMenu);
            DEF_ACTION_CHECKED_GROUP(tr("Default"), ActionKind::DefaultFrame, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(("4:3"), ActionKind::Ratio4x3Frame, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(("16:9"), ActionKind::Ratio16x9Frame, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(("16:10"), ActionKind::Ratio16x10Frame, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(("1.85:1"), ActionKind::Ratio185x1Frame, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(("2.35:1"), ActionKind::Ratio235x1Frame, pActionGroup);
            pMenu->addSeparator();
            DEF_ACTION_GROUP(tr("Clockwise"), ActionKind::ClockwiseFrame, pActionGroup);
            DEF_ACTION_GROUP(tr("Counterclockwise"), ActionKind::CounterclockwiseFrame, pActionGroup);
            pMenu->addSeparator();
            DEF_ACTION_GROUP(tr("Next frame"), ActionKind::NextFrame, pActionGroup);
            DEF_ACTION_GROUP(tr("Previous frame"), ActionKind::PreviousFrame, pActionGroup);
            pParent->addMenu(pMenu);
            pMenu->setEnabled(false);
            connect(this, &ActionFactory::frameMenuEnable, this, [ = ](bool statu) {
                pMenu->setEnabled(statu);
            });
        }
        {
            //sound pMenu
            DMenu *pParent = pMenu_p;
            DMenu *pMenu = new DMenu(tr("Sound"));
            m_pSound = pMenu;
            {
                DMenu *pParent_channel = pMenu;
                DMenu *pMenu = new DMenu(tr("Channel"));
                m_pSoundMenu = pMenu;
                QActionGroup *pActionGroup = new QActionGroup(pMenu);
                DEF_ACTION_CHECKED_GROUP(tr("Stereo"), ActionKind::Stereo, pActionGroup);
                DEF_ACTION_CHECKED_GROUP(tr("Left channel"), ActionKind::LeftChannel, pActionGroup);
                DEF_ACTION_CHECKED_GROUP(tr("Right channel"), ActionKind::RightChannel, pActionGroup);
                pParent_channel->addMenu(pMenu);
            }
            {
                DMenu *parent_track = pMenu;
                DMenu *pMenutemp = new DMenu(tr("Track"));
                m_pTracksMenu = pMenutemp;
                //DEF_ACTION(tr("Select Track"), ActionKind::SelectTrack);
                parent_track->addMenu(pMenutemp);
            }
            pParent->addMenu(pMenu);
            connect(this, &ActionFactory::soundMenuEnable, this, [ = ](bool statu) {
                pMenu->setEnabled(statu);
            });
        }
        {
            //sub pMenu
            DMenu *pParent = pMenu_p;
            DMenu *pMenu = new DMenu(tr("Subtitle"));
            QActionGroup *pActionGroup = new QActionGroup(pMenu);
            DEF_ACTION_GROUP(tr("Load"), ActionKind::LoadSubtitle, pActionGroup);
            DEF_ACTION_GROUP(tr("Online search"), ActionKind::MatchOnlineSubtitle, pActionGroup);
            //DEF_ACTION(tr("Select"), ActionKind::SelectSubtitle);
            {
                DMenu *pParent_select = pMenu;
                DMenu *pMenu = new DMenu(tr("Select"));
                m_pSubtitleMenu = pMenu;
                pParent_select->addMenu(pMenu);
            }
            {
                DMenu *parent_encoding = pMenu;
                DMenu *pMenu = new DMenu(tr("Encodings"));
                QActionGroup *pGroup_encoding = new QActionGroup(pMenu);
                //title <-> codepage
                static QVector<QPair<QString, QString>> list = {
                    {"Auto", "auto"},
                    {"Universal (UTF-8)", "UTF-8"},
                    {"Universal (UTF-16)", "UTF-16"},
                    {"Universal (UTF-16BE)", "UTF-16BE"},
                    {"Universal (UTF-16LE)", "UTF-16LE"},
                    {"Arabic (ISO-8859-6)", "ISO-8859-6"},
                    {"Arabic (WINDOWS-1256)", "WINDOWS-1256"},
                    {"Baltic (LATIN7)", "LATIN7"},
                    {"Baltic (WINDOWS-1257)", "WINDOWS-1257"},
                    {"Celtic (LATIN8)", "LATIN8"},
                    {"Central European (WINDOWS-1250)", "WINDOWS-1250"},
                    {"Cyrillic (ISO-8859-5)", "ISO-8859-5"},
                    {"Cyrillic (WINDOWS-1251)", "WINDOWS-1251"},
                    {"Eastern European (ISO-8859-2)", "ISO-8859-2"},
                    {"Western Languages (WINDOWS-1252)", "WINDOWS-1252"},
                    {"Greek (ISO-8859-7)", "ISO-8859-7"},
                    {"Greek (WINDOWS-1253)", "WINDOWS-1253"},
                    {"Hebrew (ISO-8859-8)", "ISO-8859-8"},
                    {"Hebrew (WINDOWS-1255)", "WINDOWS-1255"},
                    {"Japanese (SHIFT-JIS)", "SHIFT-JIS"},
                    {"Japanese (ISO-2022-JP-2)", "ISO-2022-JP-2"},
                    {"Korean (EUC-KR)", "EUC-KR"},
                    {"Korean (CP949)", "CP949"},
                    {"Korean (ISO-2022-KR)", "ISO-2022-KR"},
                    {"Nordic (LATIN6)", "LATIN6"},
                    {"North European (LATIN4)", "LATIN4"},
                    {"Russian (KOI8-R)", "KOI8-R"},
                    {"Simplified Chinese (GBK)", "GBK"},
                    {"Simplified Chinese (GB18030)", "GB18030"},
                    {"Simplified Chinese (ISO-2022-CN-EXT)", "ISO-2022-CN-EXT"},
                    {"South European (LATIN3)", "LATIN3"},
                    {"South-Eastern European (LATIN10)", "LATIN10"},
                    {"Thai (TIS-620)", "TIS-620"},
                    {"Thai (WINDOWS-874)", "WINDOWS-874"},
                    {"Traditional Chinese (EUC-TW)", "EUC-TW"},
                    {"Traditional Chinese (BIG5)", "BIG5"},
                    {"Traditional Chinese (BIG5-HKSCS)", "BIG5-HKSCS"},
                    {"Turkish (LATIN5)", "LATIN5"},
                    {"Turkish (WINDOWS-1254)", "WINDOWS-1254"},
                    {"Ukrainian (KOI8-U)", "KOI8-U"},
                    {"Vietnamese (WINDOWS-1258)", "WINDOWS-1258"},
                    {"Vietnamese (VISCII)", "VISCII"},
                    {"Western European (LATIN1)", "LATIN1"},
                    {"Western European (LATIN-9)", "LATIN-9"}
                };
                auto p = list.begin();
                while (p != list.end()) {
                    DEF_ACTION_CHECKED_GROUP(p->first, ActionKind::ChangeSubCodepage, pGroup_encoding);
                    QAction *pAct = pMenu->actions().last();
                    pAct->setProperty("args", QList<QVariant>() << p->second);
                    if (p->second == "auto") pMenu->addSeparator();
                    p++;
                }
                parent_encoding->addMenu(pMenu);
            }
            pParent->addMenu(pMenu);
            connect(this, &ActionFactory::subtitleMenuEnable, this, [ = ](bool statu) {
                pMenu->setEnabled(statu);
            });
        }
        {
            //sub pMenu
            DMenu *parent = pMenu_p;
            DMenu *pMenu = new DMenu(tr("Screenshot"));
            //cppcheck 误报
            QActionGroup *pActionGroup = new QActionGroup(pMenu);
            DEF_ACTION_GROUP(tr("Film screenshot"), ActionKind::Screenshot, pActionGroup);
            DEF_ACTION_GROUP(tr("Burst shooting"), ActionKind::BurstScreenshot, pActionGroup);
            DEF_ACTION_GROUP(tr("Open screenshot folder"), ActionKind::GoToScreenshotSolder, pActionGroup);
            pMenu->setEnabled(false);
            parent->addMenu(pMenu);
            connect(this, &ActionFactory::frameMenuEnable, this, [ = ](bool statu) {
                pMenu->setEnabled(statu);
            });
        }
        pMenu_p->addSeparator();
        DEF_ACTION_CHECKED(tr("Playlist"), ActionKind::TogglePlaylist);
        DEF_ACTION(tr("Film info"), ActionKind::MovieInfo);
        DEF_ACTION(tr("Settings"), ActionKind::Settings);
        m_pContextMenu = pMenu_p;
    } else {
        DMenu *pMenu_p = new DMenu();
        DEF_ACTION(tr("Open file"), ActionKind::OpenFileList);
        DEF_ACTION(tr("Open folder"), ActionKind::OpenDirectory);
        DEF_ACTION(tr("Open URL"), ActionKind::OpenUrl);
        DEF_ACTION(tr("Open CD/DVD"), ActionKind::OpenCdrom);
        pMenu_p->addSeparator();
        DEF_ACTION_CHECKED(tr("Fullscreen"), ActionKind::ToggleFullscreen);
        DEF_ACTION_CHECKED(tr("Mini mode"), ActionKind::ToggleMiniMode);
        DEF_ACTION_CHECKED(tr("Always on Top"), ActionKind::WindowAbove);
        pMenu_p->addSeparator();
        {
            DMenu *pParent = pMenu_p;         //这里使用代码块和局部变量为了使结构清晰
            DMenu *pMenu = new DMenu(tr("Play"));
            QActionGroup *pActionGroup = new QActionGroup(pMenu);
            DEF_ACTION_GROUP(tr("Previous"), ActionKind::GotoPlaylistPrev, pActionGroup);
            DEF_ACTION_GROUP(tr("Next"), ActionKind::GotoPlaylistNext, pActionGroup);
            pMenu->addSeparator();
            DEF_ACTION_CHECKED_GROUP(tr("Order play"), ActionKind::OrderPlay, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("Shuffle play"), ActionKind::ShufflePlay, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("Single play"), ActionKind::SinglePlay, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("Single loop"), ActionKind::SingleLoop, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("List loop"), ActionKind::ListLoop, pActionGroup);
            pParent->addMenu(pMenu);
        }
        {
            DMenu *pParent = pMenu_p;
            DMenu *pMenu = new DMenu(tr("Playback speed"));
            QActionGroup *pActionGroup = new QActionGroup(pMenu);
            DEF_ACTION_CHECKED_GROUP(tr("0.5x"), ActionKind::ZeroPointFiveTimes, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("1.0x"), ActionKind::OneTimes, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("1.2x"), ActionKind::OnePointTwoTimes, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("1.5x"), ActionKind::OnePointFiveTimes, pActionGroup);
            DEF_ACTION_CHECKED_GROUP(tr("2.0x"), ActionKind::Double, pActionGroup);
            pParent->addMenu(pMenu);
            pMenu->setEnabled(false);
            connect(this, &ActionFactory::playSpeedMenuEnable, this, [ = ](bool statu) {
                pMenu->setEnabled(statu);
            });
        }
        {
            //sub pMenu
            DMenu *parent = pMenu_p;
            DMenu *pMenu = new DMenu(tr("Screenshot"));
            //cppcheck 误报
            QActionGroup *pActionGroup = new QActionGroup(pMenu);
            DEF_ACTION_GROUP(tr("Film screenshot"), ActionKind::Screenshot, pActionGroup);
            DEF_ACTION_GROUP(tr("Open screenshot folder"), ActionKind::GoToScreenshotSolder, pActionGroup);
            pMenu->setEnabled(false);
            parent->addMenu(pMenu);
            connect(this, &ActionFactory::frameMenuEnable, this, [ = ](bool statu) {
                pMenu->setEnabled(statu);
            });
        }
        pMenu_p->addSeparator();
        DEF_ACTION_CHECKED(tr("Playlist"), ActionKind::TogglePlaylist);
        DEF_ACTION(tr("Film info"), ActionKind::MovieInfo);
        DEF_ACTION(tr("Settings"), ActionKind::Settings);
        m_pContextMenu = pMenu_p;
    }
    return m_pContextMenu;
}
DMenu *ActionFactory::playlistContextMenu()
{
    if (!m_pPlaylistMenu) {
        DMenu *pMenu_p = new DMenu();
        DEF_ACTION(tr("Delete from playlist"), ActionKind::PlaylistRemoveItem);
        DEF_ACTION(tr("Empty playlist"), ActionKind::EmptyPlaylist);
        DEF_ACTION(tr("Display in file manager"), ActionKind::PlaylistOpenItemInFM);
        DEF_ACTION(tr("Film info"), ActionKind::PlaylistItemInfo);
        m_pPlaylistMenu = pMenu_p;
    }
    return m_pPlaylistMenu;
}
QList<QAction *> ActionFactory::findActionsByKind(ActionKind target_kd)
{
    QList<QAction *> listAction;
    auto itor = m_listContextMenuActions.begin();
    while (itor != m_listContextMenuActions.end()) {
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
        auto kd = (ActionKind)(*p)->property("kind").value<int>();
#else
        auto kd = (*itor)->property("kind").value<ActionKind>();
#endif
        if (kd == target_kd) {
            listAction.append(*itor);
        }
        ++itor;
    }
    return listAction;
}
void ActionFactory::updateMainActionsForMovie(const PlayingMovieInfo &pmf)
{
    qInfo() << __func__;
    if (m_pSubtitleMenu) {
        DMenu *pMenu = m_pSubtitleMenu;
        pMenu->clear();
        if (!m_pSubgroup) {
            m_pSubgroup = new QActionGroup(pMenu); // mem leak ?
        }
        DEF_ACTION_CHECKED_GROUP(tr("None"), ActionKind::HideSubtitle, m_pSubgroup);
        QAction *pAct = pMenu->actions().first();
        pAct->setChecked(true);
        pAct->setEnabled(true);
        for (int i = 0; i < pmf.subs.size(); i++) {
            DEF_ACTION_CHECKED_GROUP(pmf.subs[i]["title"].toString(), ActionKind::SelectSubtitle, m_pSubgroup);
            QAction *pAct = pMenu->actions().last();
            pAct->setProperty("args", QList<QVariant>() << i+1);
        }
    }
    if (m_pSound) {
        DMenu *pMenu = m_pTracksMenu;
        pMenu->clear();
        if (!m_pAudiosgroup) {
            m_pAudiosgroup = new QActionGroup(pMenu); // mem leak ?
        }
        for (int i = 0; i < pmf.audios.size(); i++) {
            if (pmf.audios[i]["title"].toString().compare("[internal]") == 0) {
                DEF_ACTION_CHECKED_GROUP(tr("Track") + QString::number(i + 1), ActionKind::SelectTrack, m_pAudiosgroup);
            } else {
                DEF_ACTION_CHECKED_GROUP(pmf.audios[i]["title"].toString(), ActionKind::SelectTrack, m_pAudiosgroup);
            }
            QAction *pAct = pMenu->actions().last();
            pAct->setProperty("args", QList<QVariant>() << i);
        }
        m_pTracksMenu->setEnabled(pmf.audios.size() > 0);
        m_pSoundMenu->setEnabled(pmf.audios.size() > 0);
        m_pSound->setEnabled(pmf.audios.size() > 0);
    }
}
ActionFactory::ActionFactory()
{
    initMember();
}
void ActionFactory::initMember()
{
    m_pTitlebarMenu = nullptr;
    m_pContextMenu = nullptr;
    m_pSubtitleMenu = nullptr;
    m_pTracksMenu = nullptr;
    m_pSoundMenu = nullptr;
    m_pPlaylistMenu = nullptr;
    m_pSound = nullptr;
    m_pSubgroup = nullptr;
    m_pAudiosgroup = nullptr;
    m_listContextMenuActions.clear();
}
#undef DEF_ACTION
#undef DEF_ACTION_CHECKED
}
