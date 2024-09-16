<a name=""></a>
##  2.8.0 (2019-05-24)


#### Bug Fixes

*   cannot select window on other screen ([4070c3ee](https://github.com/linuxdeepin/deepin-screen-recorder/commit/4070c3ee7623c8bf934903eac7d873f370ae0825))



<a name="2.7.11"></a>
### 2.7.11 (2019-05-09)


#### Bug Fixes

*   cannot select window on other screen ([2a7ad997](https://github.com/linuxdeepin/deepin-screen-recorder/commit/2a7ad997decaedef7528af3145c966e1030c0347))



<a name="2.7.10"></a>
### 2.7.10 (2019-04-26)


#### Others

*   Can't select window correctly ([ef7a5b91](https://github.com/linuxdeepin/deepin-screen-recorder/commit/ef7a5b91b0e7e6de2d4d249ea6310a7c32ce43a2))



<a name="2.7.9"></a>
### 2.7.9 (2019-04-22)


#### Bug Fixes

*   multiple screens cannot be recorded after set window scale ([be5ff358](https://github.com/linuxdeepin/deepin-screen-recorder/commit/be5ff358588a0187ea46d4e28259f8ef85dbd47a))



<a name="2.7.8"></a>
### 2.7.8 (2019-04-09)


#### Others

*   auto pull translation files from transifex ([1094511f](https://github.com/linuxdeepin/deepin-screen-recorder/commit/1094511f0561b2556486f78fee09116c54ba2ff8))



<a name=""></a>
##  2.7.7 (2018-11-23)


#### Features

* **desktop:**  add generic name. ([cd01fc01](https://github.com/linuxdeepin/deepin-screen-recorder/commit/cd01fc0162a97919910bfff6d02f084201c31cae))



<a name="2.7.6"></a>
### 2.7.6 (2018-11-01)


#### Bug Fixes

*   use DISPLAY directly ([670e2ee7](https://github.com/linuxdeepin/deepin-screen-recorder/commit/670e2ee77edb99365cbcb05a9d561466161cfab6))



<a name="2.7.5"></a>
### 2.7.5 (2018-07-20)


#### Features

*   update run depend ([e1cb3245](https://github.com/linuxdeepin/deepin-screen-recorder/commit/e1cb3245bb54ac139e1a8f661556bb0da53c68f2))



# Change Log
This is default screen recorder application for Deepin.

## [Unreleased]

## [2.7.4] - 2018-05-14
- Add displayNumber to fixed that ffmpe will crash if multi-user with x11grab display argument :0.0

## [2.7.3] - 2018-01-25
- Synchronous translations.

## [2.7.2] - 2018-01-25
- Add mkv_framerate and mp4_framerate customize options
- Don't need append char to gif end since gif exit process is correct now, otherwise gif file will break if you stop recording too fast. ;)

## [2.7.1] - 2018-01-25
- Use -vf scale fix "division by 2" error, and now we can use yuv420p pix_fmt to make mp4 play on mobile phone

## [2.7] - 2018-01-25
- Fix tooltip not above on screen-recorder window
- Support flv format recording
- Synchronous translations from transifex.com
- Support deepin-manual newest version
- Fix error that GIF's content is not complete
- Fix GIF file break when stop recording less than 2 seconds
- Fix zero byte mp4 problem

## [2.6.5.1] - 2017-11-22
- Update translation

## [2.6.5] - 2017-11-17
### Bug Fixes
- Bypass window manager
- **flatpak:**  No composite warning dialog
- **hidpi:**  Gif record area calculated wrong

### Features
- Install dbus service ([b1a492f1](b1a492f1))

## [2.6.4] - 2017-11-06
- Add FlatPak support: single instance, keystroke dbus service
- Add composite warning dialog when window manager not support composite
- Fix GIF record area is not correctly under HiDPI
