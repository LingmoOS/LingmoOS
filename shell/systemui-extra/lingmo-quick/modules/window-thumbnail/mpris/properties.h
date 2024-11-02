/*
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_PROPERTIES_H
#define LINGMO_QUICK_PROPERTIES_H
#include <QObject>
class MprisProperties
{
    Q_GADGET
public:
    enum Properties {
        Pid = 0,
        Valid,
        IsCurrentMediaVideo,
        IsCurrentMediaAudio,
        //media player2 properties
        CanQuit,
        FullScreen,
        CanSetFullScreen,
        CanRaise,
        HasTrackList,
        Identity,
        DesktopEntry,
        SupportedUriSchemes,
        SupportedMimeTypes,
        //media player2 player properties
        PlaybackStatus,
        LoopStatus,
        Rate,
        Shuffle,
        MetaData,
        Volume,
        Position,
        MinimumRate,
        MaximumRate,
        CanGoNext,
        CanGoPrevious,
        CanPlay,
        CanPause,
        CanSeek,
        CanControl
    };
    Q_ENUM(Properties)
    enum Operations {
        //media player2 methods
        Raise,
        Quit,
        //media player2 player methods
        Next,
        Previous,
        Pause,
        PlayPause,
        Stop,
        Play,
        Seek,
        SetPosition,
        OpenUri
    };
    Q_ENUM(Operations)

};
#endif //LINGMO_QUICK_PROPERTIES_H
