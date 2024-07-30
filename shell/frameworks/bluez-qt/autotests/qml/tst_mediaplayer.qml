/*
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtTest 1.0
import QtQuick 2.2
import org.kde.bluezqt.fakebluez 1.0
import org.kde.bluezqt 1.0 as BluezQt

TestCase {
    name: "MediaPlayer"

    readonly property BluezQt.Manager manager: BluezQt.Manager

    property var mediaPlayer1props;
    property var mediaPlayer2props;

    function initTestCase()
    {
        FakeBluez.start();
        FakeBluez.runTest("bluez-standard");

        // Create adapter
        var adapter1path = "/org/bluez/hci0";
        var adapter1props = {
            Path: adapter1path,
            Address: "1C:E5:C3:BC:94:7E",
            Name: "TestAdapter",
            _toDBusObjectPath: [ "Path" ]
        }
        FakeBluez.runAction("devicemanager", "create-adapter", adapter1props);

        // Create devices
        mediaPlayer1props = {
            Path: "/org/bluez/hci0/dev_40_79_6A_0C_39_75/player0",
            Name: "Player1",
            Equalizer: "on",
            Repeat: "singletrack",
            Shuffle: "group",
            Status: "stopped",
            Position: 150,
            Device: "/org/bluez/hci0/dev_40_79_6A_0C_39_75",
            Track: {
                Title: "Song1",
                Artist: "Band1",
                Album: "First Album",
                Genre: "Rock",
                NumberOfTracks: 24,
                TrackNumber: 4,
                Duration: 12403
            },
            _toDBusObjectPath: [ "Path", "Device" ]
        }
        var device1props = {
            Path: "/org/bluez/hci0/dev_40_79_6A_0C_39_75",
            Adapter: adapter1path,
            Address: "40:79:6A:0C:39:75",
            Name: "TestDevice",
            UUIDs: [ "0000110E-0000-1000-8000-00805F9B34FB" ],
            MediaPlayer: mediaPlayer1props,
            _toDBusObjectPath: [ "Path", "Adapter" ]
        }
        FakeBluez.runAction("devicemanager", "create-device", device1props);

        mediaPlayer2props = {
            Path: "/org/bluez/hci0/dev_50_79_6A_0C_39_75/player0",
            Name: "Player2",
            Equalizer: "off",
            Repeat: "alltracks",
            Shuffle: "off",
            Status: "paused",
            Position: 2500,
            Device: "/org/bluez/hci0/dev_50_79_6A_0C_39_75",
            Track: {
                Title: "Song2",
                Artist: "Band2",
                Album: "Second Album",
                Genre: "Heavy Metal",
                NumberOfTracks: 666,
                TrackNumber: 22,
                Duration: 22403
            },
            _toDBusObjectPath: [ "Path", "Device" ]
        }
        var device2props = {
            Path: "/org/bluez/hci0/dev_50_79_6A_0C_39_75",
            Adapter: adapter1path,
            Address: "50:79:6A:0C:39:75",
            Name: "TestDevice2",
            UUIDs: [ "0000110E-0000-1000-8000-00805F9B34FB" ],
            MediaPlayer: mediaPlayer2props,
            _toDBusObjectPath: [ "Path", "Adapter" ]
        }
        FakeBluez.runAction("devicemanager", "create-device", device2props);

        tryCompare(manager, "operational", true);
        compare(manager.adapters.length, 1, "adapters-length");
        compare(manager.devices.length, 2, "devices-length");
    }

    function cleanupTestCase()
    {
        FakeBluez.stop();
    }

    SignalSpy {
        id: mediaPlayerChangedSpy
        signalName: "mediaPlayerChanged"
    }

    function test_a_connect()
    {
        for (var i = 0; i < manager.devices.length; ++i) {
            var device = manager.devices[i];
            verify(!device.mediaPlayer);

            mediaPlayerChangedSpy.clear();
            mediaPlayerChangedSpy.target = device;

            device.connectToDevice();

            tryCompare(mediaPlayerChangedSpy, "count", 1, 100 * 1000);
            verify(device.mediaPlayer);
        }
    }

    function compareProperties(mediaPlayer, props)
    {
        compare(mediaPlayer.name, props.Name, "name");
        compare(equalizerString(mediaPlayer), props.Equalizer, "equalizer");
        compare(repeatString(mediaPlayer), props.Repeat, "repeat");
        compare(shuffleString(mediaPlayer), props.Shuffle, "shuffle");
        compare(statusString(mediaPlayer), props.Status, "status");
        compare(mediaPlayer.position, props.Position, "position");

        compare(mediaPlayer.track.title, props.Track.Title, "track-title");
        compare(mediaPlayer.track.artist, props.Track.Artist, "track-artist");
        compare(mediaPlayer.track.album, props.Track.Album, "track-album");
        compare(mediaPlayer.track.genre, props.Track.Genre, "track-genre");
        compare(mediaPlayer.track.numberOfTracks, props.Track.NumberOfTracks, "track-#tracks");
        compare(mediaPlayer.track.trackNumber, props.Track.TrackNumber, "track-trackNumber");
        compare(mediaPlayer.track.duration, props.Track.Duration, "track-duration");
    }

    function test_getProperties()
    {
        var mediaPlayer1 = manager.deviceForUbi(mediaPlayer1props.Device).mediaPlayer;
        var mediaPlayer2 = manager.deviceForUbi(mediaPlayer2props.Device).mediaPlayer;
        compareProperties(mediaPlayer1, mediaPlayer1props);
        compareProperties(mediaPlayer2, mediaPlayer2props);
    }

    SignalSpy {
        id: equalizerChangedSpy
        signalName: "equalizerChanged"
    }

    function test_setEqualizer()
    {
        for (var i = 0; i < manager.devices.length; ++i) {
            var mediaPlayer = manager.devices[i].mediaPlayer;
            var value = mediaPlayer.equalizer === BluezQt.MediaPlayer.EqualizerOff ?
                BluezQt.MediaPlayer.EqualizerOn : BluezQt.MediaPlayer.EqualizerOff;

            equalizerChangedSpy.clear();
            equalizerChangedSpy.target = mediaPlayer;

            mediaPlayer.equalizer = value;

            tryCompare(equalizerChangedSpy, "count", 1);
            compare(mediaPlayer.equalizer, value);
        }
    }

    SignalSpy {
        id: repeatChangedSpy
        signalName: "repeatChanged"
    }

    function test_setRepeat()
    {
        for (var i = 0; i < manager.devices.length; ++i) {
            var mediaPlayer = manager.devices[i].mediaPlayer;
            var value = mediaPlayer.repeat === BluezQt.MediaPlayer.RepeatGroup ?
                BluezQt.MediaPlayer.RepeatSingleTrack : BluezQt.MediaPlayer.RepeatGroup;

            repeatChangedSpy.clear();
            repeatChangedSpy.target = mediaPlayer;

            mediaPlayer.repeat = value;

            tryCompare(repeatChangedSpy, "count", 1);
            compare(mediaPlayer.repeat, value);
        }
    }

    SignalSpy {
        id: shuffleChangedSpy
        signalName: "shuffleChanged"
    }

    function test_setShuffle()
    {
        for (var i = 0; i < manager.devices.length; ++i) {
            var mediaPlayer = manager.devices[i].mediaPlayer;
            var value = mediaPlayer.shuffle === BluezQt.MediaPlayer.ShuffleAllTracks ?
                BluezQt.MediaPlayer.ShuffleOff : BluezQt.MediaPlayer.ShuffleAllTracks;

            shuffleChangedSpy.clear();
            shuffleChangedSpy.target = mediaPlayer;

            mediaPlayer.shuffle = value;

            tryCompare(shuffleChangedSpy, "count", 1);
            compare(mediaPlayer.shuffle, value);
        }
    }

    SignalSpy {
        id: trackChangedSpy
        signalName: "trackChanged"
    }

    function test_za_changeTrack()
    {
        for (var i = 0; i < manager.devices.length; ++i) {
            var mediaPlayer = manager.devices[i].mediaPlayer;

            trackChangedSpy.target = mediaPlayer;

            trackChangedSpy.clear();
            mediaPlayer.next();
            tryCompare(trackChangedSpy, "count", 1);
            compare(mediaPlayer.track.valid, true);

            trackChangedSpy.clear();
            mediaPlayer.previous();
            tryCompare(trackChangedSpy, "count", 1);
            compare(mediaPlayer.track.valid, true);
        }
    }

    SignalSpy {
        id: statusChangedSpy
        signalName: "statusChanged"
    }

    function test_zb_changeStatus()
    {
        for (var i = 0; i < manager.devices.length; ++i) {
            var mediaPlayer = manager.devices[i].mediaPlayer;

            statusChangedSpy.clear();
            statusChangedSpy.target = mediaPlayer;

            if (mediaPlayer.status !== BluezQt.MediaPlayer.Stopped) {
                mediaPlayer.stop();
                tryCompare(statusChangedSpy, "count", 1);
                compare(mediaPlayer.status, BluezQt.MediaPlayer.Stopped);
            }

            statusChangedSpy.clear();
            mediaPlayer.play();
            tryCompare(statusChangedSpy, "count", 1);
            compare(mediaPlayer.status, BluezQt.MediaPlayer.Playing);

            statusChangedSpy.clear();
            mediaPlayer.pause();
            tryCompare(statusChangedSpy, "count", 1);
            compare(mediaPlayer.status, BluezQt.MediaPlayer.Paused);

            statusChangedSpy.clear();
            mediaPlayer.fastForward();
            tryCompare(statusChangedSpy, "count", 1);
            compare(mediaPlayer.status, BluezQt.MediaPlayer.ForwardSeek);

            statusChangedSpy.clear();
            mediaPlayer.rewind();
            tryCompare(statusChangedSpy, "count", 1);
            compare(mediaPlayer.status, BluezQt.MediaPlayer.ReverseSeek);

            statusChangedSpy.clear();
            mediaPlayer.stop();
            tryCompare(statusChangedSpy, "count", 1);
            compare(mediaPlayer.status, BluezQt.MediaPlayer.Stopped);
        }
    }

    function test_zz_disconnect()
    {
        for (var i = 0; i < manager.devices.length; ++i) {
            var device = manager.devices[i];
            verify(device.mediaPlayer);

            mediaPlayerChangedSpy.clear();
            mediaPlayerChangedSpy.target = device;

            device.disconnectFromDevice();

            tryCompare(mediaPlayerChangedSpy, "count", 1, 100 * 1000);
            verify(!device.mediaPlayer);
        }
    }


    function equalizerString(mediaPlayer)
    {
        switch (mediaPlayer.equalizer) {
        case BluezQt.MediaPlayer.EqualizerOn:
            return "on";
        default:
            return "off";
        }
    }

    function repeatString(mediaPlayer)
    {
        switch (mediaPlayer.repeat) {
        case BluezQt.MediaPlayer.RepeatSingleTrack:
            return "singletrack";
        case BluezQt.MediaPlayer.RepeatAllTracks:
            return "alltracks";
        case BluezQt.MediaPlayer.RepeatGroup:
            return "group";
        default:
            return "off";
        }
    }

    function shuffleString(mediaPlayer)
    {
        switch (mediaPlayer.shuffle) {
        case BluezQt.MediaPlayer.ShuffleAllTracks:
            return "alltracks";
        case BluezQt.MediaPlayer.ShuffleGroup:
            return "group";
        default:
            return "off";
        }
    }

    function statusString(mediaPlayer)
    {
        switch (mediaPlayer.status) {
        case BluezQt.MediaPlayer.Playing:
            return "playing";
        case BluezQt.MediaPlayer.Stopped:
            return "stopped";
        case BluezQt.MediaPlayer.Paused:
            return "paused";
        case BluezQt.MediaPlayer.ForwardSeek:
            return "forward-seek";
        case BluezQt.MediaPlayer.ReverseSeek:
            return "reverse-seek";
        default:
            return "error";
        }
    }
}
